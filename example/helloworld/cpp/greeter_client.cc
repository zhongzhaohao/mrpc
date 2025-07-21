#include <condition_variable>
#include <iostream>
#include <string>

#include "helloworld.mrpc.h"

using helloworld::GreeterStub;
using helloworld::SayHelloRequest;
using helloworld::SayHelloResponse;

class GreeterClient {
public:
  GreeterClient() : stub_(new GreeterStub("127.0.0.1:8080")) {}

  std::string SayHello(const std::string &user) {
    SayHelloRequest request(user);
    SayHelloResponse response;

    mrpc::Status status = stub_->SayHello(request, response);

    if (status.ok()) {
      return response.message;
    } else {
      std::cout << status.error_code() << ": " << status.message() << std::endl;
      return "RPC failed";
    }
  }

  std::string AsyncSayHello(const std::string &user) {
    SayHelloRequest request(user);
    SayHelloResponse response;

    std::string key;
    mrpc::Status status = stub_->AsyncSayHello(request, key);

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.message() << std::endl;
      return "Async RPC failed";
    }

    status = stub_->Receive(key, response);
    if (status.ok()) {
      return response.message;
    } else {
      std::cout << status.error_code() << ": " << status.message() << std::endl;
      return "RPC failed";
    }
  }

  std::string CallbackSayHello(const std::string &user) {
    SayHelloRequest request(user);
    SayHelloResponse response;

    // The actual RPC.
    std::mutex mu;
    std::condition_variable cv;
    bool done = false;
    mrpc::Status status;
    stub_->CallbackSayHello(request, response,
                            [&mu, &cv, &done, &status](mrpc::Status s) {
                              status = std::move(s);
                              std::lock_guard<std::mutex> lock(mu);
                              done = true;
                              cv.notify_one();
                            });

    std::unique_lock<std::mutex> lock(mu);
    while (!done) {
      cv.wait(lock);
    }

    // Act upon its status.
    if (status.ok()) {
      return response.message;
    } else {
      std::cout << status.error_code() << ": " << status.message() << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<GreeterStub> stub_;
};

int main() {
  GreeterClient greeter;

  std::string response1 = greeter.SayHello("sync RPC");
  std::cout << "Greeter 1 received: " << response1 << std::endl;

  std::string response2 = greeter.AsyncSayHello("async RPC");
  std::cout << "Greeter 2 received: " << response2 << std::endl;

  std::string response3 = greeter.CallbackSayHello("callback RPC");
  std::cout << "Greeter 3 received: " << response3 << std::endl;

  return 0;
}