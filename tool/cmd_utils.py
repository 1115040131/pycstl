import json
import subprocess
import shlex

from logger import Logger

logger = Logger()


def run_cmd(cmd, check=True):
    logger.info(cmd)
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


def run_docker(image, container_name, args=[]):
    try:
        # 列出所有容器（包括未运行的）
        output = subprocess.check_output(shlex.split('docker ps -a --format {{.Names}}'))
        containers = output.decode('utf-8').strip().split('\n')

        command = ''
        if container_name not in containers:
            # 容器不存在，根据提供的参数运行新的容器
            command = f'docker run -d --name {container_name} {" ".join(args)} {image}'
            logger.info(f"Creating and starting container '{container_name}' with arguments: {' '.join(args)}")

        else:
            # 检查容器是否处于运行状态
            inspect_output = subprocess.check_output(['docker', 'inspect', '-f', '{{.State.Running}}', container_name])
            is_running = json.loads(inspect_output.decode('utf-8').strip().lower())

            if not is_running:
                # 容器存在但未启动，启动容器
                command = f'docker start {container_name}'
                logger.info(f"Starting existing container '{container_name}'...")
            else:
                # 容器已经在运行，无需执行任何操作
                logger.info(f"Container '{container_name}' is already running.")

        if command:
            logger.debug(command)
            subprocess.run(shlex.split(command))

    except subprocess.CalledProcessError as e:
        logger.error(f"An error occurred: {e.output.decode('utf-8')}")
