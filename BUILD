load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

# bazel run :refresh_compile_commands
refresh_compile_commands(
    name = "refresh_compile_commands",
    exclude_external_sources = True,
    exclude_headers = "all",

    # Specify the targets of interest.
    # For example, specify a dict of targets and any flags required to build.
    targets = [
        "//...",
    ],
)

cc_library(
    name = "mrpc",
    hdrs = [
        "include/mrpc.h",
    ],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [
        "//:test_yaml",
        "//src:mrpc_core",
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
    name = "mrpc_headers",
    hdrs = ["include/mrpc.h"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)