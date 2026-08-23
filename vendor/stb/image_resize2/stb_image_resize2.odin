package stb_image_resize2

import "core:c"

@(private)
LIB :: (
             "../lib/stb_image_resize2.lib"      when ODIN_OS == .Windows
        else "../lib/stb_image_resize2.a"        when ODIN_OS == .Linux
        else "../lib/darwin/stb_image_resize2.a" when ODIN_OS == .Darwin
        else "../lib/stb_image_resize2_wasm.o"   when ODIN_ARCH == .wasm32 || ODIN_ARCH == .wasm64p32
        else ""
)

when LIB != "" {
    when !#exists(LIB) {
        #panic("Could not find the compiled STB image resize2 library; run `vendor/stb/src/build_stb.sh`", #trigger_location)
    }
}

foreign import stbir_resize2 {
    LIB when LIB != "" else "system:stb_image_resize2",
}

Pixel_Layout :: enum c.int {
    BGR         = 0,
    CHANNEL_1   = 1,
    CHANNEL_2   = 2,
    RGB         = 3,
    RGBA        = 4,
    CHANNEL_4   = 5,
    BGRA        = 6,
    ARGB        = 7,
    ABGR        = 8,
    RA          = 9,
    AR          = 10,
    RGBA_PM     = 11,
    BGRA_PM     = 12,
    ARGB_PM     = 13,
    ABGR_PM     = 14,
    RA_PM       = 15,
    AR_PM       = 16,
    RGBA_NO_AW  = 11,
    BGRA_NO_AW  = 12,
    ARGB_NO_AW  = 13,
    ABGR_NO_AW  = 14,
    RA_NO_AW    = 15,
    AR_NO_AW    = 16,
}

Edge :: enum c.int {
    CLAMP   = 0,
    REFLECT = 1,
    WRAP    = 2,
    ZERO    = 3,
}

Filter :: enum c.int {
    DEFAULT      = 0,
    BOX          = 1,
    TRIANGLE     = 2,
    CUBICBSPLINE = 3,
    CATMULLROM   = 4,
    MITCHELL     = 5,
    POINT_SAMPLE = 6,
    OTHER        = 7,
}

Data_Type :: enum c.int {
    UINT8            = 0,
    UINT8_SRGB       = 1,
    UINT8_SRGB_ALPHA = 2,
    UINT16           = 3,
    FLOAT            = 4,
    HALF_FLOAT       = 5,
}

Input_Callback :: #type proc "c" (optional_output: rawptr, input_ptr: rawptr, num_pixels, x, y: c.int, ctx: rawptr) -> rawptr
Output_Callback :: #type proc "c" (output_ptr: rawptr, num_pixels, y: c.int, ctx: rawptr)
Kernel_Callback :: #type proc "c" (x, scale: f32, user_data: rawptr) -> f32
Support_Callback :: #type proc "c" (scale: f32, user_data: rawptr) -> f32

Resize :: struct {
    user_data:                     rawptr,
    input_pixels:                  rawptr,
    input_w, input_h:              c.int,
    input_s0, input_t0:            f64,
    input_s1, input_t1:            f64,
    input_cb:                      Input_Callback,
    output_pixels:                 rawptr,
    output_w, output_h:            c.int,
    output_subx, output_suby:      c.int,
    output_subw, output_subh:      c.int,
    output_cb:                     Output_Callback,
    input_stride_in_bytes:         c.int,
    output_stride_in_bytes:        c.int,
    splits:                        c.int,
    fast_alpha:                    c.int,
    needs_rebuild:                 c.int,
    called_alloc:                  c.int,
    input_pixel_layout_public:     Pixel_Layout,
    output_pixel_layout_public:    Pixel_Layout,
    input_data_type:               Data_Type,
    output_data_type:              Data_Type,
    horizontal_filter, vertical_filter: Filter,
    horizontal_edge, vertical_edge:     Edge,
    horizontal_filter_kernel:      Kernel_Callback,
    horizontal_filter_support:     Support_Callback,
    vertical_filter_kernel:        Kernel_Callback,
    vertical_filter_support:       Support_Callback,
    samplers:                      rawptr,
}

