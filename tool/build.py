#!/usr/bin/env python3

import argparse
import re
import shutil
import subprocess
import sys
import tempfile

from pathlib import Path
from typing import Any, Callable, NoReturn
from cmd_utils import mysql_service_is_ready, run_cmd, run_docker, run_tmux, setup_directory, wait_until
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

# 路径定义
root_path = Path(__file__).resolve().parent.parent
tool_path = root_path / 'tool'  # tool 目录


class BuildArgumentParser(argparse.ArgumentParser):
    """用法出错时给出完整帮助, 与 help 目标的提示样式保持一致"""

    def error(self, message: str) -> NoReturn:
        self.print_help(sys.stderr)
        self.exit(1, f'\n{self.prog}: error: {message}\n')


def parse_args(target_names: list[str]) -> tuple[str, list[str]]:
    """校验目标名与编译模式, 未识别的参数原样透传给 bazel"""
    parser = BuildArgumentParser(
        description='Project build entry, unrecognized args are passed through to bazel',
        epilog=f"available targets: {', '.join(target_names)}",
        # bazel 的长选项 (如 --define) 不应被缩写匹配到本脚本的选项上
        allow_abbrev=False,
    )
    # 目标名不用 choices 校验, 否则报错时会重复列出 epilog 里已有的目标清单
    parser.add_argument('target', metavar='target',
                        help="build target, or 'help' to list all targets")

    # 编译模式互斥, 展开为 bazel 的 --config
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('-d', dest='config', action='store_const', const='--config=dbg',
                      help='dbg mode (-O0 -g), for debugging')
    mode.add_argument('-o', dest='config', action='store_const', const='--config=opt',
                      help='opt mode (-O2), for benchmarks and releases')

    args, passthrough = parser.parse_known_args()

    if args.target == 'help':
        parser.print_help()
        parser.exit()

    if args.target not in target_names:
        parser.error(f'unknown target: {args.target}')

    # --config 追加在末尾, 避免占用 build/run/test 等入口的 args[0] (目标名)
    return args.target, passthrough + ([args.config] if args.config else [])


def run_bazel_build(target: str, check: bool = True, args: list[str] | None = None) -> None:
    command = f'bazel build {target} {" ".join(args or [])}'
    run_cmd(command, check=False)


def run_bazel_run(target: str, check: bool = False, args: list[str] | None = None) -> None:
    # 交互式运行, Ctrl+C 退出时返回非零码属正常情况, 因此不做 check
    command = f'bazel run {target} {" ".join(args or [])}'
    run_cmd(command, check=False)


def run_bazel_test(target: str, test_output: bool = True, check: bool = True,
                   args: list[str] | None = None) -> None:
    command = f'bazel test {target} {" ".join(args or [])}'
    if test_output:
        command += ' --test_output=all'
    run_cmd(command, check=False)


def run_bazel_coverage(target: str, check: bool = True, args: list[str] | None = None) -> None:
    command = (f'bazel coverage {target} --nocache_test_results '
               f'--instrumentation_filter="//..." {" ".join(args or [])}')
    run_cmd(command, check=False)


def get_lcov_major_version() -> int:
    try:
        output = subprocess.check_output(['lcov', '--version'], stderr=subprocess.STDOUT).decode()
        match = re.search(r'(\d+)\.', output)
        return int(match.group(1)) if match else 1
    except Exception:
        return 1


def generate_coverage_report() -> None:
    report_dat = root_path / 'bazel-out/_coverage/_coverage_report.dat'
    output_dir = root_path / 'coverage_report'

    if not report_dat.exists():
        logger.error(f"Coverage data not found: {report_dat}")
        logger.fatal("Run a coverage command first (e.g. all_coverage)")

    if shutil.which('genhtml') is None:
        logger.fatal("genhtml not found, install it first (e.g. sudo apt install lcov)")

    if get_lcov_major_version() >= 2:
        run_cmd(
            f'genhtml {report_dat} --output-directory {output_dir} '
            f"--exclude 'external/*' --exclude '/usr/*' "
            f'--ignore-errors source,unmapped,unused'
        )
    else:
        if shutil.which('lcov') is None:
            logger.fatal("lcov not found, install it first (e.g. sudo apt install lcov)")
        with tempfile.NamedTemporaryFile(suffix='.dat', delete=True) as tmp:
            filtered_dat = tmp.name
            run_cmd(
                f'lcov --remove {report_dat} '
                f"'external/*' '/usr/*' "
                f'--output-file {filtered_dat} --ignore-errors source'
            )
            run_cmd(f'genhtml {filtered_dat} --output-directory {output_dir} --ignore-errors source')

    logger.info(f"Coverage report generated: {output_dir}/index.html")


