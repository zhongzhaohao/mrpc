#include "helloworld.mrpc.h"
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <string>

std::atomic<bool> stop_signal_received(false);
std::mutex cv_mutex;
std::condition_variable cv;

void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::cout << "Signal " << signal << " received." << std::endl;
    stop_signal_received = true;
    cv.notify_all(); // 唤醒等待线程
  }
}

using helloworld::GreeterService;
using helloworld::SayHelloRequest;
using helloworld::SayHelloResponse;

class GreeterServiceImpl : public GreeterService {
public:
  mrpc::Status SayHello(const SayHelloRequest &request,
                        SayHelloResponse &response) override {
    response.message = "Hello " + request.name;
    return mrpc::Status::OK();
  }
};

int main() {

  GreeterServiceImpl greeter_service;

  auto server = mrpc::server::MrpcServer::Create("0.0.0.0:8080");

  server->RegisterService(&greeter_service);

  auto status = server->Start();
  if (!status.ok()) {
    std::cerr << "Failed to start server: " << status.message() << std::endl;
    return 1;
  }

  std::cout << "Server started successfully, listening on 0.0.0.0:8080"
            << std::endl;

  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::unique_lock<std::mutex> lock(cv_mutex);
  cv.wait(lock, [] { return stop_signal_received.load(); });

  server->Stop();
  std::cout << "Server stopped." << std::endl;

  return 0;
}