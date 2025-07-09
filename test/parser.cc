#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct RpcField {
  std::string name;
  std::string type;
};

struct RpcMethodInfo {
  std::string name;
  std::vector<RpcField> request_fields;
  std::vector<RpcField> response_fields;
};

struct RpcServiceInfo {
  std::string name;
  std::map<std::string, RpcMethodInfo> methods;
};

RpcField parse_field(const std::string &key, const YAML::Node &value_node) {
  RpcField field;
  field.name = key;
  field.type = value_node.as<std::string>();
  return field;
}

RpcMethodInfo parse_method(const std::string& method_name, const YAML::Node& node) {
    RpcMethodInfo method;
    method.name = method_name;

    auto parse_fields = [](const YAML::Node& parent_node, std::vector<RpcField>& out_fields) {
        if (parent_node && parent_node.IsMap()) {
            for (const auto& item : parent_node) {
                out_fields.push_back(RpcField{
                    item.first.as<std::string>(),
                    item.second.as<std::string>()
                });
            }
        }
    };

    parse_fields(node["request"], method.request_fields);
    parse_fields(node["response"], method.response_fields);
    return method;
}

RpcServiceInfo parse_service(const YAML::Node &node) {
  RpcServiceInfo service;
  service.name = node["service"]["name"].as<std::string>();

  const YAML::Node &methods_node = node["service"]["methods"];
  if (methods_node && methods_node.IsMap()) {
    for (auto it = methods_node.begin(); it != methods_node.end(); ++it) {
      std::string method_name = it->first.as<std::string>();
      service.methods[method_name] = parse_method(method_name, it->second);
    }
  }

  return service;
}

// 后续就根据这个 RpcService 来生成代码
int main() {
  try {
    YAML::Node config = YAML::LoadFile("/home/what/mrpc/example/helloworld/helloworld.yaml");

    RpcServiceInfo service = parse_service(config);

    std::cout << "Service Name: " << service.name << std::endl;
    for (const auto &[method_name, method] : service.methods) {
      std::cout << "  Method: " << method.name << std::endl;
      std::cout << "    Request Fields:" << std::endl;
      for (const auto &field : method.request_fields) {
        std::cout << "      - " << field.name << ": " << field.type
                  << std::endl;
      }
      std::cout << "    Response Fields:" << std::endl;
      for (const auto &field : method.response_fields) {
        std::cout << "      - " << field.name << ": " << field.type
                  << std::endl;
      }
    }
  } catch (const YAML::ParserException &e) {
    std::cerr << "YAML parsing error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}