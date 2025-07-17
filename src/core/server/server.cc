#include "server.h"

#define MRPC_API

namespace mrpc {
// 内部实现已在server.h中
} // namespace mrpc

MRPC_API mrpc_server *mrpc_create_server(const char *addr) {
  return (new mrpc::Server(addr))->c_ptr();
}

MRPC_API void mrpc_destroy_server(mrpc_server *server) {
  delete mrpc::Server::FromC(server);
}

MRPC_API mrpc_status mrpc_register_service(mrpc_server *server, 
                                           mrpc_service *service) {
  return mrpc::Server::FromC(server)->RegisterService(service);
}

MRPC_API mrpc_status mrpc_start_server(mrpc_server *server) {
  return mrpc::Server::FromC(server)->Start();
} 