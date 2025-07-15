#include "channel.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <iostream>
// #include <Python.h>

static const std::string CALL_DELIMITER = "#MRPC#";
static const std::string KEY_DELIMITER = "#-#";

std::string GetSendMessage(mrpc_call *call) {
  std::ostringstream oss;
  oss << call->key << KEY_DELIMITER << call->message << CALL_DELIMITER;
  return oss.str();
}

std::tuple<std::string, std::string, mrpc_status>
ParseMessage(const std::string &input) {
  size_t key_delim_pos = input.find(KEY_DELIMITER);
  size_t call_delim_pos = input.find(CALL_DELIMITER);

  if (key_delim_pos == std::string::npos ||
      call_delim_pos == std::string::npos || call_delim_pos <= key_delim_pos) {
    return {"", "", MRPC_PARSE_FAILURE};
  }

  return {input.substr(0, key_delim_pos),
          input.substr(key_delim_pos + KEY_DELIMITER.length(),
                       call_delim_pos - key_delim_pos - KEY_DELIMITER.length()),
          MRPC_OK};
}

// class PythonGilHolder
// {
// public:
//   PythonGilHolder() : state_(PyGILState_Ensure()) {}
//   ~PythonGilHolder() { PyGILState_Release(state_); }

// private:
//   PyGILState_STATE state_;
// };

namespace mrpc {

Channel::Channel(io_context &ctx, Target &target)
    : socket_(tcp::socket(ctx)), resolver_(ctx), target_(target), data_(2048) {}

void Channel::Connect(std::function<void()> func) {
  // CONNECTING or NOT_CONNECT
  assert(!connect_status_.connected());

  if (func) {
    func();
  }

  if (!connect_status_.connecting()) {
    // NOT_CONNECT -> CONNECTING
    connect_status_.to_connecting();
    auto endpoints = target_.resolve(resolver_);
    auto self(shared_from_this());
    boost::asio::async_connect(
        socket_, endpoints,
        [self](const boost::system::error_code &ec, const tcp::endpoint &) {
          self->OnConnect(ec);
        });
  }
}

void Channel::OnConnect(const boost::system::error_code &ec) {
  if (!ec) {
    {
      // CONNECTING -> CONNECTED
      std::unique_lock<std::mutex> lock(status_mutex_);
      connect_status_.to_connected();
    }

    // 连接成功后自动开启接收队列
    DoRead();

    // 并发送等待中的消息
    pending_queue.consume([this](mrpc_call *call) { this->Send(call); });
  } else {
    std::cerr << "Connect failed: " << ec.message() << "\n";
    // 可以尝试重连？
    {
      // CONNECTING -> NOT_CONNECT
      std::unique_lock<std::mutex> lock(status_mutex_);
      connect_status_.to_not_connect();
    }
  }
}

void Channel::DoRead() {
  auto self(shared_from_this());

  boost::asio::async_read_until(
      socket_, data_, CALL_DELIMITER,
      [self](const boost::system::error_code &ec,
             size_t /*bytes_transferred*/) { self->OnRead(ec); });
}

void Channel::OnRead(const boost::system::error_code &ec) {
  if (ec) {
#ifndef NDEBUG
    std::cerr << "Read response error: " << ec.value() << ec.message() << "\n";
#endif
  } else {
    // 提取 streambuf 数据为 string
    std::string buffer_content(boost::asio::buffers_begin(data_.data()),
                               boost::asio::buffers_end(data_.data()));

    data_.consume(buffer_content.length());

    // std::cout << "Received message: " << buffer_content << "\n";

    auto [key, result, status] = ParseMessage(buffer_content);

    {
      // 目前暂时不考虑并发的情况，如果并发则需要获取GIL
      // PythonGilHolder gil;
      auto handler = wait_result_queue.consume_with_handler(key);
      handler(key.c_str(), result.c_str(), status);
    }
  }
  DoRead();
}

Channel::~Channel() {
  socket_.shutdown(tcp::socket::shutdown_both);
  socket_.close();
#ifndef NDEBUG
  std::cout << "socket destroy" << std::endl;
#endif
}

mrpc_status Channel::Send(mrpc_call *call) {
  try {
    // 这里有可能只是将请求暂存到 pending_queue
    // 中，并没有发送，在add函数中会进行内存深拷贝
    {
      std::unique_lock<std::mutex> lock(status_mutex_);
      if (!connect_status_.connected()) {
        Connect([this, call] { pending_queue.add(call); });
        return MRPC_OK;
      }
    }

    wait_result_queue.add(call);

    auto send_message = GetSendMessage(call);

    auto write_buf = boost::asio::buffer(send_message);
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_, std::move(write_buf),
        [self, call](const boost::system::error_code &ec,
                     size_t /*bytes_transferred*/) {
          if (ec) {
#ifndef NDEBUG
            std::cerr << "Write error for req [" << call->key
                      << "] : " << ec.message() << std::endl;
#endif
            call->handler(call->key, ec.message().c_str(), MRPC_SEND_FAILURE);
          }
        });
    return MRPC_OK;
  } catch (const std::exception &e) {
    return MRPC_SEND_FAILURE;
  }
}

} // namespace mrpc
