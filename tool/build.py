#!/usr/bin/env python3

import subprocess
import os
import shlex
import sys

from pathlib import Path
from cmd_utils import run_cmd, run_tmux, run_docker
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

# 设置PATH环境变量以防止bazel重复编译
os.environ['PATH'] = '/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'


def run_bazel_build(target, check=False, args=[]):
    command = f'bazel build {target} {" ".join(args)}'
    logger.info(command)
    subprocess.run(shlex.split(command), check=check)


def run_bazel_test(target, test_output=True, check=False, args=[]):
    command = f'bazel test {target} {" ".join(args)}'
    if test_output:
        command += ' --test_output=all'
    logger.info(command)
    subprocess.run(shlex.split(command), check=check)


def run_bazel_run(target, check=False, args=[]):
    command = f'bazel run {target} {" ".join(args)}'
    logger.info(command)
    subprocess.run(shlex.split(command), check=check)


def run_valgrind(target, args=[]):
    command = f'valgrind --leak-check=full --track-origins=yes {target} {" ".join(args)}'
    if len(args) == 0:
        command += '--gtest_filter=ThreadSafeAdaptorTest.*:ThreadSafeHashTableTest.*:ThreadSafeListTest.*'
    logger.info(command)
    subprocess.run(shlex.split(command))


def main():
    if len(sys.argv) < 2:
        logger.error("Usage: ./make [target] [...args]")
        return

    # 第一个参数是目标名称，其余的是传递给bazel命令的参数
    target = sys.argv[1]
    additional_args = sys.argv[2:]

    root_path = Path(__file__).resolve().parent.parent
    tool_path = root_path / 'tool'  # tool 目录

    targets = {
        ######################### basic command #########################
        "build": lambda args: (
            logger.error("Please give target name") if len(args) < 1 else
            run_bazel_build(args[0], args=args[1:])
        ),
        "run": lambda args: (
            logger.error("Please give target name") if len(args) < 1 else
            run_bazel_run(args[0], args=args[1:])
        ),
        "test": lambda args: (
            logger.error("Please give target name") if len(args) < 1 else
            run_bazel_test(args[0], args=args[1:])
        ),

        ######################### build for all #########################
        "all": lambda args: run_bazel_build('//...', args=args),
        "all_test": lambda args: run_bazel_test('//...', test_output=False, args=args),

        ######################### build for chat #########################
        "chat": lambda args: run_bazel_build('//chat/...', args),
        "chat_client": lambda args: run_bazel_run('//chat/client', args=args),
        "chat_gate_server": lambda args: run_bazel_run('//chat/server/gate_server', args=args),
        "chat_gate_server_test": lambda args: run_bazel_test('//chat/server/gate_server/test:gate_server_test',
                                                             args=args),
        "chat_redis_server": lambda args: run_docker(
            'llfc-redis',
            args=shlex.split('-p 6380:6379 redis --requirepass "123456"')
        ),
        "chat_verify_server": lambda args: run_bazel_run('//chat/server/verify_server', args=args),
        "chat_run": lambda args: (
            run_bazel_build('//chat/...', args),
            run_tmux(f"python3 {tool_path / 'build.py'} chat_gate_server",
                     f"python3 {tool_path / 'build.py'} chat_verify_server",
                     f"python3 {tool_path / 'build.py'} chat_client")
        ),

        ######################### build for common #########################
        "common": lambda args: run_bazel_build('//common', args),
        "common_test": lambda args: run_bazel_test('//common/test:common_all_test', args=args),

        ######################### build for co_async #########################
        "co_async": lambda args: run_bazel_build('//co_async //co_async/example/... //co_async/test/...',
                                                 args=args),
        "co_async_test": lambda args: run_bazel_test('//co_async/test:co_async_all_test', args=args),

        ######################### build for concurrency #########################
        "concurrency": lambda args: run_bazel_build('//concurrency //concurrency/test/...', args=args),
        "concurrency_test": lambda args: run_bazel_test('//concurrency/test:concurrency_all_test', args=args),
        "concurrency_valgrind": lambda args: run_valgrind('./bazel-bin/concurrency/test/concurrency_all_test',
                                                          args=args),

        ######################### build for cpp20_stl #########################
        "cpp20_stl": lambda args: run_bazel_build('//cpp20_stl:cpp20_stl_all_test', args=args),
        "cpp20_stl_test": lambda args: run_bazel_test('//cpp20_stl:cpp20_stl_all_test', args=args),

        ######################### build for design_pattern #########################
        "design_pattern": lambda args: run_bazel_build('//design_pattern:design_pattern_test', args=args),
        "design_pattern_test": lambda args: run_bazel_test('//design_pattern:design_pattern_test', args=args),

        ######################### build for logger #########################
        "logger": lambda args: run_bazel_build('//logger //logger/test/...', args=args),
        "logger_test": lambda args: run_bazel_test('//logger/test:logger_all_test', args=args),

        ######################### build for network #########################
        "network": lambda args: run_bazel_build('//network //network/example/... //network/test/...', args=args),
        "network_run": lambda args: (
            logger.error("Please give config name") if len(args) < 1 else
            run_cmd(f'python3 {tool_path / "start_server.py"} {args[0]}')
        ),
        "network_test": lambda args: run_bazel_test('//network/test:network_all_test', args=args),

        ######################### build for pycstl #########################
        "pycstl": lambda args: run_bazel_build('//pycstl //pycstl/test/...', args=args),
        "pycstl_test": lambda args: run_bazel_test('//pycstl/test:pycstl_all_test', args=args),

        ######################### build for tetris #########################
        "tetris": lambda args: run_bazel_build('//tetris', args=args),
        "tetris_run": lambda args: run_bazel_run('//tetris', args=args),

        ######################### build for tiny_db #########################
        "tiny_db": lambda args: run_bazel_build('//tiny_db', args=args),
        "tiny_db_run": lambda args: run_bazel_run('//tiny_db', args=args),
        "tiny_db_test": lambda args: run_bazel_test('//tiny_db/test:db_test', args=args),

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
