# status_server_test.py

import time
import unittest

import grpc
from status_pb2 import GetChatServerReq
from status_pb2_grpc import StatusServiceStub

from test_define import *


class StatusServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 启动服务器
        cls.processes = start_server([
            Server.kStatusServer
        ])

        # 初始化配置解析器
        cls.config: configparser.ConfigParser = read_config()

        # 读取所有聊天服务器地址
        cls.chat_servers = get_chat_servers(cls.config)

        # 初始化 grpc 客户端
        status_server_host = cls.config['StatusServer']['Host']
        status_server_port = cls.config['StatusServer']['Port']
        channel = grpc.insecure_channel(f'{status_server_host}:{status_server_port}')
        cls.client = StatusServiceStub(channel)

        # 给服务器足够时间启动
        time.sleep(1)

    @classmethod
    def tearDownClass(cls):
        # 停止服务器
        terminate_server(cls.processes)

    def test_GetChatServer(self):
        request = GetChatServerReq(uid=12345)
        response = self.client.GetChatServer(request)
        self.assertEqual(response.error, ErrorCode.kSuccess.value)
        self.assertIn([response.host, response.port], self.chat_servers)
        print(response.token)

        response = self.client.GetChatServer(request)
        print(response.token)


if __name__ == '__main__':
    unittest.main()
