bazel clean --expunge

bazel build network --explain=explain.log --verbose_explanations

bazel run -- @pnpm --dir $PWD install --lockfile-only