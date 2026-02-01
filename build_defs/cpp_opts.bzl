"""C++ compile options"""

COPTS = select({
    "//conditions:default": [
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unknown-pragmas",
    ],
})

STRICT_COPTS = select({
    "//conditions:default": [
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unknown-pragmas",
        "-Wreturn-type",
        "-Wnon-virtual-dtor",
    ],
}) + select({
    "@bazel_tools//tools/cpp:gcc": [
        "-Wno-missing-requires",
    ],
    "@bazel_tools//tools/cpp:clang": [
        "-fexperimental-library",
    ],
    "//conditions:default": [],
})
