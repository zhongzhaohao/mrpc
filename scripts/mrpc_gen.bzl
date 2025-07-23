""" 
generate stub file
"""

def mrpc_gen(name, src):
    """ 
    generate .mrpc.h

    Args:
        name: just for genrule
        src: filename
    """
    if ":" in src:
        f = src.split(":")[-1]  # 取冒号后部分
    else:
        f = src

    proto = f.split(".")[0]

    native.genrule(
        name = name + "_mrpc_gen",
        srcs = [src],
        outs = [proto + ".mrpc.h"],
        cmd = "$(location @mrpc//src/gen:cpp_gen) $< > $@",
        tools = ["@mrpc//src/gen:cpp_gen"],
        visibility = ["//visibility:public"],
    )

    native.genrule(
        name = name + "_empty_cc_gen",
        outs = [proto + ".mrpc.cc"],
        cmd = "echo '// Empty implementation file for " + proto + "' > $@",
        visibility = ["//visibility:public"],
    )

    native.cc_library(
        name = name,
        hdrs = [proto + ".mrpc.h"],
        srcs = [proto + ".mrpc.cc"],
        deps = ["@mrpc//:mrpc-cpp"],
        # alwayslink = True,
        visibility = ["//visibility:public"],
    )
