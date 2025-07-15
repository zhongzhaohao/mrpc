#pragma once

#include "json.h"
#include "status.h"

#include <functional>
#include <mutex>

namespace mrpc {

struct Call {
  bool success;
  Status status;
  std::string result;

  // 链式赋值函数
  void set(bool s, const char *r, Status &&st) {
    success = s;
    result = r;
    status = std::move(st);
  }
};

class ClientQueue {
public:
  ClientQueue() : queue_() {}

  void wait_result(const std::string &key) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(key, Call());
  }

  bool success(const std::string &key) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto call = queue_.find(key);
    assert(call != queue_.end());
    return call->second.success;
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
    auto result = call->second.result;
    auto status = call->second.status;
    queue_.erase(key);
    return {result, status};
  }

private:
  std::map<std::string, Call> queue_;
  std::mutex mutex_;
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
  ClientQueue queue_;
};
} // namespace mrpc