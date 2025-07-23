load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_go//go:def.bzl", "go_library")
load("@rules_python//python:py_library.bzl", "py_library")

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
