#pragma once

#include "src/core/common/parser.h"
#include <boost/asio.hpp>
#include <iostream>

#include "mrpc/mrpc.h"

namespace mrpc {

using boost::asio::ip::tcp;

template <typename Derived>
class BaseConnection : public std::enable_shared_from_this<Derived> {
public:
  using std::enable_shared_from_this<Derived>::shared_from_this;

  virtual ~BaseConnection() {
    Close();
#ifndef NDEBUG
    std::cout << "connection destroy" << std::endl;
#endif
  }

  // 由客户端来关闭连接
  virtual void Close() {};

  const tcp::socket &socket() const { return socket_; }

  tcp::socket &socket() { return socket_; }

  mrpc_status Send(mrpc_call *call) {
    try {
      auto status = PreSend(call);
      if (status == MRPC_CANCELED) {
        return MRPC_OK;
      }

      auto send_message = GetSendMessage(call);

      auto write_buf = boost::asio::buffer(send_message);
      auto self(shared_from_this());
      boost::asio::async_write(socket_, std::move(write_buf),
                               [self, call](const boost::system::error_code &ec,
                                            size_t /*bytes_transferred*/) {
                                 if (ec) {
#ifndef NDEBUG
                                   std::cerr
                                       << "Write error for req [" << call->key
                                       << "] : " << ec.message() << std::endl;
#endif
                                   self->SendFailed(call, ec);
                                 }
                               });
      return MRPC_OK;
    } catch (const std::exception &e) {
      return MRPC_SEND_FAILURE;
    }
  }

  void Read() {
    auto self(shared_from_this());

    boost::asio::async_read_until(
        socket_, data_, CALL_DELIMITER,
        [self](const boost::system::error_code &ec,
               size_t /*bytes_transferred*/) {
          if (ec) {
            if (ec == boost::asio::error::eof) {
              self->Close();
            } else {
#ifndef NDEBUG
              std::cerr << "Read error: " << ec.message() << "\n";
#endif
            }
          } else {
            auto data = &self->data_;
            std::string buffer_content(boost::asio::buffers_begin(data->data()),
                                       boost::asio::buffers_end(data->data()));

            data->consume(buffer_content.length());
            self->HandleReadData(buffer_content);
          }
          self->Read();
        });
  }

  virtual void HandleReadData(const std::string &data) = 0;

  virtual mrpc_status PreSend(mrpc_call *call) { return MRPC_OK; }

  virtual void SendFailed(mrpc_call *call,
                          const boost::system::error_code &ec) {}

  std::string get_source() {
    if (!source_.empty()) {
      return source_;
    }
    auto ep = socket_.remote_endpoint();
    source_ = ep.address().to_string() + ":" + std::to_string(ep.port());
    return source_;
  }

protected:
  explicit BaseConnection(boost::asio::io_context &io) : socket_(io) {}

private:
  tcp::socket socket_;
  boost::asio::streambuf data_;
  std::string source_;
};

} // namespace mrpc
