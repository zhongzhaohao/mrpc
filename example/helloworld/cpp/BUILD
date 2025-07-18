load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "hello_handler",
    srcs = ["hello_handler.cc"],
    hdrs = [
        "hello_handler.h",
        "helloworld.mrpc.h",
    ],
    deps = [
        "//:mrpc-cpp-server",
        "@nlohmann_json//:json",
    ],
)

cc_binary(
    name = "greeter_server",
    srcs = [
        "greeter_server.cc",
        "helloworld.mrpc.h",
    ],
    deps = [
        ":hello_handler",
        "//:mrpc-cpp-server",
        "@nlohmann_json//:json",
    ],
)

cc_binary(
    name = "greeter_client",
    srcs = [
        "greeter_client.cc",
        "helloworld.mrpc.h",
    ],
    deps = [
        "//:mrpc-cpp",
    ],
)
