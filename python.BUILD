load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "python_headers",
    hdrs = glob(["*.h","cpython/*.h","internal/*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
    alwayslink = True,
)
