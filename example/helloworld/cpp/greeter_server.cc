#include <iostream>
#include <string>
#include <cstring>
#include "helloworld.mrpc.h"

// SayHello的处理函数 - 这是真正的业务逻辑
void HandleSayHello(const char* key, const char* request_str, 
                   const char** response, mrpc_status* status) {
  try {
    // 解析请求
    json req_json = json::parse(request_str);
    std::string name = req_json.value("name", "");
    
    // 构造响应
    json resp_json;
    resp_json["message"] = "hello " + name + " !";
    std::string resp_str = resp_json.dump();
    
    // 返回响应（需要动态分配内存）
    *response = strdup(resp_str.c_str());
    *status = MRPC_OK;
    
  } catch (const std::exception& e) {
    std::cerr << "Handler error: " << e.what() << std::endl;
    *status = MRPC_PARSE_FAILURE;
  }
}

int main() {
  // 创建服务器
  mrpc::MrpcServer server("0.0.0.0:8080");
  
  // 创建Hello服务
  auto hello_service = std::make_shared<HelloService>();
  
  // 注册SayHello方法的处理函数
  hello_service->RegisterMethod("SayHello", HandleSayHello);
  
  // 注册服务到服务器
  mrpc::Status status = server.RegisterService(hello_service);
  if (!status.ok()) {
    std::cerr << "Failed to register service: " << status.message() << std::endl;
    return 1;
  }
  
  // 启动服务器
  status = server.Start();
  if (!status.ok()) {
    std::cerr << "Failed to start server: " << status.message() << std::endl;
    return 1;
  }
  
  std::cout << "Server started successfully, listening on 0.0.0.0:8080" << std::endl;
  
  // 阻塞等待
  server.Wait();
  
  return 0;
}