#include "src/core/server/connection.h"
#include "src/core/common/parser.h"

namespace mrpc::server {

Connection::Connection(boost::asio::io_context &io, request_handler handler)
    : BaseConnection(io), hander_(handler) {}

void Connection::Connect(
    tcp::acceptor &acceptor,
    std::function<void(const boost::system::error_code &ec)> on_accept,
    std::function<void()> on_close) {
  auto self = shared_from_this();
  acceptor.async_accept(
      self->socket(),
      [self, on_accept, on_close](const boost::system::error_code &ec) {
        if (!ec) {
          self->SetOnClose(on_close);
          self->Read();
        }
        on_accept(ec);
      });
}

void Connection::HandleReadData(const std::string &data) {
  auto [key, request, status] = ParseMessage(data);
  auto source = get_source();
  auto method = ExtractFunc(key);
  std::cout << "content : " << data << std::endl;
  hander_(method.c_str(), key.c_str(), request.c_str(), source.c_str());
}



} // namespace mrpc::server