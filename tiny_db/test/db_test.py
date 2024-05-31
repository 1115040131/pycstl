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

        # TODO: 拆分 internal 节点
        script = [
            f"insert {i} user{i} person{i}@example.com" for i in range(1, 36)]
        script.append(".exit")
        result = self.run_script(script)

        expected = [
            "db > Executed.",
            "db > Need to implement splitting internal node."
        ]
        self.assertEqual(result[-2:], expected)

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
            "- leaf (size 3)",
            "  - 1",
            "  - 2",
            "  - 3",
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

    def test_prints_structure_of_a_3_leaf_node_btree(self):
        """测试3个叶子节点的 btree 结构"""

        script = [
            f"insert {i} user{i} person{i}@example.com" for i in range(1, 15)]
        script.append(".btree")
        script.append("insert 15 user15 person15@example.com")
        script.append(".btree")
        script.append(".exit")
        result = self.run_script(script)

        expected = [
            "db > Tree:",
            "- internal (size 1)",
            "  - leaf (size 7)",
            "    - 1",
            "    - 2",
            "    - 3",
            "    - 4",
            "    - 5",
            "    - 6",
            "    - 7",
            "  - key 7",
            "  - leaf (size 7)",
            "    - 8",
            "    - 9",
            "    - 10",
            "    - 11",
            "    - 12",
            "    - 13",
            "    - 14",
            "db > Executed.",
            "db > Tree:",
            "- internal (size 1)",
            "  - leaf (size 7)",
            "    - 1",
            "    - 2",
            "    - 3",
            "    - 4",
            "    - 5",
            "    - 6",
            "    - 7",
            "  - key 7",
            "  - leaf (size 8)",
            "    - 8",
            "    - 9",
            "    - 10",
            "    - 11",
            "    - 12",
            "    - 13",
            "    - 14",
            "    - 15",
            "db > Bye!"
        ]
        self.assertEqual(result[14:], expected)

    def test_print_all_rows_in_a_multi_level_tree(self):
        """测试多级 btree 结构 select 的结果"""

        script = [
            f"insert {i} user{i} person{i}@example.com" for i in range(1, 16)]
        script.append("select")
        script.append(".exit")
        result = self.run_script(script)

        expected_results = [
            "db > (1, user1, person1@example.com)",
            "(2, user2, person2@example.com)",
            "(3, user3, person3@example.com)",
            "(4, user4, person4@example.com)",
            "(5, user5, person5@example.com)",
            "(6, user6, person6@example.com)",
            "(7, user7, person7@example.com)",
            "(8, user8, person8@example.com)",
            "(9, user9, person9@example.com)",
            "(10, user10, person10@example.com)",
            "(11, user11, person11@example.com)",
            "(12, user12, person12@example.com)",
            "(13, user13, person13@example.com)",
            "(14, user14, person14@example.com)",
            "(15, user15, person15@example.com)",
            "Executed.",
            "db > Bye!"
        ]
        self.assertEqual(result[15:], expected_results)

    def test_allows_printing_out_the_structure_of_a_4_leaf_node_btree(self):
        """测试4个叶子节点的 btree 结构"""

        script = [
            "insert 18 user18 person18@example.com",
            "insert 7 user7 person7@example.com",
            "insert 10 user10 person10@example.com",
            "insert 29 user29 person29@example.com",
            "insert 23 user23 person23@example.com",
            "insert 4 user4 person4@example.com",
            "insert 14 user14 person14@example.com",
            "insert 30 user30 person30@example.com",
            "insert 15 user15 person15@example.com",
            "insert 26 user26 person26@example.com",
            "insert 22 user22 person22@example.com",
            "insert 19 user19 person19@example.com",
            "insert 2 user2 person2@example.com",
            "insert 1 user1 person1@example.com",
            "insert 21 user21 person21@example.com",
            "insert 11 user11 person11@example.com",
            "insert 6 user6 person6@example.com",
            "insert 20 user20 person20@example.com",
            "insert 5 user5 person5@example.com",
            "insert 8 user8 person8@example.com",
            "insert 9 user9 person9@example.com",
            "insert 3 user3 person3@example.com",
            "insert 12 user12 person12@example.com",
            "insert 27 user27 person27@example.com",
            "insert 17 user17 person17@example.com",
            "insert 16 user16 person16@example.com",
            "insert 13 user13 person13@example.com",
            "insert 24 user24 person24@example.com",
            "insert 25 user25 person25@example.com",
            "insert 28 user28 person28@example.com",
            ".btree",
            ".exit",
        ]
        result = self.run_script(script)

        expected_results = [
            "db > Tree:",
            "- internal (size 3)",
            "  - leaf (size 7)",
            "    - 1",
            "    - 2",
            "    - 3",
            "    - 4",
            "    - 5",
            "    - 6",
            "    - 7",
            "  - key 7",
            "  - leaf (size 8)",
            "    - 8",
            "    - 9",
            "    - 10",
            "    - 11",
            "    - 12",
            "    - 13",
            "    - 14",
            "    - 15",
            "  - key 15",
            "  - leaf (size 7)",
            "    - 16",
            "    - 17",
            "    - 18",
            "    - 19",
            "    - 20",
            "    - 21",
            "    - 22",
            "  - key 22",
            "  - leaf (size 8)",
            "    - 23",
            "    - 24",
            "    - 25",
            "    - 26",
            "    - 27",
            "    - 28",
            "    - 29",
            "    - 30",
            "db > Bye!",
        ]
        self.assertEqual(result[30:], expected_results)


if __name__ == '__main__':
    unittest.main()
