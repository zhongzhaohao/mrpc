#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "helloworld.mrpc.h"

class HelloClient {
public:
  HelloClient() : stub_(new HelloStub("127.0.0.1:8080")) {}

  std::string SayHello(const std::string &user) {
    SayHelloRequest request(user);
    SayHelloResponse response;

    mrpc::Status status = stub_->SayHello(request, response);

    if (status.ok()) {
      return response.message;
    } else {
      std::cout << status.error_code() << ": " << status.message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<HelloStub> stub_;
};

int main() {
  HelloClient hello;

  std::string response1 = hello.SayHello("what");
  std::cout << "Greeter 1 received: " << response1 << std::endl;

  std::string response2 = hello.SayHello("???");
  std::cout << "Greeter 2 received: " << response2 << std::endl;
  
  return 0;
}