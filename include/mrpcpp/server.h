#pragma once

#include "mrpc/mrpc.h"
#include "status.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace mrpc {

// 服务处理函数的类型
using ServiceHandler = std::function<Status(const std::string& request, 
                                          std::string& response)>;

class ServiceBase {
public:
  virtual ~ServiceBase() = default;
  virtual std::string GetServiceName() const = 0;
  virtual std::vector<std::string> GetMethodNames() const = 0;
  virtual std::vector<request_handler> GetHandlers() const = 0;
};

class MrpcServer {
public:
  MrpcServer(const std::string& addr);
  ~MrpcServer();
  
  // 注册服务
  Status RegisterService(std::shared_ptr<ServiceBase> service);
  
  // 启动服务器
  Status Start();
  
  // 阻塞等待服务器停止
  void Wait();
  
private:
  mrpc_server* server_;
  std::vector<std::shared_ptr<ServiceBase>> services_;
};

// 用于生成服务实现的基类模板
template<typename ServiceImpl>
class Service : public ServiceBase {
public:
  Service(ServiceImpl* impl) : impl_(impl) {}
  
protected:
  ServiceImpl* impl_;
};

} // namespace mrpc 