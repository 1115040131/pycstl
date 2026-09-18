#!/usr/bin/env python3

import subprocess

from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from cmd_utils import run_cmd
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

# 路径定义
tool_path = Path(__file__).resolve().parent
root_path = tool_path.parent


class VenvName(str, Enum):
    """虚拟环境名称: 新增环境时在此加成员, 并登记到 VENVS"""
    DEV = 'dev'
    NN = 'nn'


@dataclass
class Venv:
    """虚拟环境: 目录与依赖文件"""
    path: str
    requirements: str


# 不指定名字时默认安装的环境
DEFAULT_VENV = VenvName.DEV

# 虚拟环境: 名称 -> 环境信息, 统一放在 .venv/ 下
# nn 供 nn/ 相关 target 使用, dev 供 bazel 之外的开发与分析工具使用
VENVS: dict[VenvName, Venv] = {
    VenvName.DEV: Venv('.venv/dev', 'requirements.in'),
    VenvName.NN: Venv('.venv/nn', 'nn/requirements.in'),
}


def check_venv(venv_name: VenvName) -> None:
    venv_cfg = VENVS[venv_name]
    python = root_path / venv_cfg.path / 'bin' / 'python'
    if not python.exists():
        logger.fatal(f'虚拟环境不存在，请先运行: tool/build.py venv {venv_name}')

    req = root_path / venv_cfg.requirements
    marker = root_path / venv_cfg.path / '.deps_installed'
    if not marker.exists() or req.stat().st_mtime > marker.stat().st_mtime:
        logger.warn(f'{venv_cfg.requirements} 已更新，虚拟环境可能过期，建议运行: tool/build.py venv {venv_name}')


def run_venv(venv_name: VenvName, command: str, args: list[str] | None = None) -> None:
    check_venv(venv_name)
    bin_path = root_path / VENVS[venv_name].path / 'bin'
    run_cmd(f'{bin_path}/{command} {" ".join(args or [])}')


def venv(args: list[str]):
    try:
        name = VenvName(args[0]) if args else DEFAULT_VENV
    except ValueError:
        logger.fatal(f"unknown venv: {args[0]}, available: {', '.join(v.value for v in VenvName)}")

    venv_cfg = VENVS[name]
    venv_path = root_path / venv_cfg.path
    script_path = tool_path / 'setup_venv.sh'

    # 必须使用 open() 打开文件，获得文件对象 f
    with open(script_path, 'r') as f:
        result = subprocess.run(
            ["bash", "-s", "--", venv_path, root_path / venv_cfg.requirements], stdin=f)

    # 子进程无法激活父 shell 的虚拟环境，成功后提示用户手动 source
    if result.returncode == 0:
        logger.info(f'虚拟环境已就绪，请在当前 shell 运行以进入：source {venv_path / "bin" / "activate"}')
