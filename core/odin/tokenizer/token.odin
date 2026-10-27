package odin_tokenizer

import "base:runtime"
import "core:strings"
import "core:sync"

Token :: struct {
	kind: Token_Kind,
	text: string,
	pos:  Pos,
}

// DUMBAI: Every AST node stores two positions. An interned id keeps the file without a string per position.
File_Id :: distinct u32

Pos :: struct {
	file:   File_Id, // zero means no file
	offset: i32, // starting at 0
	line:   i32, // starting at 1
	column: i32, // starting at 1
}

@(private = "file")
file_table: struct {
	mutex: sync.Mutex,
	ids:   map[string]File_Id,
	paths: [dynamic]string,
}

// DUMBAI: The table owns heap copies of the paths, so ids stay valid after the caller frees its source buffers.
file_id :: proc(path: string) -> File_Id {
	if path == "" {
		return 0
	}
	sync.guard(&file_table.mutex)
	if id, found := file_table.ids[path]; found {
		return id
	}
	allocator := runtime.heap_allocator()
	if file_table.paths == nil {
		file_table.ids = make(map[string]File_Id, allocator)
		file_table.paths = make([dynamic]string, 1, allocator)
	}
	owned := strings.clone(path, allocator)
	id := File_Id(len(file_table.paths))
	append(&file_table.paths, owned)
	file_table.ids[owned] = id
	return id
}

file_path :: proc(id: File_Id) -> string {
	if id == 0 {
		return ""
	}
	sync.guard(&file_table.mutex)
	return file_table.paths[id]
}

pos_compare :: proc(lhs, rhs: Pos) -> int {
	if lhs.offset != rhs.offset {
		return -1 if (lhs.offset < rhs.offset) else +1
	}
	if lhs.line != rhs.line {
		return -1 if (lhs.line < rhs.line) else +1
	}
	if lhs.column != rhs.column {
		return -1 if (lhs.column < rhs.column) else +1
	}
	if lhs.file == rhs.file {
		return 0
	}
	return strings.compare(file_path(lhs.file), file_path(rhs.file))
}

Token_Kind :: enum u32 {
	Invalid,
	EOF,
	Comment,
	File_Tag,

	B_Literal_Begin,
		Ident,   // main
		Integer, // 12345
		Float,   // 123.45
		Imag,    // 123.45i
		Rune,    // 'a'
		String,  // "abc"
	B_Literal_End,

	B_Operator_Begin,
		Eq,       // =
		Not,      // !
		Hash,     // #
		At,       // @
		Dollar,   // $
		Pointer,  // ^
		Question, // ?
		Add,      // +
		Sub,      // -
		Mul,      // *
		Quo,      // /
		Mod,      // %
		Mod_Mod,  // %%
		And,      // &
		Or,       // |
		Xor,      // ~
		And_Not,  // &~
		Shl,      // <<
		Shr,      // >>

		Cmp_And,  // &&
		Cmp_Or,   // ||

		Mul_Mul,  // **

	B_Assign_Op_Begin,
		Add_Eq,     // +=
		Sub_Eq,     // -=
		Mul_Eq,     // *=
		Quo_Eq,     // /=
		Mod_Eq,     // %=
		Mod_Mod_Eq, // %%=
		And_Eq,     // &=
		Or_Eq,      // |=
		Xor_Eq,     // ~=
		And_Not_Eq, // &~=
		Shl_Eq,     // <<=
		Shr_Eq,     // >>=
		Cmp_And_Eq, // &&=
		Cmp_Or_Eq,  // ||=
	B_Assign_Op_End,

		Increment,          // ++
		Decrement,          // --
		Arrow_Right,        // ->
		Undef,              // ---

	B_Comparison_Begin,
		Cmp_Eq, // ==
		Not_Eq, // !=
		Lt,     // <
		Gt,     // >
		Lt_Eq,  // <=
		Gt_Eq,  // >=
	B_Comparison_End,

		Open_Paren,    // (
		Close_Paren,   // )
		Open_Bracket,  // [
		Close_Bracket, // ]
		Open_Brace,    // {
		Close_Brace,   // }
		Colon,         // :
		Semicolon,     // ;
		Period,        // .
		Comma,         // ,
		Ellipsis,      // ..
		Range_Half,    // ..<
		Range_Full,    // ..=
	B_Operator_End,

	B_Keyword_Begin,
		Import,      // import
		Foreign,     // foreign
		Package,     // package
		Typeid,      // typeid
		When,        // when
		With,        // with
		Where,       // where
		If,          // if
		Else,        // else
		For,         // for
		Switch,      // switch
		In,          // in
		Not_In,      // not_in
		Do,          // do
		Case,        // case
		Break,       // break
		Continue,    // continue
		Fallthrough, // fallthrough
		Defer,       // defer
		Return,      // return
		Proc,        // proc
		Lambda,      // lambda
		Struct,      // struct
		Union,       // union
		Enum,        // enum
		Bit_Set,     // bit_set
		Bit_Field,   // bit_field
		Map,         // map
		Dynamic,     // dynamic
		Auto_Cast,   // auto_cast
		Cast,        // cast
		Transmute,   // transmute
		Distinct,    // distinct
		Using,       // using
		Context,     // context
		Or_Else,     // or_else
		Or_Return,   // or_return
		Or_Break,    // or_break
		Or_Continue, // or_continue
		Asm,         // asm
		Matrix,      // matrix
	B_Keyword_End,

	COUNT,

	B_Custom_Keyword_Begin = COUNT+1,
	// ... Custom keywords
}

