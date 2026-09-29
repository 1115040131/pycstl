#!/usr/bin/env python3
"""检查 (可用 --fix 就地修正) 全仓 C++20 模块 import 的排序与分块.

排序与分块约定:
  1. 块按「模块名第一个 `.` 之前那一段」划分, 分区的块取本文件 module 声明的首段;
     块之间按首段字典序, 恰好一个空行, 块内不得有空行
  2. 块内分区在最前 (分区之间也按名字字典序), 其余按模块名字典序
  3. `export` 只是属性, 完全不参与比较

用法:
  python3 tool/check_import_order.py             # 检查, 有违规则以 1 退出
  python3 tool/check_import_order.py --fix       # 就地修正
  python3 tool/check_import_order.py chat/ sdl3/ # 只检查指定文件或目录
"""

import argparse
import re

from collections.abc import Iterator
from pathlib import Path
from logger import Logger, LogStyle

logger = Logger(LogStyle.NO_DEBUG_INFO)

root_path = Path(__file__).resolve().parent.parent

# .h 里也可能有 import (被文本包含进模块 purview 的头)
SOURCE_SUFFIXES = ('.cpp', '.cppm', '.cc', '.h')

# 不进入检查的目录
SKIP_DIRS = {'third_party', 'external', '.venv', '.git', 'node_modules'}

# 规范形式的 import 行, 带尾注等写法一律视为需手工整理
IMPORT_RE = re.compile(r'^(\s*)(export\s+)?import\s+([^;]+);[ \t]*$')

# 宽松匹配, 用来揪出 IMPORT_RE 认不出的 import 行
IMPORT_LOOSE_RE = re.compile(r'^[ \t]*(export[ \t]+)?import[ \t]')

# 本文件的模块声明, 只为给分区找「所在块」; 实现单元写 module, 接口单元写 export module
MODULE_RE = re.compile(r'^\s*(?:export\s+)?module\s+([A-Za-z_][\w.]*)\s*(?::\s*[\w.]+)?\s*;')

MANUAL_FIX = '请手工整理'


def iter_sources(paths) -> Iterator[Path]:
    """展开待检查的源文件, 跳过构建产物与三方目录"""
    for path in paths:
        if path.is_file():
            yield path
            continue
        for file in sorted(path.rglob('*')):
            if file.suffix not in SOURCE_SUFFIXES:
                continue
            if any(part in SKIP_DIRS or part.startswith('bazel-') for part in file.parts):
                continue
            yield file


def module_first_segment(lines: list[str], end: int) -> str:
    """取本文件 module 声明的首段, 用来给分区归块; 全局模块 TU 返回空串"""
    for line in lines[:end]:
        matched = MODULE_RE.match(line)
        if matched:
            return matched.group(1).split('.')[0]
    return ''


def import_key(name: str, owner: str) -> tuple:
    """排序键: (块首段, 分区优先, 模块名)"""
    if name.startswith(':'):
        return (owner, 0, name)
    return (name.split('.')[0], 1, name)


def scan(lines: list[str]) -> tuple[list, list]:
    """返回 (hits, problems), hits 为 [(行下标, 模块名)], problems 为 [(行号, 说明)]"""
    hits, problems = [], []
    for index, line in enumerate(lines):
        matched = IMPORT_RE.match(line)
        if matched:
            hits.append((index, matched.group(3).strip()))
        elif IMPORT_LOOSE_RE.match(line):
            problems.append((index + 1, f'无法解析的 import 行, {MANUAL_FIX}'))

    if not hits:
        return hits, problems

    for index in range(hits[0][0], hits[-1][0] + 1):
        line = lines[index]
        if IMPORT_RE.match(line) or not line.strip() or line.lstrip().startswith('//'):
            continue
        # 认不出的 import 行上面已经报过, 不重复
        if IMPORT_LOOSE_RE.match(line):
            continue
        problems.append((index + 1, f'import 区间内混入非 import 行, {MANUAL_FIX}'))

    return hits, problems


def check(lines: list[str]) -> list:
    """返回违规列表 [(行号, 说明)]"""
    hits, violations = scan(lines)
    if len(hits) < 2:
        return violations

    owner = module_first_segment(lines, hits[0][0])
    keys = [import_key(name, owner) for _, name in hits]

    for index in range(len(hits) - 1):
        lineno, name = hits[index]
        next_lineno, next_name = hits[index + 1]

        if keys[index] > keys[index + 1]:
            violations.append((lineno + 1, f'顺序错误: {name} 应排在 {next_name} 之后'))

        blanks = [index for index in range(lineno + 1, next_lineno) if not lines[index].strip()]
        if keys[index][0] == keys[index + 1][0]:
            if blanks:
                violations.append((blanks[0] + 1, f'同一块 ({keys[index][0]}) 内不允许空行'))
        elif len(blanks) != 1:
            violations.append((next_lineno + 1,
                               f'块 {keys[index][0]} 与 {keys[index + 1][0]} 之间应恰好一个空行, '
                               f'实际 {len(blanks)} 个'))

    return violations


def sort_imports(lines: list[str]) -> list[str]:
    """返回重排后的行列表, 注释跟着它下面那条 import 一起搬"""
    hits, _ = scan(lines)
    lo, hi = hits[0][0], hits[-1][0]
    owner = module_first_segment(lines, lo)

    entries, pending = [], []
    for index in range(lo, hi + 1):
        line = lines[index]
        matched = IMPORT_RE.match(line)
        if matched:
            name = matched.group(3).strip()
            entries.append((import_key(name, owner), pending, line))
            pending = []
        elif not line.strip():
            continue
        else:
            pending.append(line)
    entries.sort()

    result, previous_block = [], None
    for (block, _, _), comments, line in entries:
        if previous_block is not None and block != previous_block:
            result.append('')
        result.extend(comments)
        result.append(line)
        previous_block = block

    return lines[:lo] + result + lines[hi + 1:]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('--fix', action='store_true', help='就地修正, 而不是只报告')
    parser.add_argument('paths', nargs='*', help='要检查的文件或目录, 默认为仓库根目录')
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    paths = [Path(path) for path in args.paths] or [root_path]

    total, bad, fixed, skipped = 0, 0, 0, 0
    for path in iter_sources(paths):
        total += 1
        text = path.read_text(encoding='utf-8')
        violations = check(text.split('\n'))
        if not violations:
            continue

        bad += 1
        for lineno, message in violations:
            logger.error(f'{path}:{lineno}: {message}')

        if not args.fix:
            continue
        # 有认不出的行时不动它, 免得把内容搬错地方
        if any(MANUAL_FIX in message for _, message in violations):
            skipped += 1
            continue
        new_text = '\n'.join(sort_imports(text.split('\n')))
        if new_text != text:
            path.write_text(new_text, encoding='utf-8')
            fixed += 1

    if bad and not args.fix:
        logger.fatal(f'{bad}/{total} 个文件不符合 import 排序约定, 加 --fix 可就地修正')
    if bad:
        logger.info(f'已修正 {fixed}/{bad} 个文件')
        if skipped:
            logger.fatal(f'{skipped} 个文件含无法解析的 import 行, 未修正')
        return
    logger.info(f'{total} 个文件的 import 排序检查通过')


if __name__ == "__main__":
    main()
