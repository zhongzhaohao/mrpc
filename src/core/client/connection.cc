#include "src/core/client/connection.h"
#include "src/core/common/parser.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <iostream>
// #include <Python.h>

// class PythonGilHolder
// {
// public:
//   PythonGilHolder() : state_(PyGILState_Ensure()) {}
//   ~PythonGilHolder() { PyGILState_Release(state_); }

// private:
//   PyGILState_STATE state_;
// };

namespace mrpc::client {

Connection::Connection(io_context &ctx, Target &target,
                       response_handler handler)
    : BaseConnection(ctx), resolver_(ctx), target_(target), handler_(handler) {}

void Connection::Connect(std::function<void()> pre_connect) {
  // CONNECTING or NOT_CONNECT
  assert(!connect_status_.connected());

  if (pre_connect) {
    pre_connect();
  }

  if (!connect_status_.connecting()) {
    // NOT_CONNECT -> CONNECTING
    connect_status_.to_connecting();
    auto endpoints = target_.resolve(resolver_);
    auto self(shared_from_this());
    boost::asio::async_connect(
        self->socket(), endpoints,
        [self](const boost::system::error_code &ec, const tcp::endpoint &) {
          self->OnConnect(ec);
        });
  }
}

void Connection::OnConnect(const boost::system::error_code &ec) {
  if (!ec) {
    {
      // CONNECTING -> CONNECTED
      std::unique_lock<std::mutex> lock(status_mutex_);
      connect_status_.to_connected();
    }

    // 连接成功后自动开启接收队列
    Read();

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

void Connection::HandleReadData(const std::string &data) {
  // std::cout << "Received message: " << buffer_content << "\n";

  auto [key, response, status] = ParseMessage(data);

  {
    // 目前暂时不考虑并发的情况，如果并发则需要获取GIL
    // PythonGilHolder gil;
    handler_(key.c_str(), response.c_str(), status);
  }
}

mrpc_status Connection::PreSend(mrpc_call *call) {
  // 这里有可能只是将请求暂存到 pending_queue
  // 中，并没有发送，在add函数中会进行内存深拷贝
  {
    std::unique_lock<std::mutex> lock(status_mutex_);
    if (!connect_status_.connected()) {
      Connect([this, call] { pending_queue.add(call); });
      return MRPC_CANCELED;
    } else {
      // 只有连接成功才进行发送，否则添加到等待队列，连接成功后一起发送
      return MRPC_OK;
    }
  }
}

void Connection::SendFailed(mrpc_call *call,
                            const boost::system::error_code &ec) {
  handler_(call->key, ec.message().c_str(), MRPC_SEND_FAILURE);
}

} // namespace mrpc::client
