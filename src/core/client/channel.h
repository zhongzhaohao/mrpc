#pragma once

#include "src/core/client/target.h"
#include <boost/asio.hpp>

#include "mrpc/mrpc.h"

namespace mrpc {
using boost::asio::io_context;
using boost::asio::ip::tcp;

class rpc_event_queue {
public:
  rpc_event_queue() : pending_requests_() {}

  void add(mrpc_call *call) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    // 防止内存被前端释放，handler为全局函数，地址不会变
    mrpc_call save{
        strdup(call->key),
        strdup(call->message),
        call->handler,
    };
    pending_requests_.emplace(call->key, save);
  }

  void consume(std::function<void(mrpc_call *)> send) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    for (auto &[key, call] : pending_requests_) {
      send(&call);
      // 在消耗掉此call后需要回收strdup分配的内存
      erase(call);
    }
    pending_requests_.clear();
  }

  response_handler consume_with_handler(const std::string key) {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    auto call = pending_requests_.find(key);
    assert(call != pending_requests_.end());
    auto handler = call->second.handler;
    // 在消耗掉此call后需要回收strdup分配的内存
    erase(call->second);
    pending_requests_.erase(key);
    return handler;
  }

private:
  void erase(mrpc_call &call) {
    free((void *)call.key);
    free((void *)call.message);
  }

  std::mutex pending_mutex_;
  std::unordered_map<std::string, mrpc_call> pending_requests_;
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

  // 主动连接（可用于重连）
  // first call outside without lock
  void Connect(std::function<void()> func = nullptr);

private:
  void OnConnect(const boost::system::error_code &ec);
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