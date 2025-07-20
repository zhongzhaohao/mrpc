#pragma once

#include "mrpc/mrpc.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

namespace mrpc {
using boost::asio::ip::tcp;

class Connection : public std::enable_shared_from_this<Connection> {
public:
  using ptr = std::shared_ptr<Connection>;

  static ptr Create(boost::asio::io_context &io, request_handler handler) {
    return std::make_shared<Connection>(io, handler);
  }

  Connection(boost::asio::io_context &io, request_handler handler);

  tcp::socket &socket() { return socket_; }

  void Start() { DoRead(); }

  mrpc_status Send(mrpc_call *call);

  std::string id() const;

  void SetOnClose(std::function<void()> on_close) {
    on_close_ = std::move(on_close);
  }

private:
  void DoRead();
  void OnRead(const boost::system::error_code &ec);
  void SendResponse(const std::string &key, const std::string &response,
                    mrpc_status status);

  tcp::socket socket_;
  boost::asio::streambuf data_;
  request_handler hander_;
  std::function<void()> on_close_;
};

} // namespace mrpc