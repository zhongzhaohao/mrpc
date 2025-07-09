load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

cc_binary(
    name = "greeter_server",
    srcs = ["greeter_server.cc"],
    deps = [
        "@nlohmann_json//:json",
        "@boost.asio//:boost.asio",
    ],
)

cc_binary(
    name = "greeter_client",
    srcs = ["greeter_client.cc","helloworld.mrpc.h"],
    deps = [
        "@nlohmann_json//:json",
        "@boost.asio//:boost.asio",
        "//:mrpc",
    ],
)
