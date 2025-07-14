#pragma once

#include "json.h"
#include "status.h"

#include <functional>

namespace mrpc {

struct CallContext {
  std::function<void(const char *)> callback;
};

class MRPCClient {
public:
  MRPCClient(const std::string &addr);

  ~MRPCClient();

  mrpc::Status Send(const std::string &func, ParseToJson &request,
                    ParseFromJson &response);

  mrpc::Status AsyncSend(const std::string &func, ParseToJson &request);

  void CallbackSend(const std::string &func, ParseToJson &request,
                    ParseFromJson &response,
                    std::function<void(mrpc::Status)> receive);

  mrpc::Status Receive(const std::string &key, ParseFromJson &response);

private:
  mrpc_client *client_;
};
} // namespace mrpc