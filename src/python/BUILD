load("@bazel_skylib//rules:copy_file.bzl", "copy_file")
load("@rules_python//python:py_library.bzl", "py_library")

copy_file(
    name = "copy-librpc_client",
    src = "//src/core:librpc_client.so",
    out = "librpc_client.so",
)

py_library(
    name = "mrpc-python-frontend",
    srcs = glob(["mrpc/*.py"]),
    data = [
        "librpc_client.so",
    ],
    imports = ["."],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:librpc_client.so",
    ],
)
