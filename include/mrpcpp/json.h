#pragma once

#include <nlohmann/json.hpp>

namespace mrpc {
class ParseToJson {
public:
  virtual ~ParseToJson() = default;

  virtual std::string toString(int indent = -1) {
    return toJson().dump(indent);
  }

private:
  virtual nlohmann::json toJson() const = 0;
};

class ParseFromJson {
public:
  virtual ~ParseFromJson() = default;

  virtual void fromString(std::string &&s) {
    fromJson(nlohmann::json::parse(s));
  }

private:
  virtual void fromJson(const nlohmann::json &j) = 0;
};
} // namespace mrpc