@(default_calling_convention="c", link_prefix="stbir_")
foreign stbir_resize2 {
    resize_uint8_srgb :: proc(input_pixels: [^]u8, input_w, input_h, input_stride_in_bytes: c.int,
                              output_pixels: [^]u8, output_w, output_h, output_stride_in_bytes: c.int,
                              pixel_type: Pixel_Layout) -> [^]u8 ---

    resize_uint8_linear :: proc(input_pixels: [^]u8, input_w, input_h, input_stride_in_bytes: c.int,
                                output_pixels: [^]u8, output_w, output_h, output_stride_in_bytes: c.int,
                                pixel_type: Pixel_Layout) -> [^]u8 ---

    resize_float_linear :: proc(input_pixels: [^]f32, input_w, input_h, input_stride_in_bytes: c.int,
                                output_pixels: [^]f32, output_w, output_h, output_stride_in_bytes: c.int,
                                pixel_type: Pixel_Layout) -> [^]f32 ---

    resize :: proc(input_pixels: rawptr, input_w, input_h, input_stride_in_bytes: c.int,
                   output_pixels: rawptr, output_w, output_h, output_stride_in_bytes: c.int,
                   pixel_layout: Pixel_Layout, data_type: Data_Type, edge: Edge, filter: Filter) -> rawptr ---

    resize_init :: proc(resize: ^Resize, input_pixels: rawptr, input_w, input_h, input_stride_in_bytes: c.int,
                        output_pixels: rawptr, output_w, output_h, output_stride_in_bytes: c.int,
                        pixel_layout: Pixel_Layout, data_type: Data_Type) ---

    set_datatypes :: proc(resize: ^Resize, input_type, output_type: Data_Type) ---
    set_pixel_callbacks :: proc(resize: ^Resize, input_cb: Input_Callback, output_cb: Output_Callback) ---
    set_user_data :: proc(resize: ^Resize, user_data: rawptr) ---
    set_buffer_ptrs :: proc(resize: ^Resize, input_pixels: rawptr, input_stride_in_bytes: c.int,
                            output_pixels: rawptr, output_stride_in_bytes: c.int) ---

    set_pixel_layouts :: proc(resize: ^Resize, input_pixel_layout, output_pixel_layout: Pixel_Layout) -> c.int ---
    set_edgemodes :: proc(resize: ^Resize, horizontal_edge, vertical_edge: Edge) -> c.int ---
    set_filters :: proc(resize: ^Resize, horizontal_filter, vertical_filter: Filter) -> c.int ---
    set_filter_callbacks :: proc(resize: ^Resize, horizontal_filter: Kernel_Callback,
                                 horizontal_support: Support_Callback, vertical_filter: Kernel_Callback,
                                 vertical_support: Support_Callback) -> c.int ---
    set_pixel_subrect :: proc(resize: ^Resize, subx, suby, subw, subh: c.int) -> c.int ---
    set_input_subrect :: proc(resize: ^Resize, s0, t0, s1, t1: f64) -> c.int ---
    set_output_pixel_subrect :: proc(resize: ^Resize, subx, suby, subw, subh: c.int) -> c.int ---
    set_non_pm_alpha_speed_over_quality :: proc(resize: ^Resize, speed_over_quality: c.int) -> c.int ---

    build_samplers :: proc(resize: ^Resize) -> c.int ---
    free_samplers :: proc(resize: ^Resize) ---
    resize_extended :: proc(resize: ^Resize) -> c.int ---
    build_samplers_with_splits :: proc(resize: ^Resize, try_splits: c.int) -> c.int ---
    resize_extended_split :: proc(resize: ^Resize, split_start, split_count: c.int) -> c.int ---
}
