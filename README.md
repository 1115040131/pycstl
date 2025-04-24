# README

## 基本依赖

### bazel

可访问 https://bazel.google.cn/install/ubuntu?hl=en 获取更多信息

安装步骤
```bash
# 将 bazel 添加到源
sudo apt install apt-transport-https curl gnupg -y
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
sudo mv bazel-archive-keyring.gpg /usr/share/keyrings
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list

# 安装 bazel
sudo apt update && sudo apt install bazel # 安装
sudo apt update && sudo apt full-upgrade  # 升级 bazel 版本
```

### 编译器

```bash
sudo apt update && sudo apt install gcc
```

### docker

部分后端项目 (chat) 依赖 docker 启动 redis, mysql 等服务

wsl 下建议使用 docker desktop \
在 ubuntu 中安装 docker 请访问 https://docs.docker.com/engine/install/ubuntu/

在宿主机上，可以将用户添加到 docker 组。这样，这个用户就能够运行 Docker 命令来管理和运行容器，包括以 root 权限运行容器。

```bash
sudo usermod -aG docker your_username
```

### python

确保拥有 python 环境, 执行 ./make 命令

## 使用

### 编译/运行/测试项目

```bash
./make xxx

./make all_test --cache_test_results=no
```

### 导出 compile_commands.json

```bash
./make refresh
```

### 清除 bazel 缓存

```bash
bazel clean --expunge
```

## 更新 pnpm

首先更新 package.json 然后运行如下命令

```bash
bazel run -- @pnpm --dir $PWD install --lockfile-only
```

## Optional

### mysql-connector-python

安装 mysql-connector-python 包, 用以检测 mysql 容器是否已经 ready

```bash
pip install mysql-connector-python --break-system-packages
```

### acl 工具

安装 acl 工具, 用户管理让 mysql 创建的所有文件当前用户都有读写权限

```bash
sudo apt update && sudo apt install acl
```

### QT Designer

```bash
sudo apt install qttools5-dev-tools
```

### 切换 Wayland 与 x11

```bash
export QT_QPA_PLATFORM=wayland
export QT_QPA_PLATFORM=xcb
```

### SDL

```bash
# 安装驱动
sudo apt install libpulse-dev libxext-dev

# 强制使用无声驱动（dummy驱动）
export SDL_AUDIODRIVER=dummy
export AUDIODEV=null
```