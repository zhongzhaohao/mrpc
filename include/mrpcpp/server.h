#pragma once

#include "json.h"
#include "mrpc/mrpc.h"
#include "status.h"
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace mrpc::server {

using Callback = std::function<void(cchar_t *, cchar_t *, cchar_t *)>;

extern "C" void ServerCallback(cchar_t *key, cchar_t *request, cchar_t *source);

void RegisterRpcCallback(const std::string &key, Callback cb);

class RpcMethodHandler {
public:
  virtual Status Run(const std::string &args, std::string &result) = 0;
};

template <typename Request, typename Response,
          typename = std::enable_if_t<std::is_base_of_v<Parser, Request> &&
                                      std::is_base_of_v<Parser, Response>>>
class RpcHandler : public RpcMethodHandler {
public:
  template <typename F>
  explicit RpcHandler(F &&func) : func_(std::forward<F>(func)) {}

  Status Run(const std::string &args, std::string &result) override {
    auto request = Request();

    try {
      request.fromString(args);
    } catch (const std::exception &e) {
      return Status(StatusCode::PARSE_FROM_JSON_FAILURE, e.what());
    }

    auto response = Response();

    try {
      func_(request, response);
    } catch (const std::exception &e) {
      return Status(StatusCode::RPC_HANDLER_FAILED, e.what());
    }

    try {
      result = response.toString();
    } catch (const std::exception &e) {
      return Status(StatusCode::PARSE_TO_JSON_FAILURE, e.what());
    }

    return Status::OK();
  }

private:
  std::function<Status(const Request &request, Response &response)> func_;
};

class MrpcService {
public:
  MrpcService(const std::string &name) : service_name(name) {}

  std::string GetServiceName() { return service_name; }

  virtual ~MrpcService() = default;

  const std::map<std::string, std::shared_ptr<RpcMethodHandler>> &
  GetHandlers() const {
    return methods_;
  }

  template <typename Request, typename Response, typename Func>
  void AddHandler(const std::string &method_name, Func &&func) {
    auto handler = std::make_shared<RpcHandler<Request, Response>>(
        std::forward<Func>(func));
    methods_.emplace(method_name, handler);
  }

private:
  std::string service_name;
  std::map<std::string, std::shared_ptr<RpcMethodHandler>> methods_;
};

class MrpcServer : public std::enable_shared_from_this<MrpcServer> {
public:
  static std::shared_ptr<MrpcServer> Create(const std::string &addr) {
    return std::make_shared<MrpcServer>(addr);
  }

  MrpcServer(const std::string &addr);

  void RegisterService(MrpcService *service);

  Status Start();

  void Stop();

private:
  mrpc_server *server_;
  std::vector<MrpcService *> services_;
};

} // namespace mrpc::server