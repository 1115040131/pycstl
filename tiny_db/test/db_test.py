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

        # TODO: 递归拆分 internal 节点
        script = [
            f"insert {i} user{i} person{i}@example.com" for i in range(1, 50)]
        script.append(".btree debug")
        script.append(".exit")
        result = self.run_script(script)

        expected = [
            "db > Executed.",
            "db > Need to implement splitting internal node recursively."
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
            "  kHeadSize: 20",
            "  kCellSize: 300",
            "  kSpaceForCells: 4076",
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
            "select",
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
            "(16, user16, person16@example.com)",
            "(17, user17, person17@example.com)",
            "(18, user18, person18@example.com)",
            "(19, user19, person19@example.com)",
            "(20, user20, person20@example.com)",
            "(21, user21, person21@example.com)",
            "(22, user22, person22@example.com)",
            "(23, user23, person23@example.com)",
            "(24, user24, person24@example.com)",
            "(25, user25, person25@example.com)",
            "(26, user26, person26@example.com)",
            "(27, user27, person27@example.com)",
            "(28, user28, person28@example.com)",
            "(29, user29, person29@example.com)",
            "(30, user30, person30@example.com)",
            "Executed.",
            "db > Bye!",
        ]
        self.assertEqual(result[30:], expected_results)

    def test_allows_printing_out_the_structure_of_a_7_leaf_node_btree(self):
        script = [
            "insert 58 user58 person58@example.com",
            "insert 56 user56 person56@example.com",
            "insert 8 user8 person8@example.com",
            "insert 54 user54 person54@example.com",
            "insert 77 user77 person77@example.com",
            "insert 7 user7 person7@example.com",
            "insert 25 user25 person25@example.com",
            "insert 71 user71 person71@example.com",
            "insert 13 user13 person13@example.com",
            "insert 22 user22 person22@example.com",
            "insert 53 user53 person53@example.com",
            "insert 51 user51 person51@example.com",
            "insert 59 user59 person59@example.com",
            "insert 32 user32 person32@example.com",
            "insert 36 user36 person36@example.com",
            "insert 79 user79 person79@example.com",
            "insert 10 user10 person10@example.com",
            "insert 33 user33 person33@example.com",
            "insert 20 user20 person20@example.com",
            "insert 4 user4 person4@example.com",
            "insert 35 user35 person35@example.com",
            "insert 76 user76 person76@example.com",
            "insert 49 user49 person49@example.com",
            "insert 24 user24 person24@example.com",
            "insert 70 user70 person70@example.com",
            "insert 48 user48 person48@example.com",
            "insert 39 user39 person39@example.com",
            "insert 15 user15 person15@example.com",
            "insert 47 user47 person47@example.com",
            "insert 30 user30 person30@example.com",
            "insert 86 user86 person86@example.com",
            "insert 31 user31 person31@example.com",
            "insert 68 user68 person68@example.com",
            "insert 37 user37 person37@example.com",
            "insert 66 user66 person66@example.com",
            "insert 63 user63 person63@example.com",
            "insert 40 user40 person40@example.com",
            "insert 78 user78 person78@example.com",
            "insert 19 user19 person19@example.com",
            "insert 46 user46 person46@example.com",
            "insert 14 user14 person14@example.com",
            "insert 81 user81 person81@example.com",
            "insert 72 user72 person72@example.com",
            "insert 6 user6 person6@example.com",
            "insert 50 user50 person50@example.com",
            "insert 85 user85 person85@example.com",
            "insert 67 user67 person67@example.com",
            "insert 2 user2 person2@example.com",
            "insert 55 user55 person55@example.com",
            "insert 69 user69 person69@example.com",
            "insert 5 user5 person5@example.com",
            "insert 65 user65 person65@example.com",
            "insert 52 user52 person52@example.com",
            "insert 1 user1 person1@example.com",
            "insert 29 user29 person29@example.com",
            "insert 9 user9 person9@example.com",
            "insert 43 user43 person43@example.com",
            "insert 75 user75 person75@example.com",
            "insert 21 user21 person21@example.com",
            "insert 82 user82 person82@example.com",
            "insert 12 user12 person12@example.com",
            "insert 18 user18 person18@example.com",
            "insert 60 user60 person60@example.com",
            "insert 44 user44 person44@example.com",
            ".btree",
            ".exit",
        ]
        result = self.run_script(script)

        expected_results = [
            "db > Tree:",
            "- internal (size 1)",
            "  - internal (size 2)",
            "    - leaf (size 7)",
            "      - 1",
            "      - 2",
            "      - 4",
            "      - 5",
            "      - 6",
            "      - 7",
            "      - 8",
            "    - key 8",
            "    - leaf (size 11)",
            "      - 9",
            "      - 10",
            "      - 12",
            "      - 13",
            "      - 14",
            "      - 15",
            "      - 18",
            "      - 19",
            "      - 20",
            "      - 21",
            "      - 22",
            "    - key 22",
            "    - leaf (size 8)",
            "      - 24",
            "      - 25",
            "      - 29",
            "      - 30",
            "      - 31",
            "      - 32",
            "      - 33",
            "      - 35",
            "  - key 35",
            "  - internal (size 3)",
            "    - leaf (size 12)",
            "      - 36",
            "      - 37",
            "      - 39",
            "      - 40",
            "      - 43",
            "      - 44",
            "      - 46",
            "      - 47",
            "      - 48",
            "      - 49",
            "      - 50",
            "      - 51",
            "    - key 51",
            "    - leaf (size 11)",
            "      - 52",
            "      - 53",
            "      - 54",
            "      - 55",
            "      - 56",
            "      - 58",
            "      - 59",
            "      - 60",
            "      - 63",
            "      - 65",
            "      - 66",
            "    - key 66",
            "    - leaf (size 7)",
            "      - 67",
            "      - 68",
            "      - 69",
            "      - 70",
            "      - 71",
            "      - 72",
            "      - 75",
            "    - key 75",
            "    - leaf (size 8)",
            "      - 76",
            "      - 77",
            "      - 78",
            "      - 79",
            "      - 81",
            "      - 82",
            "      - 85",
            "      - 86",
            "db > Bye!",
        ]
        self.assertEqual(result[-len(expected_results):], expected_results)

        # 落盘
        result2 = self.run_script([
            ".btree",
            ".exit",
        ])
        self.assertEqual(result2[-len(expected_results):], expected_results)


if __name__ == '__main__':
    unittest.main()
