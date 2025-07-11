load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "rpc_client",
    srcs = [
        "client/client.cc",
    ],
    hdrs = [
        "client/client.h",
        "client/target.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":client_channel",
        ":common",
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
        "@boost.asio//:boost.asio",
        "//:mrpc_core_headers",
    ],
)

cc_library(
    name = "common",
    hdrs = ["common/c_cpp_trans.h"],
    visibility = ["//visibility:__pkg__"],
)
