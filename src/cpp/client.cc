#include "mrpcpp/mrpcpp.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace mrpc {

MRPCClient::MRPCClient(const std::string &addr) {
  client_ = mrpc_create_client(addr.c_str());
}

MRPCClient::~MRPCClient() { mrpc_destroy_client(client_); }

Status MRPCClient::Send(const std::string &func, ParseToJson &request,
                        ParseFromJson &response) {
  auto req = request.toString();

  // key = func-uuid-cpp
  auto id = boost::uuids::random_generator()();
  std::ostringstream oss;
  oss << func << "-" << id << "-cpp";
  auto key = oss.str();

  mrpc_call call{key.c_str(), req.c_str()};
  Status status = mrpc_send_request(client_, &call);

  // and get respce

  if (status.ok()) {
    return status;
  } else {
    return status << "send func : " << func << "error";
  }
}
} // namespace mrpc