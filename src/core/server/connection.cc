#include "connection.h"
#include <iostream>
#include <sstream>

static const std::string CALL_DELIMITER = "#MRPC#";
static const std::string KEY_DELIMITER = "#-#";

std::string GetSendMessage(const std::string& key, const std::string& message) {
  std::ostringstream oss;
  oss << key << KEY_DELIMITER << message << CALL_DELIMITER;
  return oss.str();
}

std::tuple<std::string, std::string, std::string, mrpc_status>
ParseMessage(const std::string &input) {
  size_t key_delim_pos = input.find(KEY_DELIMITER);
  size_t call_delim_pos = input.find(CALL_DELIMITER);

  if (key_delim_pos == std::string::npos ||
      call_delim_pos == std::string::npos || call_delim_pos <= key_delim_pos) {
    return {"", "", "", MRPC_PARSE_FAILURE};
  }

  std::string key = input.substr(0, key_delim_pos);
  std::string message = input.substr(key_delim_pos + KEY_DELIMITER.length(),
                       call_delim_pos - key_delim_pos - KEY_DELIMITER.length());
  
  // 从key中提取方法名
  size_t last_dash = key.rfind('-');
  std::string method = key.substr(0, last_dash);
  
  return {key, method, message, MRPC_OK};
}

namespace mrpc {

void Connection::DoRead() {
  auto self(shared_from_this());
  
  boost::asio::async_read_until(
      socket_, data_, CALL_DELIMITER,
      [self](const boost::system::error_code &ec,
             size_t /*bytes_transferred*/) { self->OnRead(ec); });
}

void Connection::OnRead(const boost::system::error_code& ec) {
  if (ec) {
    if (ec != boost::asio::error::eof) {
      std::cerr << "Read error: " << ec.message() << "\n";
    }
    return;
  }
  
  // 提取数据
  std::string buffer_content(boost::asio::buffers_begin(data_.data()),
                           boost::asio::buffers_end(data_.data()));
  data_.consume(buffer_content.length());
  
  // 解析消息
  auto [key, method, request, status] = ParseMessage(buffer_content);
  
  if (status != MRPC_OK) {
    SendResponse(key, "Parse error", MRPC_PARSE_FAILURE);
    DoRead();
    return;
  }
  
  // 查找处理函数
  auto handler = registry_->GetHandler(method);
  if (!handler) {
    SendResponse(key, "Method not found: " + method, MRPC_PARSE_FAILURE);
    DoRead();
    return;
  }
  
  // 调用处理函数
  const char* response = nullptr;
  mrpc_status handler_status = MRPC_OK;
  handler(key.c_str(), request.c_str(), &response, &handler_status);
  
  // 发送响应
  if (response) {
    SendResponse(key, response, handler_status);
    // 释放响应内存（由handler分配）
    free((void*)response);
  } else {
    SendResponse(key, "", handler_status);
  }
  
  // 继续读取
  DoRead();
}

void Connection::SendResponse(const std::string& key, const std::string& response, 
                            mrpc_status status) {
  if (status != MRPC_OK) {
    // 如果有错误，可以在响应中包含错误信息
    std::cerr << "Handler error for key " << key << ": status=" << status << "\n";
  }
  
  auto message = GetSendMessage(key, response);
  auto self(shared_from_this());
  
  boost::asio::async_write(
      socket_, boost::asio::buffer(message),
      [self](const boost::system::error_code &ec, size_t /*bytes_transferred*/) {
        if (ec) {
          std::cerr << "Write error: " << ec.message() << "\n";
        }
      });
}

} // namespace mrpc