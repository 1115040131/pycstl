# 定义默认目标，这里没有定义具体行为，因为我们希望用户显式调用 'make network'
all:
	@echo "Please specify a target to make, e.g., make network"

.PHONY: common
common:
	bazel build //common

.PHONY: network
network:
	bazel build //network
	bazel build //network/...

.PHONY: pycstl
pycstl:
	bazel build //pycstl
	bazel build //pycstl/...