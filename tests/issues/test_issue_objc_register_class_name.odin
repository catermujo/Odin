package main
import "base:intrinsics"
main :: proc() {
    c := intrinsics.objc_register_class("OdinAuditFreshRoot")
    assert(c != nil)
}
