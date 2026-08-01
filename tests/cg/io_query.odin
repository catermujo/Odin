package main

import "core:io"

main :: proc() {
	stream: io.Stream
	_ = io.query(stream)
}
