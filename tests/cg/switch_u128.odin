package main

U128_HIGH_CASE :: (u128(1) << 96) + 8
U128_TOP_CASE  :: (u128(1) << 127) + 23

@(export)
switch_u128 :: proc(value: u128) -> u128 {
	switch value {
	case 0:               return 1
	case 8:               return 2
	case U128_HIGH_CASE:  return 3
	case U128_TOP_CASE:   return 4
	case:                 return 5
	}
}

I128_HIGH_CASE     :: (i128(1) << 96) + 8
I128_NEG_HIGH_CASE :: -(i128(1) << 100) + 8

@(export)
switch_i128 :: proc(value: i128) -> i128 {
	switch value {
	case -8:                 return 1
	case 8:                  return 2
	case I128_HIGH_CASE:     return 3
	case I128_NEG_HIGH_CASE: return 4
	case:                    return 5
	}
}

@(export)
switch_typeid :: proc(value: typeid) -> i32 {
	switch value {
	case i32:    return 32
	case string: return 64
	case:        return 0
	}
}

main :: proc() {}
