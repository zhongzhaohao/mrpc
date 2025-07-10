#pragma once

#include "mrpc.h"
#include "src/client/channel.h"
#include "src/client/target.h"
#include "src/common/c_cpp_trans.h"
#include "src/common/json_utils.h"
#include "src/common/status.h"

#include <iostream>
#include <thread>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace mrpc {
class RpcClient : public CppImplOf<RpcClient, mrpc_client> {
public:
  RpcClient(const std::string &addr)
      : io_(), work_guard_(boost::asio::make_work_guard(io_)),
        io_thread_(std::thread([this] { io_.run(); })), target_(addr) {}

  ~RpcClient() {
    std::cout << "clear rpc client" << std::endl;
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
  }

  Status post(std::string func, ParseToJson &t, ParseFromJson &f);

private:
  void _init_channel() { channel_ = std::make_shared<Channel>(io_, target_); }

private:
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread io_thread_;
  Target target_;
  std::shared_ptr<Channel> channel_;
  std::once_flag channel_flag_;
};
} // namespace mrpc