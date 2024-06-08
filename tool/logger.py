import copy
import logging
import os
import sys
import uuid
from enum import Enum, auto


class LogStyle(Enum):
    STANDARD = auto()
    NO_DEBUG_INFO = auto()


class LogColors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    WHITE = '\033[97m'      # 白色文本
    BACKGROUND_RED = '\033[41m'  # 背景色为红色
    RESET = '\033[0m'


class StandardFormatter(logging.Formatter):
    def __init__(self, datefmt="%Y-%m-%d %H:%M:%S", *args, **kwargs):
        # 默认日志格式，此处使用提供的log_fmt格式
        fmt = "[%(asctime)s] [%(levelname)s] <%(custom_filename)s:%(custom_lineno)d> [%(custom_funcName)s]: %(message)s"
        super().__init__(fmt, datefmt, *args, **kwargs)
        self.FORMATS = {
            logging.DEBUG: LogColors.BLUE + fmt + LogColors.RESET,
            logging.INFO: LogColors.GREEN + fmt + LogColors.RESET,
            logging.WARNING: LogColors.YELLOW + fmt + LogColors.RESET,
            logging.ERROR: LogColors.RED + fmt + LogColors.RESET,
            logging.CRITICAL: LogColors.BACKGROUND_RED +
            LogColors.WHITE + fmt + LogColors.RESET
        }

    # 重写 format 方法来添加颜色和自定义字段
    def format(self, record):
        if len(record.levelname) > 5:
            record.levelname = record.levelname[:5]
        else:
            record.levelname = record.levelname.ljust(5)  # 确保宽度为5字符

        # 动态设置消息格式
        self._style._fmt = self.FORMATS.get(record.levelno, self._fmt)

        # 使用父类的format方法来完成格式化
        formatted_message = super().format(record)

        # 恢复原始格式以避免影响其他处理器/格式化器
        self._style._fmt = self._fmt

        return formatted_message


class NoDebugInfoFormatter(logging.Formatter):
    # 赋予每个日志级别特定的颜色
    LEVEL_COLORS = {
        logging.DEBUG: LogColors.BLUE,
        logging.INFO: LogColors.GREEN,
        logging.WARNING: LogColors.YELLOW,
        logging.ERROR: LogColors.RED,
        logging.CRITICAL: LogColors.BACKGROUND_RED + LogColors.WHITE
    }

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def format(self, record):
        # 创建原始记录的副本以保持线程安全
        new_record = copy.copy(record)

        # 根据日志等级为记录添加颜色前缀和重置颜色序列
        color_prefix = self.LEVEL_COLORS.get(new_record.levelno, "")
        new_record.msg = f"{color_prefix}{new_record.msg}{LogColors.RESET}"

        # 使用父类的format方法来完成格式化
        return super().format(new_record)


# 自定义 Logger 类
class Logger:
    def __init__(self, logger_style=LogStyle.STANDARD):
        unique_name = __name__ + '.' + str(uuid.uuid4())
        self.logger = logging.getLogger(unique_name)
        self.logger.setLevel(logging.DEBUG)

        # 控制台输出
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.DEBUG)
        if logger_style == LogStyle.STANDARD:
            ch.setFormatter(StandardFormatter())
        elif logger_style == LogStyle.NO_DEBUG_INFO:
            ch.setFormatter(NoDebugInfoFormatter())
        self.logger.addHandler(ch)

    def _log(self, level, msg, *args, **kwargs):
        """
        Generic log function that updates the record with the caller's information.
        """
        if self.logger.isEnabledFor(level):
            # Adjusted to capture the actual caller information
            frame = sys._getframe(2)
            kwargs["extra"] = {
                "custom_filename": os.path.basename(frame.f_code.co_filename),
                "custom_lineno": frame.f_lineno,
                "custom_funcName": frame.f_code.co_name
            }
            self.logger.log(level, msg, *args, **kwargs)

    def debug(self, message, *args, **kwargs):
        self._log(logging.DEBUG, message, *args, **kwargs)

    def info(self, message, *args, **kwargs):
        self._log(logging.INFO, message, *args, **kwargs)

    def warn(self, message, *args, **kwargs):
        self._log(logging.WARNING, message, *args, **kwargs)

    def error(self, message, *args, **kwargs):
        self._log(logging.ERROR, message, *args, **kwargs)

    def fatal(self, message, *args, **kwargs):
        self._log(logging.CRITICAL, message, *args, **kwargs)


# 测试日志函数
def test_logging():
    logger = Logger()
    logger.debug('This is a debug message.')
    logger.info('This is an info message.')
    logger.warn('This is a warning message.')
    logger.error('This is an error message.')
    logger.fatal('This is a fatal message.')

    logger = Logger(LogStyle.NO_DEBUG_INFO)
    logger.debug('This is a debug message.')
    logger.info('This is an info message.')
    logger.warn('This is a warning message.')
    logger.error('This is an error message.')
    logger.fatal('This is a fatal message.')


if __name__ == "__main__":
    test_logging()
