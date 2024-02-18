# PATH 环境变量在不同的终端会话中有所不同
# 明确设置所需的 PATH  防止 bazel 重复编译
export PATH=/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

######################### build for all #########################
.PHONY: all

all:
	bazel build //...

all_test:
	bazel test //...

######################### build for common #########################
.PHONY: common

common:
	bazel build //common

######################### build for concurrency #########################
.PHONY: concurrency

concurrency:
	bazel build //concurrency
concurrency_test:
	bazel test //concurrency/test:concurrency_all_test --test_output=all

######################### build for network #########################
.PHONY: network

network:
	bazel build //network //network/example/... //network/test/...
network_test: network
	bazel test //network/test:network_all_test --test_output=all

######################### build for pycstl #########################
.PHONY: pycstl

pycstl:
	bazel build //pycstl //pycstl/test/...
pycstl_test: pycstl
	bazel test //pycstl/test:pycstl_all_test --test_output=all