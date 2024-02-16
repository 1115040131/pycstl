# build for all
.PHONY: all all_test

all: common network pycstl

all_test: all
	bazel test //...

# build for common
.PHONY: common

common:
	bazel build //common

# build for network
.PHONY: network network_test

network:
	bazel build //network
	bazel build //network/...

network_test: network
	bazel test //network/test:network_all_test --test_output=all

#build for pycstl
.PHONY: pycstl pycstl_test

pycstl:
	bazel build //pycstl
	bazel build //pycstl/...
pycstl_test: pycstl
	bazel test //pycstl/test:pycstl_all_test --test_output=all