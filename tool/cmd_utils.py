import subprocess
import shlex

from logger import Logger

logger = Logger()


def run_cmd(cmd, check=True):
    subprocess.run(shlex.split(cmd), check=check)


def tmux_send_keys(keys, session=None, window=None, pane=None, enter=True):
    """
    Send keys to a specific tmux session, window, or pane.

    :param keys: The keys to send (string).
    :param session: The tmux session to send keys to (optional).
    :param window: The tmux window to send keys to (optional).
    :param pane: The tmux pane to send keys to (optional).
    :param enter: If True, simulate pressing Enter at the end (optional).
    """
    cmd = 'tmux send-keys'

    # Specify tmux target if provided (format is session:window.pane)
    if session or window or pane:
        target = ''
        if session:
            target += f'{session}:'
        if window:
            target += f'{window}.'
        if pane:
            target += f'{pane}'
        cmd += f' -t {target}'

    # Prepare the key sequence
    keys_formatted = keys.replace('"', '\\"')  # Escape double quotes
    cmd += f' "{keys_formatted}"'

    # Append 'C-m' (Enter) if needed
    if enter:
        cmd += ' C-m'

    # Run the command
    run_cmd(cmd)


def run_tmux(*args):
    argc = len(args)
    if argc < 1:
        logger.error("At least one arg")
        return

    for arg in args:
        logger.debug(arg)

    session_name = "run_tmux"
    run_cmd(f"tmux kill-session -t {session_name}", check=False)
    run_cmd(f"tmux new-session -d -s {session_name}")

    # 均分成 n 个窗口
    for i in range(argc - 1):
        run_cmd('tmux split-window -h')
    run_cmd('tmux select-layout even-horizontal')

    # 分别在每一个窗口中执行对应命令
    for i, sub_command in enumerate(args):
        run_cmd(f'tmux select-pane -t {i}')
        tmux_send_keys(sub_command)

    try:
        # 附加到tmux会话以查看输出
        run_cmd(f"tmux attach-session -t {session_name}")
    finally:
        for i in range(argc):
            run_cmd(f"tmux send-keys -t {session_name}:0.{i} C-c")
        run_cmd(f"tmux kill-session -t {session_name}")
