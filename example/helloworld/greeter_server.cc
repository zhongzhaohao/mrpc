#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using json = nlohmann::json;

void handle_client(tcp::socket socket) {
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

        // 解析 JSON
        try {
            json received_json = json::parse(data);
            std::cout << "Received JSON: " << received_json.dump(4) << "\n";

            // 构造响应 JSON
            json response_json;
            response_json["status"] = "success";
            response_json["message"] = "JSON received and processed";

            std::string response_str = response_json.dump();
            boost::asio::write(socket, boost::asio::buffer(response_str));
        } catch (const json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << "\n";
            std::string err_msg = "Invalid JSON";
            boost::asio::write(socket, boost::asio::buffer(err_msg));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
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
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}