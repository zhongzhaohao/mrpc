load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@mrpc//scripts:mrpc_gen.bzl","mrpc_gen")

mrpc_gen(
    name = "helloworld_stub",
    src = "//:helloworld.yaml",
)

cc_binary(
    name = "greeter_server",
    srcs = [
        "greeter_server.cc",
    ],
    deps = [
        "@mrpc//:mrpc-cpp",
        ":helloworld_stub"
    ],
    includes = ["."],
)

cc_binary(
    name = "greeter_client",
    srcs = [
        "greeter_client.cc",
    ],
    deps = [
        "@mrpc//:mrpc-cpp",
        "helloworld_stub",
    ],
    includes = ["."],
)
