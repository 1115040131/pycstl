"""rules_foreign_cc build options"""

# 传给 cmake --build 的并行度。这些 cmake() target 的 ninja 并行度与 Bazel 自己的
# --jobs 是叠加的: Bazel 可能同时跑好几个 cmake() action, 每个再各自开 -j8。
# ubuntu-latest 的 runner 只有 4 核, CI 上会超订; 集中成一个常量, 要调只改这里。
BUILD_ARGS = ["-j8"]
