#include "src/core/server/connection.h"
#include "src/core/common/parser.h"
#include <iostream>

namespace mrpc {

std::string Connection::id() const {
  auto ep = socket_.remote_endpoint();
  return ep.address().to_string() + ":" + std::to_string(ep.port());
}

Connection::Connection(boost::asio::io_context &io, request_handler handler)
    : socket_(io), data_(2048), hander_(handler) {}

void Connection::DoRead() {
  boost::asio::async_read_until(
      socket_, data_, CALL_DELIMITER,
      [self = shared_from_this()](const boost::system::error_code &ec,
                                  size_t /*bytes_transferred*/) {
        self->OnRead(ec);
      });
}

void Connection::OnRead(const boost::system::error_code &ec) {
  if (ec) {
    if (ec != boost::asio::error::eof) {
      std::cerr << "Read error: " << ec.message() << "\n";
    }
    return;
  }

  // 提取数据
  std::string buffer_content(boost::asio::buffers_begin(data_.data()),
                             boost::asio::buffers_end(data_.data()));
  data_.consume(buffer_content.length());

  auto [key, request, status] = ParseMessage(buffer_content);
  auto source = id();
  auto method = ExtractFunc(key);
  std::cout << "content : " << buffer_content << std::endl;
  hander_(method.c_str(), key.c_str(), request.c_str(), source.c_str());

  DoRead();
}

mrpc_status Connection::Send(mrpc_call *call) {
  try {
    auto send_message = GetSendMessage(call);

    auto write_buf = boost::asio::buffer(send_message);
    boost::asio::async_write(
        socket_, std::move(write_buf),
        [self = shared_from_this(), call](const boost::system::error_code &ec,
                                          size_t /*bytes_transferred*/) {
          if (ec) {
#ifndef NDEBUG
            std::cerr << "Write error for req [" << call->key
                      << "] : " << ec.message() << std::endl;
#endif
            // send failure
          }
        });
    return MRPC_OK;
  } catch (const std::exception &e) {
    return MRPC_SEND_FAILURE;
  }
}

} // namespace mrpc