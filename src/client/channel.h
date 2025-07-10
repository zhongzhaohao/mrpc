#pragma once

#include "src/common/json_utils.h"
#include "src/client/target.h"
#include <boost/asio.hpp>

namespace mrpc {
using boost::asio::io_context;
using boost::asio::ip::tcp;

class Channel {
public:
  Channel(boost::asio::io_context &ctx, Target &target);

  ~Channel();

  void Send(ParseToJson &t);

  void Receive(ParseFromJson &f);

private:
  std::unique_ptr<tcp::socket> socket_;
};
} // namespace mrpc