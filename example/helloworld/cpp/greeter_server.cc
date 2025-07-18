#include <iostream>
#include <string>
#include <cstring>
#include "helloworld.mrpc.h"
#include "hello_handler.h"

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