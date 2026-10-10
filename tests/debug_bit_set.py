"""Check bit-set fields through LLDB against a running Odin program on macOS."""

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SOURCE = '''package main

import "base:intrinsics"
import "vendor:vulkan"

Flag :: enum { low = 0, high = 6, middle = 3, alias = high }
Wide_Flag :: enum { low = 0, top = 127, high = 64, alias = top }
Flags8 :: bit_set[Flag; u8]
Flags16 :: bit_set[Flag; u16]
Flags32 :: bit_set[Flag; u32]
Flags64 :: bit_set[Flag; u64]
Flags128 :: bit_set[Wide_Flag; u128]
Shifted :: bit_set[-8..<24]
Range128 :: bit_set[0..<128; u128]

Values :: struct {
    prefix: u8,
    flags8: Flags8,
    flags16: Flags16,
    flags32: Flags32,
    flags64: Flags64,
    flags128: Flags128,
    shifted: Shifted,
    range128: Range128,
    image: vulkan.ImageCreateInfo,
    suffix: u64,
}

debug_stop :: #force_no_inline proc(values: ^Values) {
    intrinsics.debug_trap()
    assert(values.prefix == 123 && values.suffix == 456)
    assert(.high in values.flags8 && .middle not_in values.flags8)
    assert(.high in values.flags16 && .middle not_in values.flags16)
    assert(.high in values.flags32 && .middle not_in values.flags32)
    assert(.high in values.flags64 && .middle not_in values.flags64)
    assert(.top in values.flags128 && .low not_in values.flags128)
    assert(-8 in values.shifted && 23 in values.shifted)
    assert(64 in values.range128 && 127 in values.range128)
    assert(.VIDEO_DECODE_DST_KHR in values.image.usage)
    assert(.VIDEO_ENCODE_EMPHASIS_MAP_KHR in values.image.usage)
}

main :: proc() {
    values := Values{
        prefix = 123,
        flags8 = {.low, .high},
        flags16 = {.low, .high},
        flags32 = {.low, .high},
        flags64 = {.low, .high},
        flags128 = {.high, .top},
        shifted = {-8, 0, 18, 23},
        range128 = {0, 63, 64, 100, 127},
        image = {usage = {.VIDEO_DECODE_DST_KHR, .VIDEO_ENCODE_EMPHASIS_MAP_KHR,
                          .SHADING_RATE_IMAGE_NV, .HOST_TRANSFER_EXT}},
        suffix = 456,
    }
    debug_stop(&values)
}
'''


@unittest.skipUnless(sys.platform == "darwin", "macOS LLDB regression")
class BitSetDebugTests(unittest.TestCase):
    def test_debugger_reads_overlapping_flags(self):
        compiler = Path(__file__).resolve().parents[1] / "odin"
        line = SOURCE.splitlines().index("    intrinsics.debug_trap()") + 1
        expected = {".prefix": 123, ".suffix": 456}
        for width in (8, 16, 32, 64):
            for name, value in {"low": 1, "high": 1, "middle": 0, "alias": 1}.items():
                expected[f".flags{width}.{name}"] = value
        for name, value in {"low": 0, "top": 1, "high": 1, "alias": 1}.items():
            expected[f".flags128.{name}"] = value
        for value in range(-8, 24):
            expected[f".shifted.{value}"] = int(value in {-8, 0, 18, 23})
        for value in range(128):
            expected[f".range128.{value}"] = int(value in {0, 63, 64, 100, 127})
        for name, value in {
            "VIDEO_DECODE_DST_KHR": 1,
            "VIDEO_ENCODE_EMPHASIS_MAP_KHR": 1,
            "SHADING_RATE_IMAGE_NV": 1,
            "FRAGMENT_SHADING_RATE_ATTACHMENT_KHR": 1,
            "HOST_TRANSFER": 1,
            "HOST_TRANSFER_EXT": 1,
            "TRANSFER_DST": 0,
        }.items():
            expected[f".image.usage.{name}"] = value

        with tempfile.TemporaryDirectory(prefix="odin-debug-bit-set-") as temporary:
            directory = Path(temporary)
            source = directory / "main.odin"
            source.write_text(SOURCE, encoding="utf-8")
            check = directory / "check.py"
            check.write_text(
                "import json, lldb\n"
                "frame = lldb.debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetFrameAtIndex(0)\n"
                "values = frame.FindVariable('values').Dereference()\n"
                f"expected = {expected!r}\n"
                "actual = {}\n"
                "for path in expected:\n"
                "    field = values.GetValueForExpressionPath(path)\n"
                "    assert field.IsValid(), path\n"
                "    error = lldb.SBError()\n"
                "    actual[path] = field.GetValueAsUnsigned(error)\n"
                "    assert error.Success(), (path, str(error))\n"
                "print('BIT_SET_VALUES=' + json.dumps(actual, sort_keys=True))\n",
                encoding="utf-8",
            )
            for optimization in ("none", "minimal"):
                with self.subTest(optimization=optimization):
                    binary = directory / f"probe-{optimization}"
                    build = subprocess.run(
                        [str(compiler), "build", str(source), "-file", "-debug",
                         f"-o:{optimization}", f"-out:{binary}"],
                        capture_output=True, text=True, timeout=60,
                    )
                    self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
                    result = subprocess.run(
                        ["xcrun", "lldb", "-b", "-o", f"breakpoint set -f main.odin -l {line}",
                         "-o", "run", "-o", f"script exec(open({str(check)!r}).read())", "-o", "quit",
                         str(binary)],
                        capture_output=True, text=True, timeout=60,
                    )
                    output = result.stdout + result.stderr
                    self.assertEqual(result.returncode, 0, output)
                    self.assertNotIn("warning:", output)
                    self.assertNotIn("Traceback", output)
                    values = next(line for line in output.splitlines() if line.startswith("BIT_SET_VALUES="))
                    self.assertEqual(json.loads(values.removeprefix("BIT_SET_VALUES=")), expected)


if __name__ == "__main__":
    unittest.main()
