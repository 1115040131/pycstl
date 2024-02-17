"""C++ compile options"""

COPTS = select({
    "//conditions:default": [
        "-std=c++20",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unknown-pragmas",
    ],
})
