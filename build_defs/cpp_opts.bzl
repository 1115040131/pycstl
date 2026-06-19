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
    "@rules_cc//cc/compiler:gcc": [
    ],
    "@rules_cc//cc/compiler:clang": [
        "-fexperimental-library",
    ],
    "//conditions:default": [],
})
