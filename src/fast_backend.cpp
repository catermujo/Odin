struct FastBackendGenerator : LinkerData {
	CheckerInfo *info;
	Checker *checker;
};

gb_internal bool fast_backend_is_local_entity(CheckerInfo *info, Entity *e) {
	if (e == nullptr || info == nullptr || e->pkg == nullptr || e->pkg != info->init_package) {
		return false;
	}
	if ((e->scope->flags & (ScopeFlag_Pkg|ScopeFlag_File)) == 0) {
		return false;
	}
	if (e->parent_proc_decl.load(std::memory_order_relaxed) != nullptr) {
		return false;
	}
	return true;
}

gb_internal BuildBackendKind get_default_build_backend_kind(void) {
	return BuildBackend_LLVM;
}

gb_internal bool fast_backend_build_mode_is_llvm_only(BuildModeKind build_mode) {
	switch (build_mode) {
	case BuildMode_Assembly:
	case BuildMode_LLVM_IR:
		return true;
	}
	return false;
}

gb_internal bool fast_backend_is_supported(String *reason) {
	if (build_context.metrics.arch != TargetArch_amd64 &&
	    build_context.metrics.arch != TargetArch_arm64) {
		if (reason) *reason = str_lit("fast backend currently supports only amd64 and arm64");
		return false;
	}

	if (build_context.optimization_level > 0) {
		if (reason) *reason = str_lit("fast backend rollout is currently limited to -o:none and -o:minimal");
		return false;
	}

	switch (build_context.build_mode) {
	case BuildMode_Object:
		break;
	case BuildMode_Executable:
	case BuildMode_DynamicLibrary:
	case BuildMode_StaticLibrary:
		if (reason) *reason = str_lit("fast backend currently emits only object files");
		return false;
	case BuildMode_Assembly:
	case BuildMode_LLVM_IR:
		if (reason) *reason = str_lit("fast backend does not own assembly or LLVM IR output");
		return false;
	}

	return true;
}

gb_internal bool fast_init_generator(FastBackendGenerator *gen, Checker *c) {
	if (global_error_collector.count != 0) {
		return false;
	}

	if (c->parser->total_token_count < 2) {
		return false;
	}

	linker_data_init(gen, &c->info, c->parser->init_fullpath);
	gen->info = &c->info;
	gen->checker = c;
	return true;
}

gb_internal String fast_backend_get_entity_name(Entity *e) {
	GB_ASSERT(e != nullptr);
	if (e->kind == Entity_Procedure && e->Procedure.link_name.len != 0) {
		return e->Procedure.link_name;
	}

	if (e->pkg == nullptr) {
		return e->token.string;
	}

	gbString w = string_canonical_entity_name(heap_allocator(), e);
	defer (gb_string_free(w));

	String name = copy_string(permanent_allocator(), make_string(cast(u8 const *)w, gb_string_length(w)));
	if (e->kind == Entity_Procedure) {
		e->Procedure.link_name = name;
	}
	return name;
}

gb_internal String fast_backend_mangle_asm_name(String name) {
	if (build_context.metrics.os != TargetOs_darwin) {
		return name;
	}
	return concatenate_strings(permanent_allocator(), str_lit("_"), name);
}

gb_internal bool fast_backend_supported_calling_convention(ProcCallingConvention cc) {
	switch (cc) {
	case ProcCC_Odin:
	case ProcCC_Contextless:
	case ProcCC_CDecl:
		return true;
	}
	return false;
}

gb_internal bool fast_backend_proc_body_is_trivial(Ast *body, Ast **unsupported_stmt) {
	if (body == nullptr || body->kind != Ast_BlockStmt) {
		*unsupported_stmt = body;
		return false;
	}

	for (Ast *stmt : body->BlockStmt.stmts) {
		if (stmt == nullptr) {
			continue;
		}
		switch (stmt->kind) {
		case Ast_EmptyStmt:
			continue;
		case Ast_ReturnStmt:
			if (stmt->ReturnStmt.results.count == 0) {
				continue;
			}
			break;
		}
		*unsupported_stmt = stmt;
		return false;
	}

	return true;
}

gb_internal bool fast_backend_allow_external_symbol(Entity *e) {
	u64 flags = e->flags.load(std::memory_order_relaxed);
	if (flags & EntityFlag_CustomLinkage_Internal) {
		return false;
	}
	if (flags & EntityFlag_CustomLinkage_Strong) {
		return true;
	}
	if (e->Procedure.is_export) {
		return true;
	}
	if (build_context.use_separate_modules) {
		return true;
	}
	return false;
}

