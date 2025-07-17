#pragma once

#include "src/core/server/connection.h"
#include "src/core/server/target.h"
#include "src/core/common/c_cpp_trans.h"
#include "mrpc/mrpc.h"

#include <boost/asio.hpp>
#include <thread>
#include <iostream>

namespace mrpc {
using boost::asio::ip::tcp;

class Server : public CppImplOf<Server, mrpc_server> {
public:
  Server(const std::string &addr)
      : io_(), work_guard_(boost::asio::make_work_guard(io_)),
        target_(addr), acceptor_(io_) {}
  
  ~Server() {
    std::cout << "Stopping RPC server" << std::endl;
    acceptor_.close();
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
  }
  
  mrpc_status RegisterService(mrpc_service* service) {
    std::vector<std::string> methods;
    std::vector<request_handler> handlers;
    
    for (int i = 0; i < service->method_count; ++i) {
      methods.push_back(service->methods[i]);
      handlers.push_back(service->handlers[i]);
    }
    
    registry_.RegisterService(service->name, methods, handlers);
    return MRPC_OK;
  }
  
  mrpc_status Start() {
    try {
      // 绑定端口
      acceptor_.open(tcp::v4());
      acceptor_.set_option(tcp::acceptor::reuse_address(true));
      acceptor_.bind(target_.endpoint());
      acceptor_.listen();
      
      std::cout << "Server listening on " << target_.endpoint() << std::endl;
      
      // 开始接受连接
      DoAccept();
      
      // 启动IO线程
      io_thread_ = std::thread([this] { io_.run(); });
      
      return MRPC_OK;
    } catch (const std::exception& e) {
      std::cerr << "Server start error: " << e.what() << std::endl;
      return MRPC_SEND_FAILURE;
    }
  }

private:
  void DoAccept() {
    auto new_connection = Connection::Create(io_, &registry_);
    
    acceptor_.async_accept(new_connection->socket(),
        [this, new_connection](const boost::system::error_code& ec) {
          if (!ec) {
            new_connection->Start();
          }
          DoAccept(); // 继续接受新连接
        });
  }

private:
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread io_thread_;
  ServerTarget target_;
  tcp::acceptor acceptor_;
  ServiceRegistry registry_;
};

} // namespace mrpc 