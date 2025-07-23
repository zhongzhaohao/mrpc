#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

// 定义参数结构
struct Parameter {
  std::string name;
  std::string type;
};

// 定义方法结构
struct Method {
  std::string name;
  std::vector<Parameter> request_params;
  std::vector<Parameter> response_params;
};

// 定义服务结构
struct Service {
  std::string name;
  std::vector<Method> methods;
};

class StubGenerator {
private:
  std::string yaml_filename; // 不含扩展名的yaml文件名
  std::string namespace_name;
  Service service;
  std::stringstream output;
  std::string output_path;

  // 首字母大写
  std::string capitalize(const std::string &str) {
    std::string result = str;
    if (!result.empty()) {
      result[0] = std::toupper(result[0]);
    }
    return result;
  }

  // 生成头文件保护和包含声明
  void generateHeader() {
    output << "#pragma once\n\n";
    output << "#include \"mrpcpp/server.h\"\n";
    output << "#include \"mrpcpp/client.h\"\n";
    output << "#include <string>\n\n";
    output << "using json = nlohmann::json;\n\n";
  }

  // 生成命名空间开始
  void generateNamespaceStart() {
    output << "namespace " << namespace_name << " {\n\n";
  }

  // 生成方法名数组
  void generateMethodNames() {
    output << "static const char *" << service.name << "_method_names[] = {\n";
    for (const auto &method : service.methods) {
      output << "    \"/" << namespace_name << "." << service.name << "/"
             << method.name << "\",\n";
    }
    output << "};\n\n";
  }

  // 生成参数的JSON处理代码
  std::string generateJsonCode(const std::vector<Parameter> &params,
                               bool isToJson) {
    std::stringstream ss;
    if (isToJson) {
      ss << "return json{";
      for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0)
          ss << ",";
        ss << "{\"" << params[i].name << "\", " << params[i].name << "}";
      }
      ss << "};";
    } else {
      for (const auto &param : params) {
        std::string defaultValue;
        if (param.type == "string")
          defaultValue = "\"\"";
        else if (param.type == "int")
          defaultValue = "0";
        else if (param.type == "float")
          defaultValue = "0.0f";
        else if (param.type == "bool")
          defaultValue = "false";

        ss << param.name << " = j.value(\"" << param.name << "\", "
           << defaultValue << "); ";
      }
    }
    return ss.str();
  }

  // 生成构造函数参数列表
  std::string generateConstructorParams(const std::vector<Parameter> &params) {
    std::stringstream ss;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        ss << ", ";
      if (params[i].type == "string")
        ss << "std::string " << params[i].name;
      else
        ss << params[i].type << " " << params[i].name;
    }
    return ss.str();
  }

  // 生成初始化列表
  std::string generateInitList(const std::vector<Parameter> &params) {
    std::stringstream ss;
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        ss << ", ";
      ss << params[i].name << "(" << params[i].name << ")";
    }
    return ss.str();
  }

  // 生成请求/响应类
  void generateRequestResponseClass(const Method &method) {
    // 请求类
    output << "class " << method.name << "Request : public mrpc::Parser {\n";
    output << "public:\n";
    output << "  " << method.name << "Request() {}\n";
    output << "  " << method.name << "Request("
           << generateConstructorParams(method.request_params)
           << ") : " << generateInitList(method.request_params) << " {}\n\n";

    output << "private:\n";
    output << "  json toJson() const override { "
           << generateJsonCode(method.request_params, true) << " }\n";
    output << "  void fromJson(const json &j) override { "
           << generateJsonCode(method.request_params, false) << "}\n\n";

    output << "public:\n";
    for (const auto &param : method.request_params) {
      if (param.type == "string")
        output << "  std::string " << param.name << ";\n";
      else
        output << "  " << param.type << " " << param.name << ";\n";
    }
    output << "};\n\n";

    // 响应类
    output << "class " << method.name << "Response : public mrpc::Parser {\n";
    output << "public:\n";
    output << "  " << method.name << "Response() {}\n";
    output << "  " << method.name << "Response("
           << generateConstructorParams(method.response_params)
           << ") : " << generateInitList(method.response_params) << " {}\n\n";

    output << "private:\n";
    output << "  json toJson() const override { "
           << generateJsonCode(method.response_params, true) << " }\n";
    output << "  void fromJson(const json &j) override { "
           << generateJsonCode(method.response_params, false) << "}\n\n";

    output << "public:\n";
    for (const auto &param : method.response_params) {
      if (param.type == "string")
        output << "  std::string " << param.name << ";\n";
      else
        output << "  " << param.type << " " << param.name << ";\n";
    }
    output << "};\n\n";
  }

  // 生成Stub类
  void generateStubClass() {
    output << "class " << service.name << "Stub : mrpc::client::MrpcClient {\n";
    output << "public:\n";
    output << "  " << service.name << "Stub(const std::string &addr) : "
           << "mrpc::client::MrpcClient(addr) {}\n\n";

    // 为每个方法生成三种调用方式
    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i];

      // 同步调用
      output << "  mrpc::Status " << method.name << "(" << method.name
             << "Request &request, " << method.name
             << "Response &response) {\n";
      output << "    return Send(" << service.name << "_method_names[" << i
             << "], request, response);\n  }\n\n";

      // 异步调用
      output << "  mrpc::Status Async" << method.name << "(" << method.name
             << "Request &request, std::string &key) {\n";
      output << "    return AsyncSend(" << service.name << "_method_names[" << i
             << "], request, key);\n  }\n\n";

      // 回调方式
      output << "  void Callback" << method.name << "(" << method.name
             << "Request &request, " << method.name << "Response &response,\n"
             << "                        std::function<void(mrpc::Status)> "
                "callback) {\n";
      output << "    CallbackSend(" << service.name << "_method_names[" << i
             << "], request, response, callback);\n  }\n\n";
    }

    // 模板化的Receive方法
    output << "  template<typename T>\n";
    output << "  mrpc::Status Receive(const std::string &key, T &response) {\n";
    output << "    return mrpc::client::MrpcClient::Receive(key, response);\n";
    output << "  }\n";
    output << "};\n\n";
  }

  // 生成Service类
  void generateServiceClass() {
    output << "class " << service.name
           << "Service : public mrpc::server::MrpcService {\n";
    output << "public:\n";
    output << "  " << service.name << "Service() : mrpc::server::MrpcService(\""
           << namespace_name << "." << service.name << "\") {\n";

    for (size_t i = 0; i < service.methods.size(); ++i) {
      const auto &method = service.methods[i];
      output << "    AddHandler<" << method.name << "Request, " << method.name
             << "Response>(\n";
      output << "        " << service.name << "_method_names[" << i << "],\n";
      output << "        [this](const " << method.name << "Request &request, "
             << method.name << "Response &response) {\n";
      output << "          return this->" << method.name
             << "(request, response);\n        });\n";
    }
    output << "  }\n\n";

    // 纯虚函数声明
    for (const auto &method : service.methods) {
      output << "  virtual mrpc::Status " << method.name << "(const "
             << method.name << "Request &request,\n"
             << "                                " << method.name
             << "Response &response) = 0;\n";
    }
    output << "};\n\n";
  }

  // 生成命名空间结束
  void generateNamespaceEnd() {
    output << "} // namespace " << namespace_name << "\n";
  }

