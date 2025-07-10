#include "mrpc.h"
#include "src/client/client.h"

mrpc_client *mrpc_create_client(const std::string &addr) {
  return (new mrpc::RpcClient(addr))->c_ptr();
}

void mrpc_destroy_client(mrpc_client *client) {
  delete mrpc::RpcClient::FromC(client);
}

mrpc::Status mrpc_sync_send_request(mrpc_client *client, std::string func,
                                    mrpc::ParseToJson &t,
                                    mrpc::ParseFromJson &f) {
  return mrpc::RpcClient::FromC(client)->post(func, t, f);
}