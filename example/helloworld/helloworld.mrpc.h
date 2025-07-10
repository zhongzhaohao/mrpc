#pragma once

#include <mrpc.h>
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

class HelloStub {
public:
  HelloStub(const std::string &addr) { client_ = mrpc_create_client(addr); }

  ~HelloStub() { mrpc_destroy_client(client_); }

  mrpc::Status SayHello(SayHelloRequest &request, SayHelloResponse &response) {
    return mrpc_sync_send_request(client_, Hello_method_names[0], request,
                                  response);
  }

private:
  mrpc_client *client_;
};