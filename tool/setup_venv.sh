#!/bin/bash

# 参数: $1 虚拟环境目录, $2 依赖文件, 均为绝对路径, 由调用方决定用哪一套
venv_path=$1
requirements_file=$2
echo "虚拟环境: $venv_path"
echo "依赖文件: $requirements_file"

# 包名各发行版不一致 (python3-venv / python3.10-venv), 直接检测模块是否可用
# 本脚本经 bash -s 由 stdin 传入, read 会读走脚本自身内容, 因此不能交互询问
if ! python3 -c "import ensurepip, venv" &>/dev/null; then
    echo "python3 的 venv 模块不可用，请先安装后再次运行此脚本。"
    echo "Debian/Ubuntu: sudo apt install -y python3-venv"
    exit 1
fi

# 如果虚拟环境不存在，则创建它
if [ ! -d "$venv_path" ]; then
    echo "虚拟环境不存在，正在创建..."
    python3 -m venv "$venv_path"
fi

# 检查当前是否处于 script/venv 的虚拟环境中，未激活则激活
if [[ "$VIRTUAL_ENV" != "$venv_path" ]]; then
    echo "未在指定的虚拟环境中，正在激活..."
    source "$venv_path/bin/activate"
else
    echo "已经处于指定的虚拟环境中。"
fi

# 安装依赖 (无论是否已激活都要执行，否则已激活时会跳过安装，导致依赖实际未更新)
pip install -r "$requirements_file"

# 检查 pip install 命令是否成功
if [ $? -eq 0 ]; then
    # 记录本次依赖安装完成的时间点，供 build.py 判断 requirements 是否已重新安装
    touch "$venv_path/.deps_installed"
    echo "依赖包安装成功。"
else
    echo "依赖包安装失败。"
    # 处理错误情况，例如退出脚本
    exit 1
fi

echo "虚拟环境已就绪。"