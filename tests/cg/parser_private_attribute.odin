package main

import "core:odin/ast"
import "core:odin/parser"
import "core:odin/tokenizer"
import "core:strings"

parse_private :: proc() {
	file := ast.new(ast.File, tokenizer.Pos{}, tokenizer.Pos{})
	file.fullpath = "test.odin"
	file.src = "package test\n@(private = \"package\")\nhidden :: proc() {}\n"
	p := parser.default_parser()
	assert(parser.parse_file(&p, file))
	assert(len(file.decls) == 1)
	decl, ok := file.decls[0].derived.(^ast.Value_Decl)
	assert(ok && len(decl.attributes) == 1)
	attr := decl.attributes[0]
	attr_text := strings.trim_space(file.src[attr.pos.offset:attr.end.offset])
	assert(strings.index("testing", "test") == 0)
	assert(strings.index("xxprivate", "private") == 2)
	assert(strings.index(attr_text, "private") == 2)
	assert(strings.contains(attr_text, "private"))
}

main :: proc() {
	parse_private()
	parse_private()
}
