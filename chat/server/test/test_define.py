import configparser
import redis
import shlex
import subprocess


from enum import Enum


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


def start_server(servers: list[Server]) -> list[subprocess.Popen]:
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

    processes = []
    for server in servers:
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

    chat_servers = []
    for chat_server_name in ['ChatServer1', 'ChatServer2']:
        chat_servers.append([config[chat_server_name]['Host'], config[chat_server_name]['Port']])
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