def run_valgrind(bazel_target: str, binary: str, args: list[str] | None = None) -> None:
    """valgrind 需要调试信息才能定位到源码行, 因此固定以 dbg 构建, 并取该配置下的产物"""
    run_bazel_build(bazel_target, args=['--config=dbg'])

    # 产物目录问 bazel 要, 不要硬编码 bazel-out/k8-dbg (k8 只是 x86_64 的平台名),
    # 也不能用 bazel-bin 软链 (它指向最后一次构建的配置, 未必是 dbg)
    info = subprocess.run(['bazel', 'info', 'bazel-bin', '--config=dbg'],
                          capture_output=True, text=True)
    if info.returncode != 0:
        logger.fatal("Failed to resolve bazel-bin for the dbg configuration")

    binary_path = Path(info.stdout.strip()) / binary
    if not binary_path.exists():
        logger.fatal(f"Binary not found: {binary_path} (dbg build may have failed)")

    # -d/-o 展开出的 --config 属于 bazel 选项, 不能传给测试二进制
    bin_args = [arg for arg in args or [] if not arg.startswith('--config=')]
    run_cmd(f'valgrind --leak-check=full --track-origins=yes {binary_path} {" ".join(bin_args)}')


def chat_run(targets: dict[str, Callable[[list[str]], Any]], args: list[str]) -> None:
    # 首个参数若为数字则表示客户端个数, 不能透传给 bazel
    client_num = 1
    bazel_args = args
    if len(args) > 0 and args[0].isdigit():
        client_num = int(args[0])
        bazel_args = args[1:]

    run_bazel_build('//chat/...', args=bazel_args)
    targets["chat_prepare"](args=[])

    make = f"python3 {tool_path / 'build.py'}"
    windows = {
        "GateServer": [
            f"{make} chat_gate_server",
            f"{make} chat_verify_server",
            f"{make} chat_status_server",
        ],
        "ChatServer": [
            f"{make} chat_chat_server ChatServer1",
            f"{make} chat_chat_server ChatServer2",
        ],
    }
    if client_num > 1:
        # 多个客户端时单独开一个窗口, 否则与 ChatServer 共用
        windows["Client"] = [f"{make} chat_client"] * client_num
    else:
        windows["ChatServer"].append(f"{make} chat_client")

    run_tmux(windows)


def check_venv() -> None:
    python = root_path / '.venv' / 'bin' / 'python'
    if not python.exists():
        logger.fatal('虚拟环境不存在，请先运行: make venv')

    req = root_path / 'requirements_venv.in'
    marker = root_path / '.venv' / '.deps_installed'
    if not marker.exists() or req.stat().st_mtime > marker.stat().st_mtime:
        logger.warn('requirements_venv.in 已更新，虚拟环境可能过期，建议运行: make venv')


def run_venv(command: str, args: list[str] | None = None) -> None:
    check_venv()
    bin_path = root_path / '.venv' / 'bin'
    run_cmd(f'{bin_path}/{command} {" ".join(args or [])}')


def venv(args: list[str]):
    script_path = tool_path / 'setup_venv.sh'

    # 必须使用 open() 打开文件，获得文件对象 f
    with open(script_path, 'r') as f:
        result = subprocess.run(["bash", "-s", "--", root_path, root_path / 'requirements_venv.in'], stdin=f)

    # 子进程无法激活父 shell 的虚拟环境，成功后提示用户手动 source
    if result.returncode == 0:
        logger.info(f'虚拟环境已就绪，请在当前 shell 运行以进入：source {root_path / ".venv" / "bin" / "activate"}')


