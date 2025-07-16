load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "mrpcpp_hearders",
    hdrs = glob(["mrpcpp/*.h"]),
    includes = ["."],
    visibility = ["//src/cpp:__pkg__"],
    deps = [
        "mrpc_core_headers",
        "@nlohmann_json//:json",
    ],
)

cc_library(
    name = "mrpc_core_headers",
    hdrs = glob(["mrpc/*.h"]),
    includes = ["."],
    visibility = ["//src:__subpackages__"],
)
