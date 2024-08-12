# gate_server_test.py
import requests
import subprocess
import time
import unittest


class GateServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # 启动服务器
        cls.server_process = subprocess.Popen(['chat/server/gate_server/gate_server'])
        time.sleep(1)  # 给服务器足够时间启动

    @classmethod
    def tearDownClass(cls):
        # 停止服务器
        cls.server_process.terminate()
        cls.server_process.wait()

    def test_not_found(self):
        # 不存在的 url
        response = requests.get('http://localhost:8080/not_found')
        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Url not found\r\n")

        response = requests.post('http://localhost:8080/not_found')
        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.headers['Content-Type'], 'text/plain')
        self.assertEqual(response.text, "Url not found\r\n")

    def test_get_test(self):
        response = requests.get('http://localhost:8080/get_test')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n")

        # 带参数的 get 请求
        response = requests.get('http://localhost:8080/get_test?')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n")

        response = requests.get('http://localhost:8080/get_test?a=1')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\na: 1\n")

        response = requests.get(
            'http://localhost:8080/get_test?a=1&=2&c=&d=+4&%E4%BB%8A%E5%A4%A9=%E6%98%9F%E6%9C%9F%E4%B8%80')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "receive get_test req\n今天: 星期一\nd:  4\na: 1\n")

    def test_get_varifycode(self):
        # json 解析错误
        incorrect_json = "{'this is not': 'a valid json'}"  # 错误：属性名没有使用双引号
        # 设置请求头来指明发送的是JSON数据
        headers = {'Content-Type': 'application/json'}
        response = requests.post('http://localhost:8080/get_varifycode', data=incorrect_json, headers=headers)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], 1)

        # email key 不存在
        response = requests.post('http://localhost:8080/get_varifycode', json='')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], 1)

        response = requests.post('http://localhost:8080/get_varifycode', json={'email': 'test_email@pyc.com'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/json')
        json_response = response.json()
        self.assertEqual(json_response['error'], 0)
        self.assertEqual(json_response['email'], 'test_email@pyc.com')


if __name__ == '__main__':
    unittest.main()