tokens := [Token_Kind.COUNT]string {
	"Invalid",
	"EOF",
	"Comment",
	"FileTag",

	"",
	"identifier",
	"integer",
	"float",
	"imaginary",
	"rune",
	"string",
	"",

	"",
	"=",
	"!",
	"#",
	"@",
	"$",
	"^",
	"?",
	"+",
	"-",
	"*",
	"/",
	"%",
	"%%",
	"&",
	"|",
	"~",
	"&~",
	"<<",
	">>",

	"&&",
	"||",

	"**",

	"",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"%%=",
	"&=",
	"|=",
	"~=",
	"&~=",
	"<<=",
	">>=",
	"&&=",
	"||=",
	"",

	"++",
	"--",
	"->",
	"---",

	"",
	"==",
	"!=",
	"<",
	">",
	"<=",
	">=",
	"",

	"(",
	")",
	"[",
	"]",
	"{",
	"}",
	":",
	";",
	".",
	",",
	"..",
	"..<",
	"..=",
	"",

	"",
	"import",
	"foreign",
	"package",
	"typeid",
	"when",
	"with",
	"where",
	"if",
	"else",
	"for",
	"switch",
	"in",
	"not_in",
	"do",
	"case",
	"break",
	"continue",
	"fallthrough",
	"defer",
	"return",
	"proc",
	"lambda",
	"struct",
	"union",
	"enum",
	"bit_set",
	"bit_field",
	"map",
	"dynamic",
	"auto_cast",
	"cast",
	"transmute",
	"distinct",
	"using",
	"context",
	"or_else",
	"or_return",
	"or_break",
	"or_continue",
	"asm",
	"matrix",
	"",
}

custom_keyword_tokens: []string


is_newline :: proc(tok: Token) -> bool {
	return tok.kind == .Semicolon && tok.text == "\n"
}


token_to_string :: proc(tok: Token) -> string {
	if is_newline(tok) {
		return "newline"
	}
	return to_string(tok.kind)
}

to_string :: proc(kind: Token_Kind) -> string {
	if .Invalid <= kind && kind < .COUNT {
		return tokens[kind]
	}
	if .B_Custom_Keyword_Begin < kind {
		n := int(u16(kind)-u16(Token_Kind.B_Custom_Keyword_Begin))
		if n < len(custom_keyword_tokens) {
			return custom_keyword_tokens[n]
		}
	}

	return "Invalid"
}

is_literal  :: proc(kind: Token_Kind) -> bool {
	return .B_Literal_Begin  < kind && kind < .B_Literal_End
}
is_operator :: proc(kind: Token_Kind) -> bool {
	#partial switch kind {
	case .B_Operator_Begin ..= .B_Operator_End:
		return true
	case .In, .Not_In:
		return true
	case .If:
		return true
	}
	return false
}
is_assignment_operator :: proc(kind: Token_Kind) -> bool {
	return .B_Assign_Op_Begin < kind && kind < .B_Assign_Op_End || kind == .Eq
}
is_keyword :: proc(kind: Token_Kind) -> bool {
	switch {
	case .B_Keyword_Begin < kind && kind < .B_Keyword_End:
		return true
	case .B_Custom_Keyword_Begin < kind:
		return true
	}
	return false
}