public:
  StubGenerator(const std::string &yaml_path) {
    // 提取文件名（不含扩展名）作为命名空间
    size_t last_slash = yaml_path.find_last_of("/\\");
    size_t last_dot = yaml_path.find_last_of(".");
    if (last_slash == std::string::npos)
      last_slash = -1;
    yaml_filename = yaml_path.substr(last_slash + 1, last_dot - last_slash - 1);
    namespace_name = yaml_filename;
    output_path = yaml_filename + ".mrpc.h";
  }

  bool parseYaml(const std::string &yaml_path) {
    try {
      YAML::Node config = YAML::LoadFile(yaml_path);

      // 解析服务名称
      service.name = config["service"]["name"].as<std::string>();

      // 解析方法
      auto methods = config["service"]["methods"];
      for (const auto &method : methods) {
        Method m;
        m.name = method.first.as<std::string>();

        // 解析请求参数
        auto request = method.second["request"];
        for (const auto &param : request) {
          Parameter p;
          p.name = param.first.as<std::string>();
          p.type = param.second.as<std::string>();
          m.request_params.push_back(p);
        }

        // 解析响应参数
        auto response = method.second["response"];
        for (const auto &param : response) {
          Parameter p;
          p.name = param.first.as<std::string>();
          p.type = param.second.as<std::string>();
          m.response_params.push_back(p);
        }

        service.methods.push_back(m);
      }
      return true;
    } catch (const YAML::Exception &e) {
      std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
      return false;
    }
  }

  bool generate() {
    generateHeader();
    generateNamespaceStart();
    generateMethodNames();

    // 生成所有方法的请求/响应类
    for (const auto &method : service.methods) {
      generateRequestResponseClass(method);
    }

    generateStubClass();
    generateServiceClass();
    generateNamespaceEnd();

    return true;
  }

  const std::string GetOutputFile() const { return output.str(); }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input.yaml> <output.h>"
              << std::endl;
    return 1;
  }

  StubGenerator generator(argv[1]);
  if (!generator.parseYaml(argv[1])) {
    std::cerr << "Failed to parse YAML file" << std::endl;
    return 1;
  }

  if (!generator.generate()) {
    std::cerr << "Failed to generate stub file" << std::endl;
    return 1;
  }

  std::cout << generator.GetOutputFile() << std::endl;
  return 0;
}