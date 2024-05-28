import os
import shlex
import subprocess
import unittest

USERNAME_SIZE = 32
EMAIL_SIZE = 255
MAX_CELLS_PER_PAGE = 13
MAX_PAGES = 100
MAX_CELLS = MAX_CELLS_PER_PAGE * MAX_PAGES


class TestDatabase(unittest.TestCase):
    @classmethod
    def setUp(self):
        # 确保每次运行测试类前，测试数据库文件不在
        if os.path.exists('test.db'):
            os.remove('test.db')

    def run_script(self, commands):
        process = subprocess.Popen(shlex.split("tiny_db/tiny_db test.db"),
                                   stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        output, _ = process.communicate(
            input='\n'.join(commands).encode('utf-8'))
        return output.decode().splitlines()

    def test_exit_and_unrecognized_command(self):
        result = self.run_script([
            "hello world",
            ".HELLO WORLD",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > Unrecognized keyword at start of 'hello world'.",
            "db > Unrecognized command: .HELLO WORLD",
            "db > Bye!",
        ])

    @unittest.skip("测试用例过期")
    def test_insert_and_select(self):
        result = self.run_script([
            "insert 1 user1",
            "select",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > Executing insert statement",
            "db > Executing select statement",
            "db > Bye!",
        ])

    def test_inserts_and_retrieves_a_row(self):
        result = self.run_script([
            "insert 1 user1 person1@example.com",
            "insert 2 user2",
            "select",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > Executed.",
            "db > Syntax error. Could not parse statement.",
            "db > (1, user1, person1@example.com)",
            "Executed.",
            "db > Bye!",
        ])

    def test_prints_error_message_when_table_is_full(self):
        """测试 table 满了之后的错误提示"""

        # TODO: 先测试 page 满了之后不能插入
        commands = [
            f"insert {i} user{i} person{i}@example.com" for i in range(MAX_CELLS_PER_PAGE + 1)]
        commands.append(".exit")
        result = self.run_script(commands)

        expect = ["db > Executed." for _ in range(MAX_CELLS_PER_PAGE)]
        expect.append("db > Error: Table full.")
        expect.append("db > Bye!")

        self.assertEqual(result, expect)

    def test_allows_inserting_strings_that_are_the_maximum_length(self):
        """输入最长的用户名和邮箱"""

        long_username = "a" * USERNAME_SIZE
        long_email = "a" * EMAIL_SIZE

        result = self.run_script([
            f"insert 1 {long_username} {long_email}",
            "select",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > Executed.",
            f"db > (1, {long_username}, {long_email})",
            "Executed.",
            "db > Bye!",
        ])

    def test_prints_error_message_if_strings_are_too_long(self):
        """测试插入超过长度的用户名和邮箱"""

        long_username = "a" * (USERNAME_SIZE + 1)
        long_email = "a" * (EMAIL_SIZE + 1)

        result = self.run_script([
            f"insert 1 {long_username} person1@example.com",
            f"insert 1 user1 {long_email}",
            "select",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > String is too long.",
            "db > String is too long.",
            "db > Executed.",
            "db > Bye!",
        ])

    def test_prints_an_error_message_if_id_is_negative(self):
        """插入id为负数"""

        result = self.run_script([
            "insert -1 cstack foo@bar.com",
            "select",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > ID must be positive.",
            "db > Executed.",
            "db > Bye!",
        ])

    def test_keeps_data_after_closing_connection(self):
        """测试数据持久化功能"""

        result1 = self.run_script([
            "insert 1 user1 person1@example.com",
            ".exit",
        ])
        self.assertEqual(result1, [
            "db > Executed.",
            "db > Bye!",
        ])

        result2 = self.run_script([
            "select",
            ".exit",
        ])
        self.assertEqual(result2, [
            "db > (1, user1, person1@example.com)",
            "Executed.",
            "db > Bye!",
        ])

    def test_print_constants(self):
        """测试打印常量"""

        script = [
            ".constants",
            ".exit",
        ]
        result = self.run_script(script)

        expected = [
            "db > Constants:",
            "  kRowSize: 296",
            "  kHeadSize: 24",
            "  kCellSize: 300",
            "  kSpaceForCells: 4072",
            "  kMaxCells: 13",
            "db > Bye!",
        ]
        self.assertEqual(result, expected)

    def test_print_btree_structure(self):
        """测试排序打印 btree 结构"""

        script = [
            f"insert {i} user{i} person{i}@example.com" for i in [3, 1, 2]
        ]
        script.append(".btree")
        script.append(".exit")
        result = self.run_script(script)

        expected = [
            "db > Executed.",
            "db > Executed.",
            "db > Executed.",
            "db > Tree:",
            "leaf (size 3)",
            "  - 0 : 1",
            "  - 1 : 2",
            "  - 2 : 3",
            "db > Bye!",
        ]
        self.assertEqual(result, expected)

    def test_prints_an_error_message_if_there_is_a_duplicate_id(self):
        """测试插入重复id"""
        script = [
            "insert 1 user1 person1@example.com",
            "insert 1 user1 person1@example.com",
            "select",
            ".exit",
        ]
        result = self.run_script(script)

        expected = [
            "db > Executed.",
            "db > Error: Duplicate key.",
            "db > (1, user1, person1@example.com)",
            "Executed.",
            "db > Bye!",
        ]
        self.assertEqual(result, expected)


if __name__ == '__main__':
    unittest.main()
