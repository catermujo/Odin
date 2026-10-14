import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]


def run_native_llvm_test(source_name, arguments=()):
    with tempfile.TemporaryDirectory(prefix="odin-llvm-native-") as temporary:
        output = Path(temporary)
        executable = output / (
            "error-cleanup.exe" if os.name == "nt" else "error-cleanup"
        )
        source = root / "tests" / source_name
        if os.name == "nt":
            installer = (
                Path(os.environ["ProgramFiles(x86)"])
                / "Microsoft Visual Studio/Installer/vswhere.exe"
            )
            visual_studio = subprocess.check_output(
                [
                    str(installer),
                    "-latest",
                    "-products",
                    "*",
                    "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-property",
                    "installationPath",
                ],
                text=True,
            ).strip()
            batch = output / "compile.cmd"
            batch.write_text(
                f'@echo off\ncall "{visual_studio}/Common7/Tools/vsdevcmd.bat" -arch=x64 -host_arch=x64\n'
                f'cl /nologo /EHsc /I"{root / "src"}" "{source}" '
                f'/Fe:"{executable}" /Fo:"{output / "error-cleanup.obj"}" '
                f'/link "{root / "bin/llvm/windows/LLVM-C.lib"}"\nexit /b %errorlevel%\n'
            )
            subprocess.run(["cmd", "/c", str(batch)], check=True)
            shutil.copy2(root / "LLVM-C.dll", output / "LLVM-C.dll")
        else:
            llvm_config = os.environ.get("LLVM_CONFIG", "llvm-config")
            flags = subprocess.check_output(
                [llvm_config, "--cxxflags", "--ldflags", "--libs", "--system-libs"],
                text=True,
            ).split()
            subprocess.run(
                [
                    os.environ.get("CXX", "c++"),
                    str(source),
                    "-I" + str(root / "src"),
                    "-o",
                    str(executable),
                    *flags,
                ],
                check=True,
            )
        run = subprocess.run(
            [str(executable), *arguments], capture_output=True, text=True
        )
        return run


if __name__ == "__main__":
    run = run_native_llvm_test("llvm_error_cleanup.cpp")
    sys.stdout.write(run.stdout)
    sys.stderr.write(run.stderr)
    assert run.returncode == 0, run.returncode
    assert (
        run.stderr.count("unknown pass name 'odin-intentionally-invalid-pass'") == 3
    ), run.stderr
    assert "LLVM pass error cleanup passed" in run.stdout, run.stdout
