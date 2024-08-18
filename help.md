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
sudo apt update && sudo apt install gc
```

### docker

部分后端项目 (chat) 依赖 docker 启动 redis, mysql 等服务

wsl 下建议使用 docker desktop \
在 ubuntu 中安装 docker 请访问 https://docs.docker.com/engine/install/ubuntu/

## 使用

### 编译/运行/测试项目

```bash
./make xxx
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