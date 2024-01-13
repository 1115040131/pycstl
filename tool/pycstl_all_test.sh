#!/bin/bash

if [ $# -eq 1 ]; then
    bazel run //pycstl/test:pycstl_all_test --test_arg=--gtest_filter=$1
else
    bazel run //pycstl/test:pycstl_all_test
fi
