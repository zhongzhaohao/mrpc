#include "hello_handler.h"
#include <iostream>
#include <string>
#include <cstring>

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