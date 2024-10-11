# chat_server_test.py

import json
import random
import requests
import socket
import struct
import time
import unittest

from test_define import *


class Message:
    HEADER_FORMAT = '>HH'  # 大端序，两个无符号短整型（2个字节的 message_id 和 2个字节的 message_len）

    def __init__(self, message_id: ReqId, message_body: str):
        self.message_id: ReqId = message_id
        self.message_body: str = message_body

    def encode(self):
        """将消息编码为字节串."""
        message_len = len(self.message_body)
        header = struct.pack(self.HEADER_FORMAT, self.message_id.value, message_len)
        return header + self.message_body.encode()

    @classmethod
    def decode(cls, message_bytes):
        """从字节串解码出消息."""
        header = message_bytes[:4]
        body = message_bytes[4:]

        message_id, message_len = struct.unpack(cls.HEADER_FORMAT, header)
        if message_len != len(body):
            raise ValueError("Message length does not match length specified in header.")

        return cls(ReqId(message_id), body.decode())

    @staticmethod
    def send(socket: socket.socket, message):
        """通过socket发送消息."""
        socket.sendall(message.encode())

    @staticmethod
    def receive(socket: socket.socket):
        """从socket接收消息."""
        header = socket.recv(4)
        if len(header) != 4:
            raise ValueError("Incomplete message header received.")

        message_id, message_len = struct.unpack(Message.HEADER_FORMAT, header)

        body = b''
        while len(body) < message_len:
            chunk = socket.recv(message_len - len(body))
            if not chunk:
                raise RuntimeError("Connection lost while receiving the message.")
            body += chunk

        return Message(ReqId(message_id), body.decode())

    @staticmethod
    def send_and_receive(socket: socket.socket, message):
        Message.send(socket, message)
        return Message.receive(socket)


class ChatServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 启动服务器
        cls.processes = start_server([
            Server.kGateServer,
            Server.kStatusServer,
            Server.kChatServer1,
            Server.kChatServer2,
        ])

        # 初始化配置解析器
        cls.config: configparser.ConfigParser = read_config()

        # 读取所有聊天服务器地址
        cls.chat_servers = get_chat_servers(cls.config)

        # 读取 gate server 地址
        gate_server_port = cls.config['GateServer']['Port']
        cls.gate_server_url = f'http://localhost:{gate_server_port}'

        # 创建 redis 连接
        cls.redis: redis.Redis = connect_redis(cls.config)

        time.sleep(1)  # 给服务器足够时间启动

    @classmethod
    def tearDownClass(cls):
        # 停止服务器
        terminate_server(cls.processes)

    def test_pack(self):
        """
        测试粘包处理
        """

        # 读取 chat server 地址
        chat_server_host = self.config['ChatServer1']['Host']
        chat_server_port = int(self.config['ChatServer1']['Port'])
        # 创建一个 socket 对象
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect((chat_server_host, chat_server_port))

            kRepeat = 100
            message = Message(ReqId.kChatLogin, json.dumps({'uid': 0})).encode()
            message = message * kRepeat

            # 全量发送 100 条消息
            client_socket.sendall(message)
            for _ in range(kRepeat):
                response = Message.receive(client_socket)
                self.assertEqual(response.message_id, ReqId.kChatLoginRes)
                self.assertEqual(json.loads(response.message_body)['error'], ErrorCode.kUidInvalid.value)

            # 100 条消息一次发送 1 个字节
            for byte in message:
                client_socket.sendall(bytes([byte]))
            for _ in range(kRepeat):
                response = Message.receive(client_socket)
                self.assertEqual(response.message_id, ReqId.kChatLoginRes)
                self.assertEqual(json.loads(response.message_body)['error'], ErrorCode.kUidInvalid.value)

            # 随机切割成 20 份并发送
            # 计算出20个随机长度的分片，总和等于消息长度
            lengths = [1] * 20  # 初始化为最小长度1，确保至少有20个分片
            remaining_length = len(message) - sum(lengths)
            while remaining_length > 0:
                index = random.randrange(0, 20)
                increment = min(random.randint(1, remaining_length), remaining_length)
                lengths[index] += increment
                remaining_length -= increment
            # 索引初始值
            start_index = 0
            # 按照每个随机长度分片并发送
            for length in lengths:
                end_index = start_index + length
                segment = message[start_index:end_index]  # 获取当前片段
                client_socket.send(segment)  # 发送片段
                start_index = end_index  # 更新起始索引
            for _ in range(kRepeat):
                response = Message.receive(client_socket)
                self.assertEqual(response.message_id, ReqId.kChatLoginRes)
                self.assertEqual(json.loads(response.message_body)['error'], ErrorCode.kUidInvalid.value)

    def test_login(self):
        """
        登录测试
        """

        # 注册账号
        user1 = f'pycstl_{time.time()}'
        email1 = f'{user1}@test.com'
        password1 = '123'
        verify_code = "qwer"
        self.redis.set("code_" + email1, verify_code)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user1,
                                       'email': email1,
                                       'password': password1,
                                       'confirm': password1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['user'], user1)
        user1_uid = json_response['uid']
        self.assertTrue(user1_uid > 0)

        # 获取连接成功
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kLogin],
                                 json={'email': email1,
                                       'password': password1})
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['uid'], user1_uid)
        self.assertEqual(json_response['user'], user1)
        self.assertIn([json_response['host'], json_response['port']], self.chat_servers)
        host = json_response['host']
        port = int(json_response['port'])
        token = json_response['token']

        # 连接服务器
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket.connect((host, port))

        # json 错误
        response = Message.send_and_receive(self.client_socket, Message(ReqId.kChatLogin, 'hello'))
        self.assertEqual(response.message_id, ReqId.kChatLoginRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # uid 无效
        response = Message.send_and_receive(self.client_socket, Message(ReqId.kChatLogin, json.dumps({'uid': 0})))
        self.assertEqual(response.message_id, ReqId.kChatLoginRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

        # token 无效
        response = Message.send_and_receive(self.client_socket, Message(
            ReqId.kChatLogin, json.dumps({'uid': user1_uid})))
        self.assertEqual(response.message_id, ReqId.kChatLoginRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kTokenInvalid.value)

        # 登录成功
        response = Message.send_and_receive(self.client_socket, Message(
            ReqId.kChatLogin, json.dumps({'uid': user1_uid,
                                          "token": token})))
        self.assertEqual(response.message_id, ReqId.kChatLoginRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['uid'], user1_uid)
        self.assertEqual(json_response['token'], token)
        self.assertEqual(json_response['name'], user1)


if __name__ == '__main__':
    unittest.main()
