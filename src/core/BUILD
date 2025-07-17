load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "librpc_client.so",
    srcs = [":rpc_client"],
    linkopts = ["-Wl,--export-dynamic"],
    linkshared = True,
    # linkstatic = True,
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":rpc_client",
    ],
)

cc_library(
    name = "rpc_client",
    srcs = [
        "client/client.cc",
    ],
    hdrs = [
        "client/client.h",
        "client/target.h",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        ":client_channel",
    ],
    alwayslink = True,
)

cc_library(
    name = "client_channel",
    srcs = [
        "client/channel.cc",
    ],
    hdrs = [
        "client/channel.h",
        "client/target.h",
    ],
    visibility = ["//visibility:__pkg__"],
    deps = [
        ":common",
        "@boost.asio//:boost.asio",
        # "@python3_12//:python_headers",
    ],
    # alwayslink = True,
)

cc_library(
    name = "common",
    srcs = ["common/id.cc"],
    hdrs = ["common/c_cpp_trans.h"],
    visibility = ["//visibility:__pkg__"],
    deps = [
        "//include:mrpc_core_headers",
    ],
    alwayslink = True,
)

# Server libraries
cc_binary(
    name = "librpc_server.so",
    srcs = [":rpc_server"],
    linkopts = ["-Wl,--export-dynamic"],
    linkshared = True,
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":rpc_server",
    ],
)

cc_library(
    name = "rpc_server",
    srcs = [
        "server/server.cc",
    ],
    hdrs = [
        "server/server.h",
        "server/target.h",
    ],
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        ":server_connection",
    ],
    alwayslink = True,
)

cc_library(
    name = "server_connection",
    srcs = [
        "server/connection.cc",
    ],
    hdrs = [
        "server/connection.h",
        "server/target.h",
    ],
    visibility = ["//visibility:__pkg__"],
    deps = [
        ":common",
        "@boost.asio//:boost.asio",
    ],
)
