"""Exercise Windows test exception ownership through a native DLL and normal test runners."""

import argparse
import ctypes
from ctypes import wintypes
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def msvc_environment(output):
    env = {key.upper(): value for key, value in os.environ.items()}
    if shutil.which("cl.exe"):
        return env
    vswhere = (
        Path(os.environ["ProgramFiles(x86)"])
        / "Microsoft Visual Studio/Installer/vswhere.exe"
    )
    installation = subprocess.check_output(
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
    if not installation:
        raise RuntimeError("Native x64 MSVC is required")
    devcmd = Path(installation) / "Common7/Tools/VsDevCmd.bat"
    # Keep the toolchain environment in memory; never record credentials from inherited variables.
    batch = output / "msvc-environment.cmd"
    batch.write_text(
        f'@echo off\ncall "{devcmd}" -arch=x64 -host_arch=x64 >nul\nif errorlevel 1 exit /b 1\nset\n'
    )
    text = subprocess.check_output(
        ["cmd.exe", "/d", "/c", str(batch)],
        text=True,
        env=env,
    )
    for line in text.splitlines():
        if "=" in line and not line.startswith("="):
            key, value = line.split("=", 1)
            env[key.upper()] = value
    return env


def debug_run(command, output, env):
    class Debug_Event(ctypes.Structure):
        _fields_ = [
            ("code", wintypes.DWORD),
            ("pid", wintypes.DWORD),
            ("tid", wintypes.DWORD),
            ("padding", wintypes.DWORD),
            ("data", ctypes.c_byte * 160),
        ]

    assert ctypes.sizeof(Debug_Event) == 176
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    wait = kernel.WaitForDebugEvent
    wait.argtypes = [ctypes.POINTER(Debug_Event), wintypes.DWORD]
    wait.restype = wintypes.BOOL
    proceed = kernel.ContinueDebugEvent
    proceed.argtypes = [wintypes.DWORD, wintypes.DWORD, wintypes.DWORD]
    proceed.restype = wintypes.BOOL
    close = kernel.CloseHandle
    close.argtypes = [wintypes.HANDLE]
    close.restype = wintypes.BOOL
    answer = {"command": command, "exceptions": [], "initial_breakpoint_handled": False}
    deadline = time.monotonic() + 30
    with output.open("w") as log:
        process = subprocess.Popen(
            command, env=env, stdout=log, stderr=subprocess.STDOUT, creationflags=2
        )
        try:
            while True:
                if time.monotonic() > deadline:
                    raise TimeoutError("native debug control exceeded 30 seconds")
                event = Debug_Event()
                if not wait(ctypes.byref(event), 100):
                    if ctypes.get_last_error() != 121:
                        raise ctypes.WinError(ctypes.get_last_error())
                    continue
                address = ctypes.addressof(event.data)
                status = 0x10002
                exited = event.code == 5
                if event.code in (3, 6):
                    # Close image handles only; Windows closes event process/thread handles on continued exit.
                    # https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-waitfordebugevent
                    image_handle = ctypes.c_void_p.from_address(address).value
                    if image_handle:
                        close(image_handle)
                elif event.code == 1:
                    code = ctypes.c_uint32.from_address(address).value
                    first = bool(ctypes.c_uint32.from_address(address + 152).value)
                    if code == 0x80000003 and not answer["initial_breakpoint_handled"]:
                        answer["initial_breakpoint_handled"] = True
                    else:
                        # The debugger never handles a real fixture exception or bypasses a test handler.
                        status = 0x80010001
                        answer["exceptions"].append(
                            {"code": hex(code), "first_chance": first}
                        )
                if not proceed(event.pid, event.tid, status):
                    raise ctypes.WinError(ctypes.get_last_error())
                if exited:
                    break
            answer["exit_code"] = process.wait(timeout=10)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait(timeout=10)
    return answer


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-odin", type=Path, required=True)
    parser.add_argument(
        "-odin-root", type=Path, default=Path(__file__).resolve().parents[1]
    )
    parser.add_argument("-output", type=Path, required=True)
    parser.add_argument("-repeats", type=int, default=3)
    args = parser.parse_args()
    if os.name != "nt" or ctypes.sizeof(ctypes.c_void_p) != 8:
        raise RuntimeError("Run natively on Windows x64")
    if args.repeats < 1:
        raise ValueError("-repeats must be positive")
    root = args.odin_root.resolve()
    odin = args.odin.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    env = msvc_environment(output)
    env["ODIN_ROOT"] = str(root)
    native = output / "native"
    native.mkdir()
    env["PATH"] = str(native) + os.pathsep + env["PATH"]
    receipts = []

    def run(name, command):
        started = time.monotonic()
        with (output / (name + ".log")).open("w") as log:
            process = subprocess.run(
                [str(value) for value in command],
                cwd=output,
                env=env,
                stdout=log,
                stderr=subprocess.STDOUT,
                timeout=30,
            )
        receipts.append(
            {
                "name": name,
                "command": [str(value) for value in command],
                "exit_code": process.returncode,
                "elapsed_seconds": time.monotonic() - started,
            }
        )
        (output / "commands.json").write_text(json.dumps(receipts, indent=2) + "\n")
        return process.returncode, (output / (name + ".log")).read_text(
            errors="replace"
        )

    cpp = Path(__file__).with_suffix(".cpp").resolve()
    dll = native / "windows_testing_exceptions.dll"
    lib = dll.with_suffix(".lib")
    cl = shutil.which("cl.exe", path=env["PATH"])
    if cl is None:
        raise RuntimeError("MSVC environment did not provide cl.exe")
    code, _ = run(
        "msvc-build",
        [
            cl,
            "/nologo",
            "/Bv",
            "/LD",
            "/EHsc",
            "/Od",
            "/MD",
            "/Zi",
            cpp,
            f"/Fe:{dll}",
            f"/Fo:{native / 'probe.obj'}",
            f"/Fd:{native / 'probe-compiler.pdb'}",
            "/link",
            f"/IMPLIB:{lib}",
            f"/PDB:{native / 'probe.pdb'}",
            "/DEBUG",
        ],
    )
    if code:
        raise RuntimeError("MSVC fixture build failed; see msvc-build.log")
    ffi = output / "ffi"
    ffi.mkdir()
    declarations = ['probe_install_predecessor :: proc "c" () ---']
    for name in (
        "caught_cpp",
        "caught_seh",
        "caught_access_violation",
        "child_predecessor",
    ):
        declarations.append(f'probe_{name} :: proc "c" () -> i32 ---')
    for name in (
        "access_violation",
        "illegal_instruction",
        "arithmetic",
        "high_code",
        "low_code",
    ):
        declarations.append(f'probe_unhandled_{name} :: proc "c" () ---')
    (ffi / "probe.odin").write_text(
        'package probe\nforeign import native "../native/windows_testing_exceptions.lib"\nforeign native {\n'
        + "\n".join(declarations)
        + '\n}\n@(init, private="file")\ninstall :: proc "contextless" () { probe_install_predecessor() }\n',
        encoding="utf-8",
    )

    ordinary = output / "ordinary"
    ordinary.mkdir()
    (ordinary / "main.odin").write_text(
        """package main
import "core:fmt"
import probe "../ffi"
main :: proc() {
    ensure(probe.probe_caught_cpp() == 42)
    ensure(probe.probe_caught_seh() == 43)
    ensure(probe.probe_caught_access_violation() == 44)
    ensure(probe.probe_child_predecessor() == 45)
    fmt.println("ordinary=42,43,44,45")
    fmt.println("compiled-root=" + ODIN_ROOT)
}
""",
        encoding="utf-8",
    )
    positive = output / "positive"
    positive.mkdir()
    common = '#+test\npackage control\nimport "core:testing"\nimport "core:fmt"\nimport probe "../ffi"\n'
    cases = []
    for name, value in (
        ("caught_cpp", 42),
        ("caught_seh", 43),
        ("caught_access_violation", 44),
        ("child_predecessor", 45),
    ):
        cases.append(
            f'@(test)\n{name} :: proc(t: ^testing.T) {{ testing.expect_value(t, probe.probe_{name}(), i32({value})); fmt.println("handled:{name}") }}\n'
        )
    (positive / "control.odin").write_text(common + "\n".join(cases), encoding="utf-8")

    fault_functions = {
        "access": "probe.probe_unhandled_access_violation()",
        "illegal": "probe.probe_unhandled_illegal_instruction()",
        "arithmetic": "probe.probe_unhandled_arithmetic()",
        "high": "probe.probe_unhandled_high_code()",
        "low": "probe.probe_unhandled_low_code()",
        "fail_now": 'testing.fail_now(t, "intentional fail_now")',
    }
    negative = output / "negative"
    negative.mkdir()
    expected_negative = {}
    source = common
    for repetition in range(2):
        for name, body in fault_functions.items():
            name = f"fault_{name}_{repetition}"
            expected_negative[name] = False
            # Statically diverging fail_now forbids defers; foreign native faults exercise abandonment.
            defer_line = (
                ""
                if "fail_now" in name
                else f'defer fmt.println("unexpected-defer:{name}")'
            )
            source += f"""cleanup_{name} :: proc(_: rawptr) {{ fmt.println("cleanup:{name}") }}
@(test)
{name} :: proc(t: ^testing.T) {{
    testing.cleanup(t, cleanup_{name}, nil)
    {defer_line}
    {body}
}}
"""
    for name in ("healthy_a", "healthy_b"):
        expected_negative[name] = True
        source += f'@(test)\n{name} :: proc(t: ^testing.T) {{ testing.expect_value(t, probe.probe_caught_cpp(), i32(42)); fmt.println("healthy:{name}") }}\n'
    (negative / "control.odin").write_text(source, encoding="utf-8")

    parallel = output / "parallel"
    parallel.mkdir()
    source = (
        common
        + """import "core:sync"
import "core:thread"
entered: i32
gate :: proc() {
    sync.atomic_add_explicit(&entered, 1, .Seq_Cst)
    for sync.atomic_load_explicit(&entered, .Seq_Cst) != 4 do thread.yield()
}
"""
    )
    expected_parallel = {
        "fault_access": False,
        "fault_illegal": False,
        "healthy_cpp": True,
        "healthy_seh": True,
    }
    for name, body in (
        ("fault_access", fault_functions["access"]),
        ("fault_illegal", fault_functions["illegal"]),
        ("healthy_cpp", "testing.expect_value(t, probe.probe_caught_cpp(), i32(42))"),
        ("healthy_seh", "testing.expect_value(t, probe.probe_caught_seh(), i32(43))"),
    ):
        source += f"""cleanup_{name} :: proc(_: rawptr) {{ fmt.println("cleanup:{name}") }}
@(test)
{name} :: proc(t: ^testing.T) {{
    testing.cleanup(t, cleanup_{name}, nil)
    gate()
    {body}
}}
"""
    (parallel / "control.odin").write_text(source, encoding="utf-8")

    def build(name, path, threads=None):
        binary = output / (name + ".exe")
        command = [odin, "build", path, "-debug", "-o:none", f"-out:{binary}"]
        if threads is not None:
            report = output / (name + ".json")
            command += [
                "-build-mode:test",
                f"-define:ODIN_TEST_THREADS={threads}",
                "-define:ODIN_TEST_FANCY=false",
                "-define:ODIN_TEST_LOG_LEVEL=warning",
                "-define:ODIN_TEST_JSON_REPORT=" + report.as_posix(),
            ]
        code, _ = run(name + "-build", command)
        if code:
            raise RuntimeError(f"{name} build failed; see {name}-build.log")
        return binary

    def check(name, binary, expected):
        code, text = run(name, [binary])
        if code != (1 if not all(expected.values()) else 0):
            raise AssertionError(f"{name}: unexpected exit {code}")
        report = json.loads(binary.with_suffix(".json").read_text())
        actual = {
            test["name"]: test["success"]
            for tests in report["packages"].values()
            for test in tests
        }
        if actual != expected:
            raise AssertionError(f"{name}: test ownership mismatch {actual}")
        if "unexpected-defer:" in text or "predecessor-restored=no" in text:
            raise AssertionError(f"{name}: abort or restoration contract broken")
        if "predecessor-restored=yes" not in text:
            raise AssertionError(f"{name}: missing process-filter restoration receipt")
        for test in expected:
            if (
                test.startswith("fault_") or name.startswith("parallel-")
            ) and text.count("cleanup:" + test + "\n") != 1:
                raise AssertionError(f"{name}: cleanup ownership broken for {test}")
        receipts[-1]["test_report"] = report
        receipts[-1]["predecessor_restored"] = True
        (output / "commands.json").write_text(json.dumps(receipts, indent=2) + "\n")

    binaries = {}
    binaries["ordinary"] = build("ordinary", ordinary)
    code, text = run("ordinary-run", [binaries["ordinary"]])
    if code or "ordinary=42,43,44,45" not in text:
        raise AssertionError("ordinary foreign control failed")
    compiled_root = next(
        line.split("=", 1)[1]
        for line in text.splitlines()
        if line.startswith("compiled-root=")
    )
    if Path(compiled_root).resolve() != root:
        raise AssertionError("compiler used the wrong library root")
    for threads in (1, 4):
        name = f"core-testing-{threads}"
        binaries[name] = build(name, root / "tests/core/testing", threads)
        code, _ = run(name + "-run", [binaries[name]])
        report = json.loads(binaries[name].with_suffix(".json").read_text())
        if code or report["total"] < 6 or report["success"] != report["total"]:
            raise AssertionError(
                "existing core/testing controls or normal JSON report failed"
            )
        receipts[-1]["test_report"] = report
        (output / "commands.json").write_text(json.dumps(receipts, indent=2) + "\n")
    expected_positive = {
        name: True
        for name in (
            "caught_cpp",
            "caught_seh",
            "caught_access_violation",
            "child_predecessor",
        )
    }
    for threads in (1, 4):
        binaries[f"positive-{threads}"] = build(
            f"positive-{threads}", positive, threads
        )
        binaries[f"negative-{threads}"] = build(
            f"negative-{threads}", negative, threads
        )
        for repeat in range(args.repeats):
            check(
                f"positive-{threads}-run-{repeat}",
                binaries[f"positive-{threads}"],
                expected_positive,
            )
            check(
                f"negative-{threads}-run-{repeat}",
                binaries[f"negative-{threads}"],
                expected_negative,
            )
    binaries["parallel-4"] = build("parallel-4", parallel, 4)
    for repeat in range(args.repeats):
        check(f"parallel-4-run-{repeat}", binaries["parallel-4"], expected_parallel)
    handled = debug_run(
        [str(binaries["positive-1"]), "-tests:caught_cpp"],
        output / "debug-handled.log",
        env,
    )
    unhandled = debug_run(
        [str(binaries["negative-1"]), "-tests:fault_access_0"],
        output / "debug-unhandled.log",
        env,
    )
    if handled["exit_code"] != 0 or any(
        not event["first_chance"] for event in handled["exceptions"]
    ):
        raise AssertionError("debugger changed internally caught exception behavior")
    if unhandled["exit_code"] & 0xFFFFFFFF != 0xC0000005 or not any(
        event == {"code": "0xc0000005", "first_chance": False}
        for event in unhandled["exceptions"]
    ):
        raise AssertionError("debugger did not own the genuine second-chance fault")
    result = {
        "passed": True,
        "compiler": str(odin),
        "compiler_sha256": sha256(odin),
        "compiler_version": subprocess.check_output(
            [str(odin), "version"], env=env, text=True
        ).strip(),
        "odin_root": str(root),
        "repeats": args.repeats,
        "commands": receipts,
        "binaries": {
            name: {"path": str(path), "sha256": sha256(path)}
            for name, path in binaries.items()
        },
        "debug_handled": handled,
        "debug_unhandled": unhandled,
    }
    (output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(
        json.dumps(
            {
                key: value
                for key, value in result.items()
                if key not in ("commands", "binaries")
            },
            indent=2,
        )
    )


if __name__ == "__main__":
    main()
