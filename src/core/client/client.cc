#include "client.h"
#include <iostream>

namespace mrpc {
mrpc_status Client::Send(mrpc_client *client, mrpc_call *call) {
  try {
    std::call_once(channel_flag_, &Client::_init_channel, this);

    channel_->Send(call);

    return MRPC_OK;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return MRPC_FAILURE;
  }
}

mrpc_status Client::Receive(mrpc_client *client, mrpc_call *call){
  try {
    channel_->Receive(call);
    return MRPC_OK;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return MRPC_FAILURE;
  }
}

} // namespace mrpc

mrpc_client *mrpc_create_client(const char *addr) {
  return (new mrpc::Client(addr))->c_ptr();
}

void mrpc_destroy_client(mrpc_client *client) {
  delete mrpc::Client::FromC(client);
}

mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call) {
  return mrpc::Client::FromC(client)->Send(client, call);
}

mrpc_status mrpc_receive_response(mrpc_client *client, mrpc_call *call) {
  return mrpc::Client::FromC(client)->Receive(client, call);
}


