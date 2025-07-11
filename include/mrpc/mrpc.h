#pragma once

#include "status.h"
#include "call.h"


extern "C" {

typedef struct mrpc_client mrpc_client;

mrpc_client *mrpc_create_client(const char *addr);

void mrpc_destroy_client(mrpc_client *client);

mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call);

}
