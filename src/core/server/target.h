#pragma once

#include <boost/asio.hpp>
#include <string>

namespace mrpc {
using boost::asio::ip::tcp;

class ServerTarget {
public:
  ServerTarget(const std::string &addr) {
    auto colon_pos = addr.find(':');
    if (colon_pos != std::string::npos) {
      host_ = addr.substr(0, colon_pos);
      port_ = addr.substr(colon_pos + 1);
    } else {
      host_ = "0.0.0.0";
      port_ = addr;
    }
  }

  tcp::endpoint endpoint() const {
    return tcp::endpoint(boost::asio::ip::make_address(host_), 
                        std::stoi(port_));
  }

private:
  std::string host_;
  std::string port_;
};

} // namespace mrpc 