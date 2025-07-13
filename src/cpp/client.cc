#include "mrpcpp/mrpcpp.h"

#include <random>
#include <string>

std::string nanoid(size_t len = 8) {
  const std::string chars =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::string result;
  std::random_device rd;
  std::mt19937 gen(rd());

  for (size_t i = 0; i < len; ++i) {
    result += chars[gen() % chars.size()];
  }
  return result;
}

std::string rpc_id(const std::string &func) {
  // key = func-id-cpp
  std::ostringstream oss;
  oss << func << "-" << nanoid() << "-cpp";
  return oss.str();
}

namespace mrpc {

MRPCClient::MRPCClient(const std::string &addr) {
  client_ = mrpc_create_client(addr.c_str());
}

MRPCClient::~MRPCClient() { mrpc_destroy_client(client_); }

Status MRPCClient::Send(const std::string &func, ParseToJson &request,
                        ParseFromJson &response) {
  auto status = AsyncSend(func, request);
  if (!status.ok()) {
    return status;
  }

  return Receive(status.message(), response);
}

mrpc::Status MRPCClient::AsyncSend(const std::string &func,
                                   ParseToJson &request) {
  auto req = request.toString();
  auto key = rpc_id(func);
  mrpc_call call{key.c_str(), req.c_str(), nullptr};
  Status status = mrpc_send_request(client_, &call);
  if (status.ok()) {
    return status << key;
  } else {
    return status << "receive func : " << func << "error";
  }
}

void MRPCClient::CallbackSend(const std::string &func, ParseToJson &request,
                              ParseFromJson &response,
                              std::function<void(mrpc::Status)> receive) {
  auto req = request.toString();
  auto key = rpc_id(func);

  auto context = new CallContext();
  context->callback = [&response, receive](const char *result) {
    if (result) {
      response.fromString(result);
    }
    receive(Status());
  };

  mrpc_call call{
      key.c_str(),
      req.c_str(),
      [](const char *result, void *data) {
        auto *ctx = static_cast<CallContext *>(data);
        if (ctx && ctx->callback) {
          ctx->callback(result);
        }
        // 这里需要手动delete，不然会内存泄漏
        delete ctx;
      },
      context,
  };

  Status status = mrpc_send_request(client_, &call);

  if (!status.ok()) {
    receive(status);
  }
}

mrpc::Status MRPCClient::Receive(const std::string &key,
                                 ParseFromJson &response) {
  mrpc_call call{key.c_str(), nullptr};
  Status status = mrpc_receive_response(client_, &call);

  response.fromString(std::string(call.message));
  // 这里需要手动delete，不然会内存泄漏
  delete call.message;

  if (status.ok()) {
    return status;
  } else {
    return status << "receive func : " << key << "error";
  }
}

} // namespace mrpc