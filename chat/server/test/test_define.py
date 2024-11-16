import configparser
import os
import redis
import shlex
import subprocess

from enum import Enum

import mysql.connector
from mysql.connector import Error


class ErrorCode(Enum):
    kSuccess = 0

    kJsonError = 1001     # json 解析失败
    kRpcFailed = 1002     # rpc 调用失败
    kNetworkError = 1003  # 网络错误

    # Gate Server
    kVerifyExpired = 2001       # 验证码过期
    kVerifyCodeError = 2002     # 验证码错误
    kUserExist = 2003           # 用户已存在
    kPasswordError = 2004       # 密码错误
    kEmailNotMatch = 2005       # 邮箱不匹配
    kPasswordUpdateFail = 2006  # 密码更新失败
    kPasswordInvalid = 2007     # 密码不合法

    # Status Server
    kUidInvalid = 3001    # uid 无效
    kTokenInvalid = 3002  # token 无效


class ReqId(Enum):
    kGetVerifyCode = 1001
    kRegUser = 1002
    kResetPassword = 1003
    kLogin = 1004
    kChatLogin = 1005
    kChatLoginRes = 1006


Url_Map = {
    ReqId.kGetVerifyCode: '/get_verifycode',
    ReqId.kRegUser: '/user_register',
    ReqId.kResetPassword: '/reset_password',
    ReqId.kLogin: '/user_login',
}


class Server(Enum):
    kGateServer = 1
    kVerifyServer = 2
    kStatusServer = 3
    kChatServer1 = 4
    kChatServer2 = 5


class RedisKey(Enum):
    kLoginCount = 'login_count'
    kUserTokenPrefix = 'utoken_'
    kUserBaseInfo = "ubaseinfo_"
    kUserIpPrefix = "uip_"


def start_server(servers: list[Server], log_dir='stdout') -> list[subprocess.Popen]:
    """
    启动服务器进程
    """

    server_map = {
        Server.kGateServer: 'chat/server/gate_server/gate_server',
        Server.kVerifyServer: 'chat/server/verify_server/verify_server_/verify_server',
        Server.kStatusServer: 'chat/server/status_server/status_server',
        Server.kChatServer1: 'chat/server/chat_server/chat_server ChatServer1',
        Server.kChatServer2: 'chat/server/chat_server/chat_server ChatServer2',
    }

    if log_dir != 'stdout':
        # 确保日志目录存在
        os.makedirs(log_dir, exist_ok=True)

    processes = []
    for server in servers:
        if log_dir != 'stdout':
            # 定义日志文件路径
            log_file_path = os.path.join(log_dir, f'{server.name}.log')
            # print(os.path.abspath(log_file_path))

            # 打开日志文件
            with open(log_file_path, 'a') as log_file:
                # 启动进程并重定向 stdout 和 stderr 到日志文件
                process = subprocess.Popen(
                    shlex.split(server_map[server]),
                    stdout=log_file,
                    stderr=subprocess.STDOUT
                )
                processes.append(process)
        else:
            process = subprocess.Popen(shlex.split(server_map[server]))
            processes.append(process)

    return processes


def terminate_server(servers: list[subprocess.Popen]):
    """
    结束服务器进程
    """

    for server in servers:
        server.terminate()
        server.wait()


def read_config() -> configparser.ConfigParser:
    """
    读取配置
    """

    # 初始化配置解析器
    config = configparser.ConfigParser()
    # 读取 ini 文件
    config.read('chat/server/common/config/config.ini')

    return config


def get_chat_servers(config: configparser.ConfigParser):
    """
    读取所有聊天服务器地址
    """

    chat_servers = {}
    for chat_server_name in ['ChatServer1', 'ChatServer2']:
        chat_servers[chat_server_name] = [config[chat_server_name]
                                          ['Host'], config[chat_server_name]['Port']]
    return chat_servers


def connect_redis(config: configparser.ConfigParser) -> redis.Redis:
    """
    连接 redis 服务器
    """

    return redis.Redis(
        host=config['Redis']['Host'],
        port=config['Redis']['Port'],
        password=config['Redis']['Password'],
    )


class Database:
    def __init__(self, config: configparser.ConfigParser):
        # 初始化数据库连接
        self.connection = None
        try:
            self.connection = mysql.connector.connect(
                host=config['Mysql']['Host'],
                port=6306,
                user=config['Mysql']['User'],
                password=config['Mysql']['Password'],
                database=config['Mysql']['Schema'])
        except Error as e:
            print(f"Error connecting to MySQL: {e}")

    def query(self, sql):
        # 执行SQL查询并返回结果
        cursor = self.connection.cursor()
        cursor.execute(sql)
        result = cursor.fetchall()
        cursor.close()
        return result

    def del_user(self, email):
        self.query(f'DELETE FROM user WHERE email = "{email}"')

    def close(self):
        # 关闭数据库连接
        if self.connection and self.connection.is_connected():
            self.connection.close()

    def __del__(self):
        # 确保在对象被销毁时关闭连接
        self.close()
