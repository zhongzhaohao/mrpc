#pragma once

#include "mrpcpp/mrpcpp.h"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

static const char *Hello_method_names[] = {
    "/Hello/SayHello",
};

class SayHelloRequest : public mrpc::ParseToJson {
public:
  SayHelloRequest() {}
  SayHelloRequest(std::string name) : name(name) {}

private:
  json toJson() const override { return json{{"name", name}}; }

public:
  const std::string name;
};

class SayHelloResponse : public mrpc::ParseFromJson {
public:
  SayHelloResponse() {}
  SayHelloResponse(std::string message) : message(message) {}

private:
  void fromJson(const json &j) override { message = j.value("message", ""); }

public:
  std::string message;
};

class HelloStub : mrpc::MrpcClient {
public:
  HelloStub(const std::string &addr) : mrpc::MrpcClient(addr) {}

  mrpc::Status SayHello(SayHelloRequest &request, SayHelloResponse &response) {
    return Send(Hello_method_names[0], request, response);
  }

  mrpc::Status AsyncSayHello(SayHelloRequest &request) {
    return AsyncSend(Hello_method_names[0], request);
  }

  void CallbackSayHello(SayHelloRequest &request, SayHelloResponse &response,std::function<void(mrpc::Status)> callback) {
    CallbackSend(Hello_method_names[0], request, response,callback);
  }

  mrpc::Status Receive(const std::string &key, SayHelloResponse &response) {
    return mrpc::MrpcClient::Receive(key,response);
  }
};