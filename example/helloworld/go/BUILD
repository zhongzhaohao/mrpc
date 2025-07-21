load("@rules_go//go:def.bzl", "go_binary", "go_library")

go_binary(
    name = "greeter_client",
    srcs = glob(["*.go"]),
    deps = [
        "helloworld_RPC",
    ],
)

go_library(
    name = "helloworld_RPC",
    srcs = glob(["helloworld/*.go"]),
    importpath = "mrpc/example/helloworld/go/helloworld",
    deps = [
        "//:mrpc-go",
    ],
)
