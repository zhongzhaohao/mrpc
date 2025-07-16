#pragma once

#include <string>

#include "mrpc/mrpc.h"

namespace mrpc {

enum StatusCode {
  // mrpc_status
  OK,
  CANCELED,
  MRPC_SEND_FAILURE,
  MRPC_PARSE_FAILURE,
  // json parse
  PARSE_FROM_JSON_FAILURE,
  PARSE_TO_JSON_FAILURE,
};

// TODO： implement custom exception
class Status {
public:
  Status() : code_(StatusCode::OK) {}

  Status(mrpc_status mrpc_code) : code_(static_cast<StatusCode>(mrpc_code)) {}

  Status(StatusCode code, const std::string &error_message)
      : code_(code), message_(error_message) {}

  Status(mrpc_status mrpc_code, const std::string &error_message)
      : code_(static_cast<StatusCode>(mrpc_code)), message_(error_message) {}

  bool ok() const { return code_ == StatusCode::OK; }

  StatusCode error_code() const { return code_; }

  std::string message() const { return message_; }

  Status &operator<<(const std::string &message) {
    message_ += message;
    return *this;
  }

private:
  StatusCode code_;
  std::string message_;
};

} // namespace mrpc