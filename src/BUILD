load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mrpc_core",
    srcs = ["mrpc.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":rpc_client",
        "//:mrpc_headers",
    ],
)

cc_library(
    name = "rpc_client",
    srcs = [
        "client/client.cc",
    ],
    hdrs = [
        "client/client.h",
        "client/target.h",
    ],
    visibility = ["//visibility:__pkg__"],
    deps = [
        ":client_channel",
        ":common",
    ],
)

cc_library(
    name = "common",
    srcs = glob(["common/*.cc"]),
    hdrs = glob(["common/*.h"]),
    visibility = ["//visibility:__pkg__"],
    deps = [
        "@boost.asio//:boost.asio",
        "@nlohmann_json//:json",
    ],
)

cc_library(
    name = "client_channel",
    srcs = [
        "client/channel.cc",
    ],
    hdrs = [
        "client/channel.h",
        "client/target.h",
    ],
    visibility = ["//visibility:__pkg__"],
    deps = [
        ":common",
        "//:mrpc_headers",
    ],
)
