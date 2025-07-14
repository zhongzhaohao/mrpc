#include "channel.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <iostream>
// #include <Python.h>

static const std::string CALL_DELIMITER = "#MRPC#";
static const std::string KEY_DELIMITER = "#-#";

std::string GetSendMessage(mrpc::Call &&call)
{
  std::ostringstream oss;
  oss << call.key << KEY_DELIMITER << call.message << CALL_DELIMITER;
  return oss.str();
}

mrpc::Call ParseMessage(const std::string &input)
{
  size_t key_delim_pos = input.find(KEY_DELIMITER);
  size_t call_delim_pos = input.find(CALL_DELIMITER);

  if (key_delim_pos == std::string::npos ||
      call_delim_pos == std::string::npos || call_delim_pos <= key_delim_pos)
  {
    return mrpc::Call();
  }

  mrpc::Call out;

  out.key = input.substr(0, key_delim_pos);
  out.message =
      input.substr(key_delim_pos + KEY_DELIMITER.length(),
                   call_delim_pos - key_delim_pos - KEY_DELIMITER.length());

  return out;
}

// class PythonGilHolder
// {
// public:
//   PythonGilHolder() : state_(PyGILState_Ensure()) {}
//   ~PythonGilHolder() { PyGILState_Release(state_); }

// private:
//   PyGILState_STATE state_;
// };

namespace mrpc
{

  Channel::Channel(io_context &ctx, Target &target)
      : socket_(tcp::socket(ctx)), resolver_(ctx), target_(target), data_(2048)
  {}

  void Channel::Connect()
  {
    assert(connect_status_.connecting());
    auto endpoints = target_.resolve(resolver_);
    auto self(shared_from_this());
    boost::asio::async_connect(
        socket_, endpoints,
        [self](const boost::system::error_code &ec, const tcp::endpoint &)
        {
          self->OnConnect(ec);
        });
  }

  void Channel::OnConnect(const boost::system::error_code &ec)
  {
    if (!ec)
    {
      {
        std::unique_lock<std::mutex> lock(status_mutex_);
        connect_status_.to_connected();
      }

      // 连接成功后自动开启接收队列
      DoRead();

      // 并发送等待中的消息
      pending_queue.doAll([this](Call &&call)
                          { this->Send(std::move(call)); });
    }
    else
    {
      std::cerr << "Connect failed: " << ec.message() << "\n";
      // 可以尝试重连？
      {
        std::unique_lock<std::mutex> lock(status_mutex_);
        connect_status_.to_not_connect();
      }
    }
  }

  void Channel::DoRead()
  {
    auto self(shared_from_this());

    boost::asio::async_read_until(
        socket_, data_, CALL_DELIMITER,
        [self](const boost::system::error_code &ec,
               size_t /*bytes_transferred*/)
        { self->OnRead(ec); });
  }

  void Channel::OnRead(const boost::system::error_code &ec)
  {
    if (ec)
    {
      std::cerr << "Read response error: " << ec.value() << ec.message() << "\n";
    }
    else
    {

      // 提取 streambuf 数据为 string
      std::string buffer_content(boost::asio::buffers_begin(data_.data()),
                                 boost::asio::buffers_end(data_.data()));

      data_.consume(buffer_content.length());

      std::cout << "Received message: " << buffer_content << "\n";

      auto response = ParseMessage(buffer_content);

      auto call = wait_result_queue.set_result(response.key, response.message);
      if (call.handler)
      {
        {
          // 目前暂时不考虑并发的情况，如果并发则需要获取GIL
          // PythonGilHolder gil;
          call.handler(call.key.c_str(), response.message.c_str());
        }

        wait_result_queue.remove(response.key);
      }
    }
    DoRead();
  }

  Channel::~Channel()
  {
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
#ifndef NDEBUG
    std::cout << "socket destroy" << std::endl;
#endif
  }

  mrpc_status Channel::Send(mrpc_call *call)
  {
    try
    {
      Send(Call(call));
      // 这里有可能只是将请求暂存到 pending_queue 中，并没有发送
      return MRPC_OK;
    }
    catch (const std::exception &e)
    {
      return MRPC_FAILURE;
    }
  }

  void Channel::Send(Call &&call)
  {
    {
      std::unique_lock<std::mutex> lock(status_mutex_);
      if (connect_status_.not_connect())
      {
        connect_status_.to_connecting();
        Connect();
      }

      if (connect_status_.connecting())
      {
        pending_queue.add(call);
        return;
      }
    }

    wait_result_queue.add(call);

    auto send_message = GetSendMessage(std::move(call));
    std::cout << "send message : " << send_message << std::endl;

    auto write_buf = boost::asio::buffer(send_message);
    auto self(shared_from_this());
    boost::asio::async_write(socket_, std::move(write_buf),
                             [self, call](const boost::system::error_code &ec,
                                          size_t /*bytes_transferred*/)
                             {
#ifndef NDEBUG
                               if (ec)
                               {
                                 std::cerr << "Write error for req [" << call.key
                                           << "] : " << ec.message() << std::endl;

                                 self->wait_result_queue.remove(call.key);
                               }
#endif
                             });
  }

  mrpc_status Channel::Receive(mrpc_call *call)
  {
    auto key = std::string(call->key);
    if (!wait_result_queue.success(key))
    {
      return MRPC_FAILURE;
    }
    else
    {
      call->message = strdup(wait_result_queue.get_result(key).c_str());
      wait_result_queue.remove(key);
      return MRPC_OK;
    }
  }

  void Channel::Wait(mrpc_call *call)
  {
    auto key = std::string(call->key);
    while (!wait_result_queue.success(key))
    {
    }
  }

} // namespace mrpc
