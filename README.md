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

或者安装 bazelisk (推荐)
```bash
wget https://github.com/bazelbuild/bazelisk/releases/download/v1.29.0/bazelisk-linux-amd64
chmod +x bazelisk-linux-amd64
sudo mv bazelisk-linux-amd64 /usr/local/bin/bazel
```

### 必备库

```bash
sudo apt update
sudo apt install gcc g++ # 编译器
sudo apt install libx11-dev  # 窗口
sudo apt install libpulse-dev libxext-dev # 音频驱动
sudo apt install libxcb-cursor0 libxkbcommon-x11-0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0 libxcb-xkb1 # qt
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

```bash
# 安装绘图软件
sudo apt install graphviz

# 进入虚拟环境
./make venv
```

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
# 查看 repository cache 路径
bazel info repository_cache

# 彻底清除
bazel clean --expunge
```

### 限制并行数量

```bash
bazel build --local_resources=cpu=24
```

## 更新 pnpm

首先更新 package.json 然后运行如下命令

```bash
bazel run -- @pnpm --dir $PWD install --lockfile-only
```

## Optional

### WSL

配置与宿主机镜像网络模式

```bash
.wslconfig

[wsl2]
networkingMode=mirrored
dnsTunneling=true
firewall=true
autoProxy=true
```

预分配端口避开后端组件常用端口

```bash
netsh interface ipv4 show excludedportrange protocol=tcp
netsh int ipv4 set dynamicport tcp start=49152 num=16384
netsh int ipv6 set dynamicport tcp start=49152 num=16384
```

### acl 工具

安装 acl 工具, 用户管理让 mysql 创建的所有文件当前用户都有读写权限

```bash
sudo apt update && sudo apt install acl
```

### pre-commit

1. 安装 pre-commit

```bash
sudo apt install pre-commit
```

2. 创建 .pre-commit-config.yaml

```yaml
# .pre-commit-config.yaml
repos:
  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v17.0.6  # 使用最新版本
    hooks:
      - id: clang-format
        types: [c++]
        args: [--style=file]  # 使用项目中的 .clang-format 文件

  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
        types: [c++]
      - id: end-of-file-fixer
        types: [c++]
      - id: check-merge-conflict
      - id: check-added-large-files

  # 可选：添加其他检查
  - repo: https://github.com/cheshirekow/cmake-format-precommit
    rev: v0.6.13
    hooks:
      - id: cmake-format
```

3. 安装 pre-commit hooks

```bash
# 把配置里所有 hook 的 rev 自动改写成各仓库的最新 tag
pre-commit autoupdate

# 在项目根目录下执行
pre-commit install

# 可选：也安装到 commit-msg hook
pre-commit install --hook-type commit-msg
```

4. 手动运行（测试用）

```bash
# 对所有文件运行
pre-commit run --all-files

# 只对暂存的文件运行
pre-commit run

# 运行特定的hook
pre-commit run clang-format --all-files

# stages: [manual] 的 hook 必须带 --hook-stage manual 才会跑
pre-commit run clang-tidy --hook-stage manual --all-files
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
# 强制使用无声驱动（dummy驱动）
export SDL_AUDIODRIVER=dummy
export AUDIODEV=null
```
