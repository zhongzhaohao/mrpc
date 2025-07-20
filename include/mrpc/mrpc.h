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
  MRPC_CONNECTION_NOT_FOUND,
} mrpc_status;

typedef struct mrpc_call {
  const char *key;
  const char *message;
} mrpc_call;

// client
typedef void (*response_handler)(cchar_t *key, cchar_t *response,
                                 mrpc_status status);

typedef struct mrpc_client mrpc_client;

MRPC_API mrpc_client *mrpc_create_client(cchar_t *addr,
                                         response_handler handler);

MRPC_API void mrpc_destroy_client(mrpc_client *client);

MRPC_API mrpc_status mrpc_send_request(mrpc_client *client, mrpc_call *call);

MRPC_API int mrpc_get_unique_id(cchar_t *func, char *buf);

// server
typedef void (*request_handler)(cchar_t *method, cchar_t *key, cchar_t *request,
                                cchar_t *source);

typedef struct mrpc_server mrpc_server;

MRPC_API mrpc_server *mrpc_create_server(cchar_t *addr,
                                         request_handler handler);

MRPC_API mrpc_status mrpc_start_server(mrpc_server *server);

MRPC_API void mrpc_destroy_server(mrpc_server *server);

MRPC_API mrpc_status mrpc_send_reponse(mrpc_server *server, mrpc_call *call,
                                       cchar_t *source);

#ifdef __cplusplus
}
#endif
