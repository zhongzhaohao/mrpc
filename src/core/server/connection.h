#pragma once

#include "mrpc/mrpc.h"
#include "src/core/common/connection.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>

namespace mrpc::server {
using boost::asio::ip::tcp;

class Connection : public BaseConnection<Connection> {
public:
  using ptr = std::shared_ptr<Connection>;
  static ptr Create(boost::asio::io_context &io, request_handler handler) {
    return std::make_shared<Connection>(io, handler);
  }

  Connection(boost::asio::io_context &io, request_handler handler);

  void
  Connect(tcp::acceptor &acceptor,
          std::function<void(const boost::system::error_code &ec)> on_accept,
          std::function<void()> on_close);

  void Close() override { on_close_(); }

  void SetOnClose(std::function<void()> on_close) {
    on_close_ = std::move(on_close);
  }

  void HandleReadData(const std::string &data) override;

private:
  request_handler hander_;
  std::function<void()> on_close_;
};

} // namespace mrpc::server