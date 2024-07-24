"""C++ compile options"""

COPTS = select({
    "//conditions:default": [
        "-std=c++23",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unknown-pragmas",
    ],
})

STRICT_COPTS = select({
    "//conditions:default": [
        "-std=c++23",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unknown-pragmas",
        "-Weffc++",
        "-Wold-style-cast",
        "-Wreturn-type",
        "-Wnon-virtual-dtor",
    ],
})
