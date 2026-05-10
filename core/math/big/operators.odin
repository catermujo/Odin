package math_big

operator_assert_ok :: #force_inline proc(err: Error, op_name: string) {
	assert(err == nil, op_name)
}


@(require_results)
@(operator="+")
operator_add_int_int :: proc(a, b: Int) -> Int {
	a_copy := a
	b_copy := b
	res: Int
	operator_assert_ok(int_add(&res, &a_copy, &b_copy), "big.Int operator '+' failed")
	return res
}

@(require_results)
@(operator="+")
operator_add_rat_rat :: proc(a, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_add_rat(&res, &a_copy, &b_copy), "big.Rat operator '+' failed")
	return res
}

@(require_results)
@(operator="+")
operator_add_rat_int :: proc(a: Rat, b: Int) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_add_int(&res, &a_copy, &b_copy), "big.Rat operator '+' failed")
	return res
}

@(require_results)
@(operator="+")
operator_add_int_rat :: proc(a: Int, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(int_add_rat(&res, &a_copy, &b_copy), "big.Rat operator '+' failed")
	return res
}


@(require_results)
@(operator="-")
operator_sub_int_int :: proc(a, b: Int) -> Int {
	a_copy := a
	b_copy := b
	res: Int
	operator_assert_ok(int_sub(&res, &a_copy, &b_copy), "big.Int operator '-' failed")
	return res
}

@(require_results)
@(operator="-")
operator_sub_rat_rat :: proc(a, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_sub_rat(&res, &a_copy, &b_copy), "big.Rat operator '-' failed")
	return res
}

@(require_results)
@(operator="-")
operator_sub_rat_int :: proc(a: Rat, b: Int) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_sub_int(&res, &a_copy, &b_copy), "big.Rat operator '-' failed")
	return res
}

@(require_results)
@(operator="-")
operator_sub_int_rat :: proc(a: Int, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(int_sub_rat(&res, &a_copy, &b_copy), "big.Rat operator '-' failed")
	return res
}


@(require_results)
@(operator="*")
operator_mul_int_int :: proc(a, b: Int) -> Int {
	a_copy := a
	b_copy := b
	res: Int
	operator_assert_ok(int_mul(&res, &a_copy, &b_copy), "big.Int operator '*' failed")
	return res
}

@(require_results)
@(operator="*")
operator_mul_rat_rat :: proc(a, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_mul_rat(&res, &a_copy, &b_copy), "big.Rat operator '*' failed")
	return res
}

@(require_results)
@(operator="*")
operator_mul_rat_int :: proc(a: Rat, b: Int) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_mul_int(&res, &a_copy, &b_copy), "big.Rat operator '*' failed")
	return res
}

@(require_results)
@(operator="*")
operator_mul_int_rat :: proc(a: Int, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(int_mul_rat(&res, &a_copy, &b_copy), "big.Rat operator '*' failed")
	return res
}


@(require_results)
@(operator="/")
operator_div_int_int :: proc(a, b: Int) -> Int {
	a_copy := a
	b_copy := b
	res: Int
	operator_assert_ok(int_div(&res, &a_copy, &b_copy), "big.Int operator '/' failed")
	return res
}

@(require_results)
@(operator="/")
operator_div_rat_rat :: proc(a, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_div_rat(&res, &a_copy, &b_copy), "big.Rat operator '/' failed")
	return res
}

@(require_results)
@(operator="/")
operator_div_rat_int :: proc(a: Rat, b: Int) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(rat_div_int(&res, &a_copy, &b_copy), "big.Rat operator '/' failed")
	return res
}

@(require_results)
@(operator="/")
operator_div_int_rat :: proc(a: Int, b: Rat) -> Rat {
	a_copy := a
	b_copy := b
	res: Rat
	operator_assert_ok(int_div_rat(&res, &a_copy, &b_copy), "big.Rat operator '/' failed")
	return res
}
