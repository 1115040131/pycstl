#!/usr/bin/python3

import argparse
import json

from pathlib import Path
from cmd_utils import run_tmux
from logger import Logger

logger = Logger()


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


def run(config_name):
    # 读取配置
    server, client = read_config(config_name)
    if server is None:
        return

    tool_path = Path(__file__).resolve().parent

    if client is None:
        run_tmux(f"python3 {tool_path / 'build.py'} //network/example:{server}")
    else:
        run_tmux(f"python3 {tool_path / 'build.py'} //network/example:{server}",
                 f"python3 {tool_path / 'build.py'} //network/example:{client}")


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
