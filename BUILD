load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@rules_go//go:def.bzl", "go_library")
load("@rules_python//python:py_library.bzl", "py_library")

# bazel run :refresh_compile_commands
refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_external_sources = True,
    exclude_headers = "all",

    # Specify the targets of interest.
    # For example, specify a dict of targets and any flags required to build.
    targets = [
        "//src/core/...",
        "//src/cpp/...",
        "//example/...",
    ],
)

cc_binary(
    name = "test_yaml",
    srcs = ["test/parser.cc"],
    deps = [
        "@yaml-cpp//:yaml-cpp",
    ],
)

cc_library(
    name = "mrpc-cpp",
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/cpp:mrpc-cpp-frontend",
    ],
)

# 这里要用嵌入，不能依赖
go_library(
    name = "mrpc-go",
    embed = ["//src/go:mrpc-go-frontend"],
    importpath = "mrpc",
    visibility = ["//visibility:public"],
    # deps = [
    #     "//src/go:mrpc-go-frontend",
    # ],
)

py_library(
    name = "mrpc-py",
    visibility = ["//visibility:public"],
    deps = [
        "//src/python:mrpc-python-frontend",
    ],
)