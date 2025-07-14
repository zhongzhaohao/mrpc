load("@rules_python//python:py_binary.bzl", "py_binary")
load("@rules_python//python:py_library.bzl", "py_library")


py_binary(
    name = "greeter_client",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = [
        "helloworld_mrpc",
    ],
)

py_library(
    name = "helloworld_mrpc",
    srcs = glob(["helloworld_mrpc/*.py"]),
    visibility = ["//visibility:__pkg__"],
    deps = [
        "//:mrpc-py",
    ],
)
