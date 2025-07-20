#pragma once

#include <nlohmann/json.hpp>

namespace mrpc {
class Parser {
public:
  virtual ~Parser() = default;

  virtual std::string toString(int indent = -1) {
    return toJson().dump(indent);
  }

  virtual void fromString(const std::string &s) {
    fromJson(nlohmann::json::parse(s));
  }

private:
  virtual nlohmann::json toJson() const = 0;
  virtual void fromJson(const nlohmann::json &j) = 0;
};

} // namespace mrpc