#pragma once

#include "src/common/json_utils.h"
#include "src/common/status.h"

typedef struct mrpc_client mrpc_client;

mrpc_client *mrpc_create_client(const std::string &addr);

void mrpc_destroy_client(mrpc_client *client);

mrpc::Status mrpc_sync_send_request(mrpc_client *client, std::string func,
                                    mrpc::ParseToJson &t,
                                    mrpc::ParseFromJson &f);