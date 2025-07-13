#pragma once

#include <functional>

extern "C" {
typedef void (*response_handler)(const char* response,void *ctx);

typedef struct mrpc_call {
  const char *key;
  const char *message;
  response_handler handler;
  void *ctx;
} mrpc_call;
}

struct CallContext {
  std::function<void(const char*)> callback;
};