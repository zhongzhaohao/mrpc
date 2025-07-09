load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name="rpc_client",
    srcs=[
        "client/client.cc",
        "client/client.h",
    ],
    deps=[
        "@nlohmann_json//:json",
        "@boost.asio//:boost.asio",
        ":common",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name="common",
    srcs = glob(["common/*.cc"]),
    hdrs = glob(["common/*.h"]),
    visibility = ["//visibility:public"],
)