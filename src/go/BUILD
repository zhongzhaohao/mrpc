load("@rules_go//go:def.bzl", "go_library")

go_library(
    name = "mrpc-go-frontend",
    srcs = glob(["*.go"]),
    cdeps = [
        "//src/core:rpc_client",
        "//src/core:rpc_server",
    ],
    cgo = True,
    copts = [
        "-I../../include",
    ],
    importpath = "mrpc",
    visibility = ["//visibility:public"],
)
