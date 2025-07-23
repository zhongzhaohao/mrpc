load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "cpp_gen",
    srcs = ["cpp_stub_gen.cc"],
    deps = [
        "@yaml-cpp//:yaml-cpp",
    ],
    visibility = ["//visibility:public"],
)
