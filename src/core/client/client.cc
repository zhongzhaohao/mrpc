#include "client.h"

namespace mrpc {
mrpc_status Client::Send(mrpc_call *call) {
  std::call_once(channel_flag_, &Client::_init_channel, this);

  return channel_->Send(call);
}

mrpc_status Client::Receive(mrpc_call *call) {
  channel_->Wait(call);
  return channel_->Receive(call);
}

} // namespace mrpc

mrpc_client *mrpc_create_client(const char *addr) {
  return (new mrpc::Client(addr))->c_ptr();
}

void mrpc_destroy_client(mrpc_client *client) {
  delete mrpc::Client::FromC(client);
}

mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call) {
  return mrpc::Client::FromC(client)->Send(call);
}

mrpc_status mrpc_send_request_with_callback(mrpc_client *client,
                                            mrpc_call *call,
                                            response_handler func) {
  return mrpc::Client::FromC(client)->Send(call);
}

mrpc_status mrpc_receive_response(mrpc_client *client, mrpc_call *call) {
  return mrpc::Client::FromC(client)->Receive(call);
}
