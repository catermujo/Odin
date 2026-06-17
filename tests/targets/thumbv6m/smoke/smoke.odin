package smoke

UP_BIT    :: u32(1 << 0)
DOWN_BIT  :: u32(1 << 1)
LEFT_BIT  :: u32(1 << 2)
RIGHT_BIT :: u32(1 << 3)

BUTTON_MASK :: u32(0xff << 4)

thumbv6m_smoke_step :: proc "c" (previous_state, raw_inputs: u32) -> u32 {
	next_state := previous_state
	direction: u32 = 0

	if raw_inputs & UP_BIT != 0 {
		direction = 1
	} else if raw_inputs & DOWN_BIT != 0 {
		direction = 2
	}

	if raw_inputs & LEFT_BIT != 0 {
		direction |= 1 << 2
	} else if raw_inputs & RIGHT_BIT != 0 {
		direction |= 2 << 2
	}

	next_state ~= direction
	next_state = (next_state << 5) | (next_state >> 27)
	next_state ~= (raw_inputs & BUTTON_MASK) >> 1

	return next_state ~ 0x00c0ffee
}
