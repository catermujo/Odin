package main

import "core:compress"
import "core:image"

Signature :: enum u64be {
    PNG   = 0x89504e470d0a1a0a,
    Later = 0x89504e470d0a1a0b,
}

u16be_matches :: #force_no_inline proc(data: ^u8) -> bool {
    return (^u16be)(data)^ == 0x1234
}

u32be_matches :: #force_no_inline proc(data: ^u8) -> bool {
    return (^u32be)(data)^ == 0x12345678
}

u64be_matches :: #force_no_inline proc(data: ^u8) -> bool {
    return (^u64be)(data)^ == 0x89504e470d0a1a0a
}

signature_matches :: #force_no_inline proc(data: ^u8) -> bool {
    signature := (^Signature)(data)^
    return signature == .PNG && signature < .Later
}

chunk_type_switch_matches :: #force_no_inline proc(data: ^u8) -> bool {
    kind := (^image.PNG_Chunk_Type)(data)^
    #partial switch kind {
    case .IDAT:
        return true
    case:
        return false
    }
}

signature_range_switch_matches :: #force_no_inline proc(data: ^u8) -> bool {
    value := (^Signature)(data)^
    #partial switch value {
    case .PNG ..< .Later:
        return true
    case:
        return false
    }
}

signature_literal_switch_matches :: #force_no_inline proc() -> bool {
    #partial switch Signature.PNG {
    case .PNG:
        return true
    case:
        return false
    }
}

context_reads :: #force_no_inline proc(data: []u8) -> bool {
    ctx := compress.Context_Memory_Input {
        input_data = data,
    }
    signature, signature_error := compress.read_data(&ctx, Signature)
    if signature_error != .None || signature != .PNG {
        return false
    }
    chunk_header, chunk_error := compress.read_data(&ctx, image.PNG_Chunk_Header)
    return(
        chunk_error == .None &&
        int(chunk_header.length) == 13 &&
        chunk_header.type == .IHDR &&
        len(ctx.input_data) == 4 \
    )
}

main :: proc() {
    u16_data := [2]u8{0x12, 0x34}
    u32_data := [4]u8{0x12, 0x34, 0x56, 0x78}
    u64_data := [8]u8{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a}
    idat_data := [4]u8{'I', 'D', 'A', 'T'}
    stream_data := [20]u8{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0, 0, 0, 13, 'I', 'H', 'D', 'R', 1, 2, 3, 4}

    assert(u16be_matches(&u16_data[0]))
    assert(u32be_matches(&u32_data[0]))
    assert(u64be_matches(&u64_data[0]))
    assert(signature_matches(&u64_data[0]))
    assert(chunk_type_switch_matches(&idat_data[0]))
    assert(signature_range_switch_matches(&u64_data[0]))
    assert(signature_literal_switch_matches())
    assert(context_reads(stream_data[:]))
}
