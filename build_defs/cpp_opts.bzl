"""C++ compile options"""

COPTS = select({
    "//conditions:default": [
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wreturn-type",
        "-Wnon-virtual-dtor",
    ],
}) + select({
    "@bazel_tools//tools/cpp:gcc": [
    ],
    "@bazel_tools//tools/cpp:clang": [
        "-fexperimental-library",
    ],
    "//conditions:default": [],
})
