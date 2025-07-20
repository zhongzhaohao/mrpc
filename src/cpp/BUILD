load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mrpc-cpp-frontend",
    srcs = [
        "client.cc",
        "server.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//include:mrpcpp_hearders",
        "//src/core:rpc_client",
        "//src/core:rpc_server",
    ],
)