#!/usr/bin/env python3

import sys
from pathlib import Path

root_path = Path(__file__).resolve().parent.parent


def patch_file(file_path: Path, search_text, replace_text):
    # 1. 检查文件是否存在
    if not file_path.exists():
        print(f"❌ 错误: 文件未找到 -> {file_path.absolute()}")
        sys.exit(1)

    print(f"正在检查文件: {file_path}")

    try:
        # 2. 读取内容 (pathlib 提供了便捷的 read_text 方法)
        content = file_path.read_text(encoding="utf-8")

        # 3. 检查是否需要替换
        count = content.count(search_text)
        if count == 0:
            print(f"⚠️ 警告: 未找到 '{search_text}'，无需修改。")
            return

        # 4. 执行替换
        new_content = content.replace(search_text, replace_text)

        # 5. 写入文件 (write_text 自动处理打开和关闭文件)
        file_path.write_text(new_content, encoding="utf-8")

        print(f"✅ 成功: 已将 {count} 处 '{search_text}' 替换为 '{replace_text}'")

    except Exception as e:
        print(f"❌ 处理文件时发生异常: {e}")
        sys.exit(1)


def patch_co_async():
    patch_file(root_path / 'co_async/test/utils.h', '1ms', '20ms')


def patch_sdl3():
    patch_file(root_path / 'sdl3/BUILD.bazel', 'name = "sdl3",', '''name = "sdl3",
    cache_entries = {
        "SDL_UNIX_CONSOLE_BUILD": "ON",
    },''')


if __name__ == "__main__":
    patch_co_async()
    patch_sdl3()
