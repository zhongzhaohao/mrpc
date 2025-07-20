

#include <mrpc/mrpc.h>
#include <sstream>
#include <string>

static const std::string CALL_DELIMITER = "#MRPC#";
static const std::string KEY_DELIMITER = "#-#";

inline std::string GetSendMessage(mrpc_call *call) {
  std::ostringstream oss;
  oss << call->key << KEY_DELIMITER << call->message << CALL_DELIMITER;
  return oss.str();
}

inline std::tuple<std::string, std::string, mrpc_status>
ParseMessage(const std::string &input) {
  size_t key_delim_pos = input.find(KEY_DELIMITER);
  size_t call_delim_pos = input.find(CALL_DELIMITER);

  if (key_delim_pos == std::string::npos ||
      call_delim_pos == std::string::npos || call_delim_pos <= key_delim_pos) {
    return {"", "", MRPC_PARSE_FAILURE};
  }

  return {input.substr(0, key_delim_pos),
          input.substr(key_delim_pos + KEY_DELIMITER.length(),
                       call_delim_pos - key_delim_pos - KEY_DELIMITER.length()),
          MRPC_OK};
}

inline std::string ExtractFunc(const std::string& key) {
    size_t pos = key.find('-');
    if (pos != std::string::npos) {
        return key.substr(0, pos);
    }
    return "";
}