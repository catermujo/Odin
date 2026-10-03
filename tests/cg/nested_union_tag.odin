package main

import "core:strings"

Button :: enum u8 { left = 3, right = 11 }
Scan   :: enum u8 { key = 5 }
Mouse  :: enum u8 { primary = 7 }
Pad    :: enum u8 { a = 13 }

Button_Or_Scan :: union {
	Button,
	Scan,
	Mouse,
	Pad,
}

Scalar :: struct {
	data: [12]u8,
}

Vector :: struct {
	data: [12]u8,
}

Token :: union {
	Button_Or_Scan,
	Scalar,
	Vector,
}

Atom :: struct {
	token: Token,
	using extra: struct #raw_union {
		flag: bool,
		count: u64,
	},
}

Step :: struct {
	atoms: [dynamic]Atom,
}

Branch :: struct {
	steps: [dynamic]Step,
}

parse_button :: proc() -> (Token, bool) {
	return Button_Or_Scan(Button.right), true
}

parse_pad :: proc() -> (Pad, bool) {
	return .a, true
}

parse_button_spec :: proc(spec: string) -> (Token, bool) {
	sep := strings.index_byte(spec, ':')
	if sep <= 0 || sep >= len(spec)-1 do return {}, false
	if spec[:sep] != "pad" || spec[sep+1:] != "right" do return {}, false
	source, source_ok := parse_pad()
	if !source_ok do return {}, false
	return Button_Or_Scan(source), true
}

main :: proc() {
	token, ok := parse_button()
	assert(ok)
	parsed, parsed_ok := parse_button_spec("pad:right")
	assert(parsed_ok)
	step: Step
	append(&step.atoms, Atom{token = Token(Scalar{})})
	append(&step.atoms, Atom{token = token})
	defer delete(step.atoms)

	branch: Branch
	append(&branch.steps, step)
	defer delete(branch.steps)

	switch value in branch.steps[0].atoms[1].token {
	case Button_Or_Scan:
		switch source in value {
		case Button:
			assert(source == .right)
		case Scan:
			assert(false)
		case Mouse:
			assert(false)
		case Pad:
			assert(false)
		}
	case Scalar, Vector:
		assert(false)
	}

	switch value in parsed {
	case Button_Or_Scan:
		switch source in value {
		case Button:
			assert(false)
		case Scan:
			assert(false)
		case Mouse:
			assert(false)
		case Pad:
			assert(source == .a)
		}
	case Scalar, Vector:
		assert(false)
	}
}