def main() -> None:
    # chat 相关数据
    data_direction = f'{root_path}/chat/server/mysql/data'
    log_direction = f'{root_path}/chat/server/mysql/logs'

    targets: dict[str, Callable[[list[str]], Any]] = {
        ######################### basic command #########################
        "build": lambda args: (
            logger.fatal("Please give target name") if len(args) < 1 else
            run_bazel_build(args[0], args=args[1:])
        ),
        "run": lambda args: (
            logger.fatal("Please give target name") if len(args) < 1 else
            run_bazel_run(args[0], args=args[1:])
        ),
        "test": lambda args: (
            logger.fatal("Please give target name") if len(args) < 1 else
            run_bazel_test(args[0], args=args[1:])
        ),
        "coverage": lambda args: (
            logger.fatal("Please give target name") if len(args) < 1 else
            run_bazel_coverage(args[0], args=args[1:])
        ),

        ######################### build for all #########################
        "all": lambda args: run_bazel_build('//...', args=args + ['-- -//hello_world']),
        "all_test": lambda args: run_bazel_test('//...', test_output=False, args=args + ['-- -//hello_world']),
        "all_coverage": lambda args: run_bazel_coverage('//...', args=args + ['-- -//hello_world']),
        "coverage_report": lambda args: generate_coverage_report(),

        ######################### build for chat #########################
        "chat": lambda args: run_bazel_build('//chat/...', args=args),

        # chat server
        "chat_redis_server": lambda args: run_docker(
            image='redis --requirepass "123456"',
            container_name='pyc-redis',
            args=['-p 6379:6379']
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
                      '--restart=on-failure:3 -p 3306:3306 -p 33060:33060 -e MYSQL_ROOT_PASSWORD=123456']
            )
        ),
        "chat_gate_server": lambda args: run_bazel_run('//chat/server/gate_server', args=args),
        "chat_verify_server": lambda args: run_bazel_run('//chat/server/verify_server', args=args),
        "chat_status_server": lambda args: run_bazel_run('//chat/server/status_server', args=args),
        "chat_chat_server": lambda args: run_bazel_run('//chat/server/chat_server', args=args),
        "chat_prepare": lambda args: (
            targets["chat_redis_server"](args=[]),
            targets["chat_mysql_server"](args=[]),
            wait_until(lambda: mysql_service_is_ready('localhost', 3306),
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
        "common": lambda args: run_bazel_build('//common/...', args=args),
        "common_test": lambda args: run_bazel_test('//common/test:common_all_test', args=args),
        "common_coverage": lambda args: run_bazel_coverage('//common/test:common_all_test', args=args),

        ######################### build for co_async #########################
        "co_async": lambda args: run_bazel_build('//co_async/...', args=args),
        "co_async_test": lambda args: run_bazel_test('//co_async/test:co_async_all_test', args=args),
        "co_async_debug_test": lambda args: run_bazel_test(
            '//co_async/test:co_async_all_test --define=co_async_debug=true', args=args),
        "co_async_coverage": lambda args: run_bazel_coverage('//co_async/test:co_async_all_test', args=args),

        ######################### build for concurrency #########################
        "concurrency": lambda args: run_bazel_build('//concurrency/...', args=args),
        "concurrency_test": lambda args: run_bazel_test('//concurrency/test:concurrency_all_test', args=args),
        "concurrency_coverage": lambda args: run_bazel_coverage('//concurrency/test:concurrency_all_test', args=args),
        "concurrency_valgrind": lambda args: run_valgrind(
            '//concurrency/test:concurrency_all_test',
            'concurrency/test/concurrency_all_test', args=args),
        # '--gtest_filter=ThreadSafeAdaptorTest.*:ThreadSafeHashTableTest.*:ThreadSafeListTest.*'


        ######################### build for cpp20_stl #########################
        "cpp20_stl": lambda args: run_bazel_build('//cpp20_stl:cpp20_stl_all_test', args=args),
        "cpp20_stl_test": lambda args: run_bazel_test('//cpp20_stl:cpp20_stl_all_test', args=args),
        "cpp20_stl_coverage": lambda args: run_bazel_coverage('//cpp20_stl:cpp20_stl_all_test', args=args),

        ######################### build for design_pattern #########################
        "design_pattern": lambda args: run_bazel_build('//design_pattern:design_pattern_test', args=args),
        "design_pattern_test": lambda args: run_bazel_test('//design_pattern:design_pattern_test', args=args),
        "design_pattern_coverage": lambda args: run_bazel_coverage('//design_pattern:design_pattern_test', args=args),

        ######################### build for logger #########################
        "logger": lambda args: run_bazel_build('//logger/...', args=args),
        "logger_test": lambda args: run_bazel_test('//logger/test:logger_all_test', args=args),
        "logger_coverage": lambda args: run_bazel_coverage('//logger/test:logger_all_test', args=args),
        "logger_bench": lambda args: run_bazel_run('//logger/bench:logger_bench --config=opt', args=args),

        ######################### build for monkey #########################
        "monkey": lambda args: run_bazel_build('//monkey/...', args=args),
        "monkey_run": lambda args: run_bazel_run('//monkey', args=args),
        "monkey_test": lambda args: run_bazel_test('//monkey/test:monkey_all_test', args=args),
        "monkey_coverage": lambda args: run_bazel_coverage('//monkey/test:monkey_all_test', args=args),
        "monkey_bench": lambda args: run_bazel_run('//monkey/bench:monkey_bench --config=opt', args=args),

        ######################### build for network #########################
        "network": lambda args: run_bazel_build('//network/...', args=args),
        "network_run": lambda args: (
            logger.fatal("Please give config name") if len(args) < 1 else
            run_cmd(f'python3 {tool_path / "start_server.py"} {args[0]}')
        ),
        "network_test": lambda args: run_bazel_test('//network/test:network_all_test', args=args),
        "network_coverage": lambda args: run_bazel_coverage('//network/test:network_all_test', args=args),

        ######################### build for nn #########################
        # download dataset
        "fineweb": lambda args: run_venv(f'python {root_path / "nn/nanoGPT/fineweb.py"}', args),
        "hellaswag": lambda args: run_venv(f'python {root_path / "nn/nanoGPT/hellaswag.py"}', args),
        # train_gpt
        "bigram": lambda args: run_venv(f'python {root_path / "nn/nanoGPT/bigram.py"}', args),
        "gpt": lambda args: run_venv(f'python {root_path / "nn/nanoGPT/gpt.py"}', args),
        # train_gpt2
        "train_gpt2": lambda args: run_venv(f'python {root_path / "nn/nanoGPT/train_gpt2.py"}', args),
        "train_gpt2_ddp": lambda args: run_venv(f'torchrun --standalone --nproc_per_node=8 {root_path / "nn/nanoGPT/train_gpt2.py"}', args),
        # micrograd
        "micrograd_test": lambda args: run_venv(
            f'python -m unittest discover -s {root_path / "nn/micrograd/test"} -t {root_path / "nn/micrograd"}', args
        ),

        ######################### build for pycstl #########################
        "pycstl": lambda args: run_bazel_build('//pycstl/...', args=args),
        "pycstl_test": lambda args: run_bazel_test('//pycstl/test:pycstl_all_test', args=args),
        "pycstl_coverage": lambda args: run_bazel_coverage('//pycstl/test:pycstl_all_test', args=args),

        ######################### build for qt #########################
        "qt": lambda args: run_bazel_build('//qt/...', args=args),
        "99table": lambda args: run_bazel_run('//qt/99table', args=args),
        "calculator": lambda args: run_bazel_run('//qt/calculator', args=args),
        "notepad": lambda args: run_bazel_run('//qt/notepad', args=args),

        ######################### build for reaction #########################
        "reaction": lambda args: run_bazel_build('//reaction/...', args=args),
        "reaction_test": lambda args: run_bazel_test('//reaction/test:reaction_all_test', args=args),
        "reaction_coverage": lambda args: run_bazel_coverage('//reaction/test:reaction_all_test', args=args),
        "reaction_valgrind": lambda args: run_valgrind(
            '//reaction/test:reaction_all_test',
            'reaction/test/reaction_all_test', args=args),

        ######################### build for sdl2 #########################
        "sdl2_demo": lambda args: run_bazel_run('//sdl2/demo', args=args),
        "chicken_evil": lambda args: run_bazel_run('//sdl2/chicken_evil', args=args),
        "shooter": lambda args: run_bazel_run('//sdl2/shooter', args=args),

        ######################### build for sdl3 #########################
        "sdl3_demo": lambda args: run_bazel_run('//sdl3/demo', args=args),
        "sdl3_demo_release": lambda args: run_bazel_run('//sdl3/demo --config=opt', args=args),
        "ghost_escape": lambda args: run_bazel_run('//sdl3/ghost_escape', args=args),
        "ghost_escape_release": lambda args: run_bazel_run('//sdl3/ghost_escape --config=opt', args=args),
        "monster_war": lambda args: run_bazel_run('//sdl3/monster_war', args=args),
        "monster_war_release": lambda args: run_bazel_run('//sdl3/monster_war --config=opt', args=args),
        "sunny_land": lambda args: run_bazel_run('//sdl3/sunny_land', args=args),
        "sunny_land_release": lambda args: run_bazel_run('//sdl3/sunny_land --config=opt', args=args),

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

        # 进入 venv
        "venv": venv,

        # 更新 compile_commands.json 文件
        "refresh_all": lambda args: run_bazel_run('@hedron_compile_commands//:refresh_all', args=args),
        "refresh": lambda args: run_bazel_run('//:refresh_compile_commands', args=args),

        ######################### 更新依赖锁文件 #########################
        # 从 requirements.in 重新生成 requirements_lock.txt
        "update_pip": lambda args: run_bazel_run('//:requirements.update', args=args),
        # 从 package.json 重新生成 pnpm-lock.yaml
        "update_pnpm": lambda args: run_cmd(
            f'bazel run -- @pnpm --dir {root_path} install --lockfile-only'),
    }

    target, additional_args = parse_args(list(targets.keys()))

    # 执行对应目标的函数
    targets[target](additional_args)


if __name__ == "__main__":
    main()
