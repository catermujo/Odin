#+test
package test_core_odin_parser

import "base:runtime"
import "core:odin/ast"
import "core:odin/parser"
import "core:testing"

@(test)
test_parse_file_scope_with_recovers :: proc(t: ^testing.T) {
	context.allocator = context.temp_allocator
	runtime.DEFAULT_TEMP_ALLOCATOR_TEMP_GUARD()

	file := ast.File{
		fullpath = "file_scope_with.odin",
		src = "package sample\nwith ui() {}\nafter :: 42\n",
	}
	p := parser.default_parser()
	p.err = nil
	p.warn = nil

	parser.parse_file(&p, &file)
	testing.expect(t, file.syntax_error_count > 0)
	testing.expect_value(t, p.curr_tok.kind, .EOF)
	after := file.decls[len(file.decls)-1].derived.(^ast.Value_Decl)
	name := after.names[0].derived.(^ast.Ident)
	testing.expect_value(t, name.name, "after")
}
