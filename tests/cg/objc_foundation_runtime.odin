package main

import Foundation "core:sys/darwin/Foundation"

main :: proc() {
    object := Foundation.alloc(Foundation.Object)
    object = Foundation.init(object)
    assert(object != nil)
    assert(Foundation.retainCount(object) == 1)
    Foundation.release(object)
}
