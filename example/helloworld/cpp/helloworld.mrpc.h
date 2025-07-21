#pragma once

#include "mrpcpp/server.h"
#include "mrpcpp/client.h"
#include <string>

using json = nlohmann::json;

namespace helloworld {

static const char *Greeter_method_names[] = {
    "/helloworld.Greeter/SayHello",
};

class SayHelloRequest : public mrpc::Parser {
public:
  SayHelloRequest() {}
  SayHelloRequest(std::string name) : name(name) {}

private:
  json toJson() const override { return json{{"name", name}}; }
  void fromJson(const json &j) override { name = j.value("name", ""); }

public:
  std::string name;
};

class SayHelloResponse : public mrpc::Parser {
public:
  SayHelloResponse() {}
  SayHelloResponse(std::string message) : message(message) {}

private:
  json toJson() const override { return json{{"message", message}}; }
  void fromJson(const json &j) override { message = j.value("message", ""); }

public:
  std::string message;
};

class GreeterStub : mrpc::client::MrpcClient {
public:
  GreeterStub(const std::string &addr) : mrpc::client::MrpcClient(addr) {}

  mrpc::Status SayHello(SayHelloRequest &request, SayHelloResponse &response) {
    return Send(Greeter_method_names[0], request, response);
  }

  mrpc::Status AsyncSayHello(SayHelloRequest &request, std::string &key) {
    return AsyncSend(Greeter_method_names[0], request, key);
  }

  void CallbackSayHello(SayHelloRequest &request, SayHelloResponse &response,
                        std::function<void(mrpc::Status)> callback) {
    CallbackSend(Greeter_method_names[0], request, response, callback);
  }

  mrpc::Status Receive(const std::string &key, SayHelloResponse &response) {
    return mrpc::client::MrpcClient::Receive(key, response);
  }
};

class GreeterService : public mrpc::server::MrpcService {
public:
  GreeterService() : mrpc::server::MrpcService("helloworld.Greeter") {
    AddHandler<SayHelloRequest, SayHelloResponse>(
        Greeter_method_names[0],
        [this](const SayHelloRequest &request, SayHelloResponse &response) {
          return this->SayHello(request, response);
        });
  }

  virtual mrpc::Status SayHello(const SayHelloRequest &request,
                                SayHelloResponse &response) = 0;
};

} // namespace helloworld