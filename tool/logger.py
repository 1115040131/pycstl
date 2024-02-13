import logging
import sys
import os

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

class CustomFormatter(logging.Formatter):
    # 重写 format 方法来添加颜色
    def format(self, record):
        levelname = record.levelname
        if len(levelname) > 5:  # 如果日志级别名称长度大于5，则截断
            levelname = levelname[:5]
        log_fmt = f"[%(asctime)s] [{levelname:<5}] <%(custom_filename)s:%(custom_lineno)d> [%(custom_funcName)s]: %(message)s"
        FORMATS = {
            logging.DEBUG: LogColors.BLUE + log_fmt + LogColors.RESET,
            logging.INFO: LogColors.GREEN + log_fmt + LogColors.RESET,
            logging.WARNING: LogColors.YELLOW + log_fmt + LogColors.RESET,
            logging.ERROR: LogColors.RED + log_fmt + LogColors.RESET,
            logging.CRITICAL: LogColors.BACKGROUND_RED + LogColors.WHITE + log_fmt + LogColors.RESET
        }
        formatter = logging.Formatter(FORMATS.get(record.levelno, log_fmt), "%Y-%m-%d %H:%M:%S")
        return formatter.format(record)

# 自定义 Logger 类
class Logger:

    def __init__(self):
        self.logger = logging.getLogger(__name__)
        self.logger.setLevel(logging.DEBUG)

        # 控制台输出
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.DEBUG)
        ch.setFormatter(CustomFormatter())
        self.logger.addHandler(ch)

    def _log(self, level, msg, *args, **kwargs):
        """
        Generic log function that updates the record with the caller's information.
        """
        if self.logger.isEnabledFor(level):
            frame = sys._getframe(2)  # Adjusted to capture the actual caller information
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
    # 使用 CustomLogger
    logger = Logger()

    logger.debug('This is a debug message.')
    logger.info('This is an info message.')
    logger.warn('This is a warning message.')
    logger.error('This is an error message.')
    logger.fatal('This is a fatal message.')


if __name__ == "__main__":
    test_logging()