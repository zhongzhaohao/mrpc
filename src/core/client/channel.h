#pragma once

#include "src/core/client/target.h"
#include <boost/asio.hpp>

#include "mrpc/mrpc.h"

namespace mrpc {
using boost::asio::io_context;
using boost::asio::ip::tcp;

class Channel : public std::enable_shared_from_this<Channel> {
public:
  using ptr = std::shared_ptr<Channel>;

  static ptr Create(boost::asio::io_context &io, Target &target) {
    return std::make_shared<Channel>(io, target);
  }

  Channel(boost::asio::io_context &ctx, Target &target);

  ~Channel();

  void Send(mrpc_call* call);

  void Receive(std::string &t);

  // 主动连接（可用于重连）
  void Connect();

private:
  void OnConnect(const boost::system::error_code &ec);
  void DoRead();
  void OnRead(const boost::system::error_code &ec, size_t bytes_transferred);

  tcp::socket socket_;
  tcp::resolver resolver_;
  Target target_;

  std::unordered_map<std::string, std::string> pending_requests_;
  std::mutex pending_mutex_;

  enum { max_length = 8192 };
  char data_[max_length];
};
} // namespace mrpc