load("@bazel_skylib//rules:copy_file.bzl", "copy_file")
load("@rules_python//python:py_library.bzl", "py_library")

copy_file(
    name = "copy-librpc_client",
    src = "//src/core:librpc_client.so",
    out = "librpc_client.so",
)

copy_file(
    name = "copy-librpc_server",
    src = "//src/core:librpc_server.so",
    out = "librpc_server.so",
)

py_library(
    name = "mrpc-python-frontend",
    srcs = glob(["mrpc/*.py"]),
    data = [
        "librpc_client.so",
        "librpc_server.so"
    ],
    imports = ["."],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:librpc_client.so",
        "//src/core:librpc_server.so"
    ],
)