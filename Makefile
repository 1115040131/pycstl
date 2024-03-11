# PATH 环境变量在不同的终端会话中有所不同
# 明确设置所需的 PATH  防止 bazel 重复编译
export PATH=/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# 设置默认的测试过滤器为空，即不进行过滤
TEST_FILTER ?= *

######################### build for all #########################
.PHONY: all

all:
	bazel build //...

all_test:
	bazel test //... --test_filter="$(TEST_FILTER)"

######################### build for common #########################
.PHONY: common

common:
	bazel build //common --test_filter="$(TEST_FILTER)"

######################### build for concurrency #########################
.PHONY: concurrency

concurrency:
	bazel build //concurrency //concurrency/test/...
concurrency_test:
	bazel test //concurrency/test:concurrency_all_test --test_output=all --test_filter="$(TEST_FILTER)"
concurrency_valgrind: concurrency_test
	valgrind --leak-check=full --track-origins=yes \
	./bazel-bin/concurrency/test/concurrency_all_test --gtest_filter='ThreadSafeAdaptorTest.*'

######################### build for network #########################
.PHONY: network

network:
	bazel build //network //network/example/... //network/test/...
network_test: network
	bazel test //network/test:network_all_test --test_output=all --test_filter="$(TEST_FILTER)"

######################### build for pycstl #########################
.PHONY: pycstl

pycstl:
	bazel build //pycstl //pycstl/test/...
pycstl_test: pycstl
	bazel test //pycstl/test:pycstl_all_test --test_output=all --test_filter="$(TEST_FILTER)"

# 测试文件, 单独编译
######################### build for hello_world #########################
.PHONY: hello_world

hello_world:
	bazel run //hello_world