from pathlib import Path
import subprocess
import sys
import tempfile


root = Path(__file__).resolve().parents[1]
compiler = root / ("odin.exe" if sys.platform == "win32" else "odin")
with tempfile.TemporaryDirectory(prefix="odin-range-cost-") as temporary:
    directory = Path(temporary)
    source = directory / "main.odin"
    source.write_text("""package main

search :: #force_no_inline proc(values: [8]int) -> int {
    total := 0
    for value in values {
        for other, index in values {
            if other == value {
                total += index
                break
            }
        }
    }
    return total
}

main :: proc() {
    assert(search({3, 1, 3, 2, 1, 5, 5, 7}) == 22)
    total := 0
    #unroll for value in 1..=4 {
        total += value
    }
    assert(total == 10)
}
""")
    ir_directory = directory / "ir"
    ir_directory.mkdir()
    subprocess.run(
        [
            str(compiler),
            "build",
            str(source),
            "-file",
            "-o:none",
            "-use-single-module",
            "-build-mode:llvm-ir",
            "-out:" + str(ir_directory),
        ],
        check=True,
    )
    ir_files = list(ir_directory.glob("*.ll"))
    assert ir_files, "No frontend IR emitted"
    # DUMBAI: The regression is the frontend's forced hint, which bypasses LLVM's ordinary loop cost budget.
    for ir in ir_files:
        assert "llvm.loop.unroll.full" not in ir.read_text(), ir
    executable = directory / (
        "range-cost.exe" if sys.platform == "win32" else "range-cost"
    )
    subprocess.run(
        [
            str(compiler),
            "build",
            str(source),
            "-file",
            "-o:speed",
            "-out:" + str(executable),
        ],
        check=True,
    )
    subprocess.run([str(executable)], check=True)
    print("LLVM ordinary range costing and explicit unroll passed")
