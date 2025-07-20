#include "server.h"

#define MRPC_API

namespace mrpc {

Server::Server(const std::string &addr, request_handler handler)
    : io_(), work_guard_(boost::asio::make_work_guard(io_)),
      io_thread_{std::thread([this] { io_.run(); })}, target_(addr),
      acceptor_(io_), handler_(handler) {}

mrpc_status Server::Start() {
  try {
    acceptor_.open(tcp::v4());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(target_.endpoint());
    acceptor_.listen();

    std::cout << "Server listening on " << target_.endpoint() << std::endl;

    DoAccept();

    return MRPC_OK;
  } catch (const std::exception &e) {
    std::cerr << "Server start error: " << e.what() << std::endl;
    return MRPC_SEND_FAILURE;
  }
}

void Server::DoAccept() {
  auto new_connection = Connection::Create(io_, handler_);

  acceptor_.async_accept(
      new_connection->socket(),
      [this, new_connection](const boost::system::error_code &ec) {
        if (!ec) {
          std::lock_guard<std::mutex> lock(connection_mutex_);
          auto id = new_connection->id();

          new_connection->SetOnClose([id, this] {
            std::lock_guard<std::mutex> lock(connection_mutex_);
            connections_.erase(id);
          });

          connections_.emplace(id, new_connection);

          new_connection->Start();
        }
        DoAccept();
      });
}

mrpc_status Server::Send(mrpc_call *call, cchar_t *source) {
  std::lock_guard<std::mutex> lock(connection_mutex_);
  auto conn = connections_.find(source);
  if (conn == connections_.end()) {
    std::cerr << "Connection not found: " << source << std::endl;
    return MRPC_CONNECTION_NOT_FOUND;
  }

  auto sent = conn->second->Send(call);
  if (sent != MRPC_OK) {
    std::cerr << "Failed to send message to " << source << std::endl;
    return MRPC_SEND_FAILURE;
  }

  return MRPC_OK;
}

} // namespace mrpc

mrpc_server *mrpc_create_server(const char *addr, request_handler handler) {
  return (new mrpc::Server(addr, handler))->c_ptr();
}

mrpc_status mrpc_start_server(mrpc_server *server) {
  return mrpc::Server::FromC(server)->Start();
}

void mrpc_destroy_server(mrpc_server *server) {
  delete mrpc::Server::FromC(server);
}

mrpc_status mrpc_send_reponse(mrpc_server *server, mrpc_call *call,
                              cchar_t *source) {
  return mrpc::Server::FromC(server)->Send(call, source);
}