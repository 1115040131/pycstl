import configparser
import redis
import subprocess


from enum import Enum


class ErrorCode(Enum):
    kSuccess = 0

    kJsonError = 1001
    kRpcFailed = 1002
    kVerifyExpired = 1003
    kVerifyCodeError = 1004
    kUserExist = 1005
    kPasswordError = 1006
    kEmailNotMatch = 1007
    kPasswordUpdateFail = 1008
    kPasswordInvalid = 1009

    kNetworkError = 1010


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
}


class Server(Enum):
    kGateServer = 1
    kVerifyServer = 2
    kStatusServer = 3


def start_server(servers: list[Server]) -> list[subprocess.Popen]:
    server_map = {
        Server.kGateServer: 'chat/server/gate_server/gate_server',
        Server.kStatusServer: 'chat/server/verify_server/verify_server_/verify_server',
        Server.kStatusServer: 'chat/server/status_server/status_server'
    }

    processes = []
    for server in servers:
        process = subprocess.Popen(server_map[server])
        processes.append(process)

    return processes


def terminate_server(servers: list[subprocess.Popen]):
    for server in servers:
        server.terminate()
        server.wait()


def read_config() -> configparser.ConfigParser:
    # 初始化配置解析器
    config = configparser.ConfigParser()
    # 读取 ini 文件
    config.read('chat/server/common/config/config.ini')

    return config


def connect_redis(config: configparser.ConfigParser) -> redis.Redis:
    return redis.Redis(
        host=config['Redis']['Host'],
        port=config['Redis']['Port'],
        password=config['Redis']['Password'],
    )
