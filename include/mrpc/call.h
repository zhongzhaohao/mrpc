#pragma once

extern "C" {
typedef struct mrpc_call {
  const char *key;
  const char *message;
} mrpc_call;
}