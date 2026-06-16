package main
import "base:runtime"
V :: struct {
    x, y: int,
}
footer :: proc "contextless" (s: ^#soa[]V) -> ^runtime.Raw_SOA_Footer_Slice {
    return runtime.raw_soa_footer_slice(s)
}
footer_dynamic :: proc "contextless" (s: ^#soa[dynamic]V) -> ^runtime.Raw_SOA_Footer_Dynamic_Array {
    return runtime.raw_soa_footer_dynamic_array(s)
}

main :: proc() {
    s: #soa[]V
    _ = footer(&s)
    d: #soa[dynamic]V
    _ = footer_dynamic(&d)
}
