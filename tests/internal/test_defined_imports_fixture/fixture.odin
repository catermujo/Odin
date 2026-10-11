#+test
package fixture

IMPORTED_CONSTANT :: 7
imported_proc :: proc() -> int { return IMPORTED_CONSTANT }
