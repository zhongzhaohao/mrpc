load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mrpc-cpp-frontend",
    srcs = [
        "client.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:rpc_client",
        "//:mrpcpp_hearders",
        "@nlohmann_json//:json",
    ],
)