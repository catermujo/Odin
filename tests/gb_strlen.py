from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="odin-string-bounds-") as temporary:
    output = Path(temporary)
    executable = output / ("bounds.exe" if os.name == "nt" else "bounds")
    if os.name == "nt":
        vswhere = (
            Path(os.environ["ProgramFiles(x86)"])
            / "Microsoft Visual Studio/Installer/vswhere.exe"
        )
        vs = subprocess.check_output(
            [
                str(vswhere),
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
        batch = output / "test.cmd"
        batch.write_text(f'''@echo off
call "{vs}/Common7/Tools/vsdevcmd.bat" -arch=x64 -host_arch=x64
cl /nologo /EHsc /MT /Od /Z7 /fsanitize=address /I"{root / "src"}" "{root / "tests/gb_strlen.cpp"}" /Fe:"{executable}" /Fo:"{output / "test.obj"}" /link User32.lib
if errorlevel 1 exit /b %errorlevel%
"{executable}"
exit /b %errorlevel%
''')
        subprocess.run(["cmd", "/c", str(batch)], cwd=output, check=True)
    else:
        subprocess.run(
            [
                os.environ.get("CXX", "c++"),
                "-O1",
                "-g",
                "-fsanitize=address",
                "-I" + str(root / "src"),
                str(root / "tests/gb_strlen.cpp"),
                "-o",
                str(executable),
                "-pthread",
            ],
            check=True,
        )
        subprocess.run([str(executable)], check=True)
