"""Check reachable cleanup before a terminal diverging call."""

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
header = """package main
finish :: proc(result: ^int) { result^ += 1 }
begin :: proc(result: ^int) #scope_exit(.implicit, finish(result)) {}
fallible :: proc(ok: bool) -> bool { return ok }
fallible_value :: proc(ok: bool) -> (int, bool) { return 3, ok }
fail :: proc(value: int) -> ! { panic("late failure") }
"""
branches = {
    "return": "if early { return }",
    "nested_return": "for i in 0..<1 { if early { return } }",
    "switch_return": "switch early { case true: return }",
    "when_return": "when true { if early { return } }",
}
procedures = []
calls = []
for name, branch in branches.items():
    for kind, cleanup in (
        ("explicit", "defer finish(result)"),
        ("implicit", "begin(result)"),
    ):
        procedure = f"{name}_{kind}"
        procedures.append(
            f"""{procedure} :: proc(result: ^int, early: bool) {{
    {cleanup}
    {branch}
    panic("late failure")
}}
"""
        )
        calls.append(f"{procedure}(&result, true)")

for name, branch in (("break", "break"), ("continue", "continue")):
    procedures.append(
        f"""{name}_exit :: proc(result: ^int, early: bool) {{
    for i in 0..<1 {{
        defer finish(result)
        if early {{ {branch} }}
        panic("late failure")
    }}
}}
"""
    )
    calls.append(f"{name}_exit(&result, true)")

procedures.append("""or_return_call :: proc(result: ^int, early: bool) -> bool {
    defer finish(result)
    fallible(!early) or_return
    panic("late failure")
}
or_return_value :: proc(result: ^int, early: bool) -> bool {
    defer finish(result)
    value := fallible_value(!early) or_return
    _ = value
    panic("late failure")
}
or_return_assignment :: proc(result: ^int, early: bool) -> bool {
    defer finish(result)
    value: int
    value = fallible_value(!early) or_return
    _ = value
    panic("late failure")
}
or_return_terminal_argument :: proc(result: ^int, early: bool) -> bool {
    defer finish(result)
    fail(fallible_value(!early) or_return)
}
or_break_exit :: proc(result: ^int, early: bool) {
    for i in 0..<1 {
        defer finish(result)
        value := fallible_value(!early) or_break
        _ = value
        panic("late failure")
    }
}
""")
calls.extend(
    [
        "assert(!or_return_call(&result, true))",
        "assert(!or_return_value(&result, true))",
        "assert(!or_return_assignment(&result, true))",
        "assert(!or_return_terminal_argument(&result, true))",
        "or_break_exit(&result, true)",
    ]
)
main = "main :: proc() {\n    result := 0\n"
for count, call in enumerate(calls, start=1):
    main += f"    {call}\n    assert(result == {count})\n"
main += "}\n"

negative_bodies = {
    "explicit": "defer finish(result)",
    "implicit": "begin(result)",
    "return_before_cleanup": "if early { return }; defer finish(result)",
    "mixed_before_after": "defer finish(result); if early { return }; defer finish(result)",
    "nested_proc_return": "defer finish(result); local :: proc() { return }; local()",
}
with tempfile.TemporaryDirectory(prefix="odin-defer-diverging-") as temporary:
    directory = Path(temporary)
    source = directory / "main.odin"
    source.write_text(header + "".join(procedures) + main)
    for optimization in ("none", "speed"):
        executable = directory / ("probe.exe" if sys.platform == "win32" else "probe")
        subprocess.run(
            [
                str(compiler),
                "build",
                str(source),
                "-file",
                f"-o:{optimization}",
                f"-out:{executable}",
            ],
            check=True,
            timeout=60,
        )
        subprocess.run([str(executable)], check=True, timeout=30)
    for name, body in negative_bodies.items():
        source.write_text(
            header
            + f"""run :: proc(result: ^int, early: bool) {{
    {body}
    panic("late failure")
}}
main :: proc() {{ result := 0; run(&result, false) }}
"""
        )
        run = subprocess.run(
            [str(compiler), "check", str(source), "-file"],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert run.returncode != 0, name
        diagnostic = (
            "Unreachable deferred procedure call"
            if name == "implicit"
            else "Unreachable defer statement"
        )
        assert diagnostic in run.stderr, (name, run.stderr)
        assert run.stderr.count("Error:") == 1, (name, run.stderr)
print(
    f"{len(calls)} reachable cleanup paths at two optimization levels and {len(negative_bodies)} diagnostics passed"
)
