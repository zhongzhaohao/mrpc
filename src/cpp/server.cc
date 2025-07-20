#include "mrpcpp/server.h"
#include <iostream>
#include <mutex>

namespace mrpc::server {

std::map<std::string, Callback> g_callbacks;
std::mutex g_callback_mutex;

const std::string UNKNOWN = "UNKNOW_FUNC";

void ServerCallback(cchar_t *method, cchar_t *key, cchar_t *request,
                    cchar_t *source) {
  Callback cb;

  {
    std::lock_guard<std::mutex> lock(g_callback_mutex);
    auto it = g_callbacks.find(method);
    if (it == g_callbacks.end()) {
      std::cout << "unregisted function: " << method << std::endl;
      return;
    }
    cb = it->second;
  }

  if (cb) {
    cb(key, request, source);
  }
}

void RegisterRpcCallback(const std::string &key, Callback cb) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);
  g_callbacks[key] = std::move(cb);
}

MrpcServer::MrpcServer(const std::string &addr) {
  server_ = mrpc_create_server(addr.c_str(), ServerCallback);
}

void MrpcServer::Stop() { mrpc_destroy_server(server_); }

void MrpcServer::RegisterService(MrpcService *service) {
  services_.push_back(service);

  auto self(shared_from_this());
  auto handler = [self](std::shared_ptr<RpcMethodHandler> method, cchar_t *key,
                        cchar_t *request, cchar_t *source) {
    std::string result;
    Status status = method->Run(request, result);
    if (!status.ok()) {
      result = status.message();
    }
    mrpc_call response = {key, result.c_str()};
    status = mrpc_send_reponse(self->server_, &response, source);
    if (!status.ok()) {
      std::cout << "send response to " << key
                << " with error: " << status.error_code();
    }
  };

  for (const auto &pair : service->GetHandlers()) {
    auto method_name = pair.first;
    auto method = pair.second;
    auto callback = [handler, method](cchar_t *k, cchar_t *r, cchar_t *s) {
      handler(method, k, r, s);
    };
    std::cout << "register func: " << method_name << std::endl;
    RegisterRpcCallback(method_name, callback);
  }
}

Status MrpcServer::Start() {
  // Register unknown handler

  auto self(shared_from_this());
  auto callback = [self](cchar_t *key, cchar_t *request, cchar_t *source) {
    mrpc_call response = {key, "no such func"};
    mrpc_send_reponse(self->server_, &response, source);
  };
  RegisterRpcCallback(UNKNOWN, callback);

  return mrpc_start_server(server_);
}

} // namespace mrpc::server