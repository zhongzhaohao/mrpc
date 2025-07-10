#include "client.h"
#include <iostream>

namespace mrpc {
Status RpcClient::post(std::string func, ParseToJson &t, ParseFromJson &f) {
  try {
    // 延迟初始化，并且只初始化一次
    // std::call_once(channel_flag_, &RpcClient::_init_channel, this);
    _init_channel();

    channel_->Send(t);

    channel_->Receive(f);

    return Status();
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return Status::FAILURE(e.what());
  }
}
} // namespace mrpc