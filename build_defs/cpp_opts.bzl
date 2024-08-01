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
        "-Wold-style-cast",
        "-Wreturn-type",
        "-Wnon-virtual-dtor",
    ],
})
