load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mrpc-cpp-frontend",
    srcs = [
        "client.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//include:mrpcpp_hearders",
        "//src/core:rpc_client",
    ],
)

cc_library(
    name = "mrpc-cpp-server-frontend",
    srcs = [
        "server.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//include:mrpcpp_hearders",
        "//src/core:rpc_server",
    ],
)
