"""Exercise Linux auxiliary-vector initialization in executables and shared libraries."""

import ctypes
from pathlib import Path
import subprocess
import sys
import tempfile


if sys.platform != "linux":
    raise SystemExit("This probe requires native Linux")

compiler = Path(__file__).resolve().parents[1] / "odin"
source = """package main
import "core:os"

main :: proc() {
    when ODIN_BUILD_MODE != .Dynamic {
        assert(len(os.args) == 3)
        assert(os.args[1] == "two words")
        assert(os.args[2] == "é")
    }
    memory := make([]u8, 8192)
    defer delete(memory)
    memory[8191] = 42
    assert(memory[8191] == 42)
}
"""

# DUMBAI: Real startup exercises the argv union and page-size lookup before application code executes.
with tempfile.TemporaryDirectory(prefix="odin-linux-auxv-") as temporary:
    directory = Path(temporary)
    program = directory / "main.odin"
    program.write_text(source, encoding="utf-8")
    executable = directory / "probe"
    subprocess.run(
        [str(compiler), "build", str(program), "-file", f"-out:{executable}"],
        check=True,
        timeout=60,
    )
    subprocess.run([str(executable), "two words", "é"], check=True, timeout=10)
    library = directory / "probe.so"
    subprocess.run(
        [
            str(compiler),
            "build",
            str(program),
            "-file",
            "-build-mode:dll",
            f"-out:{library}",
        ],
        check=True,
        timeout=60,
    )
    ctypes.CDLL(str(library))

print(
    "PASS Linux auxiliary vector: executable arguments, allocation, shared-library startup"
)
