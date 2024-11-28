# chat_server_test.py

import json
import random
import requests
import socket
import struct
import time
import unittest

from test_define import *


class UserInfo:
    def __init__(self, uid: int, name: str, email: str, password: str):
        self.uid: int = uid
        self.name: str = name
        self.email: str = email
        self.password: str = password


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
        # 初始化配置解析器
        cls.config: configparser.ConfigParser = read_config()

        # 读取所有聊天服务器地址
        cls.chat_servers = get_chat_servers(cls.config)

        # 读取 gate server 地址
        gate_server_port = cls.config['GateServer']['Port']
        cls.gate_server_url = f'http://localhost:{gate_server_port}'

        # 创建 redis 连接
        cls.redis: redis.Redis = connect_redis(cls.config)

        # 注册账号并创建一个客户端连接
        cls.user_info = cls.register()
        cls.client_socket = cls.connect_server(cls.user_info)

    @classmethod
    def tearDownClass(cls):
        if cls.client_socket:
            cls.client_socket.close()

    @classmethod
    def register(cls) -> UserInfo:
        """
        注册账号
        """

        name = f'pycstl_{time.time()}'
        email = f'{name}@test.com'
        password = '123'
        verify_code = "qwer"
        cls.redis.set("code_" + email, verify_code)
        response = requests.post(cls.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': name,
                                       'email': email,
                                       'password': password,
                                       'confirm': password,
                                       'verify_code': verify_code})
        assert response.status_code == 200
        assert response.headers['Content-Type'] == 'text/json'
        json_response = response.json()
        assert json_response['error'] == ErrorCode.kSuccess.value
        assert json_response['user'] == name
        uid = json_response['uid']
        assert uid > 0

        return UserInfo(uid, name, email, password)

    @classmethod
    def get_server(cls, user_info: UserInfo):
        """
        获取服务器
        """

        response = requests.post(cls.gate_server_url + Url_Map[ReqId.kLogin],
                                 json={'email': user_info.email,
                                       'password': user_info.password})
        json_response = response.json()
        assert json_response['error'] == ErrorCode.kSuccess.value
        assert json_response['uid'] == user_info.uid
        assert json_response['user'] == user_info.name
        assert [json_response['host'], json_response['port']] in cls.chat_servers.values()

        host = json_response['host']
        port = int(json_response['port'])
        token = json_response['token']

        return host, port, token

    @classmethod
    def connect_server(cls, user_info: UserInfo) -> socket.socket:
        """
        连接服务器
        """

        host, port, token = cls.get_server(user_info)
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((host, port))

        return client_socket

    @classmethod
    def login_server(cls, user_info: UserInfo) -> socket.socket:
        """
        登录服务器
        """

        host, port, token = cls.get_server(user_info)
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((host, port))

        response = Message.send_and_receive(client_socket, Message(
            ReqId.kChatLogin, json.dumps({'uid': user_info.uid,
                                            "token": token})))
        assert response.message_id == ReqId.kChatLoginRes
        json_response = json.loads(response.message_body)
        assert json_response['error'] == ErrorCode.kSuccess.value
        assert json_response['token'] == token
        assert json_response['base_info']['uid'] == user_info.uid
        assert json_response['base_info']['name'] == user_info.name
        server_name = cls.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info.uid}').decode('utf-8')
        assert server_name in cls.chat_servers.keys()

        return client_socket

    # @unittest.skip("tmp skip")
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
        user_info = self.register()

        # 获取连接
        host, port, token = self.get_server(user_info)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            # 连接服务器
            client_socket.connect((host, port))

            # json 错误
            response = Message.send_and_receive(client_socket, Message(ReqId.kChatLogin, 'hello'))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

            # uid 无效
            response = Message.send_and_receive(client_socket, Message(ReqId.kChatLogin, json.dumps({'uid': 0})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

            # token 无效
            response = Message.send_and_receive(client_socket, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info.uid})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kTokenInvalid.value)

            # 登录前无缓存
            self.assertIsNone(self.redis.get(f'{RedisKey.kUserBaseInfo.value}{user_info.uid}'))

            # 登录成功
            response = Message.send_and_receive(client_socket, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info.uid,
                                              "token": token})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_response['token'], token)
            self.assertEqual(json_response['base_info']['uid'], user_info.uid)
            self.assertEqual(json_response['base_info']['name'], user_info.name)
            server_name = self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info.uid}').decode('utf-8')
            self.assertIn(server_name, self.chat_servers.keys())

            # 登录后有缓存
            cache = json.loads(self.redis.get(f'{RedisKey.kUserBaseInfo.value}{user_info.uid}').decode('utf-8'))
            self.assertEqual(cache['uid'], user_info.uid)
            self.assertEqual(cache['name'], user_info.name)
            self.assertEqual(cache['email'], user_info.email)
            self.assertEqual(cache['password'], user_info.password)

        # 登录人数变化
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer1", 0)
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer2", 0)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect((self.config["ChatServer1"]["Host"], int(self.config["ChatServer1"]["Port"])))

            response = Message.send_and_receive(client_socket, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info.uid,
                                              "token": token})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(int(self.redis.hget(RedisKey.kLoginCount.value, "ChatServer1").decode('utf-8')), 1)
            self.assertEqual(int(self.redis.hget(RedisKey.kLoginCount.value, "ChatServer2").decode('utf-8')), 0)

            # TODO: 重复登录
            response = Message.send_and_receive(client_socket, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info.uid,
                                              "token": token})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(int(self.redis.hget(RedisKey.kLoginCount.value, "ChatServer1").decode('utf-8')), 2)
            self.assertEqual(int(self.redis.hget(RedisKey.kLoginCount.value, "ChatServer2").decode('utf-8')), 0)

    def test_search_info(self):
        """
        搜索用户测试
        """

        # json 错误
        response = Message.send_and_receive(self.client_socket, Message(ReqId.kSearchUserReq, 'hello'))
        self.assertEqual(response.message_id, ReqId.kSearchUserRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # 用户不存在
        response = Message.send_and_receive(self.client_socket, Message(ReqId.kSearchUserReq, json.dumps({})))
        self.assertEqual(response.message_id, ReqId.kSearchUserRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

        response = Message.send_and_receive(self.client_socket, Message(ReqId.kSearchUserReq, json.dumps({"uid": "0"})))
        self.assertEqual(response.message_id, ReqId.kSearchUserRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

        # 按 id 找
        response = Message.send_and_receive(self.client_socket, Message(
            ReqId.kSearchUserReq, json.dumps({"uid": f'{self.user_info.uid}'})))
        self.assertEqual(response.message_id, ReqId.kSearchUserRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['search_info']['uid'], self.user_info.uid)
        self.assertEqual(json_response['search_info']['name'], self.user_info.name)
        self.assertEqual(json_response['search_info']['email'], self.user_info.email)
        self.assertEqual(json_response['search_info']['password'], self.user_info.password)

        # 按 name 找
        response = Message.send_and_receive(self.client_socket, Message(
            ReqId.kSearchUserReq, json.dumps({"uid": f'{self.user_info.uid}'})))
        self.assertEqual(response.message_id, ReqId.kSearchUserRes)
        json_response = json.loads(response.message_body)
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['search_info']['uid'], self.user_info.uid)
        self.assertEqual(json_response['search_info']['name'], self.user_info.name)
        self.assertEqual(json_response['search_info']['email'], self.user_info.email)
        self.assertEqual(json_response['search_info']['password'], self.user_info.password)

    def test_add_friend(self):
        """
        添加好友测试
        """

        # 将两个服务器人数都设置为 0, 确保 client1, client3 在一个服务器上, client2 在另一个服务器上
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer1", 0)
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer2", 0)

        # 注册账号
        user_info_1 = self.register()
        user_info_2 = self.register()
        user_info_3 = self.register()
        user_info_4 = self.register()

        with self.login_server(user_info_1) as client_1, self.login_server(user_info_2) as client_2, self.login_server(user_info_3) as client_3:
            # 确保用户 1 和 3 在同一个服务器上
            self.assertEqual(self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_1.uid}'),
                             self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_3.uid}'))
            self.assertNotEqual(self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_1.uid}'),
                                self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_2.uid}'))
            self.assertIsNone(self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_4.uid}'))

            # json 错误
            response = Message.send_and_receive(client_1, Message(ReqId.kAddFriendReq, 'hello'))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

            # uid 错误, 自己的基本信息查询失败
            response = Message.send_and_receive(client_1, Message(ReqId.kAddFriendReq, json.dumps({"uid": 0})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

            # 对方不在线, 查询对方服务器地址失败
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAddFriendReq, json.dumps({"uid": user_info_1.uid, "to_uid": user_info_4.uid})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)

            # 对方在同一个服务器上
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAddFriendReq, json.dumps({"uid": user_info_1.uid, "to_uid": user_info_3.uid, "apply_name": user_info_1.name})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)

            notify = Message.receive(client_3)
            self.assertEqual(notify.message_id, ReqId.kNotifyAddFriendReq)
            json_notify = json.loads(notify.message_body)
            self.assertEqual(json_notify['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_notify['apply_uid'], user_info_1.uid)
            self.assertEqual(json_notify['apply_name'], user_info_1.name)

            # 对方在不同服务器上
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAddFriendReq, json.dumps({"uid": user_info_1.uid, "to_uid": user_info_2.uid, "apply_name": user_info_1.name})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)

            notify = Message.receive(client_2)
            self.assertEqual(notify.message_id, ReqId.kNotifyAddFriendReq)
            json_notify = json.loads(notify.message_body)
            self.assertEqual(json_notify['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_notify['apply_uid'], user_info_1.uid)
            self.assertEqual(json_notify['apply_name'], user_info_1.name)

        # 测试登录后的好友申请列表
        with self.connect_server(user_info_2) as client_2:
            token = self.redis.get(f'{RedisKey.kUserTokenPrefix.value}{user_info_2.uid}').decode('utf-8')
            response = Message.send_and_receive(client_2, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info_2.uid,
                                                "token": token})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(len(json_response['apply_list']), 1)
            self.assertEqual(json_response['apply_list'][0]['uid'], user_info_1.uid)
            self.assertEqual(json_response['apply_list'][0]['name'], user_info_1.name)

    def test_auth_friend(self):
        """
        同意好友测试
        """

        # 将两个服务器人数都设置为 0, 确保 client1, client3 在一个服务器上, client2 在另一个服务器上
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer1", 0)
        self.redis.hset(RedisKey.kLoginCount.value, "ChatServer2", 0)

        # 注册账号
        user_info_1 = self.register()
        user_info_2 = self.register()
        user_info_3 = self.register()

        with self.login_server(user_info_1) as client_1, self.login_server(user_info_2) as client_2, self.login_server(user_info_3) as client_3:
            # 确保用户 1 和 3 在同一个服务器上
            self.assertEqual(self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_1.uid}'),
                             self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_3.uid}'))
            self.assertNotEqual(self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_1.uid}'),
                                self.redis.get(f'{RedisKey.kUserIpPrefix.value}{user_info_2.uid}'))

            # json 错误
            response = Message.send_and_receive(client_1, Message(ReqId.kAuthFriendReq, 'hello'))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

            # uid 错误, 自己的基本信息查询失败
            response = Message.send_and_receive(client_1, Message(ReqId.kAuthFriendReq, json.dumps({"uid": 0})))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kUidInvalid.value)

            # 对方未申请好友, 同意添加好友失败
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAuthFriendReq, json.dumps({"from_uid": user_info_1.uid, "to_uid": user_info_2.uid})))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kNetworkError.value)

            # 申请好友
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAddFriendReq, json.dumps({"uid": user_info_1.uid, "to_uid": user_info_2.uid, "apply_name": user_info_1.name})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)

            response = Message.send_and_receive(client_1, Message(
                ReqId.kAddFriendReq, json.dumps({"uid": user_info_1.uid, "to_uid": user_info_3.uid, "apply_name": user_info_1.name})))
            self.assertEqual(response.message_id, ReqId.kAddFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)

            # 客户端 2, 3 先读取好友申请的数据
            Message.receive(client_2)
            Message.receive(client_3)

            # 己方同意好友失败, 需对方同意
            response = Message.send_and_receive(client_1, Message(
                ReqId.kAuthFriendReq, json.dumps({"from_uid": user_info_1.uid, "to_uid": user_info_2.uid})))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kNetworkError.value)

            # client_2 同意好友申请
            response = Message.send_and_receive(client_2, Message(
                ReqId.kAuthFriendReq, json.dumps({"from_uid": user_info_2.uid, "to_uid": user_info_1.uid})))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_response['to_uid'], user_info_1.uid)
            self.assertEqual(json_response['name'], user_info_1.name)

            # client_1 收到通知
            notify = Message.receive(client_1)
            self.assertEqual(notify.message_id, ReqId.kNotifyAuthFriendReq)
            json_notify = json.loads(notify.message_body)
            self.assertEqual(json_notify['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_notify['from_uid'], user_info_2.uid)
            self.assertEqual(json_notify['name'], user_info_2.name)

            # client_3 同意好友申请
            response = Message.send_and_receive(client_3, Message(
                ReqId.kAuthFriendReq, json.dumps({"from_uid": user_info_3.uid, "to_uid": user_info_1.uid})))
            self.assertEqual(response.message_id, ReqId.kAuthFriendRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_response['to_uid'], user_info_1.uid)
            self.assertEqual(json_response['name'], user_info_1.name)

            # client_1 收到通知
            notify = Message.receive(client_1)
            self.assertEqual(notify.message_id, ReqId.kNotifyAuthFriendReq)
            json_notify = json.loads(notify.message_body)
            self.assertEqual(json_notify['error'], ErrorCode.kSuccess.value)
            self.assertEqual(json_notify['from_uid'], user_info_3.uid)
            self.assertEqual(json_notify['name'], user_info_3.name)

        # 测试登录后的好友列表
        with self.connect_server(user_info_1) as client_1:
            token = self.redis.get(f'{RedisKey.kUserTokenPrefix.value}{user_info_1.uid}').decode('utf-8')
            response = Message.send_and_receive(client_1, Message(
                ReqId.kChatLogin, json.dumps({'uid': user_info_1.uid,
                                                "token": token})))
            self.assertEqual(response.message_id, ReqId.kChatLoginRes)
            json_response = json.loads(response.message_body)
            self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
            self.assertEqual(len(json_response['friend_list']), 2)
            self.assertEqual(json_response['friend_list'][0]['uid'], user_info_2.uid)
            self.assertEqual(json_response['friend_list'][0]['name'], user_info_2.name)
            self.assertEqual(json_response['friend_list'][1]['uid'], user_info_3.uid)
            self.assertEqual(json_response['friend_list'][1]['name'], user_info_3.name)


if __name__ == '__main__':
    unittest.main()
