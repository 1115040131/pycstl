#!/usr/bin/env python3

import subprocess
import os
import shlex
import sys
import time

from pathlib import Path
from cmd_utils import *
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

    # chat 相关数据
    data_direction = f'{root_path}/chat/server/mysql/data'
    log_direction = f'{root_path}/chat/server/mysql/logs'

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
        "chat": lambda args: run_bazel_build('//chat/...', args=args),

        # chat server
        "chat_redis_server": lambda args: run_docker(
            image='redis --requirepass "123456"',
            container_name='pyc-redis',
            args=['-p 6380:6379']
        ),
        "chat_mysql_server": lambda args: (
            # 创建目录并设置访问权限
            setup_directory(data_direction),
            setup_directory(log_direction),

            # 运行容器
            run_docker(
                image='mysql:8.0',
                container_name='pyc-mysql',
                args=[f'-v {root_path}/chat/server/mysql/config/my.cnf:/etc/my.cnf',
                      f'-v {data_direction}:/var/lib/mysql',
                      f'-v {log_direction}:/logs',
                      '--restart=on-failure:3 -p 6306:3306 -p 33060:33060 -e MYSQL_ROOT_PASSWORD=123456']
            )
        ),
        "chat_gate_server": lambda args: run_bazel_run('//chat/server/gate_server', args=args),
        "chat_verify_server": lambda args: run_bazel_run('//chat/server/verify_server', args=args),
        "chat_prepare": lambda args: (
            targets["chat_redis_server"](args=[]),
            targets["chat_mysql_server"](args=[]),
            wait_until(lambda: mysql_service_is_ready('root', '123456', 'localhost', 6306),
                       'mysql_service_is_ready', interval=1),
        ),
        "chat_clear": lambda args: (
            run_cmd('docker stop pyc-mysql', check=False),
            run_cmd('docker rm -v pyc-mysql', check=False),
            run_cmd(f'sudo rm -rf {data_direction}'),
            run_cmd(f'sudo rm -rf {log_direction}'),
        ),

        # chat server test
        "chat_server_common_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/common/test:common_test', args=args),
        ),
        "chat_gate_server_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/gate_server/test:gate_server_test', args=args)
        ),

        # chat client
        "chat_client": lambda args: run_bazel_run('//chat/client', args=args),

        # chat run
        "chat_run": lambda args: (
            run_bazel_build('//chat/...', args),
            targets["chat_prepare"](args=[]),
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
        "hello_world": lambda args: run_bazel_run('//hello_world', args=args),
    }

    if target not in targets:
        print(f"Unknown target: {target}")
        print("Available targets: [", ', '.join(targets.keys()), "]")
        return

    # 执行对应目标的函数
    targets[target](additional_args)


if __name__ == "__main__":
    main()
