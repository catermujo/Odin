from pathlib import Path
import subprocess
import sys
import tempfile

from llvm_error_cleanup import run_native_llvm_test


root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="odin-ir-visibility-") as temporary:
    directory = Path(temporary)
    source = directory / "main.odin"
    source.write_text(
        'package main\n@(linkage="internal") local: int\nmain :: proc() { local = 1 }\n'
    )
    compiler = root / ("odin.exe" if sys.platform == "win32" else "odin")
    inputs = []
    for mode, module_flag in (
        ("single", "-use-single-module"),
        ("separate", "-use-separate-modules"),
    ):
        ir_directory = directory / mode
        ir_directory.mkdir()
        subprocess.run(
            [
                str(compiler),
                "build",
                str(source),
                "-file",
                "-o:none",
                module_flag,
                "-build-mode:llvm-ir",
                "-out:" + str(ir_directory),
            ],
            check=True,
        )
        inputs.extend(str(ir) for ir in ir_directory.glob("*.ll"))
    run = run_native_llvm_test("llvm_ir_visibility.cpp", inputs)
    sys.stdout.write(run.stdout)
    sys.stderr.write(run.stderr)
    assert run.returncode == 0, run.returncode
    assert "LLVM IR visibility passed" in run.stdout, run.stdout
