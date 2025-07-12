#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using boost::asio::ip::tcp;
using json = nlohmann::json;

static const std::string CALL_DELIMITER = "#MRPC#";
static const std::string KEY_DELIMITER = "#-#";

struct mrpc_call {
  std::string key;
  std::string message;
};

mrpc_call ParseMessage(const std::string &input) {
  size_t key_delim_pos = input.find(KEY_DELIMITER);
  size_t call_delim_pos = input.find(CALL_DELIMITER);

  if (key_delim_pos == std::string::npos ||
      call_delim_pos == std::string::npos || call_delim_pos <= key_delim_pos) {
    return mrpc_call{};
  }

  mrpc_call out;

  out.key = input.substr(0, key_delim_pos);
  out.message =
      input.substr(key_delim_pos + KEY_DELIMITER.length(),
                   call_delim_pos - key_delim_pos - KEY_DELIMITER.length());

  return out;
}

void handle_client(tcp::socket socket) {
  for (;;) {
    try {
      std::vector<char> buffer(1024);
      boost::system::error_code ec;
      size_t len = socket.read_some(boost::asio::buffer(buffer), ec);

      if (ec && ec != boost::asio::error::eof) {
        std::cerr << "Read error: " << ec.message() << "\n";
        return;
      }

      // 将接收到的数据转换为字符串
      std::string data(buffer.data(), len);
      std::cout << "Received raw: " << data << "\n";

      auto receive = ParseMessage(data);

      // 解析 JSON
      try {
        json received_json = json::parse(receive.message);
        std::cout << "Received JSON: " << received_json.dump(4) << "\n";

        // 构造响应 JSON
        json response_json;
        response_json["message"] = "hello " + received_json.value("name", "") + " !";

        std::ostringstream oss;
        oss << receive.key << KEY_DELIMITER << response_json.dump()
            << CALL_DELIMITER;

        std::string response_str = oss.str();

        std::cout << "send response : " << response_str << std::endl;
        boost::asio::write(socket, boost::asio::buffer(response_str));
      } catch (const json::parse_error &e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        std::string err_msg = "Invalid JSON";
        boost::asio::write(socket, boost::asio::buffer(err_msg));
      }
    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << "\n";
      return;
    }
  }
}

int main() {
  try {
    boost::asio::io_context io_context;

    // 创建 acceptor 并绑定到端口 8080
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
    std::cout << "Server is listening on port 8080...\n";

    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      std::thread(&handle_client, std::move(socket)).detach();
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}