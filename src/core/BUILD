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
    ],
)

cc_library(
    name = "common",
    srcs = ["common/id.cc"],
    hdrs = ["common/c_cpp_trans.h"],
    visibility = ["//visibility:__pkg__"],
    deps = ["//:mrpc_core_headers"],
)
