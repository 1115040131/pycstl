cc_library(
    name = "hiredis",
    srcs = [
        "alloc.c",
        "async.c",
        "hiredis.c",
        "net.c",
        "read.c",
        "sds.c",
        "sockcompat.c",
    ],
    hdrs = glob(["*.h"]) + ["dict.c"],
    includes = ["."],
    visibility = ["//visibility:public"],
)
