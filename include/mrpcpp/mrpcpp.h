#pragma once

#include "mrpc/mrpc.h"
#include "json.h"
#include "status.h"

namespace mrpc {
class MRPCClient {
public:
  MRPCClient(const std::string &addr);

  ~MRPCClient();

  mrpc::Status Send(const std::string &func, ParseToJson &request,
                    ParseFromJson &response);

private:
  mrpc_client *client_;
};
} // namespace mrpc