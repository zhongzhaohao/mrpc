#pragma once

#include <string>

namespace mrpc {
enum StatusCode {
  OK,
  CANCELED,
  FAILURE,
};

class Status {
public:
  Status() : code_(StatusCode::OK) {}

  Status(StatusCode code, const std::string &error_message)
      : code_(code), error_message_(error_message) {}

  static Status FAILURE(const std::string &error_message) {
    return Status(StatusCode::FAILURE, error_message);
  }

  bool ok() const { return code_ == StatusCode::OK; }

  StatusCode error_code() const { return code_; }

  std::string error_message() const { return error_message_; }

private:
  StatusCode code_;
  std::string error_message_;
};

} // namespace mrpc