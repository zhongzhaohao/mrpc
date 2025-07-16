#include "mrpcpp/client.h"
#include "mrpcpp/callback.h"

std::string rpc_id(const std::string &func) {
  const size_t buf_size = 128;
  char buf[buf_size];
  mrpc_get_unique_id(func.c_str(), buf);
  return std::string(buf);
}

namespace mrpc {

MrpcClient::MrpcClient(const std::string &addr) {
  client_ = mrpc_create_client(addr.c_str());
}

MrpcClient::~MrpcClient() { mrpc_destroy_client(client_); }

Status MrpcClient::Send(const std::string &func, ParseToJson &request,
                        ParseFromJson &response) {
  auto status = AsyncSend(func, request);
  if (!status.ok()) {
    return status;
  }

  return Receive(status.message(), response);
}

mrpc::Status MrpcClient::AsyncSend(const std::string &func,
                                   ParseToJson &request) {
  auto req = request.toString();
  auto key = rpc_id(func);

  queue_.wait_result(key);

  auto callback = [key, this](cchar_t *result, mrpc_status s) {
    queue_.set_result(key, result, Status(s));
  };

  RegisterRpcCallback(key, callback);

  mrpc_call call{
      key.c_str(),
      req.c_str(),
      GlobalRpcCallback,
  };

  Status status = mrpc_send_request(client_, &call);

  if (status.ok()) {
    return status << key;
  } else {
    return status << "receive func : " << func << "error";
  }
}

mrpc::Status MrpcClient::Receive(const std::string &key,
                                 ParseFromJson &response) {
  auto [result, status] = queue_.get_result(key);
  if (!status.ok()) {
    // MRPC_PARSE_FAILURE
    return status << "receive func : " << key << "error";
  }

  try {
    response.fromString(std::string(result));
  } catch (const std::exception &e) {
    return Status(PARSE_FROM_JSON_FAILURE, e.what());
  }
  // timeout ?

  return status;
}

void MrpcClient::CallbackSend(const std::string &func, ParseToJson &request,
                              ParseFromJson &response,
                              std::function<void(mrpc::Status)> receive) {
  auto req = request.toString();
  auto key = rpc_id(func);

  auto callback = [&response, receive](cchar_t *result, mrpc_status s) {
    auto status = Status(s);
    if (!status.ok()) {
      // core lib error
      receive(status);
      return;
    }

    try {
      response.fromString(result);
    } catch (const std::exception &e) {
      receive(Status(PARSE_FROM_JSON_FAILURE, e.what()));
      return;
    }

    receive(Status());
  };

  RegisterRpcCallback(key, callback);

  mrpc_call call{
      key.c_str(),
      req.c_str(),
      GlobalRpcCallback,
  };

  Status status = mrpc_send_request(client_, &call);

  if (!status.ok()) {
    receive(status);
  }
}

} // namespace mrpc