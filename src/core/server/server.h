#pragma once

#include "mrpc/mrpc.h"
#include "src/core/common/c_cpp_trans.h"
#include "src/core/server/connection.h"
#include "src/core/server/endpoint.h"

#include <boost/asio.hpp>
#include <iostream>
#include <thread>

namespace mrpc {
using boost::asio::ip::tcp;

class Server : public CppImplOf<Server, mrpc_server> {
public:
  Server(const std::string &addr, request_handler handler);

  ~Server() {
    std::cout << "Stopping RPC server" << std::endl;
    acceptor_.close();
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
  }

  mrpc_status Start();

  mrpc_status Send(mrpc_call *call, cchar_t *source);

private:
  void DoAccept();

private:
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread io_thread_;
  server::Endpoint target_;
  tcp::acceptor acceptor_;
  request_handler handler_;
  std::unordered_map<std::string, std::shared_ptr<server::Connection>>
      connections_;
  std::mutex connection_mutex_;
};

} // namespace mrpc