#pragma once

#include "src/core/client/target.h"
#include <boost/asio.hpp>

#include "mrpc/call.h"
#include "mrpc/status.h"

namespace mrpc {
using boost::asio::io_context;
using boost::asio::ip::tcp;

class Call {
public:
  Call() {}
  explicit Call(mrpc_call *c_call)
      : key(c_call->key), message(c_call->message), handler(c_call->handler),
        ctx(c_call->ctx) {}

  Call(std::string &key, std::string &message) : key(key), message(message) {}

  std::string key;
  std::string message;
  std::function<void(const char *, void *ctx)> handler;
  void *ctx;
};

class rpc_event_queue {
public:
  rpc_event_queue() : pending_requests_() {}

  void add(Call &call) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    pending_requests_.emplace(call.key, rpc_event(false, call));
  }

  void remove(const std::string &key) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    pending_requests_.erase(key);
  }

  void doAll(std::function<void(Call &&)> send) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    for (auto &[key, event] : pending_requests_) {
      send(std::move(event.call));
    }
  }

  bool success(const std::string &key) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    auto call = pending_requests_.find(key);
    if (call != pending_requests_.end()) {
      return call->second.success;
    }
    return false;
  }

  std::string get_result(const std::string &key) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    return pending_requests_.find(key)->second.result;
  }

  Call& set_result(const std::string &key, std::string result) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    auto event = &pending_requests_.find(key)->second;
    event->result = result;
    event->success = true;
    return event->call;
  }

private:
  class rpc_event {
  public:
    explicit rpc_event(bool s, Call &c) : success(s), call(c), result() {}

    bool success;
    Call call;
    std::string result;
  };
  std::mutex pending_mutex_;
  std::unordered_map<std::string, rpc_event> pending_requests_;
};

class connect_status {
public:
  connect_status() : status_(status::NOT_CONNECT) {}

  bool not_connect() { return status_ == status::NOT_CONNECT; }

  bool connected() { return status_ == status::CONNECTED; }

  bool connecting() { return status_ == status::CONNECTING; }

  void to_connecting() { status_ = status::CONNECTING; }

  void to_connected() { status_ = status::CONNECTED; }

  void to_not_connect() { status_ = status::NOT_CONNECT; }

private:
  enum class status {
    NOT_CONNECT,
    CONNECTING,
    CONNECTED,
  };

  status status_;
};

class Channel : public std::enable_shared_from_this<Channel> {
public:
  using ptr = std::shared_ptr<Channel>;

  static ptr Create(boost::asio::io_context &io, Target &target) {
    return std::make_shared<Channel>(io, target);
  }

  Channel(boost::asio::io_context &ctx, Target &target);

  ~Channel();

  mrpc_status Send(mrpc_call *call);

  mrpc_status Receive(mrpc_call *call);

  void Wait(mrpc_call *call);

  // 主动连接（可用于重连）
  void Connect();

private:
  void OnConnect(const boost::system::error_code &ec);
  void Send(Call &&call);
  void DoRead();
  void OnRead(const boost::system::error_code &ec);

  tcp::socket socket_;
  tcp::resolver resolver_;
  Target target_;

  rpc_event_queue wait_result_queue;
  rpc_event_queue pending_queue;

  connect_status connect_status_;
  std::mutex status_mutex_;

  boost::asio::streambuf data_;
};
} // namespace mrpc