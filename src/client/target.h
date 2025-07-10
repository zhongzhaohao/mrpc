#pragma once

#include <boost/asio.hpp>
#include <string>

namespace mrpc {
using boost::asio::ip::tcp;

class Target {
public:
  explicit Target(const std::string &input) {
    auto pos = input.find(':');
    if (pos != std::string::npos &&
        input.find(':', pos + 1) == std::string::npos) {
      // 形如 "ip:port"
      host_ = input.substr(0, pos);
      service_ = input.substr(pos + 1);
    } else {
      // 否则视为 host，默认服务为 http
      host_ = input;
      service_ = "http";
    }
  }

  tcp::resolver::results_type resolve(boost::asio::io_context &ctx) {
    tcp::resolver resolver(ctx);
    return resolver.resolve(host_, service_);
  }

private:
  std::string host_;
  std::string service_;
};
} // namespace mrpc