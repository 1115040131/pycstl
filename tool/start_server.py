#!/usr/bin/python3

import argparse
import time
import json
import os
import subprocess
import shlex
from pathlib import Path
from logger import Logger

logger = Logger()


def run_cmd(cmd, check=True):
    subprocess.run(shlex.split(cmd), check=check)


def read_config(config_name):
    try:
        # 打开并读取 JSON 文件
        file_name = f"{Path(__file__).resolve().parent}/server_client_config.json"
        with open(file_name, 'r') as file:
            data = json.load(file)

        # 在 JSON 数据中查找匹配的配置对象
        config = data.get(config_name)
        if config is not None:
            # 检查是否存在 server 和 client 属性
            client = config.get('client')
            server = config.get('server')

            # 没有配置 server 直接退出
            if server is None:
                logger.error(
                    f"The '{config_name}' config is missing the 'server' property.")
                return None, None

            # 没有配置 client
            if client is None:
                logger.warn(
                    f"The '{config_name}' config is missing the 'client' property.")

            return server, client
        else:
            logger.error(f"No config found with name '{config_name}'.")
            return None, None

    except FileNotFoundError:
        logger.error(f"The file '{file_name}' was not found.")
        return None, None
    except json.JSONDecodeError as e:
        logger.error(f"Error decoding JSON: {e}")
        return None, None
    except ValueError as e:
        logger.error(e)
        return None, None


def run_tmux(server, client):
    # 获取仓库的绝对路径
    repo_path = Path(__file__).resolve().parent.parent

    bin_path = f"{repo_path}/bazel-bin/network/example"

    # 检查 server 文件是否存在
    if server is None:
        return
    server_file = f"{bin_path}/{server}"
    if not os.path.isfile(server_file):
        logger.error(f"Server file {server_file} not exist.")
        return
    logger.info(f"Start Server \"{server}\"")

    # 检查 client 文件是否存在
    if client is not None:
        client_file = f"{bin_path}/{client}"
        if not os.path.isfile(client_file):
            logger.error(f"Client file {client_file} not exist.")
            return
        logger.info(f"Start Client \"{client}\"")

    time.sleep(1)

    # 创建新的tmux会话，名称为"my_session"
    session_name = "sync_server"
    run_cmd(f"tmux kill-session -t {session_name}", check=False)
    run_cmd(f"tmux new-session -d -s {session_name}")

    # 将会话分割成左右两个窗格
    run_cmd(f"tmux split-window -h -t {session_name}")

    # 在第一个窗格运行程序A
    run_cmd(
        f"tmux send-keys -t {session_name}:0.0 '{server_file}' C-m")

    # 在第二个窗格运行程序B
    if client is not None:
        run_cmd(f"tmux send-keys -t {session_name}:0.1 'sleep 1' C-m")
        run_cmd(
            f"tmux send-keys -t {session_name}:0.1 '{client_file}' C-m")

    try:
        # 附加到tmux会话以查看输出
        run_cmd(f"tmux attach-session -t {session_name}")
    finally:
        run_cmd(f"tmux send-keys -t {session_name}:0.0 C-c")
        run_cmd(f"tmux send-keys -t {session_name}:0.1 C-c")
        run_cmd(f"tmux kill-session -t {session_name}")


def run(config_name):
    # 读取配置
    server, client = read_config(config_name)
    if server is None:
        return

    # 运行
    run_tmux(server, client)


if __name__ == "__main__":
    # 设置 argparse 解析器
    parser = argparse.ArgumentParser(
        description='Application to start server and client')

    # 添加 config 参数，这里设定为必需的命令行参数
    parser.add_argument('config', type=str,
                        help='the config name in server_client_pair.json')

    # 解析命令行参数
    args = parser.parse_args()

    run(args.config)
