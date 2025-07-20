#include "mrpcpp/client.h"

std::string rpc_id(const std::string &func) {
  const size_t buf_size = 128;
  char buf[buf_size];
  mrpc_get_unique_id(func.c_str(), buf);
  return std::string(buf);
}

namespace mrpc::client {

std::map<std::string, Callback> g_callbacks;
std::mutex g_callback_mutex;

void ClientCallback(cchar_t *key, cchar_t *result, mrpc_status status) {
  Callback cb;

  {
    std::lock_guard<std::mutex> lock(g_callback_mutex);
    auto it = g_callbacks.find(key);
    assert(it != g_callbacks.end());
    cb = it->second;
    g_callbacks.erase(it);
  }

  if (cb) {
    cb(result, status);
  }
}

void RegisterRpcCallback(const std::string &key, Callback cb) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);
  g_callbacks[key] = std::move(cb);
}

MrpcClient::MrpcClient(const std::string &addr) {
  client_ = mrpc_create_client(addr.c_str(), ClientCallback);
}

MrpcClient::~MrpcClient() { mrpc_destroy_client(client_); }

Status MrpcClient::Send(const std::string &func, Parser &request,
                        Parser &response) {
  std::string key;
  auto status = AsyncSend(func, request, key);
  if (!status.ok()) {
    return status;
  }

  return Receive(key, response);
}

mrpc::Status MrpcClient::AsyncSend(const std::string &func, Parser &request,
                                   std::string &key) {
  auto req = request.toString();
  key = rpc_id(func);

  queue_.wait_result(key);

  auto callback = [key, this](cchar_t *result, mrpc_status s) {
    queue_.set_result(key, result, Status(s));
  };

  auto status = send(key, req, callback);

  if (status.ok()) {
    return status << key;
  } else {
    return status << "receive func : " << func << "error";
  }
}

mrpc::Status MrpcClient::Receive(const std::string &key, Parser &response) {
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

void MrpcClient::CallbackSend(const std::string &func, Parser &request,
                              Parser &response,
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

  auto status = send(key, req, callback);

  if (!status.ok()) {
    receive(status);
  }
}

Status MrpcClient::send(const std::string &key, const std::string &req,
                        Callback cb) {
  RegisterRpcCallback(key, cb);

  mrpc_call call{
      key.c_str(),
      req.c_str(),
  };

  return mrpc_send_request(client_, &call);
}

} // namespace mrpc::client