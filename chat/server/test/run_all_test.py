import argparse
import sys
import time
import unittest

from gate_server_test import GateServerTest
from status_server_test import StatusServerTest
from chat_server_test import ChatServerTest

from test_define import *


if __name__ == '__main__':
    # 创建解析器
    parser = argparse.ArgumentParser(add_help=False)  # 禁用帮助信息，以免与 unittest 冲突
    parser.add_argument('--log_dir', help='Log dir for server process output')

    # 解析已知的自定义参数，未知的留给 unittest
    args, remaining_argv = parser.parse_known_args()

    log_dir = 'stdout'
    if args.log_dir:
        log_dir = args.log_dir

    # 启动服务器
    server_processes = start_server([
        Server.kGateServer,
        Server.kStatusServer,
        Server.kChatServer1,
        Server.kChatServer2,
    ], log_dir)

    time.sleep(1)  # 给服务器足够时间启动

    # 删除数据库中的测试用户
    config: configparser.ConfigParser = read_config()
    db = Database(config)
    cursor = db.connection.cursor()
    cursor.execute('DELETE FROM user WHERE email LIKE "pycstl_%@test.com"')
    cursor.execute('TRUNCATE TABLE friend')
    cursor.execute('TRUNCATE TABLE friend_apply')
    db.connection.commit()  # 确认更改
    print("Db Clear success")

    try:
        # 利用剩余的 argv 运行 unittest
        unittest.main(argv=sys.argv[:1] + remaining_argv)

    except Error as err:
        print("Error: ", err)

    finally:
        # 关闭服务器
        terminate_server(server_processes)
