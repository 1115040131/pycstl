bazel clean --expunge

bazel build network --explain=explain.log --verbose_explanations

./make all -- -//tiny_db/test:db_test
./make all_test -- -//tiny_db/test:db_test