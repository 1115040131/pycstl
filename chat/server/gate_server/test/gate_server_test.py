# gate_server_test.py
import configparser
import redis
import requests
import subprocess
import time
import unittest

from enum import Enum


class ErrorCode(Enum):
    kSuccess = 0

    kJsonError = 1001
    kRpcFailed = 1002
    kVerifyExpired = 1003
    kVerifyCodeError = 1004
    kUserExist = 1005
    kPasswordError = 1006
    kEmailNotExist = 1007
    kPasswordUpdateFaile = 1008
    kPasswordInvalid = 1009

    kNetworkError = 1010


class GateServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 启动服务器
        cls.server_process = subprocess.Popen(['chat/server/gate_server/gate_server'])

        # 初始化配置解析器
        cls.config = configparser.ConfigParser()
        # 读取 ini 文件
        cls.config.read('chat/server/common/config/config.ini')

        gate_server_port = cls.config['GateServer']['Port']
        cls.gate_server_url = f'http://localhost:{gate_server_port}'

        # 创建 redis 连接
        cls.redis = redis.Redis(
            host=cls.config['Redis']['Host'],
            port=cls.config['Redis']['Port'],
            password=cls.config['Redis']['Password'],
        )

        time.sleep(1)  # 给服务器足够时间启动

    @classmethod
    def tearDownClass(cls):
        # 停止服务器
        cls.server_process.terminate()
        cls.server_process.wait()

    def test_not_found(self):
        # 不存在的 url
        response = requests.get(f'{self.gate_server_url}/not_found')
        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Url not found\r\n")

        response = requests.post(f'{self.gate_server_url}/not_found')
        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Url not found\r\n")

    def test_get_test(self):
        response = requests.get(f'{self.gate_server_url}/get_test')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n")

        # 带参数的 get 请求
        response = requests.get(f'{self.gate_server_url}/get_test?')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n")

        response = requests.get(f'{self.gate_server_url}/get_test?a=1')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\na: 1\n")

        response = requests.get(
            f'{self.gate_server_url}/get_test?a=1&=2&c=&d=+4&%E4%BB%8A%E5%A4%A9=%E6%98%9F%E6%9C%9F%E4%B8%80')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n今天: 星期一\nd:  4\na: 1\n")

    def test_get_verifycode(self):
        # json 解析错误
        incorrect_json = "{'this is not': 'a valid json'}"  # 错误：属性名没有使用双引号
        # 设置请求头来指明发送的是JSON数据
        headers = {'Content-Type': 'application/json'}
        response = requests.post(f'{self.gate_server_url}/get_verifycode', data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # email key 不存在
        response = requests.post(f'{self.gate_server_url}/get_verifycode', json='')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        response = requests.post(f'{self.gate_server_url}/get_verifycode', json={'email': 'test_email@pyc.com'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['email'], 'test_email@pyc.com')

    def test_user_register(self):
        # json 解析错误
        incorrect_json = "{'this is not': 'a valid json'}"  # 错误：属性名没有使用双引号
        # 设置请求头来指明发送的是JSON数据
        headers = {'Content-Type': 'application/json'}
        response = requests.post(f'{self.gate_server_url}/user_register', data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # 测试邮箱
        email = 'pyc@stl.test'

        # 确认密码不一致
        response = requests.post(f'{self.gate_server_url}/user_register', json={'email': email,
                                                                                'password': '123',
                                                                                "confirm": '456'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kPasswordError.value)

        # 验证码不存在或超时
        self.redis.delete("code_" + email)
        response = requests.post(f'{self.gate_server_url}/user_register', json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyExpired.value)

        # 验证码错误
        verify_code = "qwer"
        self.redis.set("code_" + email, verify_code)
        response = requests.post(f'{self.gate_server_url}/user_register', json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyCodeError.value)

        # 注册成功
        response = requests.post(f'{self.gate_server_url}/user_register', json={'email': email,
                                                                                'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['email'], email)
        self.assertEqual(json_response['verify_code'], verify_code)


if __name__ == '__main__':
    unittest.main()
