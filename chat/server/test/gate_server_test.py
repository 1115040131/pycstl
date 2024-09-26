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


class GateServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 启动服务器
        gate_server_process = subprocess.Popen(['chat/server/gate_server/gate_server'])
        # 防止邮件过多, 不启动验证服务器
        # verify_server_process = subprocess.Popen(['chat/server/verify_server/verify_server_/verify_server'])
        cls.server_process = [
            gate_server_process,
            # verify_server_process
        ]

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
        for process in cls.server_process:
            process.terminate()
            process.wait()

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
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kGetVerifyCode],
                                 data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # email key 不存在
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kGetVerifyCode], json='')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        response = requests.post(self.gate_server_url + Url_Map[ReqId.kGetVerifyCode],
                                 json={'email': 'test_email@pyc.com'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kRpcFailed.value)
        # self.assertEqual(json_response['email'], 'test_email@pyc.com')

    def test_user_register(self):
        # json 解析错误
        incorrect_json = "{'this is not': 'a valid json'}"  # 错误：属性名没有使用双引号
        # 设置请求头来指明发送的是JSON数据
        headers = {'Content-Type': 'application/json'}
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # 测试邮箱
        email = 'pyc@stl.test'

        # 确认密码不一致
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'email': email,
                                       'password': '123',
                                       "confirm": '456'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kPasswordError.value)

        # 验证码不存在或超时
        self.redis.delete("code_" + email)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser], json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyExpired.value)

        # 验证码错误
        verify_code = "qwer"
        self.redis.set("code_" + email, verify_code)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser], json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyCodeError.value)

        # 测试账户
        user1 = f'pycstl_{time.time()}'
        email1 = f'{user1}@test.com'
        user2 = f'pycstl_{time.time()}'
        email2 = f'{user2}@test.com'
        self.redis.set("code_" + email1, verify_code)
        self.redis.set("code_" + email2, verify_code)

        # user1 注册成功
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user1,
                                       'email': email1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['user'], user1)
        self.assertEqual(json_response['email'], email1)
        self.assertTrue(json_response['uid'] > 0)
        self.assertEqual(json_response['verify_code'], verify_code)

        # 用户已存在
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user1,
                                       'email': email1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kUserExist.value)

        # 邮箱已存在
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user2,
                                       'email': email1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kUserExist.value)

        # user2 注册成功
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user2,
                                       'email': email2,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['user'], user2)
        self.assertEqual(json_response['email'], email2)
        self.assertTrue(json_response['uid'] > 0)
        self.assertEqual(json_response['verify_code'], verify_code)

    def test_reset_pwd(self):
        # json 解析错误
        incorrect_json = "{'this is not': 'a valid json'}"  # 错误：属性名没有使用双引号
        # 设置请求头来指明发送的是JSON数据
        headers = {'Content-Type': 'application/json'}
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kJsonError.value)

        # 测试账户
        user = f'pycstl_{time.time()}'
        email = f'{user}@test.com'
        password1 = '123'
        password2 = '456'

        # 验证码不存在或超时
        self.redis.delete("code_" + email)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword], json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyExpired.value)

        # 验证码错误
        verify_code = "qwer"
        self.redis.set("code_" + email, verify_code)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword], json={'email': email})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kVerifyCodeError.value)

        self.redis.set("code_" + email, verify_code)

        # 用户名和邮箱不匹配 (用户不存在)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 json={'user': user,
                                       'email': email,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kEmailNotMatch.value)

        # 注册账号
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kRegUser],
                                 json={'user': user,
                                       'email': email,
                                       'password': password1,
                                       'confirm': password1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['user'], user)
        self.assertTrue(json_response['uid'] > 0)
        self.assertEqual(json_response['email'], email)
        self.assertEqual(json_response['password'], password1)
        self.assertEqual(json_response['confirm'], password1)
        self.assertEqual(json_response['verify_code'], verify_code)

        # 用户名和邮箱不匹配
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 json={'user': 'wrong_user',
                                       'email': email,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kEmailNotMatch.value)

        # 用户名和邮箱不匹配
        wrong_email = 'wrong_email'
        self.redis.set("code_" + wrong_email, verify_code)
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 json={'user': user,
                                       'email': wrong_email,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kEmailNotMatch.value)

        # 密码相同, 更新失败
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 json={'user': user,
                                       'email': email,
                                       'password': password1,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kPasswordUpdateFail.value)

        # 密码不同, 更新成功
        response = requests.post(self.gate_server_url + Url_Map[ReqId.kResetPassword],
                                 json={'user': user,
                                       'email': email,
                                       'password': password2,
                                       'verify_code': verify_code})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], ErrorCode.kSuccess.value)
        self.assertEqual(json_response['user'], user)
        self.assertEqual(json_response['email'], email)
        self.assertEqual(json_response['password'], password2)
        self.assertEqual(json_response['verify_code'], verify_code)


if __name__ == '__main__':
    unittest.main()