gb_internal bool fast_backend_collect_procedures(FastBackendGenerator *gen, Array<Entity *> *procedures) {
	for (Entity *e : gen->info->definitions) {
		if (!fast_backend_is_local_entity(gen->info, e)) {
			continue;
		}

		u64 flags = e->flags.load(std::memory_order_relaxed);
		if (flags & EntityFlag_Disabled) {
			continue;
		}

		switch (e->kind) {
		case Entity_Procedure: {
			if (e->Procedure.is_foreign) {
				continue;
			}

			if (flags & (EntityFlag_Overridden|
			             EntityFlag_Init|
			             EntityFlag_Fini|
			             EntityFlag_Test|
			             EntityFlag_Require|
			             EntityFlag_CustomLinkage_Weak|
			             EntityFlag_CustomLinkage_LinkOnce)) {
				error(e->token, "Fast backend only supports plain top-level procedures right now");
				return false;
			}

			Type *type = base_type(e->type);
			if (type == nullptr || type->kind != Type_Proc) {
				error(e->token, "Fast backend expected a procedure type");
				return false;
			}

			TypeProc *pt = &type->Proc;
			if (pt->param_count != 0 || pt->result_count != 0 || pt->variadic || pt->return_by_pointer || pt->diverging) {
				error(e->token, "Fast backend currently only supports procedures with no parameters and no results");
				return false;
			}
			if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
				error(e->token, "Fast backend does not yet support this calling convention");
				return false;
			}
			if (e->Procedure.generated_from_polymorphic || pt->is_polymorphic) {
				error(e->token, "Fast backend does not yet support polymorphic procedures");
				return false;
			}

			DeclInfo *decl = e->decl_info;
			if (decl == nullptr || decl->proc_lit == nullptr || decl->proc_lit->kind != Ast_ProcLit) {
				error(e->token, "Fast backend expected a concrete procedure body");
				return false;
			}

			Ast *unsupported_stmt = nullptr;
			if (!fast_backend_proc_body_is_trivial(decl->proc_lit->ProcLit.body, &unsupported_stmt)) {
				error(unsupported_stmt ? unsupported_stmt : decl->proc_lit,
				      "Fast backend currently only supports empty procedure bodies and bare `return`");
				return false;
			}

			array_add(procedures, e);
			break;
		}

		case Entity_Variable:
			if (!e->Variable.is_foreign) {
				error(e->token, "Fast backend does not yet support global variables");
				return false;
			}
			break;

		case Entity_Constant:
		case Entity_TypeName:
		case Entity_ProcGroup:
		case Entity_ImportName:
		case Entity_LibraryName:
			break;

		default:
			error(e->token, "Fast backend does not yet support this top-level declaration");
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_write_object_assembly(FastBackendGenerator *gen, Array<Entity *> const &procedures, String asm_path) {
	gbFile file = {};
	gbFileError err = gb_file_open_mode(&file, gbFileMode_Write, cast(char const *)asm_path.text);
	if (err != gbFileError_None) {
		gb_printf_err("Failed to open fast backend assembly output: %.*s\n", LIT(asm_path));
		return false;
	}
	defer (gb_file_close(&file));

	gb_fprintf(&file, ".text\n");

	for (Entity *e : procedures) {
		String symbol = fast_backend_get_entity_name(e);
		String asm_name = fast_backend_mangle_asm_name(symbol);

		if (fast_backend_allow_external_symbol(e)) {
			gb_fprintf(&file, ".globl \"%.*s\"\n", LIT(asm_name));
		}

		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(&file, ".p2align 4, 0x90\n");
		} else {
			gb_fprintf(&file, ".p2align 2\n");
		}

		gb_fprintf(&file, "\"%.*s\":\n", LIT(asm_name));
		gb_fprintf(&file, "\tret\n");
		gb_fprintf(&file, "\n");
	}

	return true;
}

gb_internal bool fast_backend_assemble_object(String asm_path, String obj_path) {
	char const *clang_path = gb_get_env("ODIN_CLANG_PATH", permanent_allocator());
	if (clang_path == nullptr) {
		clang_path = "clang";
	}

	i32 result = system_exec_command_line_app("clang",
		"%s "
		"-c -x assembler "
		"\"%.*s\" "
		"-o \"%.*s\" "
		"-target %.*s "
		"%.*s "
		"",
		clang_path,
		LIT(asm_path),
		LIT(obj_path),
		LIT(build_context.metrics.target_triplet),
		LIT(build_context.extra_assembler_flags));
	if (result != 0) {
		gb_printf_err("executing `clang` for fast backend assembly failed\n");
		return false;
	}

	return true;
}

gb_internal bool fast_generate_code(FastBackendGenerator *gen) {
	GB_ASSERT(gen != nullptr);
	GB_ASSERT(build_context.build_mode == BuildMode_Object);

	auto procedures = array_make<Entity *>(heap_allocator(), 0, gen->info->definitions.count);
	if (!fast_backend_collect_procedures(gen, &procedures)) {
		return false;
	}

	String obj_path = path_to_string(permanent_allocator(), build_context.build_paths[BuildPath_Output]);
	String asm_path = concatenate_strings(permanent_allocator(), gen->output_base, str_lit(".fast.S"));

	if (!fast_backend_write_object_assembly(gen, procedures, asm_path)) {
		return false;
	}
	if (!fast_backend_assemble_object(asm_path, obj_path)) {
		return false;
	}

	array_add(&gen->output_object_paths, obj_path);
	if (build_context.keep_temp_files) {
		array_add(&gen->output_temp_paths, asm_path);
	} else {
		gb_file_remove(cast(char const *)asm_path.text);
	}

	return true;
}
