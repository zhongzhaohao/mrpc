#include "mrpcpp/server.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace mrpc {

MrpcServer::MrpcServer(const std::string& addr) {
  server_ = mrpc_create_server(addr.c_str());
}

MrpcServer::~MrpcServer() {
  mrpc_destroy_server(server_);
}

Status MrpcServer::RegisterService(std::shared_ptr<ServiceBase> service) {
  services_.push_back(service);
  
  // 获取服务信息
  std::string service_name = service->GetServiceName();
  std::vector<std::string> method_names = service->GetMethodNames();
  std::vector<request_handler> handlers = service->GetHandlers();
  
  // 转换为C接口需要的格式
  std::vector<const char*> method_ptrs;
  for (const auto& method : method_names) {
    method_ptrs.push_back(method.c_str());
  }
  
  mrpc_service c_service {
    service_name.c_str(),
    method_ptrs.data(),
    handlers.data(),
    static_cast<int>(method_names.size())
  };
  
  mrpc_status status = mrpc_register_service(server_, &c_service);
  return Status(status);
}

Status MrpcServer::Start() {
  mrpc_status status = mrpc_start_server(server_);
  return Status(status);
}

void MrpcServer::Wait() {
  // 简单的阻塞等待实现
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

} // namespace mrpc 