#!/usr/bin/env python3

import subprocess
import os
import shlex
import sys
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

# 设置PATH环境变量以防止bazel重复编译
os.environ['PATH'] = '/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'


def run_bazel_build(target, args=[]):
    command = f'bazel build {target} {' '.join(args)}'
    logger.info(command)
    subprocess.run(shlex.split(command))


def run_bazel_test(target, test_output=False, args=[]):
    command = f'bazel test {target} {' '.join(args)}'
    if test_output:
        command += ' --test_output=all'
    logger.info(command)
    subprocess.run(shlex.split(command))


def run_bazel_run(target, args=[]):
    command = f'bazel run {target} {' '.join(args)}'
    logger.info(command)
    subprocess.run(shlex.split(command))



def run_valgrind(target, args=[]):
    command = f'valgrind --leak-check=full --track-origins=yes {target} {' '.join(args)}'
    if len(args) == 0:
        command += '--gtest_filter=ThreadSafeAdaptorTest.*:ThreadSafeHashTableTest.*:ThreadSafeListTest.*'
    logger.info(command)
    subprocess.run(shlex.split(command))


def main():
    if len(sys.argv) < 2:
        logger.error("Usage: ./build.py [target] [...args]")
        return

    # 第一个参数是目标名称，其余的是传递给bazel命令的参数
    target = sys.argv[1]
    additional_args = sys.argv[2:]

    targets = {
        ######################### build for all #########################
        "all": lambda args: run_bazel_build('//...', args),
        "all_test": lambda args: run_bazel_test('//...', args),

        ######################### build for common #########################
        "common": lambda args: run_bazel_build('//common', args),

        ######################### build for concurrency #########################
        "concurrency": lambda args: run_bazel_build('//concurrency //concurrency/test/...', args),
        "concurrency_test": lambda args: run_bazel_test('//concurrency/test:concurrency_all_test', True, args),
        "concurrency_valgrind": lambda args: run_valgrind('./bazel-bin/concurrency/test/concurrency_all_test', args),

        ######################### build for logger #########################
        "logger": lambda args: run_bazel_build('//logger //logger/test/...', args),
        "logger_test": lambda args: run_bazel_test('//logger/test:logger_all_test', True, args),

        ######################### build for network #########################
        "network": lambda args: run_bazel_build('//network //network/example/... //network/test/...', args),
        "network_test": lambda args: run_bazel_test('//network/test:network_all_test', True, args),

        ######################### build for pycstl #########################
        "pycstl": lambda args: run_bazel_build('//pycstl //pycstl/test/...', args),
        "pycstl_test": lambda args: run_bazel_test('//pycstl/test:pycstl_all_test', True, args),

        ######################### build for tetris #########################
        "tetris": lambda args: run_bazel_build('//tetris', args),
        "tetris_run": lambda args: run_bazel_run('//tetris', args),

        ######################### build for tiny_db #########################
        "tiny_db": lambda args: run_bazel_build('//tiny_db', args),
        "tiny_db_run": lambda args: run_bazel_run('//tiny_db', args),
        "tiny_db_test": lambda args: run_bazel_test('//tiny_db/test:db_test', True, args),

        # 测试文件, 单独编译
        ######################### build for hello_world #########################
        "hello_world": lambda args: run_bazel_run('//hello_world', args),
    }

    if target not in targets:
        print(f"Unknown target: {target}")
        print("Available targets: [", ', '.join(targets.keys()), "]")
        return

    # 执行对应目标的函数
    targets[target](additional_args)


if __name__ == "__main__":
    main()
