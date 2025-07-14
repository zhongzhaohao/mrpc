load("@rules_go//go:def.bzl", "go_binary")

go_binary(
    name = "greeter_client",
    srcs = glob(["*.go"]),
    deps = [
        "//:mrpc-go",
    ]
)