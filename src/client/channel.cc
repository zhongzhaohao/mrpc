#include "channel.h"
#include <iostream>

namespace mrpc {

Channel::Channel(io_context &ctx, Target &target)
    : socket_(std::make_unique<tcp::socket>(ctx)) {
  // 如果是这样的话，怎么重连呢？
  auto endpoints = target.resolve(ctx);

  // 这里自动实现负载均衡，貌似是首次成功
  // 如果要手动实现负载均衡的话需要调用 socket_->connect
  // 那么 Channel 中需要持有 endpoints
  boost::asio::connect(*socket_, endpoints);
#ifndef NDEBUG
  std::cout << "create channel to :" << std::endl;
  for (const auto &ep : endpoints) {
    std::cout << "  Endpoint: " << ep.endpoint() << std::endl;
  }
#endif
}

Channel::~Channel() {
  // 不应该在这里，因为完全有可能因为网络或超时等原因主动断开。
  socket_->shutdown(tcp::socket::shutdown_both);
  socket_->close();
#ifndef NDEBUG
  std::cout << "socket destroy" << std::endl;
#endif
}

void Channel::Receive(ParseFromJson &f) {
  // 这里的buffer是能共享？
  std::string response;
  char buffer[1024];
  // 这里不应该是同步函数
  size_t len = socket_->read_some(boost::asio::buffer(buffer));

  response.assign(buffer, len);
  f.fromString(response);

#ifndef NDEBUG
  std::cout << "Response: " << response << "\n";
#endif
}

void Channel::Send(ParseToJson &t) {
  boost::asio::write(*socket_, boost::asio::buffer(t.toString()));
#ifndef NDEBUG
  std::cout << "Sent JSON: " << t.toString(4) << "\n";
#endif
}
} // namespace mrpc