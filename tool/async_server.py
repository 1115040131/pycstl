#!/usr/bin/python3

import subprocess
import shlex
from pathlib import Path


def run_cmd(cmd):
    subprocess.run(shlex.split(cmd))


def run():
    # 获取仓库的绝对路径
    repo_path = Path(__file__).resolve().parent.parent

    bin_path = f"{repo_path}/bazel-bin"

    server_path = "network/example/async_server"
    client_path = "network/example/sync_client"

    # 创建新的tmux会话，名称为"my_session"
    session_name = "sync_server"
    run_cmd(f"tmux new-session -d -s {session_name}")

    # 将会话分割成左右两个窗格
    run_cmd(f"tmux split-window -h -t {session_name}")

    # 在第一个窗格运行程序A
    run_cmd(f"tmux send-keys -t {session_name}:0.0 '{bin_path}/{server_path}' C-m")

    # 在第二个窗格运行程序B
    run_cmd(f"tmux send-keys -t {session_name}:0.1 'sleep 1' C-m")
    run_cmd(f"tmux send-keys -t {session_name}:0.1 '{bin_path}/{client_path}' C-m")

    try:
        # 附加到tmux会话以查看输出
        run_cmd(f"tmux attach-session -t {session_name}")
    finally:
        run_cmd(f"tmux send-keys -t {session_name}:0.0 C-c")
        run_cmd(f"tmux send-keys -t {session_name}:0.1 C-c")
        run_cmd(f"tmux kill-session -t {session_name}")


if __name__ == "__main__":
    run()
