#+build arm64,arm32
package hash

import "base:intrinsics"
import "core:simd/arm"

CRC32_HW_SUPPORTED :: intrinsics.has_target_feature("crc")

@(enable_target_feature = "crc")
crc32_hw :: proc "contextless" (data: []byte, seed: u32) -> u32 #no_bounds_check {
	crc := ~seed
	buffer := raw_data(data)
	length := len(data)

	when ODIN_ARCH == .arm64 {
		for length != 0 && uintptr(buffer) & 7 != 0 {
			crc = arm.__crc32b(crc, buffer[0])
			buffer = buffer[1:]
			length -= 1
		}
		for length >= 8 {
			crc = arm.__crc32d(crc, (^u64)(buffer)^)
			buffer = buffer[8:]
			length -= 8
		}
	} else {
		for length != 0 && uintptr(buffer) & 3 != 0 {
			crc = arm.__crc32b(crc, buffer[0])
			buffer = buffer[1:]
			length -= 1
		}
		for length >= 4 {
			crc = arm.__crc32w(crc, (^u32)(buffer)^)
			buffer = buffer[4:]
			length -= 4
		}
	}

	for length != 0 {
		crc = arm.__crc32b(crc, buffer[0])
		buffer = buffer[1:]
		length -= 1
	}

	return ~crc
}
