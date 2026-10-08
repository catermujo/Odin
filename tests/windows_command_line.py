"""Exercise Windows compiler argument decoding through real builds and runs."""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


@unittest.skipUnless(sys.platform == "win32", "Windows command line only")
class WindowsCommandLineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = Path(__file__).resolve().parents[1] / "odin.exe"
        cls.temporary = tempfile.TemporaryDirectory(prefix="odin cli ü ")
        cls.directory = Path(cls.temporary.name)
        cls.source = cls.directory / "main.odin"
        cls.source.write_text(
            """package main
import "core:encoding/json"
import "core:fmt"
import "core:os"
VALUE :: #config(VALUE, "default")
main :: proc() {
    value, _ := json.marshal(VALUE)
    defer delete(value)
    args, _ := json.marshal(os.args[1:])
    defer delete(args)
    fmt.printf("%s\\n%s\\n", string(value), string(args))
}
""",
            encoding="utf-8",
        )
        cls.command = [
            str(cls.compiler),
            "run",
            str(cls.source),
            "-file",
            f"-out:{cls.directory / 'probe.exe'}",
        ]

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def invoke(self, command):
        result = subprocess.run(
            command,
            cwd=self.directory,
            capture_output=True,
            encoding="utf-8",
            timeout=60,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        return [json.loads(line) for line in result.stdout.splitlines()]

    def test_define_round_trip(self):
        for value in (
            "revision",
            '"revision"',
            "'123'",
            "'true'",
            '"value with spaces"',
            "C:\\folder with spaces\\tail\\",
            'embedded "quote" text',
            "unicode é 日本",
        ):
            with self.subTest(value=value):
                expected = value[1:-1] if value.startswith("'") else value
                actual, args = self.invoke([*self.command, f"-define:VALUE={value}"])
                self.assertEqual(actual, expected)
                self.assertEqual(args, [])

    def test_run_preserves_raw_argument_tail(self):
        arguments = [
            "",
            "two words",
            'embedded "quote"',
            "C:\\folder with spaces\\",
            "--",
            "-file",
        ]
        value, actual = self.invoke([*self.command, "--", *arguments])
        self.assertEqual(value, "default")
        self.assertEqual(actual, arguments)

    def test_doubled_quote_in_raw_command_line(self):
        command = subprocess.list2cmdline(self.command) + ' -define:VALUE="a""b"'
        value, args = self.invoke(command)
        self.assertEqual(value, 'a"b')
        self.assertEqual(args, [])


if __name__ == "__main__":
    unittest.main()
