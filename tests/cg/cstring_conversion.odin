package main

from_cstring :: proc(value: cstring) -> string {
	return string(value)
}

@(export)
from_cstring16 :: proc(value: cstring16) -> cstring16 {
	copy := value
	indirect := &copy
	return indirect^
}

main :: proc() {
	from_cstring(nil)
	from_cstring16(nil)
}
