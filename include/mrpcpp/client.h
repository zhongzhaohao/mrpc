#pragma once

// #include "callback.h"
#include "json.h"
#include "status.h"
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

#include "mrpc/mrpc.h"
#include <cassert>
#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace mrpc::client {

using Callback = std::function<void(cchar_t *, mrpc_status)>;

extern "C" void ClientCallback(cchar_t *key, cchar_t *response,
                               mrpc_status status);

void RegisterRpcCallback(const std::string &key, Callback cb);

struct Call {
  bool finish;
  Status status;
  std::string result;
  std::unique_ptr<std::condition_variable> cond_;

  Call() : cond_(std::make_unique<std::condition_variable>()) {}

  void set(bool f, const char *r, Status &&st) {
    finish = f;
    result = r;
    status = std::move(st);
    cond_->notify_all();
  }
};

class ClientQueue {
public:
  ClientQueue() : queue_() {}

  void wait_result(const std::string &key) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(key, Call());
  }

  void set_result(const std::string &key, const char *result, Status &&status) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto call = queue_.find(key);
    assert(call != queue_.end());
    call->second.set(true, result, std::move(status));
  }

  std::pair<std::string, Status> get_result(const std::string &key) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto call = queue_.find(key);
    assert(call != queue_.end());

    call->second.cond_->wait(lock);
    auto result = call->second.result;
    auto status = call->second.status;
    queue_.erase(key);
    return {result, status};
  }

private:
  std::map<std::string, Call> queue_;
  std::mutex mutex_;
};

class MrpcClient {
public:
  MrpcClient(const std::string &addr);

  virtual ~MrpcClient();

  mrpc::Status Send(const std::string &func, Parser &request, Parser &response);

  mrpc::Status AsyncSend(const std::string &func, Parser &request,
                         std::string &key);

  void CallbackSend(const std::string &func, Parser &request, Parser &response,
                    std::function<void(mrpc::Status)> receive);

  mrpc::Status Receive(const std::string &key, Parser &response);

private:
  Status send(const std::string &key, const std::string &req, Callback cb);

  mrpc_client *client_;
  ClientQueue queue_;
};

} // namespace mrpc::client
