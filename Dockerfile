FROM silkeh/clang:16

# 安装 Bazel
RUN apt-get update && apt-get install -y git \
    && wget https://github.com/bazelbuild/bazelisk/releases/download/v1.19.0/bazelisk-linux-amd64 \
    && chmod +x bazelisk-linux-amd64 \
    && mv bazelisk-linux-amd64 /usr/local/bin/bazel

WORKDIR /workspace