import subprocess
import unittest

class TestDatabase(unittest.TestCase):
    def run_script(self, commands):
        process = subprocess.Popen("tiny_db/tiny_db", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        output, _ = process.communicate(input='\n'.join(commands).encode('utf-8'))
        return output.decode().splitlines()

    def test_exit_and_unrecognized_command(self):
        result = self.run_script([
            "hello world",
            "HELLO WORLD",
            ".exit",
        ])
        self.assertEqual(result, [
            "db > Unrecognized command: hello world",
            "db > Unrecognized command: HELLO WORLD",
            "db > Bye!",
        ])

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

if __name__ == '__main__':
    unittest.main()