"""Check parent completion before nested anonymous procedure bodies."""

from pathlib import Path
import subprocess
import sys
import tempfile


root = Path(__file__).resolve().parents[1]
compiler = (
    Path(sys.argv[1]).resolve()
    if len(sys.argv) > 1
    else root / ("odin.exe" if sys.platform == "win32" else "odin")
)
callback_count = 256
specialization_count = 32
body = """    return proc(value: ^int) {
        alias :: increment
        nested :: proc(value: ^int) { alias(value) }
        nested(value)
        deeper := proc(value: ^int) {
            deeper_alias :: increment
            leaf :: proc(value: ^int) { deeper_alias(value) }
            leaf(value)
        }
        deeper(value)
        increment(value)
    }
"""
source = """package main
Callback :: #type proc(value: ^int)
increment :: proc(value: ^int) {
    assert(context.user_index == 42)
    value^ += 1
}
"""
for index in range(callback_count):
    source += f"make_{index} :: proc() -> Callback {{\n" + body + "}\n"
source += "make_generic :: proc($N: int) -> Callback {\n"
source += body.replace(
    "        increment(value)\n", "        increment(value)\n        value^ += N\n"
)
source += "}\nmain :: proc() {\n    context.user_index = 42\n    value := 0\n"
for index in range(callback_count):
    source += f"    {{ callback := make_{index}(); callback(&value) }}\n"
for index in range(specialization_count):
    source += f"    {{ callback := make_generic({index}); callback(&value) }}\n"
expected = 3 * (callback_count + specialization_count)
expected += specialization_count * (specialization_count - 1) // 2
source += f"    assert(value == {expected})\n}}\n"

with tempfile.TemporaryDirectory(prefix="odin-anonymous-ordering-") as temporary:
    directory = Path(temporary)
    program = directory / "main.odin"
    program.write_text(source)
    configurations = [
        ([], 32),
        (["-thread-count:8"], 32),
        (["-thread-count:1"], 1),
        (["-no-threaded-checker"], 1),
    ]
    for flags, attempts in configurations:
        for attempt in range(attempts):
            result = subprocess.run(
                [str(compiler), "check", str(program), "-file", *flags],
                capture_output=True,
                text=True,
                timeout=60,
            )
            assert result.returncode == 0, (
                flags,
                attempt,
                result.stdout,
                result.stderr,
            )
    for optimization in ("none", "speed"):
        executable = directory / (
            "callbacks.exe" if sys.platform == "win32" else "callbacks"
        )
        subprocess.run(
            [
                str(compiler),
                "build",
                str(program),
                "-file",
                f"-o:{optimization}",
                f"-out:{executable}",
            ],
            check=True,
            timeout=60,
        )
        subprocess.run([str(executable)], check=True, timeout=30)
    program.write_text("""package main
requires_context :: proc() {}
main :: proc() {
    callback := proc "contextless" () { requires_context() }
    callback()
}
""")
    for flags in ([], ["-no-threaded-checker"]):
        result = subprocess.run(
            [str(compiler), "check", str(program), "-file", *flags],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert result.returncode != 0, flags
        assert "'context' has not been defined within this scope" in result.stderr, (
            result.stderr
        )
print("Anonymous procedure parent ordering and callback context passed")
