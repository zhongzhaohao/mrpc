#pragma once

#include <mrpc/mrpc.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

static const char *Hello_method_names[] = {
    "/Hello/SayHello",
};

class SayHelloRequest : public mrpc::ParseToJson {
public:
  SayHelloRequest(std::string name) : name(name) {}
  const std::string name;

  json toJson() const override { return json{{"name", name}}; }
};

class SayHelloResponse : public mrpc::ParseFromJson {
public:
  std::string message;

  void fromJson(const json &j) override { message = j.value("message", ""); }
};

class HelloStub {
public:
  HelloStub() : client_() {
    std::cout << "1233"<<std::endl;
  }

  mrpc::Status SayHello(SayHelloRequest &request, SayHelloResponse &response) {
    return client_.post(Hello_method_names[0], request, response);
  }

private:
  mrpc::RpcClient client_;
};