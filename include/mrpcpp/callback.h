#pragma once

#include "mrpc/mrpc.h"
#include <cassert>
#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace mrpc {

using Callback = std::function<void(cchar_t *, mrpc_status)>;

inline std::map<std::string, Callback> g_callbacks;
inline std::mutex g_callback_mutex;

extern "C" inline void GlobalRpcCallback(cchar_t *key, cchar_t *result,
                                         mrpc_status status) {
  Callback cb;

  {
    std::lock_guard<std::mutex> lock(g_callback_mutex);
    auto it = g_callbacks.find(key);
    assert(it != g_callbacks.end());
    cb = it->second;
    g_callbacks.erase(it);
  }

  if (cb) {
    cb(result, status);
  }
}

inline void RegisterRpcCallback(const std::string &key, Callback cb) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);
  g_callbacks[key] = std::move(cb);
}

} // namespace mrpc
