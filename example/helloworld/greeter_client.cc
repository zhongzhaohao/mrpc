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
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<HelloStub> stub_;
};

int main() {
  HelloClient hello;

  std::string response = hello.SayHello("what");

  std::cout << "Greeter received: " << response << std::endl;

  return 0;
}