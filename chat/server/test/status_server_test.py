# status_server_test.py

import time
import unittest

import grpc
from status_pb2 import GetChatServerReq, LoginReq
from status_pb2_grpc import StatusServiceStub

from test_define import *


class StatusServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 初始化配置解析器
        cls.config: configparser.ConfigParser = read_config()

        # 读取所有聊天服务器地址
        cls.chat_servers = get_chat_servers(cls.config)

        # 初始化 grpc 客户端
        status_server_host = cls.config['StatusServer']['Host']
        status_server_port = cls.config['StatusServer']['Port']
        channel = grpc.insecure_channel(f'{status_server_host}:{status_server_port}')
        cls.client = StatusServiceStub(channel)

    @classmethod
    def tearDownClass(cls):
        pass

    def test_GetChatServer(self):
        request = GetChatServerReq(uid=12345)
        response = self.client.GetChatServer(request)
        self.assertEqual(response.error, ErrorCode.kSuccess.value)
        self.assertIn([response.host, response.port], self.chat_servers)
        print(response.token)

        response = self.client.GetChatServer(request)
        print(response.token)

    def test_Login(self):
        # 无效的 uid
        uid = 1234567
        request = LoginReq(uid=uid, token='token')
        response = self.client.Login(request)
        self.assertEqual(response.error, ErrorCode.kUidInvalid.value)

        # 分配聊天服务器获取 token
        request = GetChatServerReq(uid=uid)
        response = self.client.GetChatServer(request)
        self.assertEqual(response.error, ErrorCode.kSuccess.value)
        self.assertIn([response.host, response.port], self.chat_servers)
        token = response.token

        # 无效的 token
        request = LoginReq(uid=uid, token='token')
        response = self.client.Login(request)
        self.assertEqual(response.error, ErrorCode.kTokenInvalid.value)

        # 登录成功
        request = LoginReq(uid=uid, token=token)
        response = self.client.Login(request)
        self.assertEqual(response.error, ErrorCode.kSuccess.value)
        self.assertEqual(response.uid, uid)
        self.assertEqual(response.token, token)


if __name__ == '__main__':
    unittest.main()
