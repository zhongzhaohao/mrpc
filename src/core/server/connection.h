#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <functional>
#include "mrpc/mrpc.h"

namespace mrpc {
using boost::asio::ip::tcp;

class ServiceRegistry {
public:
  using HandlerFunc = std::function<void(const std::string&, const std::string&, 
                                        std::string*, mrpc_status*)>;
  
  void RegisterService(const std::string& service_name, 
                      const std::vector<std::string>& methods,
                      const std::vector<request_handler>& handlers) {
    for (size_t i = 0; i < methods.size(); ++i) {
      std::string full_method = "/" + service_name + "/" + methods[i];
      handlers_[full_method] = handlers[i];
    }
  }
  
  request_handler GetHandler(const std::string& method) const {
    auto it = handlers_.find(method);
    if (it != handlers_.end()) {
      return it->second;
    }
    return nullptr;
  }

private:
  std::unordered_map<std::string, request_handler> handlers_;
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
  using ptr = std::shared_ptr<Connection>;
  
  static ptr Create(boost::asio::io_context& io, ServiceRegistry* registry) {
    return std::make_shared<Connection>(io, registry);
  }
  
  Connection(boost::asio::io_context& io, ServiceRegistry* registry)
      : socket_(io), registry_(registry), data_(2048) {}
  
  tcp::socket& socket() { return socket_; }
  
  void Start() { DoRead(); }

private:
  void DoRead();
  void OnRead(const boost::system::error_code& ec);
  void SendResponse(const std::string& key, const std::string& response, 
                   mrpc_status status);
  
  tcp::socket socket_;
  ServiceRegistry* registry_;
  boost::asio::streambuf data_;
};

} // namespace mrpc 