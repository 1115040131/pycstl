import subprocess
import unittest


class TestDatabase(unittest.TestCase):
    def run_script(self, commands):
        process = subprocess.Popen(
            "tiny_db/tiny_db", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
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

        commands = [
            f"insert {i} user{i} person{i}@example.com" for i in range(1, 1402)]
        commands.append(".exit")
        result = self.run_script(commands)

        expect = ["db > Executed." for i in range(1, 1401)]
        expect.append("db > Error: Table full.")
        expect.append("db > Bye!")

        self.assertEqual(result, expect)

    def test_allows_inserting_strings_that_are_the_maximum_length(self):
        """输入最长的用户名和邮箱"""

        long_username = "a" * 31
        long_email = "a" * 255

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

        long_username = "a" * 32
        long_email = "a" * 256

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


if __name__ == '__main__':
    unittest.main()
