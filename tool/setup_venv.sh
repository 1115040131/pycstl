#!/bin/bash

root_path=$1
echo $root_path

# 虚拟环境的路径
venv_path="$root_path/.venv"

# 检查 python3-venv 包是否已安装
if ! dpkg -s python3-venv &>/dev/null; then
    echo "python3-venv 未安装。"

    # 询问用户是否自动安装
    read -p "是否尝试自动安装 python3-venv? (y/N) " answer
    if [[ $answer = [Yy]* ]]; then
        sudo apt update && sudo apt install -y python3-venv
        if [ $? -ne 0 ]; then
            echo "自动安装失败，请手动安装 python3-venv 后再次运行此脚本。"
            exit 1
        fi
    else
        echo "请手动安装 python3-venv 后再次运行此脚本。"
        exit 1
    fi
fi

# 检查当前是否处于 script/venv 的虚拟环境中
if [[ "$VIRTUAL_ENV" != "$venv_path" ]]; then
    echo "未在指定的虚拟环境中，正在尝试激活..."

    # 如果虚拟环境不存在，则创建它
    if [ ! -d "$venv_path" ]; then
        echo "虚拟环境不存在，正在创建..."
        python3 -m venv "$venv_path"
    fi

    # 激活虚拟环境
    source "$venv_path/bin/activate"

    # 安装依赖
    pip install -r "$root_path/requirements.in"

    # 检查 pip install 命令是否成功
    if [ $? -eq 0 ]; then
        echo "依赖包安装成功。"
    else
        echo "依赖包安装失败。"
        # 处理错误情况，例如退出脚本
        exit 1
    fi

    # 提醒用户虚拟环境已被激活
    echo "虚拟环境已激活，并且依赖已安装。"
else
    echo "已经处于指定的虚拟环境中。"
fi