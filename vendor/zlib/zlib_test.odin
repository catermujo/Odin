#+test
package vendor_zlib

import "core:testing"

@(test)
compression_bound_and_compress2_round_trip :: proc(t: ^testing.T) {
    source: [65536]Bytef
    for &value, i in source do value = Bytef((i * 97 + i / 11) % 256)
    counts := []int{0, 1, 257, len(source)}
    for count in counts {
        compressed := make([]Bytef, int(compressBound(uLong(count))))
        defer delete(compressed)
        compressed_size := uLongf(len(compressed))
        result := compress2(raw_data(compressed), &compressed_size, &source[0], uLong(count), BEST_SPEED)
        testing.expect_value(t, result, OK)
        if result != OK do continue
        restored: [65536]Bytef
        restored_size := uLongf(len(restored))
        result = uncompress(&restored[0], &restored_size, raw_data(compressed), compressed_size)
        testing.expect_value(t, result, OK)
        testing.expect_value(t, restored_size, uLongf(count))
        for i in 0 ..< count do testing.expect_value(t, restored[i], source[i])
    }
    text := "123456789"
    testing.expect_value(t, crc32(0, raw_data(text), uInt(len(text))), uLong(0xcbf43926))
}
