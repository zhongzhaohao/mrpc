#include "channel.h"
#include <iostream>

namespace mrpc {

Channel::Channel(io_context &ctx, Target &target)
    : socket_(tcp::socket(ctx)), resolver_(ctx), target_(target) {
  Connect();
}

void Channel::Connect() {
  auto endpoints = target_.resolve(resolver_);
  boost::asio::async_connect(
      socket_, endpoints,
      [self = shared_from_this()](const boost::system::error_code &ec,
                                  const tcp::endpoint &) {
        self->OnConnect(ec);
      });
}

void Channel::OnConnect(const boost::system::error_code &ec) {
  if (!ec) {
    DoRead(); // 开始监听响应
  } else {
    std::cerr << "Connect failed: " << ec.message() << "\n";
    // 可以尝试重连
  }
}

void Channel::DoRead() {
  socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      [self = shared_from_this()](const boost::system::error_code &ec,
                                  size_t bytes_transferred) {
        self->OnRead(ec, bytes_transferred);
      });
}

void Channel::OnRead(const boost::system::error_code &ec,
                     size_t bytes_transferred) {
  if (ec) {
    std::cerr << "Read error: " << ec.message() << "\n";
    return;
  }

  std::string response(data_, bytes_transferred);
  // 解析 response 获取 request_id
  std::string request_id = ""; // ParseRequestId(response); // 实现这个函数

  DoRead(); // 继续监听
}

Channel::~Channel() {
  // 不应该在这里，因为完全有可能因为网络或超时等原因主动断开。
  socket_.shutdown(tcp::socket::shutdown_both);
  socket_.close();
#ifndef NDEBUG
  std::cout << "socket destroy" << std::endl;
#endif
}

void Channel::Send(mrpc_call *call) {
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    pending_requests_[call->key] = "";
  }

  boost::asio::async_write(
      socket_, boost::asio::buffer(std::string(call->message)),
      [self = shared_from_this(), key = std::string(call->key), ](
          const boost::system::error_code &ec, size_t /*bytes*/) {
        if (ec) {
          std::cerr << "Write error for req " << key << ": " << ec.message()
                    << "\n";
        }
      });
}

} // namespace mrpc
