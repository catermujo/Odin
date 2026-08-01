package main

import "core:odin/ast"
import "core:odin/parser"
import "core:odin/tokenizer"
import "core:strings"

main :: proc() {
	file := ast.new(ast.File, tokenizer.Pos{}, tokenizer.Pos{})
	file.fullpath = "test.odin"
	file.src = "package test\n@(private = \"file\")\nthing :: proc(seed: u64, #no_alias state: ^int) {}\n"
	state := parser.default_parser()
	tokenizer.init(&state.tok, file.src, file.fullpath, state.err)
	saved := state
	_ = parser.advance_token(&state)
	state = saved
	_ = parser.advance_token(&state)
	assert(state.curr_tok.kind == .Package)
	for state.curr_tok.kind != .Hash {
		_ = parser.advance_token(&state)
	}
	assert(state.curr_tok.kind == .Hash)
	peeked := parser.peek_token(&state)
	assert(peeked.kind == .Ident && peeked.text == "no_alias")
	_ = parser.advance_token(&state)
	assert(state.curr_tok.kind == .Ident && state.curr_tok.text == "no_alias")
	tok: tokenizer.Token
	tok = state.curr_tok
	assert(tok.kind == .Ident && tok.text == "no_alias")
	found := false
	for flag in ast.field_hash_flag_strings {
		if flag.key == tok.text {
			found = true
		}
	}
	assert(found)

	p := parser.default_parser()
	_ = parser.parse_file(&p, file)
	assert(len(file.decls) == 1)
	#partial switch decl in file.decls[0].derived {
	case ^ast.Value_Decl:
		assert(len(decl.attributes) == 1)
		attr := decl.attributes[0]
		assert(file.src[attr.pos.offset] == '@')
		assert(file.src[attr.end.offset-1] == ')')
		attr_text := file.src[attr.pos.offset:attr.end.offset]
		assert(attr_text == "@(private = \"file\")")
		assert(strings.contains("@(private = \"file\")", "private"))
		assert(strings.contains(attr_text, "private"))
	case:
		assert(false)
	}
}
