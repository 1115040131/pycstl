#!/usr/bin/env python3

import subprocess
import os
import shlex
import sys

from pathlib import Path
from cmd_utils import *
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

# 设置PATH环境变量以防止bazel重复编译
os.environ['PATH'] = '/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'


def run_bazel_build(target, check=False, args=[]):
    command = f'bazel build {target} {" ".join(args)}'
    run_cmd(command, check)


def run_bazel_test(target, test_output=True, check=False, args=[]):
    command = f'bazel test {target} {" ".join(args)}'
    if test_output:
        command += ' --test_output=all'
    run_cmd(command, check)


def run_bazel_run(target, check=False, args=[]):
    command = f'bazel run {target} {" ".join(args)}'
    run_cmd(command, check)


def run_valgrind(target, args=[]):
    command = f'valgrind --leak-check=full --track-origins=yes {target} {" ".join(args)}'
    if len(args) == 0:
        command += '--gtest_filter=ThreadSafeAdaptorTest.*:ThreadSafeHashTableTest.*:ThreadSafeListTest.*'
    run_cmd(command)

# 路径定义
root_path = Path(__file__).resolve().parent.parent
tool_path = root_path / 'tool'  # tool 目录

def chat_run(targets, args):
    run_bazel_build('//chat/...', args)
    targets["chat_prepare"](args=[])
    if len(args) > 0 and int(args[0]) > 1:
        client_num = int(args[0])
        run_tmux({
            "GateServer": [
                f"python3 {tool_path / 'build.py'} chat_gate_server",
                f"python3 {tool_path / 'build.py'} chat_verify_server",
                f"python3 {tool_path / 'build.py'} chat_status_server",
            ],
            "ChatServer": [
                f"python3 {tool_path / 'build.py'} chat_chat_server ChatServer1",
                f"python3 {tool_path / 'build.py'} chat_chat_server ChatServer2"
            ],
            "Client": [f"python3 {tool_path / 'build.py'} chat_client"] * client_num
        })
    else:
        run_tmux({
            "GateServer": [
                f"python3 {tool_path / 'build.py'} chat_gate_server",
                f"python3 {tool_path / 'build.py'} chat_verify_server",
                f"python3 {tool_path / 'build.py'} chat_status_server",
            ],
            "ChatServer": [
                f"python3 {tool_path / 'build.py'} chat_chat_server ChatServer1",
                f"python3 {tool_path / 'build.py'} chat_chat_server ChatServer2",
                f"python3 {tool_path / 'build.py'} chat_client"
            ]
        })

def main():
    if len(sys.argv) < 2:
        logger.error("Usage: ./make [target] [...args]")
        return

    # 第一个参数是目标名称，其余的是传递给bazel命令的参数
    target = sys.argv[1]
    additional_args = sys.argv[2:]

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
        "all": lambda args: run_bazel_build('//...', args=args + ['-- -//hello_world']),
        "all_test": lambda args: run_bazel_test('//...', test_output=False, args=args + ['-- -//hello_world']),

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
                      f'-v {root_path}/chat/server/mysql/sql/init-script.sql:/docker-entrypoint-initdb.d/init-script.sql',
                      f'-v {data_direction}:/var/lib/mysql',
                      f'-v {log_direction}:/logs',
                      '--restart=on-failure:3 -p 6306:3306 -p 33060:33060 -e MYSQL_ROOT_PASSWORD=123456']
            )
        ),
        "chat_gate_server": lambda args: run_bazel_run('//chat/server/gate_server', args=args),
        "chat_verify_server": lambda args: run_bazel_run('//chat/server/verify_server', args=args),
        "chat_status_server": lambda args: run_bazel_run('//chat/server/status_server', args=args),
        "chat_chat_server": lambda args: run_bazel_run('//chat/server/chat_server', args=args),
        "chat_prepare": lambda args: (
            targets["chat_redis_server"](args=[]),
            targets["chat_mysql_server"](args=[]),
            wait_until(lambda: mysql_service_is_ready('root', '123456', 'localhost', 6306),
                       'mysql_service_is_ready', interval=1),
        ),
        "chat_clear": lambda args: (
            run_cmd('docker stop pyc-redis', check=False),
            run_cmd('docker rm -v pyc-redis', check=False),
            run_cmd('docker stop pyc-mysql', check=False),
            run_cmd('docker rm -v pyc-mysql', check=False),
            run_cmd(f'sudo rm -rf {data_direction}'),
            run_cmd(f'sudo rm -rf {log_direction}'),
        ),

        # chat test
        "chat_server_common_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/common/test:common_test', args=args),
        ),
        "chat_gate_server_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/test:run_all_test --test_arg=-k --test_arg=GateServerTest', args=args)
        ),
        "chat_status_server_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/test:run_all_test --test_arg=-k --test_arg=StatusServerTest', args=args)
        ),
        "chat_chat_server_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/test:run_all_test --test_arg=-k --test_arg=ChatServerTest', args=args)
        ),
        "chat_server_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/server/test:run_all_test --test_arg=--log_dir --test_arg=logs', args=args)
        ),
        "chat_test": lambda args: (
            targets["chat_prepare"](args=[]),
            run_bazel_test('//chat/...', args=args),
        ),

        # chat client
        "chat_client": lambda args: run_bazel_run('//chat/client', args=args),

        # chat run
        "chat_run": lambda args: chat_run(targets, args),

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

        ######################### build for qt #########################
        "qt": lambda args: run_bazel_build('//qt/...', args=args),
        "99table": lambda args: run_bazel_run('//qt/99table', args=args),
        "calculator": lambda args: run_bazel_run('//qt/calculator', args=args),
        "notepad": lambda args: run_bazel_run('//qt/notepad', args=args),

        ######################### build for sdl2 #########################
        "sdl2_demo": lambda args: run_bazel_run('//sdl2/demo', args=args),
        "chicken_evil": lambda args: run_bazel_run('//sdl2/chicken_evil', args=args),
        "shooter": lambda args: run_bazel_run('//sdl2/shooter', args=args),

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

        # 更新 compile_commands.json 文件
        "refresh_all": lambda args: run_bazel_run('@hedron_compile_commands//:refresh_all', args=args),
        "refresh": lambda args: run_bazel_run('//:refresh_compile_commands', args=args),
    }

    if target not in targets:
        print(f"Unknown target: {target}")
        print("Available targets: [", ', '.join(targets.keys()), "]")
        return

    # 执行对应目标的函数
    targets[target](additional_args)


if __name__ == "__main__":
    main()
