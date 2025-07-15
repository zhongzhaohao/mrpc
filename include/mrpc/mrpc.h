#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MRPC_API

typedef const char cchar_t;

typedef enum mrpc_status {
  MRPC_OK,
  MRPC_CANCELED,
  MRPC_SEND_FAILURE,
  MRPC_PARSE_FAILURE,
} mrpc_status;

typedef void (*response_handler)(cchar_t *key, cchar_t *response,
                                 mrpc_status status);

typedef struct mrpc_call {
  const char *key;
  const char *message;
  response_handler handler;
} mrpc_call;

typedef struct mrpc_client mrpc_client;

MRPC_API mrpc_client *mrpc_create_client(const char *addr);

MRPC_API void mrpc_destroy_client(mrpc_client *client);

// 不要认为发送是同步进行发送，首次调用 mrpc_send_request
// 必定是异步发送，因为没有完成连接
MRPC_API mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call);

MRPC_API int mrpc_get_unique_id(const char *func, char *buf);

#ifdef __cplusplus
}
#endif
