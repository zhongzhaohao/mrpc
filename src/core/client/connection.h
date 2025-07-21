#pragma once

#include "src/core/client/target.h"
#include "src/core/common/connection.h"
#include <boost/asio.hpp>

#include "mrpc/mrpc.h"

namespace mrpc::client {
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

class Connection : public BaseConnection<Connection> {
public:
  using ptr = std::shared_ptr<Connection>;
  static ptr Create(boost::asio::io_context &io, Target &target,
                    response_handler handler) {
    return std::make_shared<Connection>(io, target, handler);
  }

  Connection(boost::asio::io_context &ctx, Target &target,
             response_handler handler);

  void Close() override {
    if (socket().is_open()) {
      socket().cancel();
      socket().shutdown(tcp::socket::shutdown_both);
      socket().close();
    }
  }

  void Connect(std::function<void()> pre_connect = nullptr);

  void HandleReadData(const std::string &data) override;

  mrpc_status PreSend(mrpc_call *call) override;

  void SendFailed(mrpc_call *call,
                  const boost::system::error_code &ec) override;

private:
  void OnConnect(const boost::system::error_code &ec);

  tcp::resolver resolver_;
  Target target_;

  rpc_event_queue pending_queue;

  // 客户端和服务端逻辑不一样：
  //    服务端接收到信息则必定已经连接
  //    客户端要发送信息前可能没有连接
  // 因此客户端需要维护连接状态
  connect_status connect_status_;
  std::mutex status_mutex_;

  response_handler handler_;
};
} // namespace mrpc::client