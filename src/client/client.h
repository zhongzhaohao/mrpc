#pragma once

#include "src/common/status.h"
#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

using json = nlohmann::json;

namespace mrpc {
class ParseToJson {
public:
  virtual json toJson() const = 0;
  virtual ~ParseToJson() = default;
};

class ParseFromJson {
public:
  virtual void fromJson(const json &j) = 0;
  virtual ~ParseFromJson() = default;
};

class RpcClient {
public:
  RpcClient()
      : io_(), work_guard_(boost::asio::make_work_guard(io_)),
        io_thread_(std::thread([this] { io_.run(); })) {}

  ~RpcClient() {
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable()) {
      io_thread_.join();
    }
  }

  Status post(std::string func, ParseToJson &t, ParseFromJson &f) {
    try {
      // boost::asio::io_context io_;

      tcp::resolver resolver(io_);
      // ??????????????????????? TODO
      auto endpoints = resolver.resolve("127.0.0.1", "8080");

      // 创建并连接 socket
      tcp::socket socket(io_);
      asio::connect(socket, endpoints);

      std::string json_str = t.toJson().dump();

      // 发送 JSON 数据
      asio::write(socket, asio::buffer(json_str));
      std::cout << "Sent JSON: " << t.toJson().dump(4) << "\n";

      // 接收响应
      std::string response_str;
      char buffer[1024];
      size_t len = socket.read_some(asio::buffer(buffer));
      response_str.assign(buffer, len);
      f.fromJson(json::parse(response_str));

      std::cout << "Response: " << response_str << "\n";

      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
      return Status();
    } catch (const std::exception &e) {
      std::cerr << "Exception: " << e.what() << "\n";
      return Status::FAILURE(e.what());
    }
  }

private:
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;
  std::thread io_thread_;
};
} // namespace mrpc