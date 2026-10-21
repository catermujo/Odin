"""Check Linux symbolizer failures and partial-result ownership."""

from pathlib import Path
import subprocess
import sys
import tempfile

if sys.platform != "linux":
    print("SKIP: Linux symbolizer regression")
    raise SystemExit(0)

root = Path(__file__).resolve().parents[1]
source_text = """package main
import "core:debug/trace"
import "core:mem"
main :: proc() {
    tracker: mem.Tracking_Allocator
    mem.tracking_allocator_init(&tracker, context.allocator)
    defer mem.tracking_allocator_destroy(&tracker)
    allocator := mem.tracking_allocator(&tracker)
    captured := trace.capture()
    assert(captured.len > 1)
    locations, err := trace.resolve(captured, allocator)
    trace.locations_destroy(locations, allocator)
    when #config(EXPECT_RESOLVE_FAILURE, true) {
        assert(err == .Resolve_Aborted)
        assert(locations == nil)
    } else {
        assert(err == .None)
        assert(len(locations) == captured.len)
    }
    assert(len(tracker.allocation_map) == 0)
}
"""
with tempfile.TemporaryDirectory(prefix="odin-trace-cleanup-") as temporary:
    directory = Path(temporary)
    source = directory / "main.odin"
    source.write_text(source_text)
    programs = {
        "empty": "exit 1\n",
        "partial": "printf 'resolved\\nfile.odin:10\\n'\n",
        "success": "shift 3\nfor address do printf 'resolved\\nfile.odin:10\\n'; done\n",
    }
    for name, script in programs.items():
        symbolizer = directory / name
        symbolizer.write_text("#!/bin/sh\n" + script)
        symbolizer.chmod(0o755)
        for optimization in ("none", "minimal", "speed"):
            binary = directory / "probe"
            subprocess.run(
                [
                    str(root / "odin"),
                    "build",
                    str(source),
                    "-file",
                    "-debug",
                    f"-o:{optimization}",
                    f"-out:{binary}",
                    f"-define:ODIN_TRACE_SYMBOLIZER_PROGRAM={symbolizer}",
                    f"-define:EXPECT_RESOLVE_FAILURE={'false' if name == 'success' else 'true'}",
                ],
                check=True,
                timeout=60,
            )
            subprocess.run([str(binary)], check=True, timeout=30)
print("9 Linux trace ownership regressions passed")
