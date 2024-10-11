import json
import os
import subprocess
import shlex
import shutil
import time

from logger import Logger

logger = Logger()


def run_cmd(cmd, check=True):
    logger.info(cmd)
    try:
        subprocess.run(shlex.split(cmd), check=check)
    except subprocess.CalledProcessError as e:
        logger.warn(f"Command '{e.cmd}' returned non-zero exit status {e.returncode}.")
    except KeyboardInterrupt:
        logger.warn("Keyboard interrupt received, stopping.")
    except Exception as e:
        logger.warn(f"An unexpected error occurred: {e}")


# 创建目录并设置权限的函数
def setup_directory(path):
    if not os.path.exists(path):
        # 获取当前用户的 UID
        uid = os.getuid()
        run_cmd(f'mkdir -p {path}')
        if shutil.which('setfacl') is not None:
            # 存在 setfacl 命令
            run_cmd(f'sudo setfacl -m d:u:{uid}:rwx {path}')
            run_cmd(f'sudo setfacl -m u:{uid}:rwx {path}')
        else:
            # 不存在 setfacl 命令
            logger.warn(f"setfacl: command not found, skip sudo setfacl -m d:u:{uid}:rwx {path}")
            logger.warn(f"setfacl: command not found, skip sudo setfacl -m u:{uid}:rwx {path}")


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


def run_tmux(windows_commands_dict: dict):
    if not windows_commands_dict:
        logger.error("The commands dictionary must not be empty.")
        return

    session_name = "run_tmux"
    run_cmd(f"tmux kill-session -t {session_name}", check=False)
    run_cmd(f"tmux new-session -d -s {session_name} -n {list(windows_commands_dict.keys())[0]}")
    # 启用鼠标控制
    run_cmd("tmux set -g mouse on")

    first_window = True
    for window_name, pane_commands in windows_commands_dict.items():
        if not first_window:
            # 从第二个窗口开始，需要先创建新窗口
            run_cmd(f'tmux new-window -t {session_name} -n {window_name}')
        else:
            first_window = False

        for pane_index, command in enumerate(pane_commands):
            if pane_index > 0:
                # 在当前窗口中水平分割出新面板
                run_cmd('tmux split-window -h')
            # 在新面板中发送命令
            tmux_send_keys(command)

            # 均衡分配面板空间
            run_cmd('tmux select-layout even-horizontal')

        # 切换到第一个面板
        run_cmd('tmux select-pane -t 0')

    try:
        # 附加到tmux会话以查看输出
        run_cmd(f"tmux attach-session -t {session_name}")
    finally:
        # 清理：发送 Ctrl+C 到所有面板，并最终杀死会话
        for window_index, window_name in enumerate(windows_commands_dict.keys()):
            pane_count = len(windows_commands_dict[window_name])
            for pane_index in range(pane_count):
                run_cmd(f"tmux send-keys -t {session_name}:{window_index}.{pane_index} C-c")
            run_cmd(f"tmux kill-window -t {session_name}:{window_index}")
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
            logger.info(f"Creating and starting container '{container_name}'")

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


def wait_until(condition_func, msg, interval=0.5):
    """等待直到condition_func返回True，在此期间以interval秒为间隔重复检查。

    Args:
        condition_func (function): 一个无参数的函数，返回布尔值。
        interval (float): 轮询条件函数的时间间隔，单位为秒。
    """
    try_cnt = 1
    while not condition_func():
        logger.info(f"Waiting for {msg} try {try_cnt}")
        try_cnt = try_cnt + 1
        time.sleep(interval)
    logger.info(f"{msg} satisfied!")


def mysql_service_is_ready(user, password, host, port):
    """尝试连接到MySQL服务器并执行一个基本查询，以验证服务是否就绪。

    Args:
        user (str): 数据库用户名称。
        password (str): 数据库用户密码。
        host (str): MySQL服务器主机名或IP地址。
        port (int): MySQL服务器端口号。

    Returns:
        bool: 如果成功执行查询，则表示MySQL服务就绪，返回True。
    """

    try:
        import mysql.connector
        from mysql.connector import Error

        conn = None  # 初始化conn为None，确保即使数据库连接失败也能进入finally块
        try:
            # 尝试创建数据库连接
            conn = mysql.connector.connect(
                user=user,
                password=password,
                host=host,
                port=port
            )

            # 创建一个cursor对象
            cursor = conn.cursor()
            cursor.execute("SHOW VARIABLES LIKE 'version';")

            # 确认我们得到了响应
            if cursor.fetchone():
                return True
            else:
                return False

        except Error as e:
            logger.warn(f"Error connecting to MySQL: {e}")
            return False
        finally:
            if conn is not None and conn.is_connected():  # 在尝试关闭前检查conn是否已创建并连接
                cursor.close()
                conn.close()

    except ImportError:
        logger.warn("mysql-connector-python not installed, check MySQL by sleep 10s...")
        time.sleep(10)
        return True
