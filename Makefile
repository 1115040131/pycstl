# PATH 环境变量在不同的终端会话中有所不同
# 明确设置所需的 PATH  防止 bazel 重复编译
export PATH=/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

######################### build for all #########################
.PHONY: all

all:
	bazel build //...

all_test:
ifdef TEST_FILTER
	bazel test //... --test_filter="$(TEST_FILTER)"
else
	bazel test //...
endif

######################### build for common #########################
.PHONY: common

common:
	bazel build //common

######################### build for concurrency #########################
.PHONY: concurrency

concurrency:
	bazel build //concurrency //concurrency/test/...

concurrency_test:
ifdef TEST_FILTER
	bazel test //concurrency/test:concurrency_all_test --test_output=all --test_filter="$(TEST_FILTER)"
else
	bazel test //concurrency/test:concurrency_all_test --test_output=all
endif

concurrency_valgrind: concurrency
ifdef TEST_FILTER
	valgrind --leak-check=full --track-origins=yes ./bazel-bin/concurrency/test/concurrency_all_test \
	--gtest_filter="$(TEST_FILTER)"
else
	valgrind --leak-check=full --track-origins=yes ./bazel-bin/concurrency/test/concurrency_all_test \
	--gtest_filter='ThreadSafeAdaptorTest.*:ThreadSafeHashTableTest.*:ThreadSafeListTest.*'
endif

######################### build for logger #########################
.PHONY: logger

logger:
	bazel build //logger //logger/test/...

logger_test:
ifdef TEST_FILTER
	bazel test //logger/test:logger_all_test --test_output=all --test_filter="$(TEST_FILTER)"
else
	bazel test //logger/test:logger_all_test --test_output=all
endif

######################### build for network #########################
.PHONY: network

network:
	bazel build //network //network/example/... //network/test/...

network_test:
ifdef TEST_FILTER
	bazel test //network/test:network_all_test --test_output=all --test_filter="$(TEST_FILTER)"
else
	bazel test //network/test:network_all_test --test_output=all
endif

######################### build for pycstl #########################
.PHONY: pycstl

pycstl:
	bazel build //pycstl //pycstl/test/...

pycstl_test:
ifdef TEST_FILTER
	bazel test //pycstl/test:pycstl_all_test --test_output=all --test_filter="$(TEST_FILTER)"
else
	bazel test //pycstl/test:pycstl_all_test --test_output=all
endif

######################### build for tetris #########################
.PHONY: tetris

tetris:
	bazel run //tetris

######################### build for tiny_db #########################
.PHONY: tiny_db

tiny_db:
	bazel build //tiny_db

tiny_db_run:
	bazel run //tiny_db

tiny_db_test:
	bazel run //tiny_db/test:db_test

# 测试文件, 单独编译
######################### build for hello_world #########################
.PHONY: hello_world

hello_world:
	bazel run //hello_world