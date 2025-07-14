#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef const char cchar_t;

typedef enum mrpc_status {
  MRPC_OK,
  MRPC_CANCELED,
  MRPC_FAILURE,
} mrpc_status;

typedef void (*response_handler)(cchar_t *key, cchar_t *response);

typedef struct mrpc_call {
  const char *key;
  const char *message;
  response_handler handler;
} mrpc_call;

typedef struct mrpc_client mrpc_client;

mrpc_client *mrpc_create_client(const char *addr);

void mrpc_destroy_client(mrpc_client *client);

mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call);

mrpc_status mrpc_receive_response(mrpc_client *client, mrpc_call *call);

int mrpc_get_unique_id(const char *func, char *buf);

#ifdef __cplusplus
}
#endif
