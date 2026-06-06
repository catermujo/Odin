struct FastLiteralBlob {
	u8 *data;
	i32 size;
	i32 align;
};

struct FastGenerator : LinkerData {
	CheckerInfo *info;
	Checker *checker;
	Array<FastLiteralBlob> literal_blobs;
	Array<Entity *> emitted_entities;
};

enum FastScalarKind {
	FastScalar_Invalid,
	FastScalar_Bool,
	FastScalar_Signed,
	FastScalar_Unsigned,
	FastScalar_Pointer,
};

struct FastScalarType {
	FastScalarKind kind;
	i64 bit_size;
};

struct FastLocalSlot {
	Entity *entity;
	Type *value_type;
	FastScalarType type;
	i32 index;
	i32 offset;
	i32 size;
	i32 align;
	bool is_scalar;
};

struct FastExprSlot {
	Ast *expr;
	FastLocalSlot slot;
};

struct FastLeafProcPlan {
	FastGenerator *gen;
	Entity *entity;
	CheckerInfo *info;
	TypeProc *type;
	Array<Entity *> params;
	Array<FastLocalSlot> slots;
	Array<FastExprSlot> expr_slots;
	Ast *body;
	Type *result_value_type;
	FastScalarType return_type;
	FastLocalSlot result_ptr_slot;
	FastLocalSlot context_slot;
	i32 spill_depth;
	i32 local_stack_size;
	i32 proc_index;
	bool has_calls;
	bool has_defer;
	bool has_context_slot;
	bool has_result;
	bool return_by_pointer;
};

struct FastGlobalVarPlan {
	FastGenerator *gen;
	Entity *entity;
	Type *value_type;
	Ast *direct_init_expr;
	u8 *init_data;
	i32 size;
	i32 align;
	bool has_init_data;
};

struct FastControlContext {
	Ast *label;
	i32 break_label;
	i32 break_scope_count;
	i32 continue_label;
	i32 continue_scope_count;
	i32 fallthrough_label;
	i32 fallthrough_scope_count;
};

struct FastDeferredStmt {
	Ast *stmt;
};

struct FastScopeState {
	Scope *scope;
	isize defer_base;
};

struct FastLeafProcEmitter {
	gbFile *file;
	FastLeafProcPlan *plan;
	Array<FastControlContext> control_stack;
	Array<FastDeferredStmt> deferred_stmts;
	Array<FastScopeState> scope_stack;
	i32 current_spill_depth;
	i32 epilogue_label_index;
	i32 next_label_index;
	bool use_frame;
};

struct FastBackendErrorState {
	i64 error_count;
	i64 warning_count;
	isize error_value_count;
};

gb_internal bool fast_backend_find_slot(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *out);
gb_internal i32 fast_backend_slot_offset(FastLeafProcPlan *plan, FastLocalSlot const &slot);
gb_internal i32 fast_backend_call_expr_spill_depth(AstCallExpr *ce);
gb_internal BuiltinProcId fast_backend_builtin_proc_id(Ast *expr);
gb_internal bool fast_backend_type_is_supported_aggregate(Type *type);
gb_internal bool fast_backend_type_is_supported_value(Type *type, FastScalarType *scalar_type_, bool *is_scalar_);
gb_internal bool fast_backend_add_slot(FastLeafProcPlan *plan, Entity *entity, Type *type);
gb_internal bool fast_backend_find_expr_slot(FastLeafProcPlan *plan, Ast *expr, FastLocalSlot *out);
gb_internal bool fast_backend_add_expr_slot(FastLeafProcPlan *plan, Ast *expr, Type *type, FastLocalSlot *out);
gb_internal FastScalarType fast_backend_context_scalar_type(void);
gb_internal i32 fast_backend_param_limit_from_proc_type(TypeProc *pt);
gb_internal bool fast_backend_expr_scalar_type(Ast *expr, Type *expected_type, FastScalarType *out);
gb_internal bool fast_backend_can_emit_address_expr(FastLeafProcPlan *plan, Ast *expr, Type **type_, FastScalarType *scalar_type_, bool *is_scalar_);
gb_internal bool fast_backend_can_emit_aggregate_call_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_slice_expr(FastLeafProcPlan *plan, AstSliceExpr *se, Type *expected_type);
gb_internal bool fast_backend_can_emit_direct_array_index_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_direct_slice_index_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_direct_struct_selector_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_slice_compound_lit_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_is_slice_compound_lit_expr(Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_scalar_compound_lit_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_constant_aggregate_expr(Type *type, Ast *expr);
gb_internal bool fast_backend_can_emit_aggregate_compare_operand(FastLeafProcPlan *plan, Ast *expr, Type *type);
gb_internal bool fast_backend_type_supports_aggregate_compare(Type *type);
gb_internal bool fast_backend_is_array_binary_expr(Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_array_binary_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_array_binary_operands(FastLeafProcPlan *plan, Type *expected_type, TokenKind op, Ast *lhs, Ast *rhs);
gb_internal bool fast_backend_can_emit_aggregate_compare_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_leaf_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_aggregate_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_value_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_expr_has_call(Ast *expr);
gb_internal bool fast_backend_stmt_has_call(Ast *stmt);
gb_internal bool fast_backend_emit_enter_scope(FastLeafProcEmitter *emitter, Scope *scope);
gb_internal bool fast_backend_emit_leave_scope(FastLeafProcEmitter *emitter, Scope *scope);
gb_internal bool fast_backend_emit_scope_exit_defers(FastLeafProcEmitter *emitter, i32 keep_scope_count);
gb_internal bool fast_backend_emit_stmt(FastLeafProcEmitter *emitter, Ast *stmt);
gb_internal bool fast_backend_find_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, Type **type_, bool *is_global_);
gb_internal bool fast_backend_find_scalar_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, FastScalarType *type_, bool *is_global_);
gb_internal bool fast_backend_exact_value_as_u64(ExactValue value, FastScalarType type, u64 *out);
gb_internal bool fast_backend_serialize_constant_value(Type *type, Ast *expr, u8 *dst);
gb_internal bool fast_backend_emit_address_expr(FastLeafProcEmitter *emitter, Ast *expr, Type **type_);
gb_internal bool fast_backend_emit_call_expr_to_address(FastLeafProcEmitter *emitter, AstCallExpr *ce);
gb_internal bool fast_backend_emit_store_constant_aggregate_to_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_slice_compound_lit_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_slice_expr_to_address(FastLeafProcEmitter *emitter, Type *type, AstSliceExpr *se);
gb_internal bool fast_backend_emit_store_array_binary_op_to_work_address(FastLeafProcEmitter *emitter, Type *type, TokenKind op, Ast *lhs, Ast *rhs);
gb_internal bool fast_backend_emit_store_array_binary_expr_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_work_address(FastLeafProcEmitter *emitter, Type *type, AstCompoundLit *cl);
gb_internal bool fast_backend_emit_store_value_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_value_to_work_address_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_value_to_entity_offset(FastLeafProcEmitter *emitter, Entity *entity, i32 offset, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_value_to_result_pointer_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_value_to_lhs_offset(FastLeafProcEmitter *emitter, Ast *lhs, i32 offset, Type *type, Ast *expr);
gb_internal bool fast_backend_emit_materialize_aggregate_compare_operand(FastLeafProcEmitter *emitter, Ast *expr, Type *type, i32 *temp_bytes);
gb_internal bool fast_backend_emit_leaf_aggregate_compare(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_expr_is_zero_aggregate_value(Type *type, Ast *expr);
gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_lhs(FastLeafProcEmitter *emitter, Ast *lhs, Type *type, AstCompoundLit *cl);
gb_internal bool fast_backend_expr_is_string_literal_value(Type *type, Ast *expr);
gb_internal bool fast_backend_get_string_literal_blob(FastGenerator *gen, Type *type, Ast *expr, i32 *blob_index_, i64 *len_);
gb_internal bool fast_backend_can_emit_builtin_call_expr(FastLeafProcPlan *plan, AstCallExpr *ce, Type *expected_type);
gb_internal bool fast_backend_emit_builtin_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce);
gb_internal i32 fast_backend_slice_expr_spill_depth(AstSliceExpr *se);
gb_internal i32 fast_backend_slice_compound_lit_spill_depth(Ast *expr, Type *expected_type);
gb_internal i32 fast_backend_scalar_compound_lit_spill_depth(Ast *expr, Type *expected_type);
gb_internal i32 fast_backend_array_binary_operands_spill_depth(Ast *lhs, Ast *rhs);
gb_internal i32 fast_backend_array_binary_expr_spill_depth(Ast *expr);
gb_internal i32 fast_backend_aggregate_compare_operand_spill_depth(FastLeafProcPlan *plan, Ast *expr, Type *type);
gb_internal i32 fast_backend_aggregate_compare_expr_spill_depth(Ast *expr);
gb_internal i32 fast_backend_supported_value_expr_spill_depth(Ast *expr, Type *expected_type);
gb_internal i32 fast_backend_builtin_call_spill_depth(AstCallExpr *ce);
gb_internal bool fast_backend_can_emit_raw_data_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_emit_raw_data_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal void fast_backend_make_label_name(char *buffer, isize buffer_size, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_jump_to_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_jump_if_zero(FastLeafProcEmitter *emitter, i32 label_index);
gb_internal bool fast_backend_emit_compare_scalar_at_offset(FastLeafProcEmitter *emitter, i32 lhs_depth, i32 rhs_depth, i32 offset, FastScalarType type, i32 mismatch_label);
gb_internal i32 fast_backend_alloc_label(FastLeafProcEmitter *emitter);
gb_internal void fast_backend_emit_address_of_spill_depth(FastLeafProcEmitter *emitter, i32 depth);
gb_internal void fast_backend_emit_push_work_reg(FastLeafProcEmitter *emitter);
gb_internal void fast_backend_emit_pop_tmp_reg(FastLeafProcEmitter *emitter);
gb_internal bool fast_backend_emit_leaf_direct_array_index_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_leaf_direct_slice_index_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_leaf_direct_struct_selector_expr(FastLeafProcEmitter *emitter, Ast *expr);

gb_internal String fast_backend_hybrid_linked_output_reason(void) {
	switch (build_context.build_mode) {
	case BuildMode_Executable:
	case BuildMode_DynamicLibrary:
		break;
	case BuildMode_Object:
	case BuildMode_StaticLibrary:
	case BuildMode_Assembly:
	case BuildMode_LLVM_IR:
		return {};
	}

	if (build_context.metrics.os != TargetOs_darwin) {
		return str_lit("fast backend linked output currently supports only macOS");
	}
	if (!build_context.use_separate_modules) {
		return str_lit("fast backend linked output requires -use-separate-modules");
	}
	if (build_context.sanitizer_flags != 0) {
		return str_lit("fast backend linked output does not yet support sanitizers");
	}
	if (build_context.lto_kind != LTO_None) {
		return str_lit("fast backend linked output does not yet support LTO");
	}
	return {};
}

gb_internal bool fast_backend_can_fallback_to_llvm_per_entity(void) {
	return fast_backend_hybrid_linked_output_reason().len != 0 ? false : build_context.build_mode == BuildMode_Executable || build_context.build_mode == BuildMode_DynamicLibrary;
}

gb_internal FastBackendErrorState fast_backend_save_error_state(void) {
	mutex_lock(&global_error_collector.mutex);
	FastBackendErrorState state = {};
	state.error_count = global_error_collector.count.load();
	state.warning_count = global_error_collector.warning_count.load();
	state.error_value_count = global_error_collector.error_values.count;
	mutex_unlock(&global_error_collector.mutex);
	return state;
}

gb_internal void fast_backend_restore_error_state(FastBackendErrorState const &state) {
	mutex_lock(&global_error_collector.mutex);
	for (isize i = state.error_value_count; i < global_error_collector.error_values.count; i++) {
		array_free(&global_error_collector.error_values[i].msg);
	}
	array_resize(&global_error_collector.error_values, state.error_value_count);
	global_error_collector.count.store(state.error_count);
	global_error_collector.warning_count.store(state.warning_count);
	global_error_collector.curr_error_value = {};
	global_error_collector.curr_error_value_set.store(false);
	mutex_unlock(&global_error_collector.mutex);
}

gb_internal bool fast_entity_is_local(CheckerInfo *info, Entity *e) {
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

gb_internal bool fast_init_generator(FastGenerator *gen, Checker *c) {
	if (global_error_collector.count != 0) {
		return false;
	}

	if (c->parser->total_token_count < 2) {
		return false;
	}

	linker_data_init(gen, &c->info, c->parser->init_fullpath);
	linker_enable_system_library_linking(gen);
	gen->info = &c->info;
	gen->checker = c;
	gen->literal_blobs = array_make<FastLiteralBlob>(heap_allocator(), 0, 8);
	gen->emitted_entities = array_make<Entity *>(heap_allocator(), 0, c->info.definitions.count);
	return true;
}

gb_internal bool fast_backend_supported_calling_convention(ProcCallingConvention cc) {
	switch (cc) {
	case ProcCC_Odin:
	case ProcCC_Contextless:
	case ProcCC_CDecl:
	case ProcCC_SysV:
	case ProcCC_Win64:
		return true;
	}
	return false;
}

gb_internal bool fast_backend_classify_scalar_type(Type *type, FastScalarType *out) {
	Type *bt = base_type(type);
	if (bt == nullptr) {
		return false;
	}

	if (is_type_boolean(bt)) {
		out->kind = FastScalar_Bool;
		out->bit_size = 1;
		return true;
	}

	if (bt->kind == Type_Enum) {
		bt = bt->Enum.base_type;
	}

	if (bt->kind == Type_Basic &&
	    (bt->Basic.kind == Basic_cstring || bt->Basic.kind == Basic_cstring16 || bt->Basic.kind == Basic_rawptr)) {
		out->kind = FastScalar_Pointer;
		out->bit_size = 8*build_context.metrics.ptr_size;
		return true;
	}

	if (is_type_integer(bt)) {
		out->kind = is_type_unsigned(bt) ? FastScalar_Unsigned : FastScalar_Signed;
		out->bit_size = 8*type_size_of(bt);
		return out->bit_size > 0 && out->bit_size <= 64;
	}

	if (is_type_pointer(type) || bt->kind == Type_MultiPointer) {
		out->kind = FastScalar_Pointer;
		out->bit_size = 8*build_context.metrics.ptr_size;
		return true;
	}

	return false;
}

gb_internal bool fast_backend_type_is_supported_aggregate(Type *type) {
	type = base_type(type);
	if (type == nullptr) {
		return false;
	}
	if (type_size_of(type) <= 0 || type_align_of(type) <= 0) {
		return false;
	}

	switch (type->kind) {
	case Type_Array:
		return fast_backend_type_is_supported_value(type->Array.elem, nullptr, nullptr);

	case Type_DynamicArray:
		return fast_backend_type_is_supported_value(type->DynamicArray.elem, nullptr, nullptr);

	case Type_FixedCapacityDynamicArray:
		return fast_backend_type_is_supported_value(type->FixedCapacityDynamicArray.elem, nullptr, nullptr);

	case Type_Struct:
		if (type->Struct.is_raw_union || type->Struct.soa_kind != StructSoa_None) {
			return false;
		}
		type_set_offsets(type);
		for (Entity *field : type->Struct.fields) {
			if (field == nullptr || field->kind != Entity_Variable) {
				return false;
			}
			if (!fast_backend_type_is_supported_value(field->type, nullptr, nullptr)) {
				return false;
			}
		}
		return true;

	case Type_Slice:
		return fast_backend_type_is_supported_value(type->Slice.elem, nullptr, nullptr);

	case Type_Basic:
		return is_type_string(type) || is_type_string16(type);
	}

	return false;
}

gb_internal bool fast_backend_assign_op_to_binary(TokenKind assign_op, TokenKind *binary_op_) {
	TokenKind binary_op = Token_Invalid;
	switch (assign_op) {
	case Token_AddEq:    binary_op = Token_Add;    break;
	case Token_SubEq:    binary_op = Token_Sub;    break;
	case Token_MulEq:    binary_op = Token_Mul;    break;
	case Token_QuoEq:    binary_op = Token_Quo;    break;
	case Token_ModEq:    binary_op = Token_Mod;    break;
	case Token_AndEq:    binary_op = Token_And;    break;
	case Token_OrEq:     binary_op = Token_Or;     break;
	case Token_XorEq:    binary_op = Token_Xor;    break;
	case Token_AndNotEq: binary_op = Token_AndNot; break;
	case Token_ShlEq:    binary_op = Token_Shl;    break;
	case Token_ShrEq:    binary_op = Token_Shr;    break;
	default:
		return false;
	}
	if (binary_op_) *binary_op_ = binary_op;
	return true;
}

gb_internal bool fast_backend_type_is_supported_value(Type *type, FastScalarType *scalar_type_, bool *is_scalar_) {
	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		if (scalar_type_) *scalar_type_ = scalar_type;
		if (is_scalar_) *is_scalar_ = true;
		return true;
	}

	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}

	if (scalar_type_) *scalar_type_ = {};
	if (is_scalar_) *is_scalar_ = false;
	return true;
}

gb_internal bool fast_backend_scalar_is_unsigned(FastScalarType type) {
	return type.kind == FastScalar_Unsigned || type.kind == FastScalar_Bool || type.kind == FastScalar_Pointer;
}

gb_internal bool fast_backend_scalar_is_integer_like(FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
	case FastScalar_Signed:
	case FastScalar_Unsigned:
		return true;
	}
	return false;
}

gb_internal bool fast_backend_scalar_supports_ordered_cmp(FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Signed:
	case FastScalar_Unsigned:
		return true;
	}
	return false;
}

gb_internal bool fast_backend_scalar_binary_op_supported(TokenKind op, FastScalarType scalar_type) {
	switch (op) {
	case Token_Add:
	case Token_Sub:
	case Token_Mul:
	case Token_Quo:
	case Token_Mod:
	case Token_And:
	case Token_Or:
	case Token_Xor:
	case Token_AndNot:
	case Token_Shl:
	case Token_Shr:
		return fast_backend_scalar_is_integer_like(scalar_type);
	case Token_CmpEq:
	case Token_NotEq:
		return fast_backend_scalar_is_integer_like(scalar_type) || scalar_type.kind == FastScalar_Pointer;
	case Token_Lt:
	case Token_LtEq:
	case Token_Gt:
	case Token_GtEq:
		return fast_backend_scalar_supports_ordered_cmp(scalar_type);
	}
	return false;
}

gb_internal i32 fast_backend_scalar_byte_size(FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
		return 1;
	case FastScalar_Signed:
	case FastScalar_Unsigned:
		return cast(i32)gb_max(type.bit_size/8, cast(i64)1);
	case FastScalar_Pointer:
		return build_context.metrics.ptr_size;
	}
	return 0;
}

gb_internal String fast_backend_get_entity_name(Entity *e) {
	GB_ASSERT(e != nullptr);
	if (e->kind == Entity_Procedure && e->Procedure.link_name.len != 0) {
		return e->Procedure.link_name;
	} else if (e->kind == Entity_Variable && e->Variable.link_name.len != 0) {
		return e->Variable.link_name;
	}

	if (e->pkg == nullptr) {
		return e->token.string;
	}

	gbString w = string_canonical_entity_name(heap_allocator(), e);
	defer (gb_string_free(w));

	String name = copy_string(permanent_allocator(), make_string(cast(u8 const *)w, gb_string_length(w)));
	if (e->kind == Entity_Procedure) {
		e->Procedure.link_name = name;
	} else if (e->kind == Entity_Variable) {
		e->Variable.link_name = name;
	}
	return name;
}

gb_internal String fast_backend_mangle_asm_name(String name) {
	if (build_context.metrics.os != TargetOs_darwin) {
		return name;
	}
	return concatenate_strings(permanent_allocator(), str_lit("_"), name);
}

gb_internal i32 fast_backend_add_literal_blob(FastGenerator *gen, u8 const *data, i32 size, i32 align) {
	GB_ASSERT(gen != nullptr);
	GB_ASSERT(size >= 0);
	GB_ASSERT(align > 0);

	FastLiteralBlob blob = {};
	blob.size = size;
	blob.align = align;
	if (size > 0) {
		blob.data = gb_alloc_array(heap_allocator(), u8, size);
		gb_memmove(blob.data, data, size);
	}
	array_add(&gen->literal_blobs, blob);
	return cast(i32)gen->literal_blobs.count-1;
}

gb_internal bool fast_backend_expr_is_string_literal_value(Type *type, Ast *expr) {
	type = default_type(type);
	if (type == nullptr || expr == nullptr || (!is_type_string(type) && !is_type_string16(type))) {
		return false;
	}

	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode != Addressing_Constant) {
		return false;
	}
	if (is_type_string16(type)) {
		return tv.value.kind == ExactValue_String || tv.value.kind == ExactValue_String16;
	}
	return tv.value.kind == ExactValue_String;
}

gb_internal bool fast_backend_get_string_literal_blob(FastGenerator *gen, Type *type, Ast *expr, i32 *blob_index_, i64 *len_) {
	if (gen == nullptr || !fast_backend_expr_is_string_literal_value(type, expr)) {
		return false;
	}

	type = default_type(type);
	TypeAndValue tv = type_and_value_of_expr(expr);
	if (is_type_string16(type)) {
		TEMPORARY_ALLOCATOR_GUARD();
		String16 s16 = {};
		if (tv.value.kind == ExactValue_String16) {
			s16 = tv.value.value_string16;
		} else {
			s16 = string_to_string16(temporary_allocator(), tv.value.value_string);
		}
		if (blob_index_) *blob_index_ = s16.len == 0 ? -1 : fast_backend_add_literal_blob(gen, cast(u8 const *)s16.text, cast(i32)(s16.len*gb_size_of(u16)), 2);
		if (len_) *len_ = s16.len;
		return true;
	}

	String s = tv.value.value_string;
	if (blob_index_) *blob_index_ = s.len == 0 ? -1 : fast_backend_add_literal_blob(gen, s.text, cast(i32)s.len, 1);
	if (len_) *len_ = s.len;
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
	if (e->kind == Entity_Procedure && e->Procedure.is_export) {
		return true;
	}
	if (e->kind == Entity_Variable && e->Variable.is_export) {
		return true;
	}
	if (build_context.use_separate_modules) {
		return true;
	}
	return false;
}

gb_internal BuiltinProcId fast_backend_builtin_proc_id(Ast *expr) {
	Entity *e = entity_of_node(expr);
	if (e != nullptr && e->kind == Entity_Builtin && e->Builtin.id && e->Builtin.id != BuiltinProc_DIRECTIVE) {
		return cast(BuiltinProcId)e->Builtin.id;
	}
	return BuiltinProc_Invalid;
}

gb_internal String fast_backend_label_name(Ast *label) {
	if (label == nullptr) {
		return {};
	}
	if (label->kind == Ast_Ident) {
		return label->Ident.token.string;
	}
	if (label->kind == Ast_Label) {
		return fast_backend_label_name(label->Label.name);
	}
	return {};
}

gb_internal i32 fast_backend_leaf_expr_spill_depth(Ast *expr) {
	if (expr == nullptr) {
		return 0;
	}

	switch (expr->kind) {
	case Ast_ParenExpr:
		return fast_backend_leaf_expr_spill_depth(expr->ParenExpr.expr);
	case Ast_TypeCast:
		return fast_backend_leaf_expr_spill_depth(expr->TypeCast.expr);
	case Ast_AutoCast:
		return fast_backend_leaf_expr_spill_depth(expr->AutoCast.expr);
	case Ast_UnaryExpr:
		return fast_backend_leaf_expr_spill_depth(expr->UnaryExpr.expr);
	case Ast_BinaryExpr: {
		if (fast_backend_type_supports_aggregate_compare(default_type(type_of_expr(expr->BinaryExpr.left))) &&
		    (expr->BinaryExpr.op.kind == Token_CmpEq || expr->BinaryExpr.op.kind == Token_NotEq)) {
			return fast_backend_aggregate_compare_expr_spill_depth(expr);
		}
		i32 left_depth  = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.left);
		i32 right_depth = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.right);
		return gb_max(left_depth, 1 + right_depth);
	}
	case Ast_CallExpr: {
		if (expr->CallExpr.proc != nullptr && type_and_value_of_expr(expr->CallExpr.proc).mode == Addressing_Type) {
			if (expr->CallExpr.args.count != 1) {
				return 0;
			}
			Ast *arg = expr->CallExpr.args[0];
			if (arg != nullptr && arg->kind == Ast_FieldValue) {
				arg = arg->FieldValue.value;
			}
			return fast_backend_leaf_expr_spill_depth(arg);
		}
		if (fast_backend_builtin_proc_id(expr->CallExpr.proc) != BuiltinProc_Invalid) {
			return fast_backend_builtin_call_spill_depth(&expr->CallExpr);
		}
		return fast_backend_call_expr_spill_depth(&expr->CallExpr);
	}
	}

	return 0;
}

gb_internal i32 fast_backend_address_expr_spill_depth(Ast *expr) {
	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return 0;
	}

	switch (expr->kind) {
	case Ast_Ident:
		return 0;

	case Ast_DerefExpr:
		return fast_backend_leaf_expr_spill_depth(expr->DerefExpr.expr);

	case Ast_SelectorExpr: {
		Type *base_expr_type = base_type(type_of_expr(expr->SelectorExpr.expr));
		if (base_expr_type != nullptr && (base_expr_type->kind == Type_Pointer || base_expr_type->kind == Type_MultiPointer)) {
			return fast_backend_leaf_expr_spill_depth(expr->SelectorExpr.expr);
		}
		if (base_expr_type != nullptr &&
		    base_expr_type->kind == Type_Struct &&
		    unparen_expr(expr->SelectorExpr.expr) != nullptr &&
		    unparen_expr(expr->SelectorExpr.expr)->kind == Ast_CompoundLit) {
			Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			return temp_slots + fast_backend_scalar_compound_lit_spill_depth(expr->SelectorExpr.expr, base_value_type);
		}
		if (base_expr_type != nullptr &&
		    base_expr_type->kind == Type_Struct &&
		    unparen_expr(expr->SelectorExpr.expr) != nullptr &&
		    unparen_expr(expr->SelectorExpr.expr)->kind == Ast_CallExpr) {
			Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			return temp_slots + fast_backend_call_expr_spill_depth(&unparen_expr(expr->SelectorExpr.expr)->CallExpr);
		}
		if (base_expr_type != nullptr && base_expr_type->kind == Type_Struct && fast_backend_can_emit_constant_aggregate_expr(default_type(type_of_expr(expr->SelectorExpr.expr)), expr->SelectorExpr.expr)) {
			Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			return temp_slots + 2;
		}
		return fast_backend_address_expr_spill_depth(expr->SelectorExpr.expr);
	}

	case Ast_IndexExpr: {
		Type *indexed_type = base_type(type_of_expr(expr->IndexExpr.expr));
		i32 base_depth = 0;
		if (indexed_type != nullptr && (indexed_type->kind == Type_Pointer || indexed_type->kind == Type_MultiPointer)) {
			base_depth = fast_backend_leaf_expr_spill_depth(expr->IndexExpr.expr);
		} else if (indexed_type != nullptr &&
		           indexed_type->kind == Type_Array &&
		           fast_backend_is_array_binary_expr(expr->IndexExpr.expr, default_type(type_of_expr(expr->IndexExpr.expr)))) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			base_depth = temp_slots + fast_backend_array_binary_expr_spill_depth(expr->IndexExpr.expr);
		} else if (indexed_type != nullptr &&
		           (indexed_type->kind == Type_Slice || indexed_type->kind == Type_DynamicArray || is_type_string(indexed_type) || is_type_string16(indexed_type))) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
			if (base_expr != nullptr && base_expr->kind == Ast_SliceExpr) {
				base_depth = temp_slots + fast_backend_slice_expr_spill_depth(&base_expr->SliceExpr);
			} else if (base_expr != nullptr && base_expr->kind == Ast_CallExpr) {
				base_depth = temp_slots + fast_backend_call_expr_spill_depth(&base_expr->CallExpr);
			} else if (base_expr != nullptr && base_expr->kind == Ast_CompoundLit) {
				base_depth = temp_slots + fast_backend_slice_compound_lit_spill_depth(base_expr, base_value_type);
			} else if (fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr)) {
				base_depth = temp_slots + 2;
			} else {
				base_depth = fast_backend_address_expr_spill_depth(expr->IndexExpr.expr);
			}
		} else {
			base_depth = fast_backend_address_expr_spill_depth(expr->IndexExpr.expr);
		}
		i32 index_depth = fast_backend_leaf_expr_spill_depth(expr->IndexExpr.index);
		return gb_max(base_depth, 1 + index_depth);
	}
	}

	return 0;
}

gb_internal bool fast_backend_expr_has_call(Ast *expr) {
	if (expr == nullptr) {
		return false;
	}

	switch (expr->kind) {
	case Ast_ParenExpr:
		return fast_backend_expr_has_call(expr->ParenExpr.expr);
	case Ast_TypeCast:
		return fast_backend_expr_has_call(expr->TypeCast.expr);
	case Ast_AutoCast:
		return fast_backend_expr_has_call(expr->AutoCast.expr);
	case Ast_UnaryExpr:
		return fast_backend_expr_has_call(expr->UnaryExpr.expr);
	case Ast_BinaryExpr:
		return fast_backend_expr_has_call(expr->BinaryExpr.left) ||
		       fast_backend_expr_has_call(expr->BinaryExpr.right);
	case Ast_SelectorExpr:
		return fast_backend_expr_has_call(expr->SelectorExpr.expr);
	case Ast_IndexExpr:
		return fast_backend_expr_has_call(expr->IndexExpr.expr) ||
		       fast_backend_expr_has_call(expr->IndexExpr.index);
	case Ast_SliceExpr:
		return fast_backend_expr_has_call(expr->SliceExpr.expr) ||
		       fast_backend_expr_has_call(expr->SliceExpr.low) ||
		       fast_backend_expr_has_call(expr->SliceExpr.high);
	case Ast_FieldValue:
		return fast_backend_expr_has_call(expr->FieldValue.value);
	case Ast_CompoundLit: {
		ast_node(cl, CompoundLit, expr);
		for (Ast *elem : cl->elems) {
			if (fast_backend_expr_has_call(elem)) {
				return true;
			}
		}
		return false;
	}
	case Ast_CallExpr:
		return true;
	}

	return false;
}

gb_internal bool fast_backend_stmt_has_call(Ast *stmt) {
	if (stmt == nullptr) {
		return false;
	}

	switch (stmt->kind) {
	case Ast_EmptyStmt:
	case Ast_BranchStmt:
		return false;
	case Ast_BlockStmt:
		for (Ast *child : stmt->BlockStmt.stmts) {
			if (fast_backend_stmt_has_call(child)) {
				return true;
			}
		}
		return false;
	case Ast_ValueDecl:
		for (Ast *value : stmt->ValueDecl.values) {
			if (fast_backend_expr_has_call(value)) {
				return true;
			}
		}
		return false;
	case Ast_AssignStmt:
		for (Ast *lhs : stmt->AssignStmt.lhs) {
			if (fast_backend_expr_has_call(lhs)) {
				return true;
			}
		}
		for (Ast *rhs : stmt->AssignStmt.rhs) {
			if (fast_backend_expr_has_call(rhs)) {
				return true;
			}
		}
		return false;
	case Ast_DeferStmt:
		return fast_backend_stmt_has_call(stmt->DeferStmt.stmt);
	case Ast_ExprStmt:
		return fast_backend_expr_has_call(stmt->ExprStmt.expr);
	case Ast_IfStmt:
		return fast_backend_stmt_has_call(stmt->IfStmt.init) ||
		       fast_backend_expr_has_call(stmt->IfStmt.cond) ||
		       fast_backend_stmt_has_call(stmt->IfStmt.body) ||
		       fast_backend_stmt_has_call(stmt->IfStmt.else_stmt);
	case Ast_ForStmt:
		return fast_backend_stmt_has_call(stmt->ForStmt.init) ||
		       fast_backend_expr_has_call(stmt->ForStmt.cond) ||
		       fast_backend_stmt_has_call(stmt->ForStmt.post) ||
		       fast_backend_stmt_has_call(stmt->ForStmt.body);
	case Ast_SwitchStmt: {
		if (fast_backend_stmt_has_call(stmt->SwitchStmt.init) ||
		    fast_backend_expr_has_call(stmt->SwitchStmt.tag)) {
			return true;
		}
		if (stmt->SwitchStmt.body == nullptr || stmt->SwitchStmt.body->kind != Ast_BlockStmt) {
			return false;
		}
		for (Ast *case_stmt : stmt->SwitchStmt.body->BlockStmt.stmts) {
			if (case_stmt == nullptr || case_stmt->kind != Ast_CaseClause) {
				continue;
			}
			for (Ast *expr : case_stmt->CaseClause.list) {
				if (fast_backend_expr_has_call(expr)) {
					return true;
				}
			}
			for (Ast *child : case_stmt->CaseClause.stmts) {
				if (fast_backend_stmt_has_call(child)) {
					return true;
				}
			}
		}
		return false;
	}
	case Ast_ReturnStmt:
		for (Ast *result : stmt->ReturnStmt.results) {
			if (fast_backend_expr_has_call(result)) {
				return true;
			}
		}
		return false;
	}

	return false;
}

gb_internal bool fast_backend_expr_scalar_type(Ast *expr, Type *expected_type, FastScalarType *out) {
	Type *type = reduce_tuple_to_single_type(expected_type);
	if (type == nullptr && expr != nullptr) {
		type = reduce_tuple_to_single_type(type_and_value_of_expr(expr).type);
	}
	if (type == nullptr || is_type_untyped_nil(type)) {
		return false;
	}
	return fast_backend_classify_scalar_type(type, out);
}

gb_internal bool fast_backend_get_selector_info(Ast *expr, Type **value_type_, Type **container_type_, i64 *offset_, bool *base_is_pointer_) {
	if (expr == nullptr || expr->kind != Ast_SelectorExpr) {
		return false;
	}
	AstSelectorExpr *se = &expr->SelectorExpr;
	if (se->selector == nullptr || se->selector->kind != Ast_Ident) {
		return false;
	}

	Type *base_expr_type = reduce_tuple_to_single_type(type_of_expr(se->expr));
	if (base_expr_type == nullptr) {
		return false;
	}

	bool base_is_pointer = false;
	Type *container_type = default_type(base_expr_type);
	Type *base = base_type(container_type);
	if (base != nullptr && (base->kind == Type_Pointer || base->kind == Type_MultiPointer)) {
		base_is_pointer = true;
		container_type = default_type(type_deref(base));
	}
	if (container_type == nullptr) {
		return false;
	}

	Selection sel = lookup_field_with_selection(container_type, se->selector->Ident.interned, false, empty_selection, false);
	if ((sel.entity == nullptr && !sel.pseudo_field) || sel.indirect || sel.is_bit_field || sel.swizzle_count != 0 || sel.index.count == 0) {
		return false;
	}

	Type *value_type = reduce_tuple_to_single_type(type_of_expr(expr));
	if (value_type == nullptr) {
		return false;
	}

	if (value_type_) *value_type_ = value_type;
	if (container_type_) *container_type_ = container_type;
	if (offset_) *offset_ = type_offset_of_from_selection(container_type, sel);
	if (base_is_pointer_) *base_is_pointer_ = base_is_pointer;
	return true;
}

gb_internal bool fast_backend_get_using_entity_info(Entity *entity, Entity **parent_, Type **value_type_, i64 *offset_, bool *parent_is_pointer_) {
	if (entity == nullptr || entity->kind != Entity_Variable || entity->using_parent == nullptr) {
		return false;
	}

	Entity *parent = entity->using_parent;
	Type *container_type = default_type(parent->type);
	if (container_type == nullptr) {
		return false;
	}

	bool parent_is_pointer = false;
	Type *base = base_type(container_type);
	if (base != nullptr && (base->kind == Type_Pointer || base->kind == Type_MultiPointer)) {
		parent_is_pointer = true;
		container_type = default_type(type_deref(base, true));
	}
	if (container_type == nullptr) {
		return false;
	}

	Selection sel = lookup_field_with_selection(container_type, entity_interned_name(entity), false, empty_selection, false);
	if ((sel.entity == nullptr && !sel.pseudo_field) || sel.indirect || sel.is_bit_field || sel.swizzle_count != 0 || sel.index.count == 0) {
		return false;
	}

	if (parent_) *parent_ = parent;
	if (value_type_) *value_type_ = entity->type;
	if (offset_) *offset_ = type_offset_of_from_selection(container_type, sel);
	if (parent_is_pointer_) *parent_is_pointer_ = parent_is_pointer;
	return true;
}

gb_internal bool fast_backend_get_index_info(AstIndexExpr *ie, Type **value_type_, i64 *elem_size_, bool *base_is_pointer_, bool *base_uses_data_pointer_) {
	if (ie == nullptr) {
		return false;
	}

	Type *indexed_type = base_type(type_of_expr(ie->expr));
	if (indexed_type == nullptr) {
		return false;
	}

	Type *value_type = nullptr;
	bool base_is_pointer = false;
	bool base_uses_data_pointer = false;
	switch (indexed_type->kind) {
	case Type_Array:
		value_type = indexed_type->Array.elem;
		break;
	case Type_EnumeratedArray:
		value_type = indexed_type->EnumeratedArray.elem;
		break;
	case Type_Pointer:
		base_is_pointer = true;
		value_type = indexed_type->Pointer.elem;
		break;
	case Type_MultiPointer:
		base_is_pointer = true;
		value_type = indexed_type->MultiPointer.elem;
		break;
	case Type_Slice:
		base_uses_data_pointer = true;
		value_type = indexed_type->Slice.elem;
		break;
	case Type_DynamicArray:
		base_uses_data_pointer = true;
		value_type = indexed_type->DynamicArray.elem;
		break;
	case Type_FixedCapacityDynamicArray:
		value_type = indexed_type->FixedCapacityDynamicArray.elem;
		break;
	case Type_Basic:
		if (indexed_type->Basic.kind == Basic_string || indexed_type->Basic.kind == Basic_UntypedString) {
			base_uses_data_pointer = true;
			value_type = t_u8;
		} else if (indexed_type->Basic.kind == Basic_string16) {
			base_uses_data_pointer = true;
			value_type = t_u16;
		} else if (indexed_type->Basic.kind == Basic_cstring) {
			base_is_pointer = true;
			value_type = t_u8;
		} else if (indexed_type->Basic.kind == Basic_cstring16) {
			base_is_pointer = true;
			value_type = t_u16;
		}
		break;
	}

	if (value_type == nullptr || type_size_of(value_type) <= 0) {
		return false;
	}

	if (value_type_) *value_type_ = value_type;
	if (elem_size_) *elem_size_ = type_size_of(value_type);
	if (base_is_pointer_) *base_is_pointer_ = base_is_pointer;
	if (base_uses_data_pointer_) *base_uses_data_pointer_ = base_uses_data_pointer;
	return true;
}

gb_internal bool fast_backend_can_emit_address_expr(FastLeafProcPlan *plan, Ast *expr, Type **type_, FastScalarType *scalar_type_, bool *is_scalar_) {
	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return false;
	}

	Type *value_type = reduce_tuple_to_single_type(type_of_expr(expr));
	FastScalarType scalar_type = {};
	bool is_scalar = false;

	switch (expr->kind) {
	case Ast_Ident: {
		Entity *entity = expr->Ident.entity.load();
		if (entity == nullptr) {
			return false;
		}
		if (!fast_backend_find_storage(plan, entity, nullptr, &value_type, nullptr)) {
			Entity *parent = nullptr;
			i64 offset = 0;
			bool parent_is_pointer = false;
			if (!fast_backend_get_using_entity_info(entity, &parent, &value_type, &offset, &parent_is_pointer)) {
				return false;
			}
			gb_unused(parent);
			gb_unused(offset);
			gb_unused(parent_is_pointer);
		}
		break;
	}

	case Ast_DerefExpr: {
		Type *pointer_type = base_type(type_of_expr(expr->DerefExpr.expr));
		if (pointer_type == nullptr || (pointer_type->kind != Type_Pointer && pointer_type->kind != Type_MultiPointer)) {
			return false;
		}
		value_type = type_deref(pointer_type);
		if (!fast_backend_can_emit_leaf_expr(plan, expr->DerefExpr.expr, type_of_expr(expr->DerefExpr.expr))) {
			return false;
		}
		break;
	}

	case Ast_SelectorExpr: {
		Type *container_type = nullptr;
		i64 offset = 0;
		bool base_is_pointer = false;
		if (!fast_backend_get_selector_info(expr, &value_type, &container_type, &offset, &base_is_pointer)) {
			return false;
		}
		gb_unused(container_type);
		gb_unused(offset);
		if (base_is_pointer) {
			if (!fast_backend_can_emit_leaf_expr(plan, expr->SelectorExpr.expr, type_of_expr(expr->SelectorExpr.expr))) {
				return false;
			}
		} else if (default_type(type_of_expr(expr->SelectorExpr.expr)) != nullptr &&
		           base_type(default_type(type_of_expr(expr->SelectorExpr.expr)))->kind == Type_Struct) {
			if (!fast_backend_can_emit_address_expr(plan, expr->SelectorExpr.expr, nullptr, nullptr, nullptr) &&
			    !fast_backend_can_emit_direct_struct_selector_expr(plan, expr)) {
				return false;
			}
		} else if (!fast_backend_can_emit_address_expr(plan, expr->SelectorExpr.expr, nullptr, nullptr, nullptr)) {
			return false;
		}
		break;
	}

	case Ast_IndexExpr: {
		i64 elem_size = 0;
		bool base_is_pointer = false;
		bool base_uses_data_pointer = false;
		if (!fast_backend_get_index_info(&expr->IndexExpr, &value_type, &elem_size, &base_is_pointer, &base_uses_data_pointer)) {
			return false;
		}
		gb_unused(elem_size);
		if (base_is_pointer) {
			if (!fast_backend_can_emit_leaf_expr(plan, expr->IndexExpr.expr, type_of_expr(expr->IndexExpr.expr))) {
				return false;
			}
		} else if (default_type(type_of_expr(expr->IndexExpr.expr)) != nullptr &&
		           base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Array) {
			if (!fast_backend_can_emit_address_expr(plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr) &&
			    !fast_backend_can_emit_direct_array_index_expr(plan, expr)) {
				return false;
			}
		} else if (base_uses_data_pointer) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
			if (!fast_backend_can_emit_address_expr(plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr) &&
			    !(base_expr != nullptr &&
			      base_value_type != nullptr &&
			      ((base_expr->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &base_expr->SliceExpr, base_value_type)) ||
			       fast_backend_can_emit_slice_compound_lit_expr(plan, expr->IndexExpr.expr, base_value_type) ||
			       fast_backend_can_emit_aggregate_call_expr(plan, expr->IndexExpr.expr, base_value_type) ||
			       fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr)))) {
				return false;
			}
		} else if (!fast_backend_can_emit_address_expr(plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
			return false;
		}
		if (!fast_backend_can_emit_leaf_expr(plan, expr->IndexExpr.index, type_of_expr(expr->IndexExpr.index))) {
			return false;
		}
		break;
	}

	default:
		return false;
	}

	if (!fast_backend_type_is_supported_value(value_type, &scalar_type, &is_scalar)) {
		return false;
	}

	if (type_) *type_ = value_type;
	if (scalar_type_) *scalar_type_ = scalar_type;
	if (is_scalar_) *is_scalar_ = is_scalar;
	return true;
}

gb_internal bool fast_backend_can_emit_deref_expr(FastLeafProcPlan *plan, Ast *expr) {
	if (expr == nullptr || expr->kind != Ast_DerefExpr) {
		return false;
	}

	Type *pointer_type = base_type(type_of_expr(expr->DerefExpr.expr));
	if (pointer_type == nullptr || (pointer_type->kind != Type_Pointer && pointer_type->kind != Type_MultiPointer)) {
		return false;
	}

	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(nullptr, type_of_expr(expr), &result_type)) {
		return false;
	}
	return fast_backend_can_emit_leaf_expr(plan, expr->DerefExpr.expr, type_of_expr(expr->DerefExpr.expr));
}

gb_internal bool fast_backend_can_emit_cast_expr(FastLeafProcPlan *plan, Ast *operand, Type *target_type, TokenKind cast_kind) {
	FastScalarType source_type = {};
	FastScalarType result_type = {};
	Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(operand).type);
	if (!fast_backend_expr_scalar_type(operand, operand_type, &source_type)) {
		return false;
	}
	if (!fast_backend_expr_scalar_type(nullptr, target_type, &result_type)) {
		return false;
	}
	if (!fast_backend_can_emit_leaf_expr(plan, operand, operand_type)) {
		return false;
	}
	if (cast_kind == Token_transmute) {
		if (fast_backend_scalar_byte_size(source_type) != fast_backend_scalar_byte_size(result_type)) {
			return false;
		}
		if ((source_type.kind == FastScalar_Bool || result_type.kind == FastScalar_Bool) &&
		    source_type.kind != result_type.kind) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_can_emit_switch_case_expr(FastLeafProcPlan *plan, Ast *expr, Type *tag_type) {
	expr = unparen_expr(expr);
	if (is_ast_range(expr)) {
		FastScalarType scalar_type = {};
		if (!fast_backend_classify_scalar_type(tag_type, &scalar_type) ||
		    !fast_backend_scalar_supports_ordered_cmp(scalar_type)) {
			return false;
		}

		Ast *lhs = expr->BinaryExpr.left;
		Ast *rhs = expr->BinaryExpr.right;
		if (fast_backend_expr_has_call(lhs) || fast_backend_expr_has_call(rhs)) {
			return false;
		}
		return fast_backend_can_emit_leaf_expr(plan, lhs, tag_type) &&
		       fast_backend_can_emit_leaf_expr(plan, rhs, tag_type);
	}

	if (fast_backend_expr_has_call(expr)) {
		return false;
	}
	return fast_backend_can_emit_leaf_expr(plan, expr, tag_type);
}

gb_internal bool fast_backend_get_call_info(AstCallExpr *ce, TypeProc **proc_type_, Entity **proc_entity_, Type **result_type_, FastScalarType *scalar_result_type_, bool *has_result_, bool *return_by_pointer_) {
	Entity *proc_entity = ce->entity_procedure_of;
	if (proc_entity == nullptr) {
		proc_entity = entity_from_expr(ce->proc);
	}
	if (proc_entity != nullptr && proc_entity->kind == Entity_ProcGroup) {
		if (proc_entity->ProcGroup.entities.count != 1) {
			return false;
		}
		proc_entity = proc_entity->ProcGroup.entities[0];
	}
	if (proc_entity == nullptr || proc_entity->kind != Entity_Procedure) {
		return false;
	}
	if (proc_entity->Procedure.is_objc_impl_or_import) {
		return false;
	}

	Type *proc_type = base_type(type_of_expr(ce->proc));
	if (proc_type == nullptr || proc_type->kind != Type_Proc) {
		return false;
	}

	TypeProc *pt = &proc_type->Proc;
	if (pt->is_closure || pt->variadic || pt->c_vararg || pt->diverging || pt->is_polymorphic || proc_entity->Procedure.generated_from_polymorphic) {
		return false;
	}
	if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
		return false;
	}
	if (ce->ellipsis.string.len != 0 || ce->optional_ok_one) {
		return false;
	}
	if (ce->split_args != nullptr && ce->split_args->named.count != 0) {
		return false;
	}

	FastScalarType result_type = {};
	Type *call_result_type = nullptr;
	bool has_result = false;
	bool return_by_pointer = false;
	if (pt->result_count > 1) {
		return false;
	}
	if (pt->result_count == 1) {
		Entity *result_entity = pt->results->Tuple.variables[0];
		if (result_entity == nullptr) {
			return false;
		}
		call_result_type = result_entity->type;
		if (!fast_backend_classify_scalar_type(result_entity->type, &result_type)) {
			if (!fast_backend_type_is_supported_aggregate(result_entity->type)) {
				return false;
			}
			return_by_pointer = true;
		}
		has_result = true;
	}

	if (proc_type_) *proc_type_ = pt;
	if (proc_entity_) *proc_entity_ = proc_entity;
	if (result_type_) *result_type_ = call_result_type;
	if (scalar_result_type_) *scalar_result_type_ = result_type;
	if (has_result_) *has_result_ = has_result;
	if (return_by_pointer_) *return_by_pointer_ = return_by_pointer;
	return true;
}

gb_internal bool fast_backend_can_emit_call_expr(FastLeafProcPlan *plan, AstCallExpr *ce, bool allow_void_result) {
	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	Type *result_type = nullptr;
	FastScalarType scalar_result_type = {};
	bool has_result = false;
	bool return_by_pointer = false;
	if (!fast_backend_get_call_info(ce, &pt, &proc_entity, &result_type, &scalar_result_type, &has_result, &return_by_pointer)) {
		return false;
	}
	gb_unused(proc_entity);
	gb_unused(result_type);
	gb_unused(scalar_result_type);

	if (!allow_void_result && !has_result) {
		return false;
	}
	if (cast(i32)ce->args.count != pt->param_count) {
		return false;
	}

	i32 total_param_count = pt->param_count + (pt->calling_convention == ProcCC_Odin ? 1 : 0) + (return_by_pointer ? 1 : 0);
	if (total_param_count > fast_backend_param_limit_from_proc_type(pt)) {
		return false;
	}
	if (pt->calling_convention == ProcCC_Odin && !plan->has_context_slot) {
		return false;
	}

	for_array(i, ce->args) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return false;
		}
		FastScalarType arg_type = {};
		if (fast_backend_classify_scalar_type(param->type, &arg_type)) {
			if (!fast_backend_can_emit_leaf_expr(plan, ce->args[i], param->type)) {
				return false;
			}
		} else {
			if (!fast_backend_type_is_supported_aggregate(param->type)) {
				return false;
			}
			if (!fast_backend_can_emit_aggregate_expr(plan, ce->args[i], param->type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, ce->args[i], param->type)) {
				return false;
			}
		}
	}

	return true;
}

gb_internal bool fast_backend_can_emit_aggregate_call_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (expr == nullptr || expected_type == nullptr) {
		return false;
	}
	expr = unparen_expr(expr);
	if (expr->kind != Ast_CallExpr) {
		return false;
	}

	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	Type *result_type = nullptr;
	FastScalarType scalar_result_type = {};
	bool has_result = false;
	bool return_by_pointer = false;
	if (!fast_backend_get_call_info(&expr->CallExpr, &pt, &proc_entity, &result_type, &scalar_result_type, &has_result, &return_by_pointer)) {
		return false;
	}
	gb_unused(pt);
	gb_unused(proc_entity);
	gb_unused(scalar_result_type);

	return has_result &&
	       return_by_pointer &&
	       result_type != nullptr &&
	       are_types_identical(default_type(result_type), default_type(expected_type)) &&
	       fast_backend_can_emit_call_expr(plan, &expr->CallExpr, false);
}

gb_internal i32 fast_backend_call_expr_spill_depth(AstCallExpr *ce) {
	i32 depth = 0;

	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	Type *result_type = nullptr;
	FastScalarType scalar_result_type = {};
	bool has_result = false;
	bool return_by_pointer = false;
	bool have_call_info = fast_backend_get_call_info(ce, &pt, &proc_entity, &result_type, &scalar_result_type, &has_result, &return_by_pointer);
	gb_unused(proc_entity);
	gb_unused(result_type);
	gb_unused(scalar_result_type);
	gb_unused(has_result);

	i32 leading_slots = return_by_pointer ? 1 : 0;
	for_array(i, ce->args) {
		i32 arg_depth = fast_backend_leaf_expr_spill_depth(ce->args[i]);
		if (have_call_info && pt != nullptr && pt->params != nullptr && pt->params->kind == Type_Tuple) {
			Entity *param = pt->params->Tuple.variables[i];
			FastScalarType param_scalar_type = {};
			if (param != nullptr && !fast_backend_classify_scalar_type(param->type, &param_scalar_type)) {
				Ast *arg = unparen_expr(ce->args[i]);
				if (fast_backend_is_slice_compound_lit_expr(arg, param->type)) {
					arg_depth = fast_backend_slice_compound_lit_spill_depth(arg, param->type);
				} else if (fast_backend_is_array_binary_expr(arg, param->type)) {
					arg_depth = fast_backend_array_binary_expr_spill_depth(arg);
				} else {
					arg_depth = type_and_value_of_expr(arg).mode == Addressing_Constant ? 0 : fast_backend_address_expr_spill_depth(arg);
				}
				if (arg != nullptr && arg->kind == Ast_CallExpr) {
					TypeProc *inner_pt = nullptr;
					Entity *inner_entity = nullptr;
					Type *inner_result_type = nullptr;
					FastScalarType inner_scalar_result_type = {};
					bool inner_has_result = false;
					bool inner_return_by_pointer = false;
					if (fast_backend_get_call_info(&arg->CallExpr, &inner_pt, &inner_entity, &inner_result_type, &inner_scalar_result_type, &inner_has_result, &inner_return_by_pointer) &&
					    inner_has_result &&
					    inner_return_by_pointer &&
					    inner_result_type != nullptr &&
					    are_types_identical(default_type(inner_result_type), default_type(param->type))) {
						arg_depth = 1 + fast_backend_call_expr_spill_depth(&arg->CallExpr);
					}
				}
			}
		}
		depth = gb_max(depth, leading_slots + cast(i32)i + arg_depth);
	}

	i32 total_arg_count = cast(i32)ce->args.count + leading_slots;
	if (have_call_info && pt != nullptr && pt->calling_convention == ProcCC_Odin) {
		total_arg_count += 1;
	} else {
		Type *proc_type = base_type(type_of_expr(ce->proc));
		if (proc_type != nullptr && proc_type->kind == Type_Proc && proc_type->Proc.calling_convention == ProcCC_Odin) {
			total_arg_count += 1;
		}
	}
	return gb_max(depth, total_arg_count);
}

gb_internal bool fast_backend_can_emit_aggregate_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (expr == nullptr || expected_type == nullptr) {
		return false;
	}

	expected_type = default_type(expected_type);
	if (!fast_backend_type_is_supported_aggregate(expected_type)) {
		return false;
	}

	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode == Addressing_Invalid) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(expected_type, expr)) {
		return true;
	}
	if (fast_backend_expr_is_string_literal_value(expected_type, expr)) {
		return are_types_identical(default_type(tv.type), expected_type);
	}

	expr = unparen_expr(expr);
	switch (expr->kind) {
	case Ast_BinaryExpr:
		return fast_backend_can_emit_array_binary_expr(plan, expr, expected_type);

	case Ast_DerefExpr:
	case Ast_SelectorExpr:
	case Ast_IndexExpr: {
		Type *value_type = nullptr;
		if (!fast_backend_can_emit_address_expr(plan, expr, &value_type, nullptr, nullptr)) {
			return false;
		}
		return value_type != nullptr && are_types_identical(default_type(value_type), expected_type);
	}

	case Ast_Ident: {
		Entity *entity = expr->Ident.entity.load();
		Type *storage_type = nullptr;
		if (entity != nullptr && fast_backend_can_emit_address_expr(plan, expr, &storage_type, nullptr, nullptr)) {
			return storage_type != nullptr && are_types_identical(default_type(storage_type), expected_type);
		}
		return tv.mode == Addressing_Constant &&
		       tv.value.kind == ExactValue_Compound &&
		       are_types_identical(default_type(tv.type), expected_type);
	}

	case Ast_CompoundLit:
		if (fast_backend_can_emit_slice_compound_lit_expr(plan, expr, expected_type)) {
			return true;
		}
		if (fast_backend_can_emit_scalar_compound_lit_expr(plan, expr, expected_type)) {
			return true;
		}
		return tv.mode == Addressing_Constant &&
		       tv.value.kind == ExactValue_Compound &&
		       are_types_identical(default_type(tv.type), expected_type);

	case Ast_SliceExpr:
		return fast_backend_can_emit_slice_expr(plan, &expr->SliceExpr, expected_type);
	}

	return false;
}

gb_internal bool fast_backend_can_emit_value_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	FastScalarType scalar_type = {};
	if (fast_backend_expr_scalar_type(expr, expected_type, &scalar_type)) {
		return fast_backend_can_emit_leaf_expr(plan, expr, expected_type);
	}
	return fast_backend_can_emit_aggregate_expr(plan, expr, expected_type);
}

gb_internal bool fast_backend_is_array_binary_expr(Ast *expr, Type *expected_type) {
	expr = unparen_expr(expr);
	Type *type = default_type(expected_type);
	Type *base = base_type(type);
	return expr != nullptr && expr->kind == Ast_BinaryExpr && type != nullptr && base != nullptr && base->kind == Type_Array;
}

gb_internal bool fast_backend_can_emit_array_binary_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (plan == nullptr || !fast_backend_is_array_binary_expr(expr, expected_type)) {
		return false;
	}
	expr = unparen_expr(expr);
	return fast_backend_can_emit_array_binary_operands(plan, expected_type, expr->BinaryExpr.op.kind, expr->BinaryExpr.left, expr->BinaryExpr.right);
}

gb_internal bool fast_backend_can_emit_array_binary_operands(FastLeafProcPlan *plan, Type *expected_type, TokenKind op, Ast *lhs, Ast *rhs) {
	if (plan == nullptr || expected_type == nullptr || lhs == nullptr || rhs == nullptr) {
		return false;
	}

	Type *type = default_type(expected_type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr || base->kind != Type_Array) {
		return false;
	}

	Type *left_type = default_type(type_of_expr(lhs));
	Type *right_type = default_type(type_of_expr(rhs));
	if (!are_types_identical(left_type, type) || !are_types_identical(right_type, type)) {
		return false;
	}

	FastScalarType elem_type = {};
	if (!fast_backend_classify_scalar_type(base->Array.elem, &elem_type) ||
	    !fast_backend_scalar_binary_op_supported(op, elem_type)) {
		return false;
	}

	Type *lhs_storage_type = nullptr;
	Type *rhs_storage_type = nullptr;
	bool lhs_scalar = false;
	bool rhs_scalar = false;
	return fast_backend_can_emit_address_expr(plan, lhs, &lhs_storage_type, nullptr, &lhs_scalar) &&
	       fast_backend_can_emit_address_expr(plan, rhs, &rhs_storage_type, nullptr, &rhs_scalar) &&
	       !lhs_scalar &&
	       !rhs_scalar &&
	       lhs_storage_type != nullptr &&
	       rhs_storage_type != nullptr &&
	       are_types_identical(default_type(lhs_storage_type), type) &&
	       are_types_identical(default_type(rhs_storage_type), type);
}

gb_internal bool fast_backend_can_emit_constant_aggregate_expr(Type *type, Ast *expr) {
	type = default_type(type);
	if (type == nullptr || expr == nullptr) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(type, expr) || fast_backend_expr_is_string_literal_value(type, expr)) {
		return true;
	}

	i32 size = cast(i32)type_size_of(type);
	if (size <= 0) {
		return false;
	}

	auto bytes = gb_alloc_array(temporary_allocator(), u8, size);
	gb_zero_size(bytes, size);
	return fast_backend_serialize_constant_value(type, expr, bytes);
}

gb_internal bool fast_backend_can_emit_aggregate_compare_operand(FastLeafProcPlan *plan, Ast *expr, Type *type) {
	if (plan == nullptr || expr == nullptr || type == nullptr) {
		return false;
	}

	Type *storage_type = nullptr;
	bool is_scalar = false;
	if (fast_backend_can_emit_address_expr(plan, expr, &storage_type, nullptr, &is_scalar) &&
	    !is_scalar &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), default_type(type))) {
		return true;
	}

	if (fast_backend_can_emit_array_binary_expr(plan, expr, type) ||
	    fast_backend_can_emit_scalar_compound_lit_expr(plan, expr, type) ||
	    fast_backend_can_emit_aggregate_call_expr(plan, expr, type) ||
	    fast_backend_can_emit_constant_aggregate_expr(type, expr)) {
		return true;
	}

	expr = unparen_expr(expr);
	return expr != nullptr &&
	       expr->kind == Ast_SliceExpr &&
	       fast_backend_can_emit_slice_expr(plan, &expr->SliceExpr, type);
}

gb_internal bool fast_backend_type_supports_aggregate_compare(Type *type) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}

	switch (base->kind) {
	case Type_Basic:
		return is_type_string(type) || is_type_string16(type);

	case Type_Array: {
		FastScalarType scalar_type = {};
		return fast_backend_classify_scalar_type(base->Array.elem, &scalar_type);
	}

	case Type_Struct:
		type_set_offsets(base);
		for (Entity *field : base->Struct.fields) {
			if (field == nullptr || field->kind != Entity_Variable) {
				return false;
			}
			FastScalarType scalar_type = {};
			if (!fast_backend_classify_scalar_type(field->type, &scalar_type)) {
				return false;
			}
		}
		return true;
	}

	return false;
}

gb_internal bool fast_backend_can_emit_aggregate_compare_expr(FastLeafProcPlan *plan, Ast *expr) {
	if (plan == nullptr || expr == nullptr) {
		return false;
	}
	expr = unparen_expr(expr);
	if (expr->kind != Ast_BinaryExpr) {
		return false;
	}

	TokenKind op = expr->BinaryExpr.op.kind;
	if (op != Token_CmpEq && op != Token_NotEq) {
		return false;
	}

	Type *type = default_type(type_of_expr(expr->BinaryExpr.left));
	Type *right_type = default_type(type_of_expr(expr->BinaryExpr.right));
	if (!are_types_identical(type, right_type) || !fast_backend_type_supports_aggregate_compare(type)) {
		return false;
	}

	return fast_backend_can_emit_aggregate_compare_operand(plan, expr->BinaryExpr.left, type) &&
	       fast_backend_can_emit_aggregate_compare_operand(plan, expr->BinaryExpr.right, type);
}

gb_internal bool fast_backend_is_slice_compound_lit_expr(Ast *expr, Type *expected_type) {
	expr = unparen_expr(expr);
	Type *type = default_type(expected_type);
	return expr != nullptr &&
	       expr->kind == Ast_CompoundLit &&
	       type != nullptr &&
	       base_type(type) != nullptr &&
	       base_type(type)->kind == Type_Slice;
}

gb_internal bool fast_backend_slice_compound_lit_count(AstCompoundLit *cl, i64 *count_) {
	if (cl == nullptr || count_ == nullptr) {
		return false;
	}
	if (cl->elems.count == 0) {
		*count_ = 0;
		return true;
	}
	if (cl->elems[0]->kind != Ast_FieldValue) {
		*count_ = cl->elems.count;
		return true;
	}

	i64 count = 0;
	for (Ast *elem : cl->elems) {
		if (elem == nullptr || elem->kind != Ast_FieldValue) {
			return false;
		}
		ast_node(fv, FieldValue, elem);
		if (is_ast_range(fv->field)) {
			ast_node(range, BinaryExpr, fv->field);
			TypeAndValue lo_tav = type_and_value_of_expr(range->left);
			TypeAndValue hi_tav = type_and_value_of_expr(range->right);
			if (lo_tav.mode != Addressing_Constant || hi_tav.mode != Addressing_Constant) {
				return false;
			}
			i64 lo = exact_value_to_i64(lo_tav.value);
			i64 hi = exact_value_to_i64(hi_tav.value);
			if (range->op.kind != Token_RangeHalf) {
				hi += 1;
			}
			if (lo < 0 || hi < lo) {
				return false;
			}
			count = gb_max(count, hi);
		} else {
			TypeAndValue index_tav = type_and_value_of_expr(fv->field);
			if (index_tav.mode != Addressing_Constant) {
				return false;
			}
			i64 index = exact_value_to_i64(index_tav.value);
			if (index < 0) {
				return false;
			}
			count = gb_max(count, index+1);
		}
	}

	*count_ = count;
	return true;
}

gb_internal bool fast_backend_can_emit_slice_compound_lit_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (plan == nullptr || !fast_backend_is_slice_compound_lit_expr(expr, expected_type)) {
		return false;
	}

	Type *type = default_type(expected_type);
	Type *base = base_type(type);
	Type *elem_type = base->Slice.elem;
	ast_node(cl, CompoundLit, unparen_expr(expr));

	if (cl->elems.count != 0) {
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				if (elem == nullptr || elem->kind != Ast_FieldValue) {
					return false;
				}
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					if (type_and_value_of_expr(range->left).mode != Addressing_Constant ||
					    type_and_value_of_expr(range->right).mode != Addressing_Constant) {
						return false;
					}
				} else if (type_and_value_of_expr(fv->field).mode != Addressing_Constant) {
					return false;
				}

				FastScalarType scalar_type = {};
				if (!fast_backend_expr_scalar_type(fv->value, elem_type, &scalar_type) ||
				    !fast_backend_can_emit_leaf_expr(plan, fv->value, elem_type)) {
					return false;
				}
			}
		} else {
			for (Ast *elem : cl->elems) {
				FastScalarType scalar_type = {};
				if (!fast_backend_expr_scalar_type(elem, elem_type, &scalar_type) ||
				    !fast_backend_can_emit_leaf_expr(plan, elem, elem_type)) {
					return false;
				}
			}
		}
	}

	i64 count = 0;
	if (!fast_backend_slice_compound_lit_count(cl, &count)) {
		return false;
	}
	if (count == 0) {
		return true;
	}

	Type *storage_type = alloc_type_array(elem_type, count);
	return storage_type != nullptr && fast_backend_add_expr_slot(plan, expr, storage_type, nullptr);
}

gb_internal bool fast_backend_can_emit_scalar_compound_lit_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_CompoundLit || expected_type == nullptr) {
		return false;
	}

	Type *type = default_type(expected_type);
	if (type == nullptr) {
		return false;
	}
	Type *base = base_type(type);
	if (base == nullptr) {
		return false;
	}

	ast_node(cl, CompoundLit, expr);
	switch (base->kind) {
	case Type_Struct: {
		type_set_offsets(base);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				if (elem == nullptr || elem->kind != Ast_FieldValue) {
					return false;
				}
				ast_node(fv, FieldValue, elem);
				Entity *field = entity_of_node(fv->field);
				if (field == nullptr || field->kind != Entity_Variable) {
					return false;
				}
				if (!fast_backend_can_emit_value_expr(plan, fv->value, field->type) &&
				    !fast_backend_can_emit_aggregate_call_expr(plan, fv->value, field->type)) {
					return false;
				}
			}
			return true;
		}
		if (cl->elems.count > base->Struct.fields.count) {
			return false;
		}
		for_array(i, cl->elems) {
			Entity *field = base->Struct.fields[i];
			if (field == nullptr) {
				return false;
			}
			if (!fast_backend_can_emit_value_expr(plan, cl->elems[i], field->type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, cl->elems[i], field->type)) {
				return false;
			}
		}
		return true;
	}

	case Type_Array: {
		Type *elem_type = base->Array.elem;
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				if (elem == nullptr || elem->kind != Ast_FieldValue) {
					return false;
				}
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					if (type_and_value_of_expr(range->left).mode != Addressing_Constant ||
					    type_and_value_of_expr(range->right).mode != Addressing_Constant) {
						return false;
					}
				} else if (type_and_value_of_expr(fv->field).mode != Addressing_Constant) {
					return false;
				}
				if (!fast_backend_can_emit_value_expr(plan, fv->value, elem_type) &&
				    !fast_backend_can_emit_aggregate_call_expr(plan, fv->value, elem_type)) {
					return false;
				}
			}
			return true;
		}
		if (cl->elems.count > base->Array.count) {
			return false;
		}
		for (Ast *elem : cl->elems) {
			if (!fast_backend_can_emit_value_expr(plan, elem, elem_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, elem, elem_type)) {
				return false;
			}
		}
		return true;
	}
	}

	return false;
}

gb_internal i32 fast_backend_slice_compound_lit_spill_depth(Ast *expr, Type *expected_type) {
	if (!fast_backend_is_slice_compound_lit_expr(expr, expected_type)) {
		return 0;
	}

	ast_node(cl, CompoundLit, unparen_expr(expr));
	i32 depth = 3;
	for (Ast *elem : cl->elems) {
		Ast *value = elem;
		if (elem != nullptr && elem->kind == Ast_FieldValue) {
			value = elem->FieldValue.value;
		}
		depth = gb_max(depth, 2 + fast_backend_leaf_expr_spill_depth(value));
	}
	return depth;
}

gb_internal i32 fast_backend_array_binary_operands_spill_depth(Ast *lhs, Ast *rhs) {
	i32 depth = 3;
	depth = gb_max(depth, 1 + fast_backend_address_expr_spill_depth(lhs));
	depth = gb_max(depth, 2 + fast_backend_address_expr_spill_depth(rhs));
	return depth;
}

gb_internal i32 fast_backend_array_binary_expr_spill_depth(Ast *expr) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_BinaryExpr) {
		return 0;
	}

	i32 depth = fast_backend_array_binary_operands_spill_depth(expr->BinaryExpr.left, expr->BinaryExpr.right);
	return depth;
}

gb_internal i32 fast_backend_aggregate_compare_expr_spill_depth(Ast *expr) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_BinaryExpr) {
		return 0;
	}

	Type *type = default_type(type_of_expr(expr->BinaryExpr.left));
	i32 depth = 5;
	depth = gb_max(depth, 1 + fast_backend_aggregate_compare_operand_spill_depth(nullptr, expr->BinaryExpr.left, type));
	depth = gb_max(depth, 2 + fast_backend_aggregate_compare_operand_spill_depth(nullptr, expr->BinaryExpr.right, type));
	return depth;
}

gb_internal i32 fast_backend_aggregate_compare_operand_spill_depth(FastLeafProcPlan *plan, Ast *expr, Type *type) {
	gb_unused(plan);

	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return 0;
	}
	if (expr->kind == Ast_CallExpr) {
		return 1 + fast_backend_call_expr_spill_depth(&expr->CallExpr);
	}
	if (expr->kind == Ast_SliceExpr) {
		return 1 + fast_backend_slice_expr_spill_depth(&expr->SliceExpr);
	}
	if (expr->kind == Ast_CompoundLit) {
		return 1 + fast_backend_scalar_compound_lit_spill_depth(expr, type);
	}
	if (expr->kind == Ast_BinaryExpr && default_type(type) != nullptr && base_type(default_type(type))->kind == Type_Array) {
		return 1 + fast_backend_array_binary_expr_spill_depth(expr);
	}
	if (fast_backend_can_emit_constant_aggregate_expr(type, expr)) {
		return 2;
	}
	return fast_backend_address_expr_spill_depth(expr);
}

gb_internal i32 fast_backend_scalar_compound_lit_spill_depth(Ast *expr, Type *expected_type) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_CompoundLit || expected_type == nullptr) {
		return 0;
	}

	Type *type = default_type(expected_type);
	if (type == nullptr) {
		return 0;
	}
	Type *base = base_type(type);
	if (base == nullptr) {
		return 0;
	}

	i32 depth = 0;
	ast_node(cl, CompoundLit, expr);
	switch (base->kind) {
	case Type_Struct:
		for_array(i, cl->elems) {
			Ast *elem = cl->elems[i];
			Ast *value = elem;
			Type *value_type = nullptr;
			if (elem != nullptr && elem->kind == Ast_FieldValue) {
				value = elem->FieldValue.value;
				Entity *field = entity_of_node(elem->FieldValue.field);
				if (field != nullptr) {
					value_type = field->type;
				}
			} else if (i < base->Struct.fields.count) {
				Entity *field = base->Struct.fields[i];
				if (field != nullptr) {
					value_type = field->type;
				}
			}
			depth = gb_max(depth, 1 + fast_backend_supported_value_expr_spill_depth(value, value_type));
		}
		break;
	case Type_Array:
		for (Ast *elem : cl->elems) {
			Ast *value = elem;
			if (elem != nullptr && elem->kind == Ast_FieldValue) {
				value = elem->FieldValue.value;
			}
			depth = gb_max(depth, 1 + fast_backend_supported_value_expr_spill_depth(value, base->Array.elem));
		}
		break;
	}
	return depth;
}

gb_internal i32 fast_backend_supported_value_expr_spill_depth(Ast *expr, Type *expected_type) {
	FastScalarType scalar_type = {};
	if (fast_backend_expr_scalar_type(expr, expected_type, &scalar_type)) {
		return fast_backend_leaf_expr_spill_depth(expr);
	}

	expr = unparen_expr(expr);
	Type *type = default_type(expected_type);
	if (expr == nullptr || type == nullptr) {
		return 0;
	}

	if (expr->kind == Ast_CompoundLit) {
		if (is_type_slice(type)) {
			return fast_backend_slice_compound_lit_spill_depth(expr, type);
		}
		return fast_backend_scalar_compound_lit_spill_depth(expr, type);
	}
	if (expr->kind == Ast_CallExpr) {
		return fast_backend_call_expr_spill_depth(&expr->CallExpr);
	}
	if (expr->kind == Ast_SliceExpr) {
		return fast_backend_slice_expr_spill_depth(&expr->SliceExpr);
	}
	if (expr->kind == Ast_BinaryExpr && base_type(type) != nullptr && base_type(type)->kind == Type_Array) {
		return fast_backend_array_binary_expr_spill_depth(expr);
	}
	if (fast_backend_can_emit_constant_aggregate_expr(type, expr)) {
		return 2;
	}
	return fast_backend_address_expr_spill_depth(expr);
}

gb_internal i32 fast_backend_builtin_call_spill_depth(AstCallExpr *ce) {
	if (ce == nullptr || ce->args.count != 1) {
		return 0;
	}

	BuiltinProcId id = fast_backend_builtin_proc_id(ce->proc);
	if (id != BuiltinProc_len && id != BuiltinProc_cap && id != BuiltinProc_raw_data) {
		return fast_backend_leaf_expr_spill_depth(ce->args.count != 0 ? ce->args[0] : nullptr);
	}

	Ast *arg = ce->args[0];
	Type *arg_type = base_type(type_of_expr(arg));
	bool arg_is_pointer_like = false;
	if (arg_type != nullptr && (arg_type->kind == Type_Pointer || arg_type->kind == Type_MultiPointer)) {
		arg_is_pointer_like = true;
		arg_type = default_type(type_deref(arg_type, true));
	}

	if (id == BuiltinProc_raw_data && arg != nullptr && unparen_expr(arg)->kind == Ast_SliceExpr) {
		return fast_backend_slice_expr_spill_depth(&unparen_expr(arg)->SliceExpr);
	}

	if (id == BuiltinProc_len && arg_type != nullptr &&
	    (is_type_cstring(arg_type) || is_type_cstring16(arg_type))) {
		return fast_backend_leaf_expr_spill_depth(arg);
	}

	if (arg_is_pointer_like) {
		return fast_backend_leaf_expr_spill_depth(arg);
	}
	return fast_backend_address_expr_spill_depth(arg);
}

gb_internal bool fast_backend_can_emit_builtin_call_expr(FastLeafProcPlan *plan, AstCallExpr *ce, Type *expected_type) {
	gb_unused(plan);

	if (ce == nullptr) {
		return false;
	}

	BuiltinProcId id = fast_backend_builtin_proc_id(ce->proc);
	if (id != BuiltinProc_len && id != BuiltinProc_cap && id != BuiltinProc_raw_data) {
		return false;
	}
	if (ce->args.count != 1) {
		return false;
	}

	FastScalarType result_type = {};
	Type *want_type = expected_type ? expected_type : reduce_tuple_to_single_type(type_of_expr(cast(Ast *)ce));
	if (!fast_backend_classify_scalar_type(want_type, &result_type)) {
		return false;
	}

	Ast *arg = ce->args[0];
	Type *arg_type = base_type(type_of_expr(arg));
	bool arg_is_pointer_like = false;
	if (arg_type != nullptr && (arg_type->kind == Type_Pointer || arg_type->kind == Type_MultiPointer)) {
		arg_is_pointer_like = true;
		arg_type = default_type(type_deref(arg_type, true));
	}
	if (arg_type == nullptr) {
		return false;
	}

	if (id == BuiltinProc_len && (is_type_string(arg_type) || is_type_string16(arg_type) || is_type_slice(arg_type))) {
		if (arg_is_pointer_like) {
			return fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg));
		}
		return fast_backend_can_emit_address_expr(plan, arg, nullptr, nullptr, nullptr);
	}

	if (id == BuiltinProc_len && (is_type_cstring(arg_type) || is_type_cstring16(arg_type))) {
		return fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg));
	}

	if (is_type_dynamic_array(arg_type) || is_type_fixed_capacity_dynamic_array(arg_type)) {
		if (arg_is_pointer_like) {
			return fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg));
		}
		return fast_backend_can_emit_address_expr(plan, arg, nullptr, nullptr, nullptr);
	}

	if (id == BuiltinProc_raw_data) {
		return fast_backend_can_emit_raw_data_expr(plan, arg);
	}

	return false;
}

gb_internal bool fast_backend_can_emit_raw_data_expr(FastLeafProcPlan *plan, Ast *expr) {
	if (plan == nullptr || expr == nullptr) {
		return false;
	}

	Type *type = base_type(type_of_expr(expr));
	if (type == nullptr) {
		return false;
	}

	expr = unparen_expr(expr);
	if (expr->kind == Ast_SliceExpr) {
		Type *slice_type = default_type(type_of_expr(expr));
		return slice_type != nullptr &&
		       fast_backend_can_emit_slice_expr(plan, &expr->SliceExpr, slice_type);
	}

	switch (type->kind) {
	case Type_Slice:
	case Type_DynamicArray:
		return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr);

	case Type_Basic:
		if (is_type_string(type) || is_type_string16(type)) {
			return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr);
		}
		break;

	case Type_Pointer:
	case Type_MultiPointer: {
		Type *elem = base_type(type_deref(type, true));
		if (elem == nullptr) {
			return false;
		}
		switch (elem->kind) {
		case Type_Array:
		case Type_EnumeratedArray:
		case Type_SimdVector:
		case Type_FixedCapacityDynamicArray:
		case Type_Matrix:
			return fast_backend_can_emit_leaf_expr(plan, expr, type_of_expr(expr));
		}
		break;
	}
	}

	return false;
}

gb_internal bool fast_backend_can_emit_slice_expr(FastLeafProcPlan *plan, AstSliceExpr *se, Type *expected_type) {
	if (plan == nullptr || se == nullptr || expected_type == nullptr) {
		return false;
	}

	Type *result_type = default_type(expected_type);
	Type *source_type = base_type(type_of_expr(se->expr));
	if (result_type == nullptr || source_type == nullptr) {
		return false;
	}

	bool can_address_source = fast_backend_can_emit_address_expr(plan, se->expr, nullptr, nullptr, nullptr);
	switch (source_type->kind) {
	case Type_Array:
	case Type_EnumeratedArray:
	case Type_Slice:
	case Type_DynamicArray:
	case Type_FixedCapacityDynamicArray:
		if (!is_type_slice(result_type) || !can_address_source) {
			return false;
		}
		break;

	case Type_Basic:
		if ((!is_type_string(source_type) && !is_type_string16(source_type)) ||
		    !are_types_identical(result_type, default_type(source_type)) ||
		    !can_address_source) {
			return false;
		}
		break;

	default:
		return false;
	}

	if (se->low != nullptr && !fast_backend_can_emit_leaf_expr(plan, se->low, type_of_expr(se->low))) {
		return false;
	}
	if (se->high != nullptr && !fast_backend_can_emit_leaf_expr(plan, se->high, type_of_expr(se->high))) {
		return false;
	}
	return true;
}

gb_internal bool fast_backend_can_emit_direct_array_index_expr(FastLeafProcPlan *plan, Ast *expr) {
	expr = unparen_expr(expr);
	if (plan == nullptr || expr == nullptr || expr->kind != Ast_IndexExpr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type)) {
		return false;
	}

	Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
	Type *base_type_ = base_type(base_value_type);
	if (base_value_type == nullptr || base_type_ == nullptr || base_type_->kind != Type_Array) {
		return false;
	}

	Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
	if (base_expr == nullptr || fast_backend_can_emit_address_expr(plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
		return false;
	}

	if (!fast_backend_can_emit_array_binary_expr(plan, expr->IndexExpr.expr, base_value_type) &&
	    !fast_backend_can_emit_scalar_compound_lit_expr(plan, expr->IndexExpr.expr, base_value_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, expr->IndexExpr.expr, base_value_type) &&
	    !fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr)) {
		return false;
	}

	return fast_backend_can_emit_leaf_expr(plan, expr->IndexExpr.index, type_of_expr(expr->IndexExpr.index));
}

gb_internal bool fast_backend_can_emit_direct_slice_index_expr(FastLeafProcPlan *plan, Ast *expr) {
	expr = unparen_expr(expr);
	if (plan == nullptr || expr == nullptr || expr->kind != Ast_IndexExpr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type)) {
		return false;
	}

	Type *value_type = nullptr;
	i64 elem_size = 0;
	bool base_is_pointer = false;
	bool base_uses_data_pointer = false;
	if (!fast_backend_get_index_info(&expr->IndexExpr, &value_type, &elem_size, &base_is_pointer, &base_uses_data_pointer)) {
		return false;
	}
	gb_unused(value_type);
	gb_unused(elem_size);
	if (base_is_pointer || !base_uses_data_pointer) {
		return false;
	}

	Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
	Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
	return base_expr != nullptr &&
	       base_value_type != nullptr &&
	       ((base_expr->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &base_expr->SliceExpr, base_value_type)) ||
	        fast_backend_can_emit_slice_compound_lit_expr(plan, expr->IndexExpr.expr, base_value_type) ||
	        fast_backend_can_emit_aggregate_call_expr(plan, expr->IndexExpr.expr, base_value_type) ||
	        fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr)) &&
	       fast_backend_can_emit_leaf_expr(plan, expr->IndexExpr.index, type_of_expr(expr->IndexExpr.index));
}

gb_internal bool fast_backend_can_emit_direct_struct_selector_expr(FastLeafProcPlan *plan, Ast *expr) {
	expr = unparen_expr(expr);
	if (plan == nullptr || expr == nullptr || expr->kind != Ast_SelectorExpr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type)) {
		return false;
	}

	Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
	Type *base_type_ = base_type(base_value_type);
	if (base_value_type == nullptr || base_type_ == nullptr || base_type_->kind != Type_Struct) {
		return false;
	}

	Ast *base_expr = unparen_expr(expr->SelectorExpr.expr);
	if (base_expr == nullptr || fast_backend_can_emit_address_expr(plan, expr->SelectorExpr.expr, nullptr, nullptr, nullptr)) {
		return false;
	}

	if (!fast_backend_can_emit_scalar_compound_lit_expr(plan, expr->SelectorExpr.expr, base_value_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, expr->SelectorExpr.expr, base_value_type) &&
	    !fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->SelectorExpr.expr)) {
		return false;
	}

	Type *value_type = nullptr;
	Type *container_type = nullptr;
	i64 offset = 0;
	bool base_is_pointer = false;
	return fast_backend_get_selector_info(expr, &value_type, &container_type, &offset, &base_is_pointer) &&
	       !base_is_pointer &&
	       value_type != nullptr;
}

gb_internal i32 fast_backend_slice_expr_spill_depth(AstSliceExpr *se) {
	if (se == nullptr || se->expr == nullptr) {
		return 0;
	}

	i32 depth = 7;
	depth = gb_max(depth, 1 + fast_backend_address_expr_spill_depth(se->expr));
	if (se->low != nullptr) {
		depth = gb_max(depth, 2 + fast_backend_leaf_expr_spill_depth(se->low));
	}
	if (se->high != nullptr) {
		depth = gb_max(depth, 3 + fast_backend_leaf_expr_spill_depth(se->high));
	}
	return depth;
}

gb_internal bool fast_backend_can_emit_leaf_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (expr == nullptr) {
		return false;
	}

	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode == Addressing_Invalid) {
		return false;
	}
	if (is_type_untyped_nil(tv.type)) {
		FastScalarType type = {};
		return fast_backend_classify_scalar_type(expected_type ? expected_type : tv.type, &type);
	}
	if (tv.mode == Addressing_Constant) {
		FastScalarType type = {};
		return fast_backend_classify_scalar_type(expected_type ? expected_type : tv.type, &type);
	}

	switch (expr->kind) {
	case Ast_ParenExpr:
		return fast_backend_can_emit_leaf_expr(plan, expr->ParenExpr.expr, expected_type);

	case Ast_Implicit:
		return plan->has_context_slot && expr->Implicit.kind == Token_context;

	case Ast_Ident: {
		Entity *e = expr->Ident.entity.load();
		if (e == nullptr) {
			return false;
		}
		if (fast_backend_find_scalar_storage(plan, e, nullptr, nullptr, nullptr)) {
			return true;
		}
		Type *value_type = nullptr;
		bool is_scalar = false;
		return fast_backend_can_emit_address_expr(plan, expr, &value_type, nullptr, &is_scalar) && is_scalar;
	}

	case Ast_DerefExpr:
		return fast_backend_can_emit_deref_expr(plan, expr);

	case Ast_SelectorExpr:
		return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr) ||
		       fast_backend_can_emit_direct_struct_selector_expr(plan, expr);

	case Ast_IndexExpr:
		return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr) ||
		       fast_backend_can_emit_direct_array_index_expr(plan, expr) ||
		       fast_backend_can_emit_direct_slice_index_expr(plan, expr);

	case Ast_CallExpr:
		if (expr->CallExpr.proc != nullptr && type_and_value_of_expr(expr->CallExpr.proc).mode == Addressing_Type) {
			if (expr->CallExpr.args.count != 1) {
				return false;
			}
			Ast *arg = expr->CallExpr.args[0];
			if (arg != nullptr && arg->kind == Ast_FieldValue) {
				return false;
			}
			Type *result_type = reduce_tuple_to_single_type(tv.type);
			return fast_backend_can_emit_cast_expr(plan, arg, result_type, Token_cast);
		}
		if (fast_backend_can_emit_builtin_call_expr(plan, &expr->CallExpr, expected_type)) {
			return true;
		}
		return fast_backend_can_emit_call_expr(plan, &expr->CallExpr, false);

	case Ast_TypeCast: {
		Type *result_type = reduce_tuple_to_single_type(tv.type);
		return fast_backend_can_emit_cast_expr(plan, expr->TypeCast.expr, result_type, expr->TypeCast.token.kind);
	}

	case Ast_AutoCast: {
		Type *result_type = reduce_tuple_to_single_type(tv.type);
		return fast_backend_can_emit_cast_expr(plan, expr->AutoCast.expr, result_type, Token_cast);
	}

	case Ast_UnaryExpr: {
		if (expr->UnaryExpr.op.kind == Token_And) {
			return fast_backend_can_emit_address_expr(plan, expr->UnaryExpr.expr, nullptr, nullptr, nullptr);
		}
		Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->UnaryExpr.expr).type);
		FastScalarType operand_scalar = {};
		if (!fast_backend_classify_scalar_type(operand_type, &operand_scalar)) {
			return false;
		}
		if (!fast_backend_can_emit_leaf_expr(plan, expr->UnaryExpr.expr, operand_type)) {
			return false;
		}

		switch (expr->UnaryExpr.op.kind) {
		case Token_Add:
			return true;
		case Token_Sub:
			return operand_scalar.kind == FastScalar_Signed || operand_scalar.kind == FastScalar_Unsigned;
		case Token_Not:
			return fast_backend_scalar_is_integer_like(operand_scalar) || operand_scalar.kind == FastScalar_Pointer;
		case Token_Xor:
			return fast_backend_scalar_is_integer_like(operand_scalar);
		}
		return false;
	}

	case Ast_BinaryExpr: {
		if (fast_backend_can_emit_aggregate_compare_expr(plan, expr)) {
			return true;
		}
		Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->BinaryExpr.left).type);
		FastScalarType operand_scalar = {};
		if (!fast_backend_classify_scalar_type(operand_type, &operand_scalar)) {
			return false;
		}

		if (!fast_backend_can_emit_leaf_expr(plan, expr->BinaryExpr.left, operand_type) ||
		    !fast_backend_can_emit_leaf_expr(plan, expr->BinaryExpr.right, operand_type)) {
			return false;
		}

		switch (expr->BinaryExpr.op.kind) {
		case Token_Add:
		case Token_Sub:
		case Token_Mul:
		case Token_Quo:
		case Token_Mod:
		case Token_And:
		case Token_Or:
		case Token_Xor:
		case Token_AndNot:
			return fast_backend_scalar_is_integer_like(operand_scalar);

		case Token_Shl:
		case Token_Shr:
			return fast_backend_scalar_is_integer_like(operand_scalar);

		case Token_CmpEq:
		case Token_NotEq:
			return fast_backend_scalar_is_integer_like(operand_scalar) || operand_scalar.kind == FastScalar_Pointer;

		case Token_Lt:
		case Token_LtEq:
		case Token_Gt:
		case Token_GtEq:
			return fast_backend_scalar_supports_ordered_cmp(operand_scalar);
		}
		return false;
	}
	}

	return false;
}

gb_internal bool fast_backend_plan_stmt(FastGenerator *gen, FastLeafProcPlan *plan, Ast *stmt);

gb_internal bool fast_backend_plan_stmt_list(FastGenerator *gen, FastLeafProcPlan *plan, Slice<Ast *> const &stmts) {
	for (Ast *stmt : stmts) {
		if (!fast_backend_plan_stmt(gen, plan, stmt)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_plan_value_decl(FastGenerator *gen, FastLeafProcPlan *plan, AstValueDecl *vd) {
	gb_unused(gen);

	if (!vd->is_mutable) {
		return true;
	}
	if (vd->values.count != 0 && vd->values.count != vd->names.count) {
		error(vd->names[0], "Fast backend currently only supports one-to-one value declarations");
		return false;
	}

	for_array(i, vd->names) {
		Ast *name = vd->names[i];
		if (is_blank_ident(name)) {
			continue;
		}

		Entity *entity = entity_of_node(name);
		if (entity == nullptr || entity->kind != Entity_Variable) {
			error(name, "Fast backend expected a local variable entity");
			return false;
		}
		if (entity->flags.load(std::memory_order_relaxed) & EntityFlag_Static) {
			error(name, "Fast backend does not yet support static local variables");
			return false;
		}

		if (!fast_backend_type_is_supported_value(entity->type, nullptr, nullptr)) {
			error(name, "Fast backend currently only supports scalar, pointer-like, array, struct, string, slice, dynamic array, and fixed-capacity dynamic array local variable types");
			return false;
		}
		if (!fast_backend_add_slot(plan, entity, entity->type)) {
			return false;
		}
	}

	for_array(i, vd->values) {
		Ast *rhs = vd->values[i];
		Ast *name = vd->names[i];
		if (is_blank_ident(name)) {
			continue;
		}
		Entity *entity = entity_of_node(name);
		GB_ASSERT(entity != nullptr);

		if (!fast_backend_can_emit_value_expr(plan, rhs, entity->type) &&
		    !fast_backend_can_emit_aggregate_call_expr(plan, rhs, entity->type) &&
		    !fast_backend_can_emit_scalar_compound_lit_expr(plan, rhs, entity->type)) {
			error(rhs, "Fast backend does not yet support this value declaration expression");
			return false;
		}
		FastScalarType scalar_type = {};
		if (fast_backend_expr_scalar_type(rhs, entity->type, &scalar_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(rhs));
		} else if (fast_backend_can_emit_aggregate_call_expr(plan, rhs, entity->type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_call_expr_spill_depth(&unparen_expr(rhs)->CallExpr));
		} else if (fast_backend_can_emit_array_binary_expr(plan, rhs, entity->type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_array_binary_expr_spill_depth(rhs));
		} else if (fast_backend_can_emit_scalar_compound_lit_expr(plan, rhs, entity->type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_scalar_compound_lit_spill_depth(rhs, entity->type));
		} else if (fast_backend_can_emit_slice_compound_lit_expr(plan, rhs, entity->type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_compound_lit_spill_depth(rhs, entity->type));
		} else if (unparen_expr(rhs)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &unparen_expr(rhs)->SliceExpr, entity->type)) {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_expr_spill_depth(&unparen_expr(rhs)->SliceExpr));
		} else if (fast_backend_can_emit_address_expr(plan, rhs, nullptr, nullptr, nullptr)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(1, fast_backend_address_expr_spill_depth(rhs)));
		}
	}

	return true;
}

gb_internal bool fast_backend_plan_assign_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstAssignStmt *as) {
	gb_unused(gen);

	if (as->op.kind != Token_Eq) {
		if (as->lhs.count != 1 || as->rhs.count != 1) {
			error(as->op, "Fast backend assignment operators require one-to-one operands");
			return false;
		}
		TokenKind binary_op = Token_Invalid;
		if (!fast_backend_assign_op_to_binary(as->op.kind, &binary_op)) {
			error(as->op, "Fast backend does not yet support this assignment operator");
			return false;
		}

		Ast *lhs = unparen_expr(as->lhs[0]);
		Ast *rhs = as->rhs[0];
		Type *target_type = nullptr;
		if (!fast_backend_can_emit_address_expr(plan, lhs, &target_type, nullptr, nullptr)) {
			error(lhs, "Fast backend expected a supported assignment target");
			return false;
		}

		FastScalarType scalar_type = {};
		if (!fast_backend_expr_scalar_type(lhs, target_type, &scalar_type)) {
			if (!fast_backend_can_emit_array_binary_operands(plan, target_type, binary_op, lhs, rhs)) {
				error(lhs, "Fast backend currently only supports scalar and fixed-array assignment operators");
				return false;
			}
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_address_expr_spill_depth(lhs), fast_backend_array_binary_operands_spill_depth(lhs, rhs)));
			return true;
		}
		if (!fast_backend_scalar_binary_op_supported(binary_op, scalar_type)) {
			error(as->op, "Fast backend does not yet support this assignment operator for the target type");
			return false;
		}
		if (!fast_backend_can_emit_leaf_expr(plan, rhs, target_type)) {
			error(rhs, "Fast backend does not yet support this assignment expression");
			return false;
		}

		plan->spill_depth = gb_max(plan->spill_depth, gb_max(1 + fast_backend_address_expr_spill_depth(lhs), 2 + fast_backend_leaf_expr_spill_depth(rhs)));
		return true;
	}
	if (as->lhs.count != as->rhs.count) {
		error(as->op, "Fast backend currently only supports one-to-one assignments");
		return false;
	}

	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		Ast *rhs = as->rhs[i];
		if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
			Type *rhs_type = reduce_tuple_to_single_type(type_and_value_of_expr(rhs).type);
			if (!fast_backend_can_emit_value_expr(plan, rhs, rhs_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, rhs, rhs_type)) {
				error(rhs, "Fast backend does not yet support this discard assignment expression");
				return false;
			}
			FastScalarType scalar_type = {};
			if (fast_backend_expr_scalar_type(rhs, rhs_type, &scalar_type)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(rhs));
			} else if (fast_backend_can_emit_aggregate_call_expr(plan, rhs, rhs_type)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_call_expr_spill_depth(&unparen_expr(rhs)->CallExpr));
			} else if (fast_backend_can_emit_array_binary_expr(plan, rhs, rhs_type)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_array_binary_expr_spill_depth(rhs));
			} else if (fast_backend_can_emit_slice_compound_lit_expr(plan, rhs, rhs_type)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_compound_lit_spill_depth(rhs, rhs_type));
			} else if (rhs_type != nullptr && unparen_expr(rhs)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &unparen_expr(rhs)->SliceExpr, rhs_type)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_expr_spill_depth(&unparen_expr(rhs)->SliceExpr));
			}
			continue;
		}

		Type *target_type = nullptr;
		if (!fast_backend_can_emit_address_expr(plan, lhs, &target_type, nullptr, nullptr)) {
			error(lhs, "Fast backend expected a supported assignment target");
			return false;
		}

		if (!fast_backend_can_emit_value_expr(plan, rhs, target_type) &&
		    !fast_backend_can_emit_scalar_compound_lit_expr(plan, rhs, target_type) &&
		    !fast_backend_can_emit_aggregate_call_expr(plan, rhs, target_type)) {
			error(rhs, "Fast backend does not yet support this assignment expression");
			return false;
		}
		FastScalarType scalar_type = {};
		if (fast_backend_expr_scalar_type(rhs, target_type, &scalar_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_leaf_expr_spill_depth(rhs), 1 + fast_backend_address_expr_spill_depth(lhs)));
		} else if (fast_backend_can_emit_array_binary_expr(plan, rhs, target_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_array_binary_expr_spill_depth(rhs), fast_backend_address_expr_spill_depth(lhs)));
		} else if (fast_backend_can_emit_scalar_compound_lit_expr(plan, rhs, target_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_scalar_compound_lit_spill_depth(rhs, target_type), fast_backend_address_expr_spill_depth(lhs)));
		} else if (fast_backend_can_emit_slice_compound_lit_expr(plan, rhs, target_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_slice_compound_lit_spill_depth(rhs, target_type), fast_backend_address_expr_spill_depth(lhs)));
		} else if (fast_backend_can_emit_aggregate_call_expr(plan, rhs, target_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_call_expr_spill_depth(&unparen_expr(rhs)->CallExpr), fast_backend_address_expr_spill_depth(lhs)));
		} else if (unparen_expr(rhs)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &unparen_expr(rhs)->SliceExpr, target_type)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_slice_expr_spill_depth(&unparen_expr(rhs)->SliceExpr), fast_backend_address_expr_spill_depth(lhs)));
		} else if (fast_backend_can_emit_address_expr(plan, rhs, nullptr, nullptr, nullptr)) {
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_address_expr_spill_depth(rhs), 1 + fast_backend_address_expr_spill_depth(lhs)));
		} else {
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_address_expr_spill_depth(lhs));
		}
	}

	return true;
}

gb_internal bool fast_backend_plan_return_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstReturnStmt *rs) {
	gb_unused(gen);

	if (!plan->has_result) {
		if (rs->results.count != 0) {
			error(rs->token, "Fast backend expected no return values");
			return false;
		}
		return true;
	}

	if (rs->results.count != 1) {
		error(rs->token, "Fast backend currently only supports a single return value");
		return false;
	}

	Ast *result = rs->results[0];
	Type *result_type = reduce_tuple_to_single_type(plan->type->results);
	if (!plan->return_by_pointer) {
		if (!fast_backend_can_emit_leaf_expr(plan, result, result_type)) {
			error(result, "Fast backend does not yet support this return expression");
			return false;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(result));
		return true;
	}

	if (!fast_backend_can_emit_aggregate_expr(plan, result, result_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, result, result_type) &&
	    !fast_backend_can_emit_scalar_compound_lit_expr(plan, result, result_type)) {
		error(result, "Fast backend does not yet support this return expression");
		return false;
	}

	if (fast_backend_can_emit_aggregate_call_expr(plan, result, result_type)) {
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_call_expr_spill_depth(&unparen_expr(result)->CallExpr));
	} else if (fast_backend_can_emit_array_binary_expr(plan, result, result_type)) {
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_array_binary_expr_spill_depth(result));
	} else if (fast_backend_can_emit_scalar_compound_lit_expr(plan, result, result_type)) {
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_scalar_compound_lit_spill_depth(result, result_type));
	} else if (fast_backend_can_emit_slice_compound_lit_expr(plan, result, result_type)) {
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_compound_lit_spill_depth(result, result_type));
	} else if (unparen_expr(result)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(plan, &unparen_expr(result)->SliceExpr, result_type)) {
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_slice_expr_spill_depth(&unparen_expr(result)->SliceExpr));
	} else if (fast_backend_can_emit_address_expr(plan, result, nullptr, nullptr, nullptr)) {
		plan->spill_depth = gb_max(plan->spill_depth, gb_max(1, fast_backend_address_expr_spill_depth(result)));
	}
	return true;
}

gb_internal bool fast_backend_plan_if_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstIfStmt *is) {
	if (is->init != nullptr && !fast_backend_plan_stmt(gen, plan, is->init)) {
		return false;
	}
	if (!fast_backend_can_emit_leaf_expr(plan, is->cond, type_and_value_of_expr(is->cond).type)) {
		error(is->cond, "Fast backend does not yet support this if condition");
		return false;
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(is->cond));
	if (!fast_backend_plan_stmt(gen, plan, is->body)) {
		return false;
	}
	if (is->else_stmt != nullptr && !fast_backend_plan_stmt(gen, plan, is->else_stmt)) {
		return false;
	}
	return true;
}

gb_internal bool fast_backend_plan_for_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstForStmt *fs) {
	if (fs->init != nullptr && !fast_backend_plan_stmt(gen, plan, fs->init)) {
		return false;
	}
	if (fs->cond != nullptr) {
		if (!fast_backend_can_emit_leaf_expr(plan, fs->cond, type_and_value_of_expr(fs->cond).type)) {
			error(fs->cond, "Fast backend does not yet support this for-loop condition");
			return false;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(fs->cond));
	}
	if (!fast_backend_plan_stmt(gen, plan, fs->body)) {
		return false;
	}
	if (fs->post != nullptr && !fast_backend_plan_stmt(gen, plan, fs->post)) {
		return false;
	}
	return true;
}

gb_internal bool fast_backend_plan_switch_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstSwitchStmt *ss) {
	if (ss->label != nullptr && fast_backend_label_name(ss->label).len == 0) {
		error(ss->label, "Fast backend expected an identifier switch label");
		return false;
	}
	if (ss->init != nullptr && !fast_backend_plan_stmt(gen, plan, ss->init)) {
		return false;
	}
	if (ss->body == nullptr || ss->body->kind != Ast_BlockStmt) {
		error(ss->token, "Fast backend expected a switch body block");
		return false;
	}

	Type *tag_type = t_bool;
	if (ss->tag != nullptr) {
		tag_type = reduce_tuple_to_single_type(type_and_value_of_expr(ss->tag).type);
		FastScalarType scalar_type = {};
		if (!fast_backend_expr_scalar_type(nullptr, tag_type, &scalar_type)) {
			error(ss->tag, "Fast backend currently only supports scalar switch tags");
			return false;
		}
		if (fast_backend_expr_has_call(ss->tag)) {
			error(ss->tag, "Fast backend switch tags cannot contain calls yet");
			return false;
		}
		if (!fast_backend_can_emit_leaf_expr(plan, ss->tag, tag_type)) {
			error(ss->tag, "Fast backend does not yet support this switch tag expression");
			return false;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(ss->tag));
	}

	for (Ast *stmt : ss->body->BlockStmt.stmts) {
		if (stmt == nullptr || stmt->kind != Ast_CaseClause) {
			error(stmt ? stmt : cast(Ast *)ss, "Fast backend expected switch case clauses");
			return false;
		}

		AstCaseClause *cc = &stmt->CaseClause;
		for (Ast *expr : cc->list) {
			if (!fast_backend_can_emit_switch_case_expr(plan, expr, tag_type)) {
				error(expr, "Fast backend does not yet support this switch case expression");
				return false;
			}

			expr = unparen_expr(expr);
			if (is_ast_range(expr)) {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.left));
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.right));
			} else {
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(expr));
			}
		}

		if (!fast_backend_plan_stmt_list(gen, plan, cc->stmts)) {
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_plan_branch_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstBranchStmt *bs) {
	gb_unused(gen);
	gb_unused(plan);

	switch (bs->token.kind) {
	case Token_break:
	case Token_continue:
		if (bs->label != nullptr && fast_backend_label_name(bs->label).len == 0) {
			error(bs->label, "Fast backend expected an identifier label");
			return false;
		}
		return true;
	case Token_fallthrough:
		if (bs->label != nullptr) {
			error(bs->label, "Fast backend does not support labeled fallthrough statements");
			return false;
		}
		return true;
	}

	error(bs->token, "Fast backend currently only supports 'break', 'continue', and 'fallthrough' branch statements");
	return false;
}

gb_internal bool fast_backend_plan_defer_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstDeferStmt *ds) {
	plan->has_defer = true;
	return fast_backend_plan_stmt(gen, plan, ds->stmt);
}

gb_internal bool fast_backend_plan_expr_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstExprStmt *es) {
	gb_unused(gen);

	if (es->expr == nullptr || es->expr->kind != Ast_CallExpr) {
		error(es->expr ? es->expr : cast(Ast *)es, "Fast backend currently only supports call expression statements");
		return false;
	}
	if (!fast_backend_can_emit_call_expr(plan, &es->expr->CallExpr, true)) {
		error(es->expr, "Fast backend does not yet support this call expression");
		return false;
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(es->expr));
	return true;
}

gb_internal bool fast_backend_plan_stmt(FastGenerator *gen, FastLeafProcPlan *plan, Ast *stmt) {
	if (stmt == nullptr) {
		return true;
	}

	switch (stmt->kind) {
	case Ast_EmptyStmt:
		return true;
	case Ast_BlockStmt:
		return fast_backend_plan_stmt_list(gen, plan, stmt->BlockStmt.stmts);
	case Ast_ValueDecl:
		return fast_backend_plan_value_decl(gen, plan, &stmt->ValueDecl);
	case Ast_AssignStmt:
		return fast_backend_plan_assign_stmt(gen, plan, &stmt->AssignStmt);
	case Ast_ExprStmt:
		return fast_backend_plan_expr_stmt(gen, plan, &stmt->ExprStmt);
	case Ast_IfStmt:
		return fast_backend_plan_if_stmt(gen, plan, &stmt->IfStmt);
	case Ast_ForStmt:
		return fast_backend_plan_for_stmt(gen, plan, &stmt->ForStmt);
	case Ast_SwitchStmt:
		return fast_backend_plan_switch_stmt(gen, plan, &stmt->SwitchStmt);
	case Ast_DeferStmt:
		return fast_backend_plan_defer_stmt(gen, plan, &stmt->DeferStmt);
	case Ast_BranchStmt:
		return fast_backend_plan_branch_stmt(gen, plan, &stmt->BranchStmt);
	case Ast_ReturnStmt:
		return fast_backend_plan_return_stmt(gen, plan, &stmt->ReturnStmt);
	}

	error(stmt, "Fast backend currently only supports blocks, mutable local declarations, assignments, defer statements, call statements, if statements, for statements, switch statements, break/continue, and returns");
	return false;
}

gb_internal bool fast_backend_plan_leaf_proc(FastGenerator *gen, Entity *e, FastLeafProcPlan *plan) {
	DeclInfo *decl = e->decl_info;
	if (decl == nullptr || decl->proc_lit == nullptr || decl->proc_lit->kind != Ast_ProcLit) {
		error(e->token, "Fast backend expected a concrete procedure body");
		return false;
	}

	Type *type = base_type(e->type);
	if (type == nullptr || type->kind != Type_Proc) {
		error(e->token, "Fast backend expected a procedure type");
		return false;
	}

	TypeProc *pt = &type->Proc;
	if (pt->variadic || pt->diverging || pt->is_polymorphic || e->Procedure.generated_from_polymorphic) {
		error(e->token, "Fast backend does not yet support this procedure kind");
		return false;
	}
	if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
		error(e->token, "Fast backend does not yet support this calling convention");
		return false;
	}

	plan->entity = e;
	plan->gen = gen;
	plan->info = gen->info;
	plan->type = pt;
	plan->body = decl->proc_lit->ProcLit.body;
	plan->has_result = false;
	plan->return_by_pointer = false;
	plan->result_value_type = nullptr;
	plan->return_type = {};
	plan->result_ptr_slot = {};
	plan->context_slot = {};
	plan->spill_depth = 0;
	plan->local_stack_size = 0;
	plan->proc_index = 0;
	plan->has_calls = false;
	plan->has_defer = false;
	plan->has_context_slot = false;
	plan->params = array_make<Entity *>(heap_allocator(), 0, pt->param_count);
	plan->slots = array_make<FastLocalSlot>(heap_allocator(), 0, pt->param_count);
	plan->expr_slots = array_make<FastExprSlot>(heap_allocator(), 0, 0);

	if (pt->param_count != 0) {
		GB_ASSERT(pt->params != nullptr && pt->params->kind == Type_Tuple);
		for_array(i, pt->params->Tuple.variables) {
			Entity *param = pt->params->Tuple.variables[i];
			if (param == nullptr || param->kind != Entity_Variable) {
				continue;
			}
			if (param->flags.load(std::memory_order_relaxed) & EntityFlag_CVarArg) {
				error(param->token, "Fast backend does not yet support variadic procedures");
				return false;
			}

			if (!fast_backend_type_is_supported_value(param->type, nullptr, nullptr)) {
				error(param->token, "Fast backend currently only supports scalar, pointer-like, array, struct, string, slice, dynamic array, and fixed-capacity dynamic array parameter types");
				return false;
			}
			array_add(&plan->params, param);
			if (!fast_backend_add_slot(plan, param, param->type)) {
				return false;
			}
		}
	}

	if (pt->calling_convention == ProcCC_Odin) {
		plan->has_context_slot = true;
		if (!fast_backend_add_slot(plan, nullptr, t_rawptr)) {
			return false;
		}
		plan->context_slot = plan->slots[plan->slots.count-1];
	}

	if (pt->result_count > 1) {
		error(e->token, "Fast backend currently only supports procedures with at most one result");
		return false;
	}
	if (pt->result_count == 1) {
		GB_ASSERT(pt->results != nullptr && pt->results->kind == Type_Tuple);
		Entity *result_entity = pt->results->Tuple.variables[0];
		GB_ASSERT(result_entity != nullptr);
		if (!fast_backend_classify_scalar_type(result_entity->type, &plan->return_type)) {
			if (!fast_backend_type_is_supported_aggregate(result_entity->type)) {
				error(result_entity->token, "Fast backend currently only supports scalar, pointer-like, array, struct, string, slice, dynamic array, and fixed-capacity dynamic array result types");
				return false;
			}
			if (!fast_backend_add_slot(plan, nullptr, t_rawptr)) {
				return false;
			}
			plan->return_by_pointer = true;
			plan->result_value_type = result_entity->type;
			plan->result_ptr_slot = plan->slots[plan->slots.count-1];
		}
		plan->has_result = true;
	}

	ast_node(bs, BlockStmt, decl->proc_lit->ProcLit.body);
	if (!fast_backend_plan_stmt_list(gen, plan, bs->stmts)) {
		return false;
	}
	plan->has_calls = fast_backend_stmt_has_call(plan->body);
	if (plan->has_defer && plan->has_result && !plan->return_by_pointer) {
		plan->spill_depth = gb_max(plan->spill_depth, 1);
	}
	return true;
}

gb_internal bool fast_backend_plan_global_var(FastGenerator *gen, Entity *e, FastGlobalVarPlan *plan) {
	GB_ASSERT(e != nullptr);
	GB_ASSERT(e->kind == Entity_Variable);

	if (e->Variable.is_foreign) {
		return false;
	}
	if (e->Variable.thread_local_model.len != 0) {
		error(e->token, "Fast backend does not yet support thread local global variables");
		return false;
	}

	if (!fast_backend_type_is_supported_value(e->type, nullptr, nullptr)) {
		error(e->token, "Fast backend currently only supports scalar, pointer-like, array, struct, string, slice, dynamic array, and fixed-capacity dynamic array global variable types");
		return false;
	}

	DeclInfo *decl = decl_info_of_entity(e);
	Ast *init_expr = decl != nullptr ? decl->init_expr : e->Variable.init_expr;

	plan->entity = e;
	plan->gen = gen;
	plan->value_type = e->type;
	plan->direct_init_expr = nullptr;
	plan->init_data = nullptr;
	plan->size = cast(i32)type_size_of(e->type);
	plan->align = cast(i32)type_align_of(e->type);
	plan->has_init_data = false;

	if (plan->size <= 0 || plan->align <= 0) {
		error(e->token, "Fast backend expected a sized global variable");
		return false;
	}
	if (init_expr == nullptr) {
		return true;
	}

	TypeAndValue tv = type_and_value_of_expr(init_expr);
	if (tv.mode == Addressing_Invalid) {
		return false;
	}
	if (fast_backend_expr_is_string_literal_value(e->type, init_expr)) {
		plan->direct_init_expr = init_expr;
		return true;
	}
	if (tv.mode == Addressing_Constant || is_type_untyped_nil(tv.type)) {
		plan->init_data = gb_alloc_array(heap_allocator(), u8, plan->size);
		gb_zero_size(plan->init_data, plan->size);
		if (!fast_backend_serialize_constant_value(e->type, init_expr, plan->init_data)) {
			error(init_expr, "Fast backend does not yet support this global constant initializer");
			return false;
		}
		plan->has_init_data = true;
		return true;
	}

	error(init_expr, "Fast backend does not yet support dynamic global initialization");
	return false;
}

gb_internal bool fast_backend_collect_program(FastGenerator *gen, Array<FastGlobalVarPlan> *globals, Array<FastLeafProcPlan> *procedures) {
	bool allow_llvm_fallback = fast_backend_can_fallback_to_llvm_per_entity();
	for (Entity *e : gen->info->definitions) {
		if (!fast_entity_is_local(gen->info, e)) {
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
				if (allow_llvm_fallback) {
					continue;
				}
				error(e->token, "Fast backend only supports plain top-level procedures right now");
				return false;
			}

			FastLeafProcPlan plan = {};
			if (allow_llvm_fallback) {
				FastBackendErrorState error_state = fast_backend_save_error_state();
				if (!fast_backend_plan_leaf_proc(gen, e, &plan)) {
					fast_backend_restore_error_state(error_state);
					continue;
				}
			} else if (!fast_backend_plan_leaf_proc(gen, e, &plan)) {
				return false;
			}
			plan.proc_index = cast(i32)procedures->count;
			array_add(procedures, plan);
			array_add(&gen->emitted_entities, e);
			break;
		}

		case Entity_Variable: {
			if (e->Variable.is_foreign) {
				break;
			}
			if (flags & (EntityFlag_Overridden|
			             EntityFlag_CustomLinkage_Weak|
			             EntityFlag_CustomLinkage_LinkOnce)) {
				if (allow_llvm_fallback) {
					break;
				}
				error(e->token, "Fast backend does not yet support this global variable linkage");
				return false;
			}
			FastGlobalVarPlan plan = {};
			if (allow_llvm_fallback) {
				FastBackendErrorState error_state = fast_backend_save_error_state();
				if (!fast_backend_plan_global_var(gen, e, &plan)) {
					fast_backend_restore_error_state(error_state);
					break;
				}
			} else if (!fast_backend_plan_global_var(gen, e, &plan)) {
				return false;
			}
			array_add(globals, plan);
			array_add(&gen->emitted_entities, e);
			break;
		}

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

struct FastX64RegNames {
	char const *r64;
	char const *r32;
	char const *r16;
	char const *r8;
};

gb_global FastX64RegNames fast_x64_reg_rax = {"rax", "eax", "ax", "al"};
gb_global FastX64RegNames fast_x64_reg_rcx = {"rcx", "ecx", "cx", "cl"};
gb_global FastX64RegNames fast_x64_reg_rdx = {"rdx", "edx", "dx", "dl"};
gb_global FastX64RegNames fast_x64_reg_rdi = {"rdi", "edi", "di", "dil"};
gb_global FastX64RegNames fast_x64_reg_rsi = {"rsi", "esi", "si", "sil"};
gb_global FastX64RegNames fast_x64_reg_r8  = {"r8",  "r8d",  "r8w",  "r8b"};
gb_global FastX64RegNames fast_x64_reg_r9  = {"r9",  "r9d",  "r9w",  "r9b"};
gb_global FastX64RegNames fast_x64_reg_r10 = {"r10", "r10d", "r10w", "r10b"};
gb_global FastX64RegNames fast_x64_reg_r11 = {"r11", "r11d", "r11w", "r11b"};

gb_internal FastX64RegNames const *fast_backend_x64_return_reg(void) {
	return &fast_x64_reg_rax;
}

gb_internal FastX64RegNames const *fast_backend_x64_work_reg(void) {
	return &fast_x64_reg_r10;
}

gb_internal FastX64RegNames const *fast_backend_x64_tmp_reg(void) {
	return &fast_x64_reg_r11;
}

gb_internal FastX64RegNames const *fast_backend_x64_scratch_reg(void) {
	return &fast_x64_reg_rax;
}

gb_internal ProcCallingConvention fast_backend_effective_calling_convention(TypeProc *pt) {
	ProcCallingConvention cc = pt->calling_convention;
	if (cc == ProcCC_CDecl) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			if (build_context.metrics.os == TargetOs_windows || build_context.metrics.abi == TargetABI_Win64) {
				return ProcCC_Win64;
			}
			return ProcCC_SysV;
		}
	}
	return cc;
}

gb_internal FastX64RegNames const *fast_backend_x64_param_reg(TypeProc *pt, i32 index) {
	static FastX64RegNames const *sysv[]  = {&fast_x64_reg_rdi, &fast_x64_reg_rsi, &fast_x64_reg_rdx, &fast_x64_reg_rcx, &fast_x64_reg_r8, &fast_x64_reg_r9};
	static FastX64RegNames const *win64[] = {&fast_x64_reg_rcx, &fast_x64_reg_rdx, &fast_x64_reg_r8, &fast_x64_reg_r9};

	ProcCallingConvention cc = fast_backend_effective_calling_convention(pt);
	if (cc == ProcCC_Win64) {
		return index < cast(i32)gb_count_of(win64) ? win64[index] : nullptr;
	}
	return index < cast(i32)gb_count_of(sysv) ? sysv[index] : nullptr;
}

gb_internal char const *fast_backend_arm64_return_reg(void) {
	return "x0";
}

gb_internal char const *fast_backend_arm64_work_reg(void) {
	return "x9";
}

gb_internal char const *fast_backend_arm64_tmp_reg(void) {
	return "x10";
}

gb_internal char const *fast_backend_arm64_div_tmp_reg(void) {
	return "x11";
}

gb_internal char const *fast_backend_arm64_addr_tmp_reg(void) {
	return "x12";
}

gb_internal char const *fast_backend_arm64_param_reg(i32 index) {
	static char const *regs[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"};
	return index < cast(i32)gb_count_of(regs) ? regs[index] : nullptr;
}

gb_internal FastScalarType fast_backend_context_scalar_type(void) {
	FastScalarType type = {};
	type.kind = FastScalar_Pointer;
	type.bit_size = 8 * build_context.metrics.ptr_size;
	return type;
}

gb_internal i32 fast_backend_param_limit_from_proc_type(TypeProc *pt) {
	if (build_context.metrics.arch == TargetArch_arm64) {
		return 8;
	}
	ProcCallingConvention cc = fast_backend_effective_calling_convention(pt);
	return cc == ProcCC_Win64 ? 4 : 6;
}

gb_internal i32 fast_backend_param_limit(FastLeafProcPlan *plan) {
	return fast_backend_param_limit_from_proc_type(plan->type);
}

gb_internal bool fast_backend_find_slot(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *out) {
	for (auto const &slot : plan->slots) {
		if (slot.entity == entity) {
			if (out) *out = slot;
			return true;
		}
	}
	return false;
}

gb_internal bool fast_backend_find_expr_slot(FastLeafProcPlan *plan, Ast *expr, FastLocalSlot *out) {
	if (plan == nullptr || expr == nullptr) {
		return false;
	}
	expr = unparen_expr(expr);
	for (auto const &entry : plan->expr_slots) {
		if (entry.expr == expr) {
			if (out) *out = entry.slot;
			return true;
		}
	}
	return false;
}

gb_internal i32 fast_backend_slot_offset(FastLeafProcPlan *plan, FastLocalSlot const &slot) {
	return 8 * plan->spill_depth + slot.offset;
}

gb_internal bool fast_backend_add_slot(FastLeafProcPlan *plan, Entity *entity, Type *type) {
	if (entity != nullptr && fast_backend_find_slot(plan, entity, nullptr)) {
		return true;
	}

	FastScalarType scalar_type = {};
	bool is_scalar = false;
	if (!fast_backend_type_is_supported_value(type, &scalar_type, &is_scalar)) {
		return false;
	}

	FastLocalSlot slot = {};
	slot.entity = entity;
	slot.value_type = type;
	slot.type = scalar_type;
	slot.index = cast(i32)plan->slots.count;
	slot.is_scalar = is_scalar;
	slot.size = cast(i32)type_size_of(type);
	slot.align = cast(i32)type_align_of(type);
	if (slot.size <= 0 || slot.align <= 0) {
		return false;
	}
	plan->local_stack_size = align_formula(plan->local_stack_size, slot.align);
	slot.offset = plan->local_stack_size + slot.size;
	plan->local_stack_size = slot.offset;
	array_add(&plan->slots, slot);
	return true;
}

gb_internal bool fast_backend_add_expr_slot(FastLeafProcPlan *plan, Ast *expr, Type *type, FastLocalSlot *out) {
	if (plan == nullptr || expr == nullptr || type == nullptr) {
		return false;
	}
	expr = unparen_expr(expr);

	FastLocalSlot slot = {};
	if (fast_backend_find_expr_slot(plan, expr, &slot)) {
		if (out) *out = slot;
		return true;
	}

	isize slot_count = plan->slots.count;
	if (!fast_backend_add_slot(plan, nullptr, type)) {
		return false;
	}
	GB_ASSERT(plan->slots.count == slot_count + 1);
	slot = plan->slots[slot_count];

	FastExprSlot entry = {};
	entry.expr = expr;
	entry.slot = slot;
	array_add(&plan->expr_slots, entry);
	if (out) *out = slot;
	return true;
}

gb_internal bool fast_backend_entity_is_supported_global(FastLeafProcPlan *plan, Entity *entity, Type **type_, FastScalarType *scalar_type_, bool *is_scalar_) {
	if (entity == nullptr || entity->kind != Entity_Variable) {
		return false;
	}
	if (entity->parent_proc_decl.load(std::memory_order_relaxed) != nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	bool is_scalar = false;
	if (!fast_backend_type_is_supported_value(entity->type, &scalar_type, &is_scalar)) {
		return false;
	}

	if (type_) *type_ = entity->type;
	if (scalar_type_) *scalar_type_ = scalar_type;
	if (is_scalar_) *is_scalar_ = is_scalar;
	return entity->Variable.is_global || entity->Variable.is_foreign || fast_entity_is_local(plan->info, entity);
}

gb_internal bool fast_backend_find_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, Type **type_, bool *is_global_) {
	FastLocalSlot slot = {};
	if (fast_backend_find_slot(plan, entity, &slot)) {
		if (slot_) *slot_ = slot;
		if (type_) *type_ = slot.value_type;
		if (is_global_) *is_global_ = false;
		return true;
	}

	Type *type = nullptr;
	if (!fast_backend_entity_is_supported_global(plan, entity, &type, nullptr, nullptr)) {
		return false;
	}

	if (slot_) *slot_ = {};
	if (type_) *type_ = type;
	if (is_global_) *is_global_ = true;
	return true;
}

gb_internal bool fast_backend_find_scalar_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, FastScalarType *type_, bool *is_global_) {
	FastLocalSlot slot = {};
	Type *type = nullptr;
	bool is_global = false;
	if (!fast_backend_find_storage(plan, entity, &slot, &type, &is_global)) {
		return false;
	}
	if (!is_global && !slot.is_scalar) {
		return false;
	}
	FastScalarType scalar_type = {};
	bool is_scalar = false;
	if (!fast_backend_type_is_supported_value(type, &scalar_type, &is_scalar) || !is_scalar) {
		return false;
	}

	if (slot_) *slot_ = slot;
	if (type_) *type_ = scalar_type;
	if (is_global_) *is_global_ = is_global;
	return true;
}

gb_internal bool fast_backend_find_param_index(FastLeafProcPlan *plan, Entity *entity, i32 *index_) {
	for_array(i, plan->params) {
		if (plan->params[i] == entity) {
			*index_ = cast(i32)i;
			return true;
		}
	}
	return false;
}

gb_internal bool fast_backend_expr_is_zero_aggregate_value(Type *type, Ast *expr) {
	type = default_type(type);
	if (type == nullptr || expr == nullptr || !fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}

	TypeAndValue tv = type_and_value_of_expr(expr);
	if (is_type_untyped_nil(tv.type)) {
		return true;
	}
	return false;
}

gb_internal bool fast_backend_exact_value_as_u64(ExactValue value, FastScalarType type, u64 *out) {
	switch (value.kind) {
	case ExactValue_Bool:
		*out = value.value_bool ? 1 : 0;
		return true;
	case ExactValue_Pointer:
		*out = cast(u64)value.value_pointer;
		return true;
	case ExactValue_Integer:
		if (fast_backend_scalar_is_unsigned(type)) {
			*out = exact_value_to_u64(value);
		} else {
			*out = cast(u64)exact_value_to_i64(value);
		}
		return true;
	}
	return false;
}

gb_internal void fast_backend_store_scalar_bytes(u8 *dst, i32 size, u64 value) {
	for (i32 i = 0; i < size; i++) {
		dst[i] = cast(u8)((value >> (8*i)) & 0xff);
	}
}

gb_internal bool fast_backend_serialize_constant_aggregate(Type *type, Ast *expr, u8 *dst);

gb_internal bool fast_backend_serialize_constant_value(Type *type, Ast *expr, u8 *dst) {
	type = default_type(type);
	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode == Addressing_Invalid) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(type, expr)) {
		gb_zero_size(dst, cast(isize)type_size_of(type));
		return true;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		u64 value = 0;
		if (is_type_untyped_nil(tv.type)) {
			if (scalar_type.kind != FastScalar_Pointer) {
				return false;
			}
		} else {
			if (tv.mode != Addressing_Constant || !fast_backend_exact_value_as_u64(tv.value, scalar_type, &value)) {
				return false;
			}
		}
		fast_backend_store_scalar_bytes(dst, fast_backend_scalar_byte_size(scalar_type), value);
		return true;
	}

	return fast_backend_serialize_constant_aggregate(type, expr, dst);
}

gb_internal bool fast_backend_serialize_constant_struct(Type *type, AstCompoundLit *cl, u8 *dst) {
	type_set_offsets(type);
	gb_zero_size(dst, cast(isize)type_size_of(type));
	if (cl->elems.count == 0) {
		return true;
	}

	if (cl->elems[0]->kind == Ast_FieldValue) {
		for (Ast *elem : cl->elems) {
			if (elem == nullptr || elem->kind != Ast_FieldValue) {
				return false;
			}
			ast_node(fv, FieldValue, elem);
			Entity *field = entity_of_node(fv->field);
			if (field == nullptr || field->kind != Entity_Variable) {
				return false;
			}
			i32 field_index = field->Variable.field_index;
			Type *field_type = nullptr;
			i64 offset = type_offset_of(type, field_index, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_serialize_constant_value(field_type, fv->value, dst + offset)) {
				return false;
			}
		}
		return true;
	}

	if (cl->elems.count > type->Struct.fields.count) {
		return false;
	}

	for_array(i, cl->elems) {
		Type *field_type = nullptr;
		i64 offset = type_offset_of(type, i, &field_type);
		if (offset < 0 || field_type == nullptr) {
			return false;
		}
		if (!fast_backend_serialize_constant_value(field_type, cl->elems[i], dst + offset)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_serialize_constant_array(Type *type, AstCompoundLit *cl, u8 *dst) {
	Type *elem_type = type->Array.elem;
	i64 elem_size = type_size_of(elem_type);
	if (elem_size < 0) {
		return false;
	}

	gb_zero_size(dst, cast(isize)type_size_of(type));
	if (cl->elems.count == 0) {
		return true;
	}

	if (cl->elems[0]->kind == Ast_FieldValue) {
		for (Ast *elem : cl->elems) {
			if (elem == nullptr || elem->kind != Ast_FieldValue) {
				return false;
			}
			ast_node(fv, FieldValue, elem);
			if (is_ast_range(fv->field)) {
				ast_node(range, BinaryExpr, fv->field);
				TypeAndValue lo_tav = type_and_value_of_expr(range->left);
				TypeAndValue hi_tav = type_and_value_of_expr(range->right);
				if (lo_tav.mode != Addressing_Constant || hi_tav.mode != Addressing_Constant) {
					return false;
				}
				i64 lo = exact_value_to_i64(lo_tav.value);
				i64 hi = exact_value_to_i64(hi_tav.value);
				if (range->op.kind != Token_RangeHalf) {
					hi += 1;
				}
				if (lo < 0 || hi < lo || hi > type->Array.count) {
					return false;
				}
				for (i64 index = lo; index < hi; index++) {
					if (!fast_backend_serialize_constant_value(elem_type, fv->value, dst + index*elem_size)) {
						return false;
					}
				}
			} else {
				TypeAndValue index_tav = type_and_value_of_expr(fv->field);
				if (index_tav.mode != Addressing_Constant) {
					return false;
				}
				i64 index = exact_value_to_i64(index_tav.value);
				if (index < 0 || index >= type->Array.count) {
					return false;
				}
				if (!fast_backend_serialize_constant_value(elem_type, fv->value, dst + index*elem_size)) {
					return false;
				}
			}
		}
		return true;
	}

	if (cl->elems.count > type->Array.count) {
		return false;
	}

	for_array(i, cl->elems) {
		if (!fast_backend_serialize_constant_value(elem_type, cl->elems[i], dst + i*elem_size)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_serialize_constant_aggregate(Type *type, Ast *expr, u8 *dst) {
	type = default_type(type);
	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}

	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode != Addressing_Constant || tv.value.kind != ExactValue_Compound) {
		return false;
	}

	Ast *compound = tv.value.value_compound;
	if (compound == nullptr || compound->kind != Ast_CompoundLit) {
		return false;
	}
	ast_node(cl, CompoundLit, compound);

	switch (base_type(type)->kind) {
	case Type_Array:
		return fast_backend_serialize_constant_array(type, cl, dst);
	case Type_Struct:
		return fast_backend_serialize_constant_struct(type, cl, dst);
	}

	return false;
}

gb_internal bool fast_backend_entity_uses_external_symbol(FastLeafProcPlan *plan, Entity *entity) {
	if (entity == nullptr || entity->kind != Entity_Variable) {
		return true;
	}
	if (entity->Variable.is_foreign) {
		return true;
	}
	return !fast_entity_is_local(plan->info, entity);
}

gb_internal void fast_backend_emit_x64_canonicalize(gbFile *file, FastX64RegNames const *reg, FastScalarType type);
gb_internal void fast_backend_emit_arm64_canonicalize(gbFile *file, char const *reg, FastScalarType type);

gb_internal void fast_backend_emit_x64_load_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, FastX64RegNames const *dst) {
	gb_fprintf(file, "\tmov %s, QWORD PTR [rbp-%d]\n", dst->r64, fast_backend_slot_offset(plan, slot));
	fast_backend_emit_x64_canonicalize(file, dst, slot.type);
}

gb_internal void fast_backend_emit_arm64_load_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, char const *dst) {
	gb_fprintf(file, "\tldr %s, [x29, #-%d]\n", dst, fast_backend_slot_offset(plan, slot));
	fast_backend_emit_arm64_canonicalize(file, dst, slot.type);
}

gb_internal void fast_backend_emit_x64_store_reg_to_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, FastX64RegNames const *src) {
	gb_fprintf(file, "\tmov QWORD PTR [rbp-%d], %s\n", fast_backend_slot_offset(plan, slot), src->r64);
}

gb_internal void fast_backend_emit_arm64_store_reg_to_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, char const *src) {
	gb_fprintf(file, "\tstr %s, [x29, #-%d]\n", src, fast_backend_slot_offset(plan, slot));
}

gb_internal void fast_backend_emit_x64_load_imm(gbFile *file, FastX64RegNames const *dst, u64 value) {
	gb_fprintf(file, "\tmov %s, 0x%llx\n", dst->r64, cast(unsigned long long)value);
}

gb_internal void fast_backend_emit_arm64_load_imm(gbFile *file, char const *dst, u64 value) {
	if (value == 0) {
		gb_fprintf(file, "\tmov %s, xzr\n", dst);
		return;
	}

	u16 parts[4] = {};
	for (i32 i = 0; i < 4; i++) {
		parts[i] = cast(u16)((value >> (16*i)) & 0xffff);
	}

	bool emitted = false;
	for (i32 i = 0; i < 4; i++) {
		if (parts[i] == 0 && emitted) {
			continue;
		}
		if (!emitted) {
			gb_fprintf(file, "\tmovz %s, #%u, lsl #%d\n", dst, cast(unsigned)parts[i], 16*i);
			emitted = true;
		} else if (parts[i] != 0) {
			gb_fprintf(file, "\tmovk %s, #%u, lsl #%d\n", dst, cast(unsigned)parts[i], 16*i);
		}
	}
}

gb_internal void fast_backend_emit_x64_load_address_of_entity(gbFile *file, FastLeafProcPlan *plan, Entity *entity, FastX64RegNames const *dst) {
	String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(entity));
	if (fast_backend_entity_uses_external_symbol(plan, entity)) {
		gb_fprintf(file, "\tmov %s, QWORD PTR [rip + \"%.*s\"@GOTPCREL]\n", dst->r64, LIT(symbol));
	} else {
		gb_fprintf(file, "\tlea %s, [rip + \"%.*s\"]\n", dst->r64, LIT(symbol));
	}
}

gb_internal void fast_backend_emit_arm64_load_address_of_entity(gbFile *file, FastLeafProcPlan *plan, Entity *entity, char const *dst) {
	String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(entity));
	if (fast_backend_entity_uses_external_symbol(plan, entity)) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, "\tadrp %s, \"%.*s\"@GOTPAGE\n", dst, LIT(symbol));
			gb_fprintf(file, "\tldr %s, [%s, \"%.*s\"@GOTPAGEOFF]\n", dst, dst, LIT(symbol));
		} else {
			gb_fprintf(file, "\tadrp %s, :got:\"%.*s\"\n", dst, LIT(symbol));
			gb_fprintf(file, "\tldr %s, [%s, :got_lo12:\"%.*s\"]\n", dst, dst, LIT(symbol));
		}
	} else {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, "\tadrp %s, \"%.*s\"@PAGE\n", dst, LIT(symbol));
			gb_fprintf(file, "\tadd %s, %s, \"%.*s\"@PAGEOFF\n", dst, dst, LIT(symbol));
		} else {
			gb_fprintf(file, "\tadrp %s, \"%.*s\"\n", dst, LIT(symbol));
			gb_fprintf(file, "\tadd %s, %s, :lo12:\"%.*s\"\n", dst, dst, LIT(symbol));
		}
	}
}

gb_internal void fast_backend_emit_blob_label(gbFile *file, i32 blob_index) {
	gb_fprintf(file, "Lfast_blob_%d", blob_index);
}

gb_internal void fast_backend_emit_x64_load_address_of_blob(gbFile *file, i32 blob_index, FastX64RegNames const *dst) {
	gb_fprintf(file, "\tlea %s, [rip + ", dst->r64);
	fast_backend_emit_blob_label(file, blob_index);
	gb_fprintf(file, "]\n");
}

gb_internal void fast_backend_emit_arm64_load_address_of_blob(gbFile *file, i32 blob_index, char const *dst) {
	if (build_context.metrics.os == TargetOs_darwin) {
		gb_fprintf(file, "\tadrp %s, ", dst);
		fast_backend_emit_blob_label(file, blob_index);
		gb_fprintf(file, "@PAGE\n");
		gb_fprintf(file, "\tadd %s, %s, ", dst, dst);
		fast_backend_emit_blob_label(file, blob_index);
		gb_fprintf(file, "@PAGEOFF\n");
	} else {
		gb_fprintf(file, "\tadrp %s, ", dst);
		fast_backend_emit_blob_label(file, blob_index);
		gb_fprintf(file, "\n");
		gb_fprintf(file, "\tadd %s, %s, :lo12:", dst, dst);
		fast_backend_emit_blob_label(file, blob_index);
		gb_fprintf(file, "\n");
	}
}

gb_internal void fast_backend_emit_x64_load_from_address(gbFile *file, FastX64RegNames const *addr, FastX64RegNames const *dst, FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
		gb_fprintf(file, "\tmovzx %s, BYTE PTR [%s]\n", dst->r64, addr->r64);
		return;
	case FastScalar_Signed:
		switch (fast_backend_scalar_byte_size(type)) {
		case 1: gb_fprintf(file, "\tmovsx %s, BYTE PTR [%s]\n", dst->r64, addr->r64); return;
		case 2: gb_fprintf(file, "\tmovsx %s, WORD PTR [%s]\n", dst->r64, addr->r64); return;
		case 4: gb_fprintf(file, "\tmovsxd %s, DWORD PTR [%s]\n", dst->r64, addr->r64); return;
		case 8: gb_fprintf(file, "\tmov %s, QWORD PTR [%s]\n", dst->r64, addr->r64); return;
		}
		break;
	case FastScalar_Unsigned:
	case FastScalar_Pointer:
		switch (fast_backend_scalar_byte_size(type)) {
		case 1: gb_fprintf(file, "\tmovzx %s, BYTE PTR [%s]\n", dst->r64, addr->r64); return;
		case 2: gb_fprintf(file, "\tmovzx %s, WORD PTR [%s]\n", dst->r64, addr->r64); return;
		case 4: gb_fprintf(file, "\tmov %s, DWORD PTR [%s]\n", dst->r32, addr->r64); return;
		case 8: gb_fprintf(file, "\tmov %s, QWORD PTR [%s]\n", dst->r64, addr->r64); return;
		}
		break;
	}
}

gb_internal void fast_backend_emit_arm64_load_from_address(gbFile *file, char const *addr, char const *dst, FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
		gb_fprintf(file, "\tldrb w%s, [%s]\n", dst+1, addr);
		return;
	case FastScalar_Signed:
		switch (fast_backend_scalar_byte_size(type)) {
		case 1: gb_fprintf(file, "\tldrsb %s, [%s]\n", dst, addr); return;
		case 2: gb_fprintf(file, "\tldrsh %s, [%s]\n", dst, addr); return;
		case 4: gb_fprintf(file, "\tldrsw %s, [%s]\n", dst, addr); return;
		case 8: gb_fprintf(file, "\tldr %s, [%s]\n", dst, addr); return;
		}
		break;
	case FastScalar_Unsigned:
		switch (fast_backend_scalar_byte_size(type)) {
		case 1: gb_fprintf(file, "\tldrb w%s, [%s]\n", dst+1, addr); return;
		case 2: gb_fprintf(file, "\tldrh w%s, [%s]\n", dst+1, addr); return;
		case 4: gb_fprintf(file, "\tldr w%s, [%s]\n", dst+1, addr); return;
		case 8: gb_fprintf(file, "\tldr %s, [%s]\n", dst, addr); return;
		}
		break;
	case FastScalar_Pointer:
		gb_fprintf(file, "\tldr %s, [%s]\n", dst, addr);
		return;
	}
}

gb_internal void fast_backend_emit_x64_store_to_address(gbFile *file, FastX64RegNames const *addr, FastX64RegNames const *src, FastScalarType type) {
	switch (fast_backend_scalar_byte_size(type)) {
	case 1: gb_fprintf(file, "\tmov BYTE PTR [%s], %s\n", addr->r64, src->r8); return;
	case 2: gb_fprintf(file, "\tmov WORD PTR [%s], %s\n", addr->r64, src->r16); return;
	case 4: gb_fprintf(file, "\tmov DWORD PTR [%s], %s\n", addr->r64, src->r32); return;
	case 8: gb_fprintf(file, "\tmov QWORD PTR [%s], %s\n", addr->r64, src->r64); return;
	}
}

gb_internal void fast_backend_emit_arm64_store_to_address(gbFile *file, char const *addr, char const *src, FastScalarType type) {
	switch (fast_backend_scalar_byte_size(type)) {
	case 1: gb_fprintf(file, "\tstrb w%s, [%s]\n", src+1, addr); return;
	case 2: gb_fprintf(file, "\tstrh w%s, [%s]\n", src+1, addr); return;
	case 4: gb_fprintf(file, "\tstr w%s, [%s]\n", src+1, addr); return;
	case 8: gb_fprintf(file, "\tstr %s, [%s]\n", src, addr); return;
	}
}

gb_internal void fast_backend_emit_arm64_add_offset(gbFile *file, char const *dst, char const *base, i32 offset) {
	if (offset == 0) {
		if (dst != base) {
			gb_fprintf(file, "\tmov %s, %s\n", dst, base);
		}
		return;
	}
	if (offset > 0 && offset <= 4095) {
		gb_fprintf(file, "\tadd %s, %s, #%d\n", dst, base, offset);
		return;
	}
	if (offset < 0 && -offset <= 4095) {
		gb_fprintf(file, "\tsub %s, %s, #%d\n", dst, base, -offset);
		return;
	}

	fast_backend_emit_arm64_load_imm(file, fast_backend_arm64_div_tmp_reg(), cast(u64)gb_abs(offset));
	gb_fprintf(file, "\t%s %s, %s, %s\n", offset > 0 ? "add" : "sub", dst, base, fast_backend_arm64_div_tmp_reg());
}

gb_internal void fast_backend_emit_add_imm_to_work_reg(FastLeafProcEmitter *emitter, i64 offset) {
	if (offset == 0) {
		return;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tadd %s, %lld\n", fast_backend_x64_work_reg()->r64, cast(long long)offset);
	} else {
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), cast(i32)offset);
	}
}

gb_internal void fast_backend_emit_add_imm_to_tmp_reg(FastLeafProcEmitter *emitter, i64 offset) {
	if (offset == 0) {
		return;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tadd %s, %lld\n", fast_backend_x64_tmp_reg()->r64, cast(long long)offset);
	} else {
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_tmp_reg(), cast(i32)offset);
	}
}

gb_internal void fast_backend_emit_scale_work_reg(FastLeafProcEmitter *emitter, i64 scale) {
	GB_ASSERT(scale > 0);
	if (scale == 1) {
		return;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		if (is_power_of_two(scale)) {
			gb_fprintf(emitter->file, "\tshl %s, %llu\n", work->r64, cast(unsigned long long)floor_log2(cast(u64)scale));
		} else {
			gb_fprintf(emitter->file, "\timul %s, %s, %lld\n", work->r64, work->r64, cast(long long)scale);
		}
	} else {
		char const *work = fast_backend_arm64_work_reg();
		if (is_power_of_two(scale)) {
			gb_fprintf(emitter->file, "\tlsl %s, %s, #%llu\n", work, work, cast(unsigned long long)floor_log2(cast(u64)scale));
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_addr_tmp_reg(), cast(u64)scale);
			gb_fprintf(emitter->file, "\tmul %s, %s, %s\n", work, work, fast_backend_arm64_addr_tmp_reg());
		}
	}
}

gb_internal void fast_backend_emit_add_tmp_reg_to_work_reg(FastLeafProcEmitter *emitter) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tadd %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tadd %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
	}
}

gb_internal void fast_backend_emit_x64_store_immediate_bytes(gbFile *file, FastX64RegNames const *base, u8 const *bytes, i32 size) {
	i32 offset = 0;
	while (offset < size) {
		i32 chunk = 1;
		if (size-offset >= 8) {
			chunk = 8;
		} else if (size-offset >= 4) {
			chunk = 4;
		} else if (size-offset >= 2) {
			chunk = 2;
		}

		u64 value = 0;
		for (i32 i = 0; i < chunk; i++) {
			value |= cast(u64)bytes[offset+i] << (8*i);
		}

		fast_backend_emit_x64_load_imm(file, fast_backend_x64_scratch_reg(), value);
		switch (chunk) {
		case 8: gb_fprintf(file, "\tmov QWORD PTR [%s + %d], %s\n", base->r64, offset, fast_backend_x64_scratch_reg()->r64); break;
		case 4: gb_fprintf(file, "\tmov DWORD PTR [%s + %d], %s\n", base->r64, offset, fast_backend_x64_scratch_reg()->r32); break;
		case 2: gb_fprintf(file, "\tmov WORD PTR [%s + %d], %s\n",  base->r64, offset, fast_backend_x64_scratch_reg()->r16); break;
		case 1: gb_fprintf(file, "\tmov BYTE PTR [%s + %d], %s\n",  base->r64, offset, fast_backend_x64_scratch_reg()->r8); break;
		}
		offset += chunk;
	}
}

gb_internal void fast_backend_emit_arm64_store_immediate_bytes(gbFile *file, char const *base, u8 const *bytes, i32 size) {
	i32 offset = 0;
	while (offset < size) {
		i32 chunk = 1;
		if (size-offset >= 8) {
			chunk = 8;
		} else if (size-offset >= 4) {
			chunk = 4;
		} else if (size-offset >= 2) {
			chunk = 2;
		}

		u64 value = 0;
		for (i32 i = 0; i < chunk; i++) {
			value |= cast(u64)bytes[offset+i] << (8*i);
		}

		fast_backend_emit_arm64_load_imm(file, fast_backend_arm64_div_tmp_reg(), value);
		fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), base, offset);
		switch (chunk) {
		case 8: gb_fprintf(file, "\tstr %s, [%s]\n",  fast_backend_arm64_div_tmp_reg(), fast_backend_arm64_addr_tmp_reg()); break;
		case 4: gb_fprintf(file, "\tstr w%s, [%s]\n", fast_backend_arm64_div_tmp_reg()+1, fast_backend_arm64_addr_tmp_reg()); break;
		case 2: gb_fprintf(file, "\tstrh w%s, [%s]\n", fast_backend_arm64_div_tmp_reg()+1, fast_backend_arm64_addr_tmp_reg()); break;
		case 1: gb_fprintf(file, "\tstrb w%s, [%s]\n", fast_backend_arm64_div_tmp_reg()+1, fast_backend_arm64_addr_tmp_reg()); break;
		}
		offset += chunk;
	}
}

gb_internal void fast_backend_emit_store_immediate_bytes(FastLeafProcEmitter *emitter, u8 const *bytes, i32 size) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_store_immediate_bytes(emitter->file, fast_backend_x64_work_reg(), bytes, size);
	} else {
		fast_backend_emit_arm64_store_immediate_bytes(emitter->file, fast_backend_arm64_work_reg(), bytes, size);
	}
}

gb_internal void fast_backend_emit_copy_bytes_between_addresses(FastLeafProcEmitter *emitter, i32 size) {
	i32 offset = 0;
	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *dst = fast_backend_x64_work_reg();
		auto *src = fast_backend_x64_tmp_reg();
		auto *scratch = fast_backend_x64_scratch_reg();
		while (offset < size) {
			i32 chunk = 1;
			if (size-offset >= 8) {
				chunk = 8;
			} else if (size-offset >= 4) {
				chunk = 4;
			} else if (size-offset >= 2) {
				chunk = 2;
			}

			switch (chunk) {
			case 8:
				gb_fprintf(emitter->file, "\tmov %s, QWORD PTR [%s + %d]\n", scratch->r64, src->r64, offset);
				gb_fprintf(emitter->file, "\tmov QWORD PTR [%s + %d], %s\n", dst->r64, offset, scratch->r64);
				break;
			case 4:
				gb_fprintf(emitter->file, "\tmov %s, DWORD PTR [%s + %d]\n", scratch->r32, src->r64, offset);
				gb_fprintf(emitter->file, "\tmov DWORD PTR [%s + %d], %s\n", dst->r64, offset, scratch->r32);
				break;
			case 2:
				gb_fprintf(emitter->file, "\tmovzx %s, WORD PTR [%s + %d]\n", scratch->r64, src->r64, offset);
				gb_fprintf(emitter->file, "\tmov WORD PTR [%s + %d], %s\n", dst->r64, offset, scratch->r16);
				break;
			case 1:
				gb_fprintf(emitter->file, "\tmovzx %s, BYTE PTR [%s + %d]\n", scratch->r64, src->r64, offset);
				gb_fprintf(emitter->file, "\tmov BYTE PTR [%s + %d], %s\n", dst->r64, offset, scratch->r8);
				break;
			}
			offset += chunk;
		}
	} else {
		char const *dst = fast_backend_arm64_work_reg();
		char const *src = fast_backend_arm64_tmp_reg();
		char const *value = fast_backend_arm64_div_tmp_reg();
		char const *addr  = fast_backend_arm64_addr_tmp_reg();
		while (offset < size) {
			i32 chunk = 1;
			if (size-offset >= 8) {
				chunk = 8;
			} else if (size-offset >= 4) {
				chunk = 4;
			} else if (size-offset >= 2) {
				chunk = 2;
			}

			fast_backend_emit_arm64_add_offset(emitter->file, addr, src, offset);
			switch (chunk) {
			case 8: gb_fprintf(emitter->file, "\tldr %s, [%s]\n",  value, addr); break;
			case 4: gb_fprintf(emitter->file, "\tldr w%s, [%s]\n", value+1, addr); break;
			case 2: gb_fprintf(emitter->file, "\tldrh w%s, [%s]\n", value+1, addr); break;
			case 1: gb_fprintf(emitter->file, "\tldrb w%s, [%s]\n", value+1, addr); break;
			}
			fast_backend_emit_arm64_add_offset(emitter->file, addr, dst, offset);
			switch (chunk) {
			case 8: gb_fprintf(emitter->file, "\tstr %s, [%s]\n",  value, addr); break;
			case 4: gb_fprintf(emitter->file, "\tstr w%s, [%s]\n", value+1, addr); break;
			case 2: gb_fprintf(emitter->file, "\tstrh w%s, [%s]\n", value+1, addr); break;
			case 1: gb_fprintf(emitter->file, "\tstrb w%s, [%s]\n", value+1, addr); break;
			}
			offset += chunk;
		}
	}
}

gb_internal void fast_backend_emit_zero_bytes_at_work_address(FastLeafProcEmitter *emitter, i32 size) {
	auto *zero_bytes = gb_alloc_array(temporary_allocator(), u8, size);
	gb_zero_size(zero_bytes, size);
	fast_backend_emit_store_immediate_bytes(emitter, zero_bytes, size);
}

gb_internal void fast_backend_emit_x64_canonicalize(gbFile *file, FastX64RegNames const *reg, FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
		gb_fprintf(file, "\tand %s, 1\n", reg->r64);
		break;
	case FastScalar_Signed:
		switch (type.bit_size) {
		case 8:  gb_fprintf(file, "\tmovsx %s, %s\n", reg->r64, reg->r8);  break;
		case 16: gb_fprintf(file, "\tmovsx %s, %s\n", reg->r64, reg->r16); break;
		case 32: gb_fprintf(file, "\tmovsxd %s, %s\n", reg->r64, reg->r32); break;
		}
		break;
	case FastScalar_Unsigned:
		switch (type.bit_size) {
		case 8:  gb_fprintf(file, "\tmovzx %s, %s\n", reg->r64, reg->r8);  break;
		case 16: gb_fprintf(file, "\tmovzx %s, %s\n", reg->r64, reg->r16); break;
		case 32: gb_fprintf(file, "\tmov %s, %s\n", reg->r32, reg->r32); break;
		}
		break;
	}
}

gb_internal void fast_backend_emit_arm64_canonicalize(gbFile *file, char const *reg, FastScalarType type) {
	switch (type.kind) {
	case FastScalar_Bool:
		gb_fprintf(file, "\tand %s, %s, #1\n", reg, reg);
		break;
	case FastScalar_Signed:
		switch (type.bit_size) {
		case 8:  gb_fprintf(file, "\tsxtb %s, w%s\n", reg, reg+1); break;
		case 16: gb_fprintf(file, "\tsxth %s, w%s\n", reg, reg+1); break;
		case 32: gb_fprintf(file, "\tsxtw %s, w%s\n", reg, reg+1); break;
		}
		break;
	case FastScalar_Unsigned:
		switch (type.bit_size) {
		case 8:  gb_fprintf(file, "\tuxtb %s, w%s\n", reg, reg+1); break;
		case 16: gb_fprintf(file, "\tuxth %s, w%s\n", reg, reg+1); break;
		case 32: gb_fprintf(file, "\tuxtw %s, w%s\n", reg, reg+1); break;
		}
		break;
	}
}

gb_internal void fast_backend_emit_convert_work_reg(FastLeafProcEmitter *emitter, FastScalarType target_type) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		if (target_type.kind == FastScalar_Bool) {
			gb_fprintf(emitter->file, "\tcmp %s, 0\n", work->r64);
			gb_fprintf(emitter->file, "\tsetne %s\n", work->r8);
			gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
		} else {
			fast_backend_emit_x64_canonicalize(emitter->file, work, target_type);
		}
	} else {
		char const *work = fast_backend_arm64_work_reg();
		if (target_type.kind == FastScalar_Bool) {
			gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
			gb_fprintf(emitter->file, "\tcset w%s, ne\n", work+1);
		} else {
			fast_backend_emit_arm64_canonicalize(emitter->file, work, target_type);
		}
	}
}

gb_internal bool fast_backend_emit_leaf_expr(FastLeafProcEmitter *emitter, Ast *expr);

gb_internal bool fast_backend_emit_leaf_cast(FastLeafProcEmitter *emitter, Ast *operand, Type *target_type) {
	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(nullptr, target_type, &result_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, operand)) {
		return false;
	}
	fast_backend_emit_convert_work_reg(emitter, result_type);
	return true;
}

gb_internal bool fast_backend_emit_address_of_storage_entity(FastLeafProcEmitter *emitter, Entity *entity) {
	FastLocalSlot slot = {};
	Type *type = nullptr;
	bool is_global = false;
	if (!fast_backend_find_storage(emitter->plan, entity, &slot, &type, &is_global)) {
		return false;
	}
	gb_unused(type);

	if (is_global) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_arm64_work_reg());
		}
		return true;
	}

	i32 offset = fast_backend_slot_offset(emitter->plan, slot);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tlea %s, [rbp-%d]\n", fast_backend_x64_work_reg()->r64, offset);
	} else {
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_work_reg(), "x29", -offset);
	}
	return true;
}

gb_internal bool fast_backend_emit_address_of_slot(FastLeafProcEmitter *emitter, FastLocalSlot const &slot) {
	if (emitter == nullptr) {
		return false;
	}
	i32 offset = fast_backend_slot_offset(emitter->plan, slot);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tlea %s, [rbp-%d]\n", fast_backend_x64_work_reg()->r64, offset);
	} else {
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_work_reg(), "x29", -offset);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_deref(FastLeafProcEmitter *emitter, Ast *expr) {
	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(nullptr, type_of_expr(expr), &result_type)) {
		return false;
	}
	if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
		return false;
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), result_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), result_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_value(FastLeafProcEmitter *emitter, Ast *expr) {
	TypeAndValue tv = type_and_value_of_expr(expr);
	Type *expr_type = reduce_tuple_to_single_type(tv.type);
	FastScalarType scalar_type = {};
	if (expr->kind == Ast_Implicit && expr->Implicit.kind == Token_context) {
		if (!emitter->plan->has_context_slot) {
			return false;
		}
		scalar_type = emitter->plan->context_slot.type;
	} else if (is_type_untyped_nil(expr_type)) {
		scalar_type = fast_backend_context_scalar_type();
	} else if (!fast_backend_classify_scalar_type(expr_type, &scalar_type)) {
		return false;
	}

	if (tv.mode == Addressing_Constant || is_type_untyped_nil(expr_type)) {
		u64 value = 0;
		if (!is_type_untyped_nil(expr_type) && !fast_backend_exact_value_as_u64(tv.value, scalar_type, &value)) {
			return false;
		}

		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), value);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), value);
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), scalar_type);
		}
		return true;
	}

	if (expr->kind == Ast_Implicit && expr->Implicit.kind == Token_context && emitter->plan->has_context_slot) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
		}
		return true;
	}

	if (expr->kind == Ast_SelectorExpr && fast_backend_can_emit_direct_struct_selector_expr(emitter->plan, expr)) {
		return fast_backend_emit_leaf_direct_struct_selector_expr(emitter, expr);
	}
	if (expr->kind == Ast_IndexExpr && fast_backend_can_emit_direct_array_index_expr(emitter->plan, expr)) {
		return fast_backend_emit_leaf_direct_array_index_expr(emitter, expr);
	}
	if (expr->kind == Ast_IndexExpr && fast_backend_can_emit_direct_slice_index_expr(emitter->plan, expr)) {
		return fast_backend_emit_leaf_direct_slice_index_expr(emitter, expr);
	}

	if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
		return false;
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_direct_struct_selector_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	expr = unparen_expr(expr);
	if (emitter == nullptr || expr == nullptr || !fast_backend_can_emit_direct_struct_selector_expr(emitter->plan, expr)) {
		return false;
	}

	Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
	FastScalarType scalar_type = {};
	Type *value_type = nullptr;
	Type *container_type = nullptr;
	i64 offset = 0;
	bool base_is_pointer = false;
	if (base_value_type == nullptr ||
	    !fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type) ||
	    !fast_backend_get_selector_info(expr, &value_type, &container_type, &offset, &base_is_pointer)) {
		return false;
	}
	gb_unused(value_type);
	gb_unused(container_type);
	gb_unused(base_is_pointer);

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
	emitter->current_spill_depth += temp_slots;
	i32 temp_depth = emitter->current_spill_depth;
	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr->SelectorExpr.expr, base_value_type)) {
		if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, base_value_type, &unparen_expr(expr->SelectorExpr.expr)->CompoundLit)) {
			return false;
		}
	} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->SelectorExpr.expr, base_value_type)) {
		if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr->SelectorExpr.expr)->CallExpr)) {
			return false;
		}
	} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->SelectorExpr.expr)) {
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
	} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->SelectorExpr.expr)) {
		return false;
	}

	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, cast(i32)offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_direct_array_index_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	expr = unparen_expr(expr);
	if (emitter == nullptr || expr == nullptr || !fast_backend_can_emit_direct_array_index_expr(emitter->plan, expr)) {
		return false;
	}

	Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
	FastScalarType scalar_type = {};
	Type *value_type = nullptr;
	i64 elem_size = 0;
	bool base_is_pointer = false;
	bool base_uses_data_pointer = false;
	if (base_value_type == nullptr ||
	    !fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type) ||
	    !fast_backend_get_index_info(&expr->IndexExpr, &value_type, &elem_size, &base_is_pointer, &base_uses_data_pointer)) {
		return false;
	}
	gb_unused(value_type);
	gb_unused(base_is_pointer);
	gb_unused(base_uses_data_pointer);

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
	emitter->current_spill_depth += temp_slots;
	i32 temp_depth = emitter->current_spill_depth;
	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
		if (!fast_backend_emit_store_array_binary_expr_to_work_address(emitter, base_value_type, expr->IndexExpr.expr)) {
			return false;
		}
	} else if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
		if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, base_value_type, &unparen_expr(expr->IndexExpr.expr)->CompoundLit)) {
			return false;
		}
	} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
		if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr->IndexExpr.expr)->CallExpr)) {
			return false;
		}
	} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->IndexExpr.expr)) {
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
	} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->IndexExpr.expr)) {
		return false;
	}

	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, expr->IndexExpr.index)) {
		return false;
	}
	fast_backend_emit_scale_work_reg(emitter, elem_size);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_direct_slice_index_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	expr = unparen_expr(expr);
	if (emitter == nullptr || expr == nullptr || !fast_backend_can_emit_direct_slice_index_expr(emitter->plan, expr)) {
		return false;
	}

	Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
	Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
	FastScalarType scalar_type = {};
	Type *value_type = nullptr;
	i64 elem_size = 0;
	bool base_is_pointer = false;
	bool base_uses_data_pointer = false;
	if (base_expr == nullptr ||
	    base_value_type == nullptr ||
	    !fast_backend_expr_scalar_type(expr, type_of_expr(expr), &scalar_type) ||
	    !fast_backend_get_index_info(&expr->IndexExpr, &value_type, &elem_size, &base_is_pointer, &base_uses_data_pointer)) {
		return false;
	}
	gb_unused(value_type);
	gb_unused(base_is_pointer);
	gb_unused(base_uses_data_pointer);

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
	emitter->current_spill_depth += temp_slots;
	i32 temp_depth = emitter->current_spill_depth;
	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	if (base_expr->kind == Ast_SliceExpr) {
		if (!fast_backend_emit_store_slice_expr_to_address(emitter, base_value_type, &base_expr->SliceExpr)) {
			return false;
		}
	} else if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
		if (!fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, base_value_type, expr->IndexExpr.expr)) {
			return false;
		}
	} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
		if (!fast_backend_emit_call_expr_to_address(emitter, &base_expr->CallExpr)) {
			return false;
		}
	} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->IndexExpr.expr)) {
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
	} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->IndexExpr.expr)) {
		return false;
	}
	fast_backend_emit_address_of_spill_depth(emitter, temp_depth);

	FastScalarType data_pointer_type = fast_backend_context_scalar_type();
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), data_pointer_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), data_pointer_type);
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, expr->IndexExpr.index)) {
		return false;
	}
	fast_backend_emit_scale_work_reg(emitter, elem_size);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
	}
	return true;
}

gb_internal void fast_backend_emit_push_work_reg(FastLeafProcEmitter *emitter) {
	i32 offset = 8 * (emitter->current_spill_depth + 1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov QWORD PTR [rbp-%d], %s\n", offset, fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tstr %s, [x29, #-%d]\n", fast_backend_arm64_work_reg(), offset);
	}
	emitter->current_spill_depth += 1;
}

gb_internal i32 fast_backend_spill_slot_offset_from_depth(i32 depth) {
	GB_ASSERT(depth > 0);
	return 8 * depth;
}

gb_internal void fast_backend_emit_load_work_from_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, QWORD PTR [rbp-%d]\n", fast_backend_x64_work_reg()->r64, offset);
	} else {
		gb_fprintf(emitter->file, "\tldr %s, [x29, #-%d]\n", fast_backend_arm64_work_reg(), offset);
	}
}

gb_internal void fast_backend_emit_load_tmp_from_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, QWORD PTR [rbp-%d]\n", fast_backend_x64_tmp_reg()->r64, offset);
	} else {
		gb_fprintf(emitter->file, "\tldr %s, [x29, #-%d]\n", fast_backend_arm64_tmp_reg(), offset);
	}
}

gb_internal void fast_backend_emit_store_work_to_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov QWORD PTR [rbp-%d], %s\n", offset, fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tstr %s, [x29, #-%d]\n", fast_backend_arm64_work_reg(), offset);
	}
}

gb_internal void fast_backend_emit_address_of_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tlea %s, [rbp-%d]\n", fast_backend_x64_work_reg()->r64, offset);
	} else {
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_work_reg(), "x29", -offset);
	}
}

gb_internal void fast_backend_emit_pop_tmp_reg(FastLeafProcEmitter *emitter) {
	GB_ASSERT(emitter->current_spill_depth > 0);
	i32 offset = 8 * emitter->current_spill_depth;
	emitter->current_spill_depth -= 1;
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, QWORD PTR [rbp-%d]\n", fast_backend_x64_tmp_reg()->r64, offset);
	} else {
		gb_fprintf(emitter->file, "\tldr %s, [x29, #-%d]\n", fast_backend_arm64_tmp_reg(), offset);
	}
}

gb_internal bool fast_backend_emit_address_expr(FastLeafProcEmitter *emitter, Ast *expr, Type **type_) {
	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return false;
	}

	Type *value_type = nullptr;

	switch (expr->kind) {
	case Ast_Ident: {
		Entity *entity = expr->Ident.entity.load();
		if (entity == nullptr) {
			return false;
		}
		if (fast_backend_find_storage(emitter->plan, entity, nullptr, &value_type, nullptr)) {
			if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
				return false;
			}
			break;
		}

		Entity *parent = nullptr;
		i64 offset = 0;
		bool parent_is_pointer = false;
		if (!fast_backend_get_using_entity_info(entity, &parent, &value_type, &offset, &parent_is_pointer)) {
			return false;
		}
		if (parent_is_pointer) {
			Ast *parent_expr = parent->identifier.load() ? parent->identifier.load() : entity->using_expr;
			if (parent_expr == nullptr || !fast_backend_emit_leaf_expr(emitter, parent_expr)) {
				return false;
			}
		} else if (!fast_backend_emit_address_of_storage_entity(emitter, parent)) {
			return false;
		}
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		break;
	}

	case Ast_DerefExpr: {
		Type *pointer_type = base_type(type_of_expr(expr->DerefExpr.expr));
		if (pointer_type == nullptr || (pointer_type->kind != Type_Pointer && pointer_type->kind != Type_MultiPointer)) {
			return false;
		}
		value_type = type_deref(pointer_type);
		if (!fast_backend_emit_leaf_expr(emitter, expr->DerefExpr.expr)) {
			return false;
		}
		break;
	}

	case Ast_SelectorExpr: {
		Type *container_type = nullptr;
		i64 offset = 0;
		bool base_is_pointer = false;
		if (!fast_backend_get_selector_info(expr, &value_type, &container_type, &offset, &base_is_pointer)) {
			return false;
		}
		gb_unused(container_type);
		if (base_is_pointer) {
			if (!fast_backend_emit_leaf_expr(emitter, expr->SelectorExpr.expr)) {
				return false;
			}
		} else if (default_type(type_of_expr(expr->SelectorExpr.expr)) != nullptr &&
		           base_type(default_type(type_of_expr(expr->SelectorExpr.expr)))->kind == Type_Struct &&
		           !fast_backend_can_emit_address_expr(emitter->plan, expr->SelectorExpr.expr, nullptr, nullptr, nullptr)) {
			Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
			i32 spill_base = emitter->current_spill_depth;
			defer (emitter->current_spill_depth = spill_base);
			i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
			emitter->current_spill_depth += temp_slots;
			i32 temp_depth = emitter->current_spill_depth;
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
			if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr->SelectorExpr.expr, base_value_type)) {
				if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, base_value_type, &unparen_expr(expr->SelectorExpr.expr)->CompoundLit)) {
					return false;
				}
			} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->SelectorExpr.expr, base_value_type)) {
				if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr->SelectorExpr.expr)->CallExpr)) {
					return false;
				}
			} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->SelectorExpr.expr)) {
				fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
			} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->SelectorExpr.expr)) {
				return false;
			}
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
		} else if (!fast_backend_emit_address_expr(emitter, expr->SelectorExpr.expr, nullptr)) {
			return false;
		}
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		break;
	}

	case Ast_IndexExpr: {
		i64 elem_size = 0;
		bool base_is_pointer = false;
		bool base_uses_data_pointer = false;
		if (!fast_backend_get_index_info(&expr->IndexExpr, &value_type, &elem_size, &base_is_pointer, &base_uses_data_pointer)) {
			return false;
		}
		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);
		if (base_is_pointer) {
			if (!fast_backend_emit_leaf_expr(emitter, expr->IndexExpr.expr)) {
				return false;
			}
		} else if (default_type(type_of_expr(expr->IndexExpr.expr)) != nullptr &&
		           base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Array &&
		           !fast_backend_can_emit_address_expr(emitter->plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
			emitter->current_spill_depth += temp_slots;
			i32 temp_depth = emitter->current_spill_depth;
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
			if (fast_backend_can_emit_array_binary_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
				if (!fast_backend_emit_store_array_binary_expr_to_work_address(emitter, base_value_type, expr->IndexExpr.expr)) {
					return false;
				}
			} else if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
				if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, base_value_type, &unparen_expr(expr->IndexExpr.expr)->CompoundLit)) {
					return false;
				}
			} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
				if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr->IndexExpr.expr)->CallExpr)) {
					return false;
				}
			} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->IndexExpr.expr)) {
				fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
			} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->IndexExpr.expr)) {
				return false;
			}
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
		} else if (base_uses_data_pointer && !fast_backend_can_emit_address_expr(emitter->plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
			Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			if (base_expr == nullptr || base_value_type == nullptr) {
				return false;
			}
			i32 temp_slots = align_formula(cast(i32)type_size_of(base_value_type), 8)/8;
			emitter->current_spill_depth += temp_slots;
			i32 temp_depth = emitter->current_spill_depth;
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
			if (base_expr->kind == Ast_SliceExpr) {
				if (!fast_backend_emit_store_slice_expr_to_address(emitter, base_value_type, &base_expr->SliceExpr)) {
					return false;
				}
			} else if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
				if (!fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, base_value_type, expr->IndexExpr.expr)) {
					return false;
				}
			} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr->IndexExpr.expr, base_value_type)) {
				if (!fast_backend_emit_call_expr_to_address(emitter, &base_expr->CallExpr)) {
					return false;
				}
			} else if (fast_backend_expr_is_zero_aggregate_value(base_value_type, expr->IndexExpr.expr)) {
				fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(base_value_type));
			} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, base_value_type, expr->IndexExpr.expr)) {
				return false;
			}
			fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
		} else if (!fast_backend_emit_address_expr(emitter, expr->IndexExpr.expr, nullptr)) {
			return false;
		}
		if (base_uses_data_pointer) {
			FastScalarType data_pointer_type = fast_backend_context_scalar_type();
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), data_pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), data_pointer_type);
			}
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_leaf_expr(emitter, expr->IndexExpr.index)) {
			return false;
		}
		fast_backend_emit_scale_work_reg(emitter, elem_size);
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
		break;
	}

	default:
		return false;
	}

	if (type_) *type_ = value_type;
	return true;
}

gb_internal void fast_backend_emit_stack_adjust(FastLeafProcEmitter *emitter, i32 bytes, bool subtract) {
	if (bytes == 0) {
		return;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\t%s rsp, %d\n", subtract ? "sub" : "add", bytes);
	} else if (bytes <= 4095) {
		gb_fprintf(emitter->file, "\t%s sp, sp, #%d\n", subtract ? "sub" : "add", bytes);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_addr_tmp_reg(), cast(u64)bytes);
		gb_fprintf(emitter->file, "\t%s sp, sp, %s\n", subtract ? "sub" : "add", fast_backend_arm64_addr_tmp_reg());
	}
}

gb_internal bool fast_backend_emit_alloc_stack_temp(FastLeafProcEmitter *emitter, i32 size, i32 align, i32 *alloc_bytes_) {
	if (size <= 0 || align <= 0) {
		return false;
	}
	i32 alloc_bytes = align_formula(size, gb_max(align, 16));
	fast_backend_emit_stack_adjust(emitter, alloc_bytes, true);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
	}
	if (alloc_bytes_) *alloc_bytes_ = alloc_bytes;
	return true;
}

gb_internal bool fast_backend_emit_materialize_aggregate_arg_pointer(FastLeafProcEmitter *emitter, Ast *arg, Type *param_type, i32 *temp_bytes_) {
	Type *src_type = nullptr;
	bool src_is_scalar = false;
	if (fast_backend_can_emit_address_expr(emitter->plan, arg, &src_type, nullptr, &src_is_scalar) &&
	    !src_is_scalar &&
	    src_type != nullptr &&
	    are_types_identical(default_type(src_type), default_type(param_type))) {
		return fast_backend_emit_address_expr(emitter, arg, nullptr);
	}

	param_type = default_type(param_type);
	if (arg != nullptr && unparen_expr(arg)->kind == Ast_CallExpr && fast_backend_can_emit_aggregate_call_expr(emitter->plan, arg, param_type)) {
		i32 alloc_bytes = 0;
		if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(param_type), cast(i32)type_align_of(param_type), &alloc_bytes)) {
			return false;
		}
		*temp_bytes_ += alloc_bytes;
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(arg)->CallExpr)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
		}
		return true;
	}

	i32 alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(param_type), cast(i32)type_align_of(param_type), &alloc_bytes)) {
		return false;
	}
	*temp_bytes_ += alloc_bytes;
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, arg, param_type)) {
		return fast_backend_emit_store_array_binary_expr_to_work_address(emitter, param_type, arg);
	}
	if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, arg, param_type)) {
		return fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, param_type, arg);
	}
	return fast_backend_emit_store_constant_aggregate_to_address(emitter, param_type, arg);
}

gb_internal bool fast_backend_emit_call_expr_internal(FastLeafProcEmitter *emitter, AstCallExpr *ce, bool has_explicit_result_address) {
	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	Type *result_type = nullptr;
	FastScalarType scalar_result_type = {};
	bool has_result = false;
	bool return_by_pointer = false;
	if (!fast_backend_get_call_info(ce, &pt, &proc_entity, &result_type, &scalar_result_type, &has_result, &return_by_pointer)) {
		return false;
	}
	if (return_by_pointer != has_explicit_result_address) {
		return false;
	}

	i32 temp_bytes = 0;
	if (return_by_pointer) {
		fast_backend_emit_push_work_reg(emitter);
	}

	for_array(i, ce->args) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return false;
		}
		FastScalarType param_scalar_type = {};
		if (fast_backend_classify_scalar_type(param->type, &param_scalar_type)) {
			if (!fast_backend_emit_leaf_expr(emitter, ce->args[i])) {
				return false;
			}
		} else if (!fast_backend_emit_materialize_aggregate_arg_pointer(emitter, ce->args[i], param->type, &temp_bytes)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
	}
	if (pt->calling_convention == ProcCC_Odin) {
		if (!emitter->plan->has_context_slot) {
			return false;
		}
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
		}
		fast_backend_emit_push_work_reg(emitter);
	}

	i32 total_arg_count = pt->param_count + (pt->calling_convention == ProcCC_Odin ? 1 : 0);
	if (return_by_pointer) {
		total_arg_count += 1;
	}
	for (i32 i = total_arg_count-1; i >= 0; i--) {
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *dst = fast_backend_x64_param_reg(pt, i);
			if (dst == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", dst->r64, fast_backend_x64_tmp_reg()->r64);
		} else {
			char const *dst = fast_backend_arm64_param_reg(i);
			if (dst == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", dst, fast_backend_arm64_tmp_reg());
		}
	}

	String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(proc_entity));
	if (build_context.metrics.arch == TargetArch_amd64) {
		ProcCallingConvention cc = fast_backend_effective_calling_convention(pt);
		if (cc == ProcCC_Win64) {
			gb_fprintf(emitter->file, "\tsub rsp, 32\n");
		}
		gb_fprintf(emitter->file, "\tcall \"%.*s\"\n", LIT(symbol));
		if (cc == ProcCC_Win64) {
			gb_fprintf(emitter->file, "\tadd rsp, 32\n");
		}
		if (has_result && !return_by_pointer) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_return_reg()->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), scalar_result_type);
		}
	} else {
		gb_fprintf(emitter->file, "\tbl \"%.*s\"\n", LIT(symbol));
		if (has_result && !return_by_pointer) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_return_reg());
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), scalar_result_type);
		}
	}

	if (temp_bytes != 0) {
		fast_backend_emit_stack_adjust(emitter, temp_bytes, false);
	}
	return true;
}

gb_internal bool fast_backend_emit_call_expr_to_address(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	return fast_backend_emit_call_expr_internal(emitter, ce, true);
}

gb_internal bool fast_backend_emit_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	return fast_backend_emit_call_expr_internal(emitter, ce, false);
}

gb_internal bool fast_backend_emit_raw_data_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	Type *type = base_type(type_of_expr(expr));
	if (type == nullptr) {
		return false;
	}

	expr = unparen_expr(expr);
	if (expr->kind == Ast_SliceExpr) {
		AstSliceExpr *se = &expr->SliceExpr;
		Type *source_type = base_type(type_of_expr(se->expr));
		if (source_type == nullptr) {
			return false;
		}

		i64 elem_size = 0;
		bool source_uses_data_pointer = false;
		switch (source_type->kind) {
		case Type_Array:
			elem_size = type_size_of(source_type->Array.elem);
			break;
		case Type_EnumeratedArray:
			elem_size = type_size_of(source_type->EnumeratedArray.elem);
			break;
		case Type_Slice:
			elem_size = type_size_of(source_type->Slice.elem);
			source_uses_data_pointer = true;
			break;
		case Type_DynamicArray:
			elem_size = type_size_of(source_type->DynamicArray.elem);
			source_uses_data_pointer = true;
			break;
		case Type_FixedCapacityDynamicArray:
			elem_size = type_size_of(source_type->FixedCapacityDynamicArray.elem);
			break;
		case Type_Basic:
			if (is_type_string(source_type)) {
				elem_size = 1;
				source_uses_data_pointer = true;
				break;
			}
			if (is_type_string16(source_type)) {
				elem_size = 2;
				source_uses_data_pointer = true;
				break;
			}
			return false;
		default:
			return false;
		}

		if (elem_size <= 0) {
			return false;
		}

		FastScalarType pointer_type = fast_backend_context_scalar_type();
		if (!fast_backend_emit_address_expr(emitter, se->expr, nullptr)) {
			return false;
		}
		if (source_uses_data_pointer) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
		}

		fast_backend_emit_push_work_reg(emitter);
		if (se->low != nullptr) {
			if (!fast_backend_emit_leaf_expr(emitter, se->low)) {
				return false;
			}
		} else if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r32, fast_backend_x64_work_reg()->r32);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
		}
		fast_backend_emit_scale_work_reg(emitter, elem_size);
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
		return true;
	}

	switch (type->kind) {
	case Type_Slice:
	case Type_DynamicArray:
	case Type_Basic:
		if (type->kind != Type_Basic || is_type_string(type) || is_type_string16(type)) {
			FastScalarType pointer_type = fast_backend_context_scalar_type();
			if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
				return false;
			}
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
			return true;
		}
		break;

	case Type_Pointer:
	case Type_MultiPointer:
		return fast_backend_emit_leaf_expr(emitter, expr);
	}

	return false;
}

gb_internal bool fast_backend_emit_builtin_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	if (emitter == nullptr || ce == nullptr) {
		return false;
	}

	BuiltinProcId id = fast_backend_builtin_proc_id(ce->proc);
	if ((id != BuiltinProc_len && id != BuiltinProc_cap && id != BuiltinProc_raw_data) || ce->args.count != 1) {
		return false;
	}

	Ast *arg = ce->args[0];
	Type *arg_type = base_type(type_of_expr(arg));
	bool arg_is_pointer_like = false;
	if (arg_type != nullptr && (arg_type->kind == Type_Pointer || arg_type->kind == Type_MultiPointer)) {
		arg_is_pointer_like = true;
		arg_type = default_type(type_deref(arg_type, true));
	}
	if (arg_type == nullptr) {
		return false;
	}

	if (id == BuiltinProc_raw_data) {
		return fast_backend_emit_raw_data_expr(emitter, arg);
	}

	if (id == BuiltinProc_len && (is_type_cstring(arg_type) || is_type_cstring16(arg_type))) {
		FastScalarType pointer_type = fast_backend_context_scalar_type();
		FastScalarType len_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
		bool is_wide = is_type_cstring16(arg_type);
		if (arg_is_pointer_like) {
			if (!fast_backend_emit_leaf_expr(emitter, arg)) {
				return false;
			}
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
		} else if (!fast_backend_emit_leaf_expr(emitter, arg)) {
			return false;
		}

		i32 loop_label = fast_backend_alloc_label(emitter);
		i32 done_label = fast_backend_alloc_label(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_work_reg()->r64);
			gb_fprintf(emitter->file, "\ttest %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_work_reg()->r64);
			char done_name[64] = {};
			fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
			gb_fprintf(emitter->file, "\tje %s\n", done_name);
			fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
			if (is_wide) {
				gb_fprintf(emitter->file, "\tmovzx %s, WORD PTR [%s]\n", fast_backend_x64_scratch_reg()->r64, fast_backend_x64_work_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\tmovzx %s, BYTE PTR [%s]\n", fast_backend_x64_scratch_reg()->r64, fast_backend_x64_work_reg()->r64);
			}
			gb_fprintf(emitter->file, "\ttest %s, %s\n", fast_backend_x64_scratch_reg()->r64, fast_backend_x64_scratch_reg()->r64);
			gb_fprintf(emitter->file, "\tje %s\n", done_name);
			gb_fprintf(emitter->file, "\tadd %s, %d\n", fast_backend_x64_work_reg()->r64, is_wide ? 2 : 1);
			fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
			fast_backend_emit_label(emitter->file, emitter->plan, done_label);
			gb_fprintf(emitter->file, "\tsub %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
			if (is_wide) {
				gb_fprintf(emitter->file, "\tshr %s, 1\n", fast_backend_x64_work_reg()->r64);
			}
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), len_type);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
			gb_fprintf(emitter->file, "\tcmp %s, #0\n", fast_backend_arm64_work_reg());
			char done_name[64] = {};
			fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
			gb_fprintf(emitter->file, "\tb.eq %s\n", done_name);
			fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
			if (is_wide) {
				gb_fprintf(emitter->file, "\tldrh w%s, [%s]\n", fast_backend_arm64_div_tmp_reg()+1, fast_backend_arm64_work_reg());
				gb_fprintf(emitter->file, "\tcmp w%s, #0\n", fast_backend_arm64_div_tmp_reg()+1);
			} else {
				gb_fprintf(emitter->file, "\tldrb w%s, [%s]\n", fast_backend_arm64_div_tmp_reg()+1, fast_backend_arm64_work_reg());
				gb_fprintf(emitter->file, "\tcmp w%s, #0\n", fast_backend_arm64_div_tmp_reg()+1);
			}
			gb_fprintf(emitter->file, "\tb.eq %s\n", done_name);
			gb_fprintf(emitter->file, "\tadd %s, %s, #%d\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), is_wide ? 2 : 1);
			fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
			fast_backend_emit_label(emitter->file, emitter->plan, done_label);
			gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
			if (is_wide) {
				gb_fprintf(emitter->file, "\tlsr %s, %s, #1\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg());
			}
		}
		return true;
	}

	if (arg_is_pointer_like) {
		if (!fast_backend_emit_leaf_expr(emitter, arg)) {
			return false;
		}
	} else if (!fast_backend_emit_address_expr(emitter, arg, nullptr)) {
		return false;
	}

	i64 offset = -1;
	if (id == BuiltinProc_len && (is_type_string(arg_type) || is_type_string16(arg_type) || is_type_slice(arg_type))) {
		offset = build_context.int_size;
	} else if (is_type_dynamic_array(arg_type)) {
		offset = id == BuiltinProc_len ? 1*build_context.int_size : 2*build_context.int_size;
	} else if (is_type_fixed_capacity_dynamic_array(arg_type) && id == BuiltinProc_len) {
		offset = type_offset_of(arg_type, 1);
	}
	if (offset < 0) {
		return false;
	}

	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_unary(FastLeafProcEmitter *emitter, Ast *expr) {
	if (expr->UnaryExpr.op.kind == Token_And) {
		return fast_backend_emit_address_expr(emitter, expr->UnaryExpr.expr, nullptr);
	}

	Type *expr_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr).type);
	FastScalarType scalar_type = {};
	if (!fast_backend_classify_scalar_type(expr_type, &scalar_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr->UnaryExpr.expr)) {
		return false;
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		switch (expr->UnaryExpr.op.kind) {
		case Token_Add:
			return true;
		case Token_Sub:
			gb_fprintf(emitter->file, "\tneg %s\n", work->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Not:
			gb_fprintf(emitter->file, "\tcmp %s, 0\n", work->r64);
			gb_fprintf(emitter->file, "\tsete %s\n", work->r8);
			gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
			return true;
		case Token_Xor:
			gb_fprintf(emitter->file, "\tnot %s\n", work->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		}
	} else {
		char const *work = fast_backend_arm64_work_reg();
		switch (expr->UnaryExpr.op.kind) {
		case Token_Add:
			return true;
		case Token_Sub:
			gb_fprintf(emitter->file, "\tneg %s, %s\n", work, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Not:
			gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
			gb_fprintf(emitter->file, "\tcset w9, eq\n");
			return true;
		case Token_Xor:
			gb_fprintf(emitter->file, "\tmvn %s, %s\n", work, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		}
	}

	return false;
}

gb_internal char const *fast_backend_x64_cmp_suffix(TokenKind op, FastScalarType type) {
	bool uns = fast_backend_scalar_is_unsigned(type);
	switch (op) {
	case Token_CmpEq: return "e";
	case Token_NotEq: return "ne";
	case Token_Lt:    return uns ? "b"  : "l";
	case Token_LtEq:  return uns ? "be" : "le";
	case Token_Gt:    return uns ? "a"  : "g";
	case Token_GtEq:  return uns ? "ae" : "ge";
	}
	return nullptr;
}

gb_internal char const *fast_backend_arm64_cmp_suffix(TokenKind op, FastScalarType type) {
	bool uns = fast_backend_scalar_is_unsigned(type);
	switch (op) {
	case Token_CmpEq: return "eq";
	case Token_NotEq: return "ne";
	case Token_Lt:    return uns ? "lo" : "lt";
	case Token_LtEq:  return uns ? "ls" : "le";
	case Token_Gt:    return uns ? "hi" : "gt";
	case Token_GtEq:  return uns ? "hs" : "ge";
	}
	return nullptr;
}

gb_internal bool fast_backend_emit_scalar_binary_op(FastLeafProcEmitter *emitter, TokenKind op, FastScalarType scalar_type, Ast *rhs) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		auto *tmp  = fast_backend_x64_tmp_reg();

		switch (op) {
		case Token_Add:
			gb_fprintf(emitter->file, "\tadd %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Sub:
			gb_fprintf(emitter->file, "\tsub %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Mul:
			gb_fprintf(emitter->file, "\timul %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Quo:
		case Token_Mod:
			gb_fprintf(emitter->file, "\txchg %s, %s\n", work->r64, tmp->r64);
			if (fast_backend_scalar_is_unsigned(scalar_type)) {
				gb_fprintf(emitter->file, "\txor edx, edx\n");
				gb_fprintf(emitter->file, "\tdiv %s\n", tmp->r64);
			} else {
				gb_fprintf(emitter->file, "\tcqo\n");
				gb_fprintf(emitter->file, "\tidiv %s\n", tmp->r64);
			}
			if (op == Token_Mod) {
				gb_fprintf(emitter->file, "\tmov %s, rdx\n", work->r64);
			}
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_And:
			gb_fprintf(emitter->file, "\tand %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Or:
			gb_fprintf(emitter->file, "\tor %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Xor:
			gb_fprintf(emitter->file, "\txor %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_AndNot:
			gb_fprintf(emitter->file, "\tnot %s\n", work->r64);
			gb_fprintf(emitter->file, "\tand %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Shl:
		case Token_Shr: {
			char const *inst = op == Token_Shl ? "shl" : (fast_backend_scalar_is_unsigned(scalar_type) ? "shr" : "sar");
			gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
			if (rhs != nullptr && type_and_value_of_expr(rhs).mode == Addressing_Constant) {
				ExactValue shift_value = type_and_value_of_expr(rhs).value;
				u64 shift = exact_value_to_u64(shift_value);
				gb_fprintf(emitter->file, "\t%s %s, %llu\n", inst, work->r64, cast(unsigned long long)shift);
			} else {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_x64_reg_rcx.r64, work->r64);
				gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
				gb_fprintf(emitter->file, "\t%s %s, cl\n", inst, work->r64);
			}
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		}
		case Token_CmpEq:
		case Token_NotEq:
		case Token_Lt:
		case Token_LtEq:
		case Token_Gt:
		case Token_GtEq: {
			char const *suffix = fast_backend_x64_cmp_suffix(op, scalar_type);
			if (suffix == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tcmp %s, %s\n", tmp->r64, work->r64);
			gb_fprintf(emitter->file, "\tset%s %s\n", suffix, work->r8);
			gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
			return true;
		}
		}
	} else {
		char const *work = fast_backend_arm64_work_reg();
		char const *tmp  = fast_backend_arm64_tmp_reg();

		switch (op) {
		case Token_Add:
			gb_fprintf(emitter->file, "\tadd %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Sub:
			gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Mul:
			gb_fprintf(emitter->file, "\tmul %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Quo:
			gb_fprintf(emitter->file, "\t%s %s, %s, %s\n", fast_backend_scalar_is_unsigned(scalar_type) ? "udiv" : "sdiv", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Mod:
			gb_fprintf(emitter->file, "\t%s %s, %s, %s\n", fast_backend_scalar_is_unsigned(scalar_type) ? "udiv" : "sdiv", fast_backend_arm64_div_tmp_reg(), tmp, work);
			gb_fprintf(emitter->file, "\tmsub %s, %s, %s, %s\n", work, fast_backend_arm64_div_tmp_reg(), work, tmp);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_And:
			gb_fprintf(emitter->file, "\tand %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Or:
			gb_fprintf(emitter->file, "\torr %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Xor:
			gb_fprintf(emitter->file, "\teor %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_AndNot:
			gb_fprintf(emitter->file, "\tbic %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Shl:
			gb_fprintf(emitter->file, "\tlsl %s, %s, %s\n", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Shr:
			gb_fprintf(emitter->file, "\t%s %s, %s, %s\n", fast_backend_scalar_is_unsigned(scalar_type) ? "lsr" : "asr", work, tmp, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_CmpEq:
		case Token_NotEq:
		case Token_Lt:
		case Token_LtEq:
		case Token_Gt:
		case Token_GtEq: {
			char const *suffix = fast_backend_arm64_cmp_suffix(op, scalar_type);
			if (suffix == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tcmp %s, %s\n", tmp, work);
			gb_fprintf(emitter->file, "\tcset w9, %s\n", suffix);
			return true;
		}
		}
	}

	return false;
}

gb_internal bool fast_backend_emit_leaf_binary(FastLeafProcEmitter *emitter, Ast *expr) {
	Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->BinaryExpr.left).type);
	FastScalarType scalar_type = {};
	if (!fast_backend_classify_scalar_type(operand_type, &scalar_type)) {
		return false;
	}

	if (!fast_backend_emit_leaf_expr(emitter, expr->BinaryExpr.left)) {
		return false;
	}

	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, expr->BinaryExpr.right)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	return fast_backend_emit_scalar_binary_op(emitter, expr->BinaryExpr.op.kind, scalar_type, expr->BinaryExpr.right);
}

gb_internal bool fast_backend_emit_leaf_aggregate_compare(FastLeafProcEmitter *emitter, Ast *expr) {
	if (emitter == nullptr || !fast_backend_can_emit_aggregate_compare_expr(emitter->plan, expr)) {
		return false;
	}

	Type *type = default_type(type_of_expr(expr->BinaryExpr.left));
	Type *base = base_type(type);
	TokenKind op = expr->BinaryExpr.op.kind;

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);
	i32 temp_bytes = 0;
	defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

	if (!fast_backend_emit_materialize_aggregate_compare_operand(emitter, expr->BinaryExpr.left, type, &temp_bytes)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 lhs_depth = spill_base + 1;

	if (!fast_backend_emit_materialize_aggregate_compare_operand(emitter, expr->BinaryExpr.right, type, &temp_bytes)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 rhs_depth = spill_base + 2;

	i32 mismatch_label = fast_backend_alloc_label(emitter);
	i32 matched_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	switch (base->kind) {
	case Type_Basic: {
		FastScalarType pointer_type = fast_backend_context_scalar_type();
		FastScalarType len_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
		i32 elem_size = is_type_string16(type) ? 2 : 1;

		if (!fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, build_context.int_size, len_type, mismatch_label)) {
			return false;
		}

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, 0);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 lhs_ptr_depth = spill_base + 3;

		fast_backend_emit_load_work_from_spill_depth(emitter, rhs_depth);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 rhs_ptr_depth = spill_base + 4;

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, build_context.int_size);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 len_depth = spill_base + 5;

		i32 loop_label = fast_backend_alloc_label(emitter);
		fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
		fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
		fast_backend_emit_jump_if_zero(emitter, matched_label);

		FastScalarType char_type = {};
		char_type.kind = FastScalar_Unsigned;
		char_type.bit_size = elem_size*8;
		if (!fast_backend_emit_compare_scalar_at_offset(emitter, lhs_ptr_depth, rhs_ptr_depth, 0, char_type, mismatch_label)) {
			return false;
		}

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_ptr_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, elem_size);
		fast_backend_emit_store_work_to_spill_depth(emitter, lhs_ptr_depth);
		fast_backend_emit_load_work_from_spill_depth(emitter, rhs_ptr_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, elem_size);
		fast_backend_emit_store_work_to_spill_depth(emitter, rhs_ptr_depth);
		fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, -1);
		fast_backend_emit_store_work_to_spill_depth(emitter, len_depth);
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
		break;
	}

	case Type_Array: {
		FastScalarType scalar_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(base->Array.elem, &scalar_type));
		i32 elem_size = cast(i32)type_size_of(base->Array.elem);
		for (i64 i = 0; i < base->Array.count; i++) {
			if (!fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, cast(i32)(i*elem_size), scalar_type, mismatch_label)) {
				return false;
			}
		}
		break;
	}

	case Type_Struct: {
		type_set_offsets(base);
		for (Entity *field : base->Struct.fields) {
			Type *field_type = nullptr;
			i32 offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
			FastScalarType scalar_type = {};
			GB_ASSERT(field_type != nullptr && fast_backend_classify_scalar_type(field_type, &scalar_type));
			if (!fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, offset, scalar_type, mismatch_label)) {
				return false;
			}
		}
		break;
	}

	default:
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, matched_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), op == Token_CmpEq ? 1 : 0);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), op == Token_CmpEq ? 1 : 0);
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, mismatch_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), op == Token_CmpEq ? 0 : 1);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), op == Token_CmpEq ? 0 : 1);
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_materialize_aggregate_compare_operand(FastLeafProcEmitter *emitter, Ast *expr, Type *type, i32 *temp_bytes) {
	if (emitter == nullptr || type == nullptr || expr == nullptr || temp_bytes == nullptr) {
		return false;
	}

	Type *storage_type = nullptr;
	bool is_scalar = false;
	if (fast_backend_can_emit_address_expr(emitter->plan, expr, &storage_type, nullptr, &is_scalar) &&
	    !is_scalar &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), default_type(type))) {
		return fast_backend_emit_address_expr(emitter, expr, nullptr);
	}

	if (!fast_backend_can_emit_constant_aggregate_expr(type, expr)) {
		if (!fast_backend_can_emit_array_binary_expr(emitter->plan, expr, type) &&
		    !fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr, type) &&
		    !fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr, type) &&
		    !(unparen_expr(expr) != nullptr &&
		      unparen_expr(expr)->kind == Ast_SliceExpr &&
		      fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(expr)->SliceExpr, type))) {
			return false;
		}
	}

	i32 alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(type), cast(i32)type_align_of(type), &alloc_bytes)) {
		return false;
	}
	*temp_bytes += alloc_bytes;
	fast_backend_emit_push_work_reg(emitter);

	if (fast_backend_can_emit_array_binary_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_store_array_binary_expr_to_work_address(emitter, type, expr)) {
			return false;
		}
	} else if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, &unparen_expr(expr)->CompoundLit)) {
			return false;
		}
	} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr)->CallExpr)) {
			return false;
		}
	} else if (unparen_expr(expr) != nullptr &&
	           unparen_expr(expr)->kind == Ast_SliceExpr &&
	           fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(expr)->SliceExpr, type)) {
		if (!fast_backend_emit_store_slice_expr_to_address(emitter, type, &unparen_expr(expr)->SliceExpr)) {
			return false;
		}
	} else if (fast_backend_expr_is_zero_aggregate_value(type, expr)) {
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));
	} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, type, expr)) {
		return false;
	}

	fast_backend_emit_pop_tmp_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode == Addressing_Constant ||
	    expr->kind == Ast_Ident ||
	    expr->kind == Ast_Implicit ||
	    expr->kind == Ast_SelectorExpr ||
	    expr->kind == Ast_IndexExpr) {
		return fast_backend_emit_leaf_value(emitter, expr);
	}

	switch (expr->kind) {
	case Ast_ParenExpr:
		return fast_backend_emit_leaf_expr(emitter, expr->ParenExpr.expr);
	case Ast_DerefExpr:
		return fast_backend_emit_leaf_deref(emitter, expr);
	case Ast_TypeCast:
		return fast_backend_emit_leaf_cast(emitter, expr->TypeCast.expr, reduce_tuple_to_single_type(tv.type));
	case Ast_AutoCast:
		return fast_backend_emit_leaf_cast(emitter, expr->AutoCast.expr, reduce_tuple_to_single_type(tv.type));
	case Ast_UnaryExpr:
		return fast_backend_emit_leaf_unary(emitter, expr);
	case Ast_BinaryExpr:
		if (fast_backend_can_emit_aggregate_compare_expr(emitter->plan, expr)) {
			return fast_backend_emit_leaf_aggregate_compare(emitter, expr);
		}
		return fast_backend_emit_leaf_binary(emitter, expr);
	case Ast_CallExpr:
		if (expr->CallExpr.proc != nullptr && type_and_value_of_expr(expr->CallExpr.proc).mode == Addressing_Type) {
			if (expr->CallExpr.args.count != 1) {
				return false;
			}
			Ast *arg = expr->CallExpr.args[0];
			if (arg != nullptr && arg->kind == Ast_FieldValue) {
				return false;
			}
			return fast_backend_emit_leaf_cast(emitter, arg, reduce_tuple_to_single_type(tv.type));
		}
		if (fast_backend_builtin_proc_id(expr->CallExpr.proc) != BuiltinProc_Invalid) {
			return fast_backend_emit_builtin_call_expr(emitter, &expr->CallExpr);
		}
		return fast_backend_emit_call_expr(emitter, &expr->CallExpr);
	}

	return false;
}

gb_internal void fast_backend_make_label_name(char *buffer, isize buffer_size, FastLeafProcPlan *plan, i32 label_index) {
	gb_snprintf(buffer, buffer_size, "Lfast_%d_%d", plan->proc_index, label_index);
}

gb_internal void fast_backend_emit_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index) {
	char label[64] = {};
	fast_backend_make_label_name(label, gb_size_of(label), plan, label_index);
	gb_fprintf(file, "%s:\n", label);
}

gb_internal void fast_backend_emit_jump_to_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index) {
	char label[64] = {};
	fast_backend_make_label_name(label, gb_size_of(label), plan, label_index);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(file, "\tjmp %s\n", label);
	} else {
		gb_fprintf(file, "\tb %s\n", label);
	}
}

gb_internal i32 fast_backend_alloc_label(FastLeafProcEmitter *emitter) {
	i32 label = emitter->next_label_index;
	emitter->next_label_index += 1;
	return label;
}

gb_internal FastControlContext *fast_backend_find_control_context(FastLeafProcEmitter *emitter, Ast *label, TokenKind token_kind) {
	for (isize i = emitter->control_stack.count; i-- > 0;) {
		FastControlContext *ctx = &emitter->control_stack[i];
		if (label != nullptr) {
			String ctx_label = fast_backend_label_name(ctx->label);
			String want_label = fast_backend_label_name(label);
			if (ctx_label.len == 0 || want_label.len == 0 || ctx_label != want_label) {
				continue;
			}
		}

		i32 target_label = -1;
		switch (token_kind) {
		case Token_break:
			target_label = ctx->break_label;
			break;
		case Token_continue:
			target_label = ctx->continue_label;
			break;
		case Token_fallthrough:
			target_label = ctx->fallthrough_label;
			break;
		}

		if (target_label >= 0) {
			return ctx;
		}
		if (label != nullptr) {
			return nullptr;
		}
	}
	return nullptr;
}

gb_internal isize fast_backend_defer_base_for_scope_count(FastLeafProcEmitter *emitter, i32 keep_scope_count) {
	if (keep_scope_count <= 0) {
		return 0;
	}
	if (keep_scope_count >= cast(i32)emitter->scope_stack.count) {
		return emitter->deferred_stmts.count;
	}
	return emitter->scope_stack[keep_scope_count].defer_base;
}

gb_internal bool fast_backend_emit_scope_exit_defers(FastLeafProcEmitter *emitter, i32 keep_scope_count) {
	isize defer_base = fast_backend_defer_base_for_scope_count(emitter, keep_scope_count);
	while (emitter->deferred_stmts.count > defer_base) {
		FastDeferredStmt deferred = emitter->deferred_stmts[emitter->deferred_stmts.count-1];
		emitter->deferred_stmts.count -= 1;
		if (!fast_backend_emit_stmt(emitter, deferred.stmt)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_emit_enter_scope(FastLeafProcEmitter *emitter, Scope *scope) {
	if (scope == nullptr) {
		return true;
	}
	FastScopeState state = {};
	state.scope = scope;
	state.defer_base = emitter->deferred_stmts.count;
	array_add(&emitter->scope_stack, state);
	return true;
}

gb_internal bool fast_backend_emit_leave_scope(FastLeafProcEmitter *emitter, Scope *scope) {
	if (scope == nullptr) {
		return true;
	}
	if (emitter->scope_stack.count == 0 || emitter->scope_stack[emitter->scope_stack.count-1].scope != scope) {
		return false;
	}
	FastScopeState state = emitter->scope_stack[emitter->scope_stack.count-1];
	if (!fast_backend_emit_scope_exit_defers(emitter, cast(i32)emitter->scope_stack.count-1)) {
		return false;
	}
	emitter->deferred_stmts.count = state.defer_base;
	emitter->scope_stack.count -= 1;
	return true;
}

gb_internal void fast_backend_emit_jump_if_zero(FastLeafProcEmitter *emitter, i32 label_index) {
	char label[64] = {};
	fast_backend_make_label_name(label, gb_size_of(label), emitter->plan, label_index);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tcmp %s, 0\n", fast_backend_x64_work_reg()->r64);
		gb_fprintf(emitter->file, "\tje %s\n", label);
	} else {
		gb_fprintf(emitter->file, "\tcmp %s, #0\n", fast_backend_arm64_work_reg());
		gb_fprintf(emitter->file, "\tb.eq %s\n", label);
	}
}

gb_internal void fast_backend_emit_jump_if_compare(FastLeafProcEmitter *emitter, TokenKind op, FastScalarType type, i32 true_label, i32 false_label) {
	char true_name[64] = {};
	char false_name[64] = {};
	fast_backend_make_label_name(true_name, gb_size_of(true_name), emitter->plan, true_label);
	fast_backend_make_label_name(false_name, gb_size_of(false_name), emitter->plan, false_label);

	if (build_context.metrics.arch == TargetArch_amd64) {
		char const *suffix = fast_backend_x64_cmp_suffix(op, type);
		GB_ASSERT(suffix != nullptr);
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_work_reg()->r64);
		gb_fprintf(emitter->file, "\tj%s %s\n", suffix, true_name);
		gb_fprintf(emitter->file, "\tjmp %s\n", false_name);
	} else {
		char const *suffix = fast_backend_arm64_cmp_suffix(op, type);
		GB_ASSERT(suffix != nullptr);
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
		gb_fprintf(emitter->file, "\tb.%s %s\n", suffix, true_name);
		gb_fprintf(emitter->file, "\tb %s\n", false_name);
	}
}

gb_internal bool fast_backend_emit_compare_exprs_branch(FastLeafProcEmitter *emitter, Ast *left, Ast *right, FastScalarType type, TokenKind op, i32 true_label, i32 false_label) {
	if (!fast_backend_emit_leaf_expr(emitter, left)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, right)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, op, type, true_label, false_label);
	return true;
}

gb_internal void fast_backend_emit_jump_if_values_not_equal(FastLeafProcEmitter *emitter, FastScalarType type, i32 mismatch_label) {
	char mismatch_name[64] = {};
	fast_backend_make_label_name(mismatch_name, gb_size_of(mismatch_name), emitter->plan, mismatch_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_work_reg()->r64);
		gb_fprintf(emitter->file, "\tjne %s\n", mismatch_name);
	} else {
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
		gb_fprintf(emitter->file, "\tb.ne %s\n", mismatch_name);
	}
	gb_unused(type);
}

gb_internal bool fast_backend_emit_compare_scalar_at_offset(FastLeafProcEmitter *emitter, i32 lhs_depth, i32 rhs_depth, i32 offset, FastScalarType type, i32 mismatch_label) {
	fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), type);
	}
	fast_backend_emit_push_work_reg(emitter);

	fast_backend_emit_load_work_from_spill_depth(emitter, rhs_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), type);
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_values_not_equal(emitter, type, mismatch_label);
	return true;
}

gb_internal void fast_backend_emit_store_work_to_slot(FastLeafProcEmitter *emitter, FastLocalSlot const &slot) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
	}
}

gb_internal void fast_backend_emit_store_work_to_global(FastLeafProcEmitter *emitter, Entity *entity, FastScalarType type) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_x64_tmp_reg());
		fast_backend_emit_x64_store_to_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), type);
	} else {
		fast_backend_emit_arm64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_arm64_tmp_reg());
		fast_backend_emit_arm64_store_to_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), type);
	}
}

gb_internal void fast_backend_emit_store_work_to_tmp_address(FastLeafProcEmitter *emitter, FastScalarType type) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_store_to_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), type);
	} else {
		fast_backend_emit_arm64_store_to_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), type);
	}
}

gb_internal void fast_backend_emit_store_tmp_to_work_address(FastLeafProcEmitter *emitter, FastScalarType type) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_store_to_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_tmp_reg(), type);
	} else {
		fast_backend_emit_arm64_store_to_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg(), type);
	}
}

gb_internal bool fast_backend_emit_store_to_scalar_storage(FastLeafProcEmitter *emitter, Entity *entity) {
	FastLocalSlot slot = {};
	FastScalarType scalar_type = {};
	bool is_global = false;
	if (!fast_backend_find_scalar_storage(emitter->plan, entity, &slot, &scalar_type, &is_global)) {
		return false;
	}

	if (is_global) {
		fast_backend_emit_store_work_to_global(emitter, entity, scalar_type);
	} else {
		fast_backend_emit_store_work_to_slot(emitter, slot);
	}
	return true;
}

gb_internal void fast_backend_emit_zero_slot(FastLeafProcEmitter *emitter, FastLocalSlot const &slot) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r32, fast_backend_x64_work_reg()->r32);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), slot.type);
		fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
	} else {
		gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), slot.type);
		fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
	}
}

gb_internal bool fast_backend_emit_zero_storage_entity(FastLeafProcEmitter *emitter, Entity *entity) {
	FastLocalSlot slot = {};
	Type *type = nullptr;
	bool is_global = false;
	if (!fast_backend_find_storage(emitter->plan, entity, &slot, &type, &is_global) || type == nullptr) {
		return false;
	}

	if (!slot.is_scalar || is_global) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));
		return true;
	}

	fast_backend_emit_zero_slot(emitter, slot);
	return true;
}

gb_internal bool fast_backend_emit_store_scalar_expr_to_entity_offset(FastLeafProcEmitter *emitter, Entity *entity, i32 offset, Type *type, Ast *expr) {
	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return false;
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
	return true;
}

gb_internal bool fast_backend_emit_store_scalar_expr_to_slot_offset(FastLeafProcEmitter *emitter, FastLocalSlot const &slot, i32 offset, Type *type, Ast *expr) {
	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_of_slot(emitter, slot)) {
		return false;
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
	return true;
}

gb_internal bool fast_backend_emit_store_scalar_expr_to_result_pointer_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr) {
	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
	return true;
}

gb_internal bool fast_backend_emit_store_scalar_expr_to_lhs_offset(FastLeafProcEmitter *emitter, Ast *lhs, i32 offset, Type *type, Ast *expr) {
	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
	return true;
}

gb_internal bool fast_backend_emit_store_scalar_expr_to_work_address_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr) {
	FastScalarType scalar_type = {};
	if (!fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, emitter->current_spill_depth-1);
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
	fast_backend_emit_pop_tmp_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
	}
	return true;
}

gb_internal bool fast_backend_emit_store_value_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = emitter->current_spill_depth;

	FastScalarType scalar_type = {};
	if (fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		if (!fast_backend_emit_leaf_expr(emitter, expr)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}

	type = default_type(type);
	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(type, expr)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}
	if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr, type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		if (!fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, &unparen_expr(expr)->CompoundLit)) {
			return false;
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, expr, type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		if (!fast_backend_emit_store_array_binary_expr_to_work_address(emitter, type, expr)) {
			return false;
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}
	if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, expr, type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		if (!fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, type, expr)) {
			return false;
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}

	if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr, type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr)->CallExpr)) {
			return false;
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}

	if (unparen_expr(expr)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(expr)->SliceExpr, type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		if (!fast_backend_emit_store_slice_expr_to_address(emitter, type, &unparen_expr(expr)->SliceExpr)) {
			return false;
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}

	Type *src_type = nullptr;
	bool src_is_scalar = false;
	if (fast_backend_can_emit_address_expr(emitter->plan, expr, &src_type, nullptr, &src_is_scalar) &&
	    !src_is_scalar &&
	    src_type != nullptr &&
	    are_types_identical(default_type(src_type), default_type(type))) {
		i32 size = cast(i32)type_size_of(type);
		if (size <= 0) {
			return false;
		}
		if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 src_depth = emitter->current_spill_depth;
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_load_tmp_from_spill_depth(emitter, src_depth);
		fast_backend_emit_copy_bytes_between_addresses(emitter, size);
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, type, expr)) {
		return false;
	}
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	return true;
}

gb_internal bool fast_backend_emit_store_value_to_work_address_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	fast_backend_emit_push_work_reg(emitter);
	i32 base_depth = emitter->current_spill_depth;
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	if (!fast_backend_emit_store_value_to_work_address(emitter, type, expr)) {
		return false;
	}
	fast_backend_emit_load_work_from_spill_depth(emitter, base_depth);
	return true;
}

gb_internal bool fast_backend_emit_store_value_to_entity_offset(FastLeafProcEmitter *emitter, Entity *entity, i32 offset, Type *type, Ast *expr) {
	if (emitter == nullptr || entity == nullptr || type == nullptr) {
		return false;
	}
	if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return false;
	}
	return fast_backend_emit_store_value_to_work_address_offset(emitter, offset, type, expr);
}

gb_internal bool fast_backend_emit_store_value_to_result_pointer_offset(FastLeafProcEmitter *emitter, i32 offset, Type *type, Ast *expr) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
	}
	return fast_backend_emit_store_value_to_work_address_offset(emitter, offset, type, expr);
}

gb_internal bool fast_backend_emit_store_value_to_lhs_offset(FastLeafProcEmitter *emitter, Ast *lhs, i32 offset, Type *type, Ast *expr) {
	if (emitter == nullptr || lhs == nullptr || type == nullptr) {
		return false;
	}
	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	return fast_backend_emit_store_value_to_work_address_offset(emitter, offset, type, expr);
}

gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_entity(FastLeafProcEmitter *emitter, Entity *entity, Type *type, AstCompoundLit *cl) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr || !fast_backend_emit_zero_storage_entity(emitter, entity)) {
		return false;
	}

	switch (base->kind) {
	case Type_Struct: {
		type_set_offsets(base);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				Entity *field = entity_of_node(fv->field);
				if (field == nullptr || field->kind != Entity_Variable) {
					return false;
				}
				Type *field_type = nullptr;
				i32 offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
				if (offset < 0 || field_type == nullptr) {
					return false;
				}
				if (!fast_backend_emit_store_value_to_entity_offset(emitter, entity, offset, field_type, fv->value)) {
					return false;
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			Type *field_type = nullptr;
			i32 offset = cast(i32)type_offset_of(base, i, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_emit_store_value_to_entity_offset(emitter, entity, offset, field_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Array: {
		Type *elem_type = base->Array.elem;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					i64 lo = exact_value_to_i64(type_and_value_of_expr(range->left).value);
					i64 hi = exact_value_to_i64(type_and_value_of_expr(range->right).value);
					if (range->op.kind != Token_RangeHalf) {
						hi += 1;
					}
					for (i64 index = lo; index < hi; index++) {
						if (!fast_backend_emit_store_value_to_entity_offset(emitter, entity, cast(i32)(index*elem_size), elem_type, fv->value)) {
							return false;
						}
					}
				} else {
					i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
					if (!fast_backend_emit_store_value_to_entity_offset(emitter, entity, cast(i32)(index*elem_size), elem_type, fv->value)) {
						return false;
					}
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			if (!fast_backend_emit_store_value_to_entity_offset(emitter, entity, i*elem_size, elem_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}
	}

	return false;
}

gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_work_address(FastLeafProcEmitter *emitter, Type *type, AstCompoundLit *cl) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}
	fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));

	switch (base->kind) {
	case Type_Struct: {
		type_set_offsets(base);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				Entity *field = entity_of_node(fv->field);
				if (field == nullptr || field->kind != Entity_Variable) {
					return false;
				}
				Type *field_type = nullptr;
				i32 offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
				if (offset < 0 || field_type == nullptr) {
					return false;
				}
				if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, field_type, fv->value)) {
					return false;
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			Type *field_type = nullptr;
			i32 offset = cast(i32)type_offset_of(base, i, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, field_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Array: {
		Type *elem_type = base->Array.elem;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					i64 lo = exact_value_to_i64(type_and_value_of_expr(range->left).value);
					i64 hi = exact_value_to_i64(type_and_value_of_expr(range->right).value);
					if (range->op.kind != Token_RangeHalf) {
						hi += 1;
					}
					for (i64 index = lo; index < hi; index++) {
						if (!fast_backend_emit_store_value_to_work_address_offset(emitter, cast(i32)(index*elem_size), elem_type, fv->value)) {
							return false;
						}
					}
				} else {
					i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
					if (!fast_backend_emit_store_value_to_work_address_offset(emitter, cast(i32)(index*elem_size), elem_type, fv->value)) {
						return false;
					}
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, i*elem_size, elem_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}
	}

	return false;
}

gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_result_pointer(FastLeafProcEmitter *emitter, Type *type, AstCompoundLit *cl) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));

	switch (base->kind) {
	case Type_Struct: {
		type_set_offsets(base);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				Entity *field = entity_of_node(fv->field);
				if (field == nullptr || field->kind != Entity_Variable) {
					return false;
				}
				Type *field_type = nullptr;
				i32 offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
				if (offset < 0 || field_type == nullptr) {
					return false;
				}
				if (!fast_backend_emit_store_value_to_result_pointer_offset(emitter, offset, field_type, fv->value)) {
					return false;
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			Type *field_type = nullptr;
			i32 offset = cast(i32)type_offset_of(base, i, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_emit_store_value_to_result_pointer_offset(emitter, offset, field_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Array: {
		Type *elem_type = base->Array.elem;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					i64 lo = exact_value_to_i64(type_and_value_of_expr(range->left).value);
					i64 hi = exact_value_to_i64(type_and_value_of_expr(range->right).value);
					if (range->op.kind != Token_RangeHalf) {
						hi += 1;
					}
					for (i64 index = lo; index < hi; index++) {
						if (!fast_backend_emit_store_value_to_result_pointer_offset(emitter, cast(i32)(index*elem_size), elem_type, fv->value)) {
							return false;
						}
					}
				} else {
					i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
					if (!fast_backend_emit_store_value_to_result_pointer_offset(emitter, cast(i32)(index*elem_size), elem_type, fv->value)) {
						return false;
					}
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			if (!fast_backend_emit_store_value_to_result_pointer_offset(emitter, i*elem_size, elem_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}
	}

	return false;
}

gb_internal bool fast_backend_emit_store_scalar_compound_lit_to_lhs(FastLeafProcEmitter *emitter, Ast *lhs, Type *type, AstCompoundLit *cl) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}
	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));

	switch (base->kind) {
	case Type_Struct: {
		type_set_offsets(base);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				Entity *field = entity_of_node(fv->field);
				if (field == nullptr || field->kind != Entity_Variable) {
					return false;
				}
				Type *field_type = nullptr;
				i32 offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
				if (offset < 0 || field_type == nullptr) {
					return false;
				}
				if (!fast_backend_emit_store_value_to_lhs_offset(emitter, lhs, offset, field_type, fv->value)) {
					return false;
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			Type *field_type = nullptr;
			i32 offset = cast(i32)type_offset_of(base, i, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_emit_store_value_to_lhs_offset(emitter, lhs, offset, field_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Array: {
		Type *elem_type = base->Array.elem;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		if (cl->elems.count == 0) {
			return true;
		}
		if (cl->elems[0]->kind == Ast_FieldValue) {
			for (Ast *elem : cl->elems) {
				ast_node(fv, FieldValue, elem);
				if (is_ast_range(fv->field)) {
					ast_node(range, BinaryExpr, fv->field);
					i64 lo = exact_value_to_i64(type_and_value_of_expr(range->left).value);
					i64 hi = exact_value_to_i64(type_and_value_of_expr(range->right).value);
					if (range->op.kind != Token_RangeHalf) {
						hi += 1;
					}
					for (i64 index = lo; index < hi; index++) {
						if (!fast_backend_emit_store_value_to_lhs_offset(emitter, lhs, cast(i32)(index*elem_size), elem_type, fv->value)) {
							return false;
						}
					}
				} else {
					i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
					if (!fast_backend_emit_store_value_to_lhs_offset(emitter, lhs, cast(i32)(index*elem_size), elem_type, fv->value)) {
						return false;
					}
				}
			}
			return true;
		}
		for_array(i, cl->elems) {
			if (!fast_backend_emit_store_value_to_lhs_offset(emitter, lhs, i*elem_size, elem_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}
	}

	return false;
}

gb_internal bool fast_backend_emit_store_string_literal_to_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr) {
	i32 blob_index = -1;
	i64 len = 0;
	if (emitter == nullptr || emitter->plan == nullptr || !fast_backend_get_string_literal_blob(emitter->plan->gen, type, expr, &blob_index, &len)) {
		return false;
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	fast_backend_emit_push_work_reg(emitter);
	if (blob_index < 0) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_work_reg()->r64);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
		}
	} else if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_address_of_blob(emitter->file, blob_index, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_address_of_blob(emitter->file, blob_index, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_store_work_to_tmp_address(emitter, pointer_type);

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)len);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)len);
	}
	fast_backend_emit_add_imm_to_tmp_reg(emitter, build_context.int_size);
	fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	return true;
}

gb_internal bool fast_backend_emit_store_slice_expr_to_address(FastLeafProcEmitter *emitter, Type *type, AstSliceExpr *se) {
	if (emitter == nullptr || se == nullptr || type == nullptr) {
		return false;
	}

	Type *result_type = default_type(type);
	Type *source_type = base_type(type_of_expr(se->expr));
	if (result_type == nullptr || source_type == nullptr) {
		return false;
	}

	i64 elem_size = 0;
	i64 source_const_len = -1;
	i64 source_len_offset = -1;
	bool source_uses_data_pointer = false;

	switch (source_type->kind) {
	case Type_Array:
		elem_size = type_size_of(source_type->Array.elem);
		source_const_len = source_type->Array.count;
		break;

	case Type_EnumeratedArray:
		elem_size = type_size_of(source_type->EnumeratedArray.elem);
		source_const_len = source_type->EnumeratedArray.count;
		break;

	case Type_Slice:
		elem_size = type_size_of(source_type->Slice.elem);
		source_len_offset = build_context.int_size;
		source_uses_data_pointer = true;
		break;

	case Type_DynamicArray:
		elem_size = type_size_of(source_type->DynamicArray.elem);
		source_len_offset = 1*build_context.int_size;
		source_uses_data_pointer = true;
		break;

	case Type_FixedCapacityDynamicArray:
		elem_size = type_size_of(source_type->FixedCapacityDynamicArray.elem);
		source_len_offset = type_offset_of(source_type, 1);
		break;

	case Type_Basic:
		if (is_type_string(source_type)) {
			elem_size = 1;
			source_len_offset = build_context.int_size;
			source_uses_data_pointer = true;
			break;
		}
		if (is_type_string16(source_type)) {
			elem_size = 2;
			source_len_offset = build_context.int_size;
			source_uses_data_pointer = true;
			break;
		}
		return false;

	default:
		return false;
	}

	if (elem_size <= 0) {
		return false;
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = spill_base + 1;

	if (!fast_backend_emit_address_expr(emitter, se->expr, nullptr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 src_depth = spill_base + 2;

	if (se->low != nullptr) {
		if (!fast_backend_emit_leaf_expr(emitter, se->low)) {
			return false;
		}
	} else if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r32, fast_backend_x64_work_reg()->r32);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 low_depth = spill_base + 3;

	if (se->high != nullptr) {
		if (!fast_backend_emit_leaf_expr(emitter, se->high)) {
			return false;
		}
	} else if (source_const_len >= 0) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)source_const_len);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)source_const_len);
		}
	} else {
		fast_backend_emit_load_work_from_spill_depth(emitter, src_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_len_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
		}
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 high_depth = spill_base + 4;

	fast_backend_emit_load_work_from_spill_depth(emitter, src_depth);
	if (source_uses_data_pointer) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = spill_base + 5;

	fast_backend_emit_load_work_from_spill_depth(emitter, low_depth);
	fast_backend_emit_scale_work_reg(emitter, elem_size);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_push_work_reg(emitter);
	i32 result_depth = spill_base + 6;

	fast_backend_emit_load_work_from_spill_depth(emitter, high_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, low_depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tsub %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), len_type);
	} else {
		gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), len_type);
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 len_depth = spill_base + 7;

	fast_backend_emit_load_tmp_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_load_work_from_spill_depth(emitter, result_depth);
	fast_backend_emit_store_work_to_tmp_address(emitter, pointer_type);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_add_imm_to_tmp_reg(emitter, build_context.int_size);
	fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	return true;
}

gb_internal bool fast_backend_emit_store_slice_compound_lit_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr) {
	if (emitter == nullptr || type == nullptr || !fast_backend_is_slice_compound_lit_expr(expr, type)) {
		return false;
	}

	type = default_type(type);
	Type *base = base_type(type);
	Type *elem_type = base->Slice.elem;
	ast_node(cl, CompoundLit, unparen_expr(expr));

	i64 count = 0;
	if (!fast_backend_slice_compound_lit_count(cl, &count)) {
		return false;
	}
	if (count == 0) {
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));
		return true;
	}

	FastLocalSlot backing_slot = {};
	if (!fast_backend_find_expr_slot(emitter->plan, expr, &backing_slot)) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = spill_base + 1;

	if (!fast_backend_emit_address_of_slot(emitter, backing_slot)) {
		return false;
	}
	fast_backend_emit_zero_bytes_at_work_address(emitter, backing_slot.size);

	i32 elem_size = cast(i32)type_size_of(elem_type);
	if (cl->elems.count != 0 && cl->elems[0]->kind == Ast_FieldValue) {
		for (Ast *elem : cl->elems) {
			ast_node(fv, FieldValue, elem);
			if (is_ast_range(fv->field)) {
				ast_node(range, BinaryExpr, fv->field);
				i64 lo = exact_value_to_i64(type_and_value_of_expr(range->left).value);
				i64 hi = exact_value_to_i64(type_and_value_of_expr(range->right).value);
				if (range->op.kind != Token_RangeHalf) {
					hi += 1;
				}
				for (i64 index = lo; index < hi; index++) {
					if (!fast_backend_emit_store_scalar_expr_to_slot_offset(emitter, backing_slot, cast(i32)(index*elem_size), elem_type, fv->value)) {
						return false;
					}
				}
			} else {
				i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
				if (!fast_backend_emit_store_scalar_expr_to_slot_offset(emitter, backing_slot, cast(i32)(index*elem_size), elem_type, fv->value)) {
					return false;
				}
			}
		}
	} else {
		for_array(i, cl->elems) {
			if (!fast_backend_emit_store_scalar_expr_to_slot_offset(emitter, backing_slot, i*elem_size, elem_type, cl->elems[i])) {
				return false;
			}
		}
	}

	if (!fast_backend_emit_address_of_slot(emitter, backing_slot)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = spill_base + 2;

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)count);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)count);
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 len_depth = spill_base + 3;

	fast_backend_emit_load_tmp_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_load_work_from_spill_depth(emitter, data_depth);
	fast_backend_emit_store_work_to_tmp_address(emitter, pointer_type);

	fast_backend_emit_load_tmp_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_add_imm_to_tmp_reg(emitter, build_context.int_size);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	return true;
}

gb_internal bool fast_backend_emit_store_array_binary_op_to_work_address(FastLeafProcEmitter *emitter, Type *type, TokenKind op, Ast *lhs, Ast *rhs) {
	if (emitter == nullptr || !fast_backend_can_emit_array_binary_operands(emitter->plan, type, op, lhs, rhs)) {
		return false;
	}

	type = default_type(type);
	Type *base = base_type(type);
	Type *elem_type = base->Array.elem;
	FastScalarType scalar_type = {};
	if (!fast_backend_classify_scalar_type(elem_type, &scalar_type)) {
		return false;
	}

	i32 elem_size = cast(i32)type_size_of(elem_type);
	i64 count = base->Array.count;
	if (elem_size <= 0 || count < 0) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = spill_base + 1;

	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 lhs_depth = spill_base + 2;

	if (!fast_backend_emit_address_expr(emitter, rhs, nullptr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 rhs_depth = spill_base + 3;

	for (i64 index = 0; index < count; index++) {
		i32 offset = cast(i32)(index*elem_size);

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_push_work_reg(emitter);

		fast_backend_emit_load_work_from_spill_depth(emitter, rhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_pop_tmp_reg(emitter);

		if (!fast_backend_emit_scalar_binary_op(emitter, op, scalar_type, nullptr)) {
			return false;
		}

		fast_backend_emit_load_tmp_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_add_imm_to_tmp_reg(emitter, offset);
		fast_backend_emit_store_work_to_tmp_address(emitter, scalar_type);
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	return true;
}

gb_internal bool fast_backend_emit_store_array_binary_expr_to_work_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr) {
	if (emitter == nullptr || !fast_backend_can_emit_array_binary_expr(emitter->plan, expr, type)) {
		return false;
	}
	expr = unparen_expr(expr);
	return fast_backend_emit_store_array_binary_op_to_work_address(emitter, type, expr->BinaryExpr.op.kind, expr->BinaryExpr.left, expr->BinaryExpr.right);
}

gb_internal bool fast_backend_emit_store_constant_aggregate_to_address(FastLeafProcEmitter *emitter, Type *type, Ast *expr) {
	if (fast_backend_expr_is_string_literal_value(type, expr)) {
		return fast_backend_emit_store_string_literal_to_address(emitter, type, expr);
	}

	i32 size = cast(i32)type_size_of(type);
	if (size <= 0) {
		return false;
	}

	auto bytes = gb_alloc_array(temporary_allocator(), u8, size);
	gb_zero_size(bytes, size);
	if (!fast_backend_serialize_constant_value(type, expr, bytes)) {
		return false;
	}
	fast_backend_emit_store_immediate_bytes(emitter, bytes, size);
	return true;
}

gb_internal bool fast_backend_emit_store_constant_aggregate_to_entity(FastLeafProcEmitter *emitter, Entity *entity, Type *type, Ast *expr) {
	if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return false;
	}
	return fast_backend_emit_store_constant_aggregate_to_address(emitter, type, expr);
}

gb_internal bool fast_backend_emit_copy_aggregate_expr_to_entity(FastLeafProcEmitter *emitter, Entity *dst_entity, Ast *src_expr, Type *type) {
	i32 size = cast(i32)type_size_of(type);
	Type *src_type = nullptr;
	bool src_is_scalar = false;
	if (!fast_backend_can_emit_address_expr(emitter->plan, src_expr, &src_type, nullptr, &src_is_scalar) || src_is_scalar) {
		return false;
	}
	if (src_type == nullptr || !are_types_identical(default_type(src_type), default_type(type))) {
		return false;
	}
	if (size <= 0) {
		return false;
	}

	if (!fast_backend_emit_address_expr(emitter, src_expr, nullptr)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_of_storage_entity(emitter, dst_entity)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_copy_bytes_between_addresses(emitter, size);
	return true;
}

gb_internal bool fast_backend_emit_store_value_to_entity(FastLeafProcEmitter *emitter, Entity *entity, Ast *expr) {
	Type *type = entity != nullptr ? entity->type : nullptr;
	if (type == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_expr_scalar_type(expr, type, &scalar_type)) {
		if (!fast_backend_emit_leaf_expr(emitter, expr)) {
			return false;
		}
		return fast_backend_emit_store_to_scalar_storage(emitter, entity);
	}

	type = default_type(type);
	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(type, expr)) {
		return fast_backend_emit_zero_storage_entity(emitter, entity);
	}

	if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, expr, type)) {
		return fast_backend_emit_store_scalar_compound_lit_to_entity(emitter, entity, type, &unparen_expr(expr)->CompoundLit);
	}
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_store_array_binary_expr_to_work_address(emitter, type, expr);
	}
	if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, type, expr);
	}

	if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, expr, type)) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(expr)->CallExpr);
	}

	if (unparen_expr(expr)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(expr)->SliceExpr, type)) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_store_slice_expr_to_address(emitter, type, &unparen_expr(expr)->SliceExpr);
	}

	Type *src_type = nullptr;
	bool src_is_scalar = false;
	if (fast_backend_can_emit_address_expr(emitter->plan, expr, &src_type, nullptr, &src_is_scalar) &&
	    !src_is_scalar &&
	    src_type != nullptr &&
	    are_types_identical(default_type(src_type), default_type(type))) {
		return fast_backend_emit_copy_aggregate_expr_to_entity(emitter, entity, expr, type);
	}

	return fast_backend_emit_store_constant_aggregate_to_entity(emitter, entity, type, expr);
}

gb_internal bool fast_backend_emit_store_value_to_lhs(FastLeafProcEmitter *emitter, Ast *lhs, Ast *rhs) {
	Type *type = reduce_tuple_to_single_type(type_of_expr(lhs));
	if (type == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_expr_scalar_type(rhs, type, &scalar_type)) {
		if (!fast_backend_emit_leaf_expr(emitter, rhs)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
		return true;
	}

	type = default_type(type);
	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}
	if (fast_backend_expr_is_zero_aggregate_value(type, rhs)) {
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(type));
		return true;
	}
	if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, rhs, type)) {
		return fast_backend_emit_store_scalar_compound_lit_to_lhs(emitter, lhs, type, &unparen_expr(rhs)->CompoundLit);
	}
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, rhs, type)) {
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		return fast_backend_emit_store_array_binary_expr_to_work_address(emitter, type, rhs);
	}
	if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, rhs, type)) {
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		return fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, type, rhs);
	}

	if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, rhs, type)) {
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		return fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(rhs)->CallExpr);
	}

	if (unparen_expr(rhs)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(rhs)->SliceExpr, type)) {
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		return fast_backend_emit_store_slice_expr_to_address(emitter, type, &unparen_expr(rhs)->SliceExpr);
	}

	Type *src_type = nullptr;
	bool src_is_scalar = false;
	if (fast_backend_can_emit_address_expr(emitter->plan, rhs, &src_type, nullptr, &src_is_scalar) &&
	    !src_is_scalar &&
	    src_type != nullptr &&
	    are_types_identical(default_type(src_type), default_type(type))) {
		if (!fast_backend_emit_address_expr(emitter, rhs, nullptr)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(type));
		return true;
	}

	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	return fast_backend_emit_store_constant_aggregate_to_address(emitter, type, rhs);
}

gb_internal bool fast_backend_emit_stmt(FastLeafProcEmitter *emitter, Ast *stmt);

gb_internal bool fast_backend_emit_stmt_list(FastLeafProcEmitter *emitter, Slice<Ast *> const &stmts) {
	for (Ast *stmt : stmts) {
		if (!fast_backend_emit_stmt(emitter, stmt)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_emit_value_decl(FastLeafProcEmitter *emitter, AstValueDecl *vd) {
	if (!vd->is_mutable) {
		return true;
	}

	for_array(i, vd->names) {
		Ast *name = vd->names[i];
		if (is_blank_ident(name)) {
			continue;
		}

		Entity *entity = entity_of_node(name);
		FastLocalSlot slot = {};
		if (entity == nullptr || !fast_backend_find_slot(emitter->plan, entity, &slot)) {
			return false;
		}

		if (vd->values.count == 0) {
			if (!fast_backend_emit_zero_storage_entity(emitter, entity)) {
				return false;
			}
			continue;
		}

		if (!fast_backend_emit_store_value_to_entity(emitter, entity, vd->values[i])) {
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_emit_assign_stmt(FastLeafProcEmitter *emitter, AstAssignStmt *as) {
	if (as->op.kind != Token_Eq) {
		TokenKind binary_op = Token_Invalid;
		if (as->lhs.count != 1 || as->rhs.count != 1 || !fast_backend_assign_op_to_binary(as->op.kind, &binary_op)) {
			return false;
		}

		Ast *lhs = unparen_expr(as->lhs[0]);
		Ast *rhs = as->rhs[0];
		Type *target_type = nullptr;
		if (!fast_backend_can_emit_address_expr(emitter->plan, lhs, &target_type, nullptr, nullptr)) {
			return false;
		}

		FastScalarType scalar_type = {};
		if (!fast_backend_expr_scalar_type(lhs, target_type, &scalar_type)) {
			if (!fast_backend_can_emit_array_binary_operands(emitter->plan, target_type, binary_op, lhs, rhs)) {
				return false;
			}
			if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
				return false;
			}
			return fast_backend_emit_store_array_binary_op_to_work_address(emitter, target_type, binary_op, lhs, rhs);
		}
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_leaf_expr(emitter, rhs)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		if (!fast_backend_emit_scalar_binary_op(emitter, binary_op, scalar_type, rhs)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_work_to_tmp_address(emitter, scalar_type);
		return true;
	}

	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
			Type *rhs_type = reduce_tuple_to_single_type(type_and_value_of_expr(as->rhs[i]).type);
			FastScalarType scalar_type = {};
			if (rhs_type != nullptr && fast_backend_expr_scalar_type(as->rhs[i], rhs_type, &scalar_type)) {
				if (!fast_backend_emit_leaf_expr(emitter, as->rhs[i])) {
					return false;
				}
			} else if (rhs_type != nullptr && fast_backend_can_emit_aggregate_call_expr(emitter->plan, as->rhs[i], rhs_type)) {
				i32 alloc_bytes = 0;
				if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(rhs_type), cast(i32)type_align_of(rhs_type), &alloc_bytes)) {
					return false;
				}
				if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(as->rhs[i])->CallExpr)) {
					return false;
				}
				fast_backend_emit_stack_adjust(emitter, alloc_bytes, false);
			} else if (!fast_backend_can_emit_aggregate_expr(emitter->plan, as->rhs[i], rhs_type)) {
				return false;
			}
			continue;
		}

		if (!fast_backend_can_emit_address_expr(emitter->plan, lhs, nullptr, nullptr, nullptr)) {
			return false;
		}
		if (!fast_backend_emit_store_value_to_lhs(emitter, lhs, as->rhs[i])) {
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_emit_defer_stmt(FastLeafProcEmitter *emitter, AstDeferStmt *ds) {
	if (emitter->scope_stack.count == 0) {
		return false;
	}
	FastDeferredStmt deferred = {};
	deferred.stmt = ds->stmt;
	array_add(&emitter->deferred_stmts, deferred);
	return true;
}

gb_internal bool fast_backend_emit_return_stmt(FastLeafProcEmitter *emitter, AstReturnStmt *rs) {
	if (!emitter->plan->return_by_pointer) {
		bool preserve_result = rs->results.count != 0 && emitter->deferred_stmts.count != 0;
		isize deferred_count = emitter->deferred_stmts.count;
		if (rs->results.count != 0) {
			if (!fast_backend_emit_leaf_expr(emitter, rs->results[0])) {
				return false;
			}
			if (preserve_result) {
				fast_backend_emit_push_work_reg(emitter);
			}
		}
		if (!fast_backend_emit_scope_exit_defers(emitter, 0)) {
			return false;
		}
		emitter->deferred_stmts.count = deferred_count;
		if (rs->results.count != 0) {
			if (preserve_result) {
				fast_backend_emit_pop_tmp_reg(emitter);
				if (build_context.metrics.arch == TargetArch_amd64) {
					gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_return_reg()->r64, fast_backend_x64_tmp_reg()->r64);
				} else {
					gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_return_reg(), fast_backend_arm64_tmp_reg());
				}
			} else if (build_context.metrics.arch == TargetArch_amd64) {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_return_reg()->r64, fast_backend_x64_work_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_return_reg(), fast_backend_arm64_work_reg());
			}
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, emitter->epilogue_label_index);
		return true;
	}

	if (rs->results.count != 0) {
		Ast *result = rs->results[0];
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
		}
		if (fast_backend_expr_is_zero_aggregate_value(emitter->plan->result_value_type, result)) {
			fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(emitter->plan->result_value_type));
		} else if (fast_backend_can_emit_aggregate_call_expr(emitter->plan, result, emitter->plan->result_value_type)) {
			if (!fast_backend_emit_call_expr_to_address(emitter, &unparen_expr(result)->CallExpr)) {
				return false;
			}
		} else if (fast_backend_can_emit_array_binary_expr(emitter->plan, result, emitter->plan->result_value_type)) {
			if (!fast_backend_emit_store_array_binary_expr_to_work_address(emitter, emitter->plan->result_value_type, result)) {
				return false;
			}
		} else if (fast_backend_can_emit_scalar_compound_lit_expr(emitter->plan, result, emitter->plan->result_value_type)) {
			if (!fast_backend_emit_store_scalar_compound_lit_to_result_pointer(emitter, emitter->plan->result_value_type, &unparen_expr(result)->CompoundLit)) {
				return false;
			}
		} else if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, result, emitter->plan->result_value_type)) {
			if (!fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, emitter->plan->result_value_type, result)) {
				return false;
			}
		} else if (unparen_expr(result)->kind == Ast_SliceExpr && fast_backend_can_emit_slice_expr(emitter->plan, &unparen_expr(result)->SliceExpr, emitter->plan->result_value_type)) {
			if (!fast_backend_emit_store_slice_expr_to_address(emitter, emitter->plan->result_value_type, &unparen_expr(result)->SliceExpr)) {
				return false;
			}
		} else {
			Type *src_type = nullptr;
			bool src_is_scalar = false;
			if (fast_backend_can_emit_address_expr(emitter->plan, result, &src_type, nullptr, &src_is_scalar) &&
			    !src_is_scalar &&
			    src_type != nullptr &&
			    are_types_identical(default_type(src_type), default_type(emitter->plan->result_value_type))) {
				fast_backend_emit_push_work_reg(emitter);
				if (!fast_backend_emit_address_expr(emitter, result, nullptr)) {
					return false;
				}
				fast_backend_emit_pop_tmp_reg(emitter);
				if (build_context.metrics.arch == TargetArch_amd64) {
					gb_fprintf(emitter->file, "\txchg %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
				} else {
					gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_addr_tmp_reg(), fast_backend_arm64_work_reg());
					gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
					gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_addr_tmp_reg());
				}
				fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(emitter->plan->result_value_type));
			} else if (!fast_backend_emit_store_constant_aggregate_to_address(emitter, emitter->plan->result_value_type, result)) {
				return false;
			}
		}
	}

	isize deferred_count = emitter->deferred_stmts.count;
	if (!fast_backend_emit_scope_exit_defers(emitter, 0)) {
		return false;
	}
	emitter->deferred_stmts.count = deferred_count;
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, emitter->epilogue_label_index);
	return true;
}

gb_internal bool fast_backend_emit_if_stmt(FastLeafProcEmitter *emitter, AstIfStmt *is) {
	if (!fast_backend_emit_enter_scope(emitter, is->scope)) {
		return false;
	}
	if (is->init != nullptr && !fast_backend_emit_stmt(emitter, is->init)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, is->cond)) {
		return false;
	}

	i32 false_label = fast_backend_alloc_label(emitter);
	i32 done_label = is->else_stmt != nullptr ? fast_backend_alloc_label(emitter) : false_label;
	fast_backend_emit_jump_if_zero(emitter, false_label);

	if (!fast_backend_emit_stmt(emitter, is->body)) {
		return false;
	}

	if (is->else_stmt != nullptr) {
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);
		fast_backend_emit_label(emitter->file, emitter->plan, false_label);
		if (!fast_backend_emit_stmt(emitter, is->else_stmt)) {
			return false;
		}
		fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	} else {
		fast_backend_emit_label(emitter->file, emitter->plan, false_label);
	}

	return fast_backend_emit_leave_scope(emitter, is->scope);
}

gb_internal bool fast_backend_emit_for_stmt(FastLeafProcEmitter *emitter, AstForStmt *fs) {
	i32 outer_scope_count = cast(i32)emitter->scope_stack.count;
	if (!fast_backend_emit_enter_scope(emitter, fs->scope)) {
		return false;
	}
	if (fs->init != nullptr && !fast_backend_emit_stmt(emitter, fs->init)) {
		return false;
	}
	i32 loop_scope_count = cast(i32)emitter->scope_stack.count;

	i32 loop_label = fast_backend_alloc_label(emitter);
	i32 post_label = fs->post != nullptr ? fast_backend_alloc_label(emitter) : loop_label;
	i32 done_label = fast_backend_alloc_label(emitter);

	FastControlContext ctx = {};
	ctx.label = fs->label;
	ctx.break_label = done_label;
	ctx.break_scope_count = outer_scope_count;
	ctx.continue_label = post_label;
	ctx.continue_scope_count = loop_scope_count;
	ctx.fallthrough_label = -1;
	ctx.fallthrough_scope_count = -1;
	array_add(&emitter->control_stack, ctx);
	defer (emitter->control_stack.count -= 1);

	fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
	if (fs->cond != nullptr) {
		if (!fast_backend_emit_leaf_expr(emitter, fs->cond)) {
			return false;
		}
		fast_backend_emit_jump_if_zero(emitter, done_label);
	}

	if (!fast_backend_emit_stmt(emitter, fs->body)) {
		return false;
	}

	if (fs->post != nullptr) {
		fast_backend_emit_label(emitter->file, emitter->plan, post_label);
		if (!fast_backend_emit_stmt(emitter, fs->post)) {
			return false;
		}
	}

	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return fast_backend_emit_leave_scope(emitter, fs->scope);
}

gb_internal bool fast_backend_emit_switch_stmt(FastLeafProcEmitter *emitter, AstSwitchStmt *ss) {
	i32 outer_scope_count = cast(i32)emitter->scope_stack.count;
	if (!fast_backend_emit_enter_scope(emitter, ss->scope)) {
		return false;
	}
	if (ss->init != nullptr && !fast_backend_emit_stmt(emitter, ss->init)) {
		return false;
	}
	if (ss->body == nullptr || ss->body->kind != Ast_BlockStmt) {
		return false;
	}

	Type *tag_type = t_bool;
	FastScalarType scalar_type = {};
	if (ss->tag != nullptr) {
		tag_type = reduce_tuple_to_single_type(type_and_value_of_expr(ss->tag).type);
	}
	i32 switch_scope_count = cast(i32)emitter->scope_stack.count;
	if (!fast_backend_expr_scalar_type(nullptr, tag_type, &scalar_type)) {
		return false;
	}

	auto body_labels = array_make<i32>(temporary_allocator(), 0, ss->body->BlockStmt.stmts.count);
	i32 default_label = -1;
	i32 done_label = fast_backend_alloc_label(emitter);

	for (Ast *stmt : ss->body->BlockStmt.stmts) {
		if (stmt == nullptr || stmt->kind != Ast_CaseClause) {
			return false;
		}
		i32 body_label = fast_backend_alloc_label(emitter);
		array_add(&body_labels, body_label);
		if (stmt->CaseClause.list.count == 0) {
			default_label = body_label;
		}
	}

	for_array(i, ss->body->BlockStmt.stmts) {
		Ast *stmt = ss->body->BlockStmt.stmts[i];
		AstCaseClause *cc = &stmt->CaseClause;
		if (cc->list.count == 0) {
			continue;
		}

		i32 body_label = body_labels[i];
		for_array(j, cc->list) {
			Ast *expr = unparen_expr(cc->list[j]);
			i32 next_label = fast_backend_alloc_label(emitter);

			if (ss->tag == nullptr) {
				if (!fast_backend_emit_leaf_expr(emitter, expr)) {
					return false;
				}
				fast_backend_emit_jump_if_zero(emitter, next_label);
				fast_backend_emit_jump_to_label(emitter->file, emitter->plan, body_label);
				fast_backend_emit_label(emitter->file, emitter->plan, next_label);
				continue;
			}

			if (is_ast_range(expr)) {
				Ast *lhs = expr->BinaryExpr.left;
				Ast *rhs = expr->BinaryExpr.right;
				i32 upper_check_label = fast_backend_alloc_label(emitter);
				TokenKind upper_op = Token_Invalid;
				switch (expr->BinaryExpr.op.kind) {
				case Token_Ellipsis:
				case Token_RangeFull:
					upper_op = Token_LtEq;
					break;
				case Token_RangeHalf:
					upper_op = Token_Lt;
					break;
				default:
					return false;
				}
				if (!fast_backend_emit_compare_exprs_branch(emitter, lhs, ss->tag, scalar_type, Token_LtEq, upper_check_label, next_label)) {
					return false;
				}
				fast_backend_emit_label(emitter->file, emitter->plan, upper_check_label);
				if (!fast_backend_emit_compare_exprs_branch(emitter, ss->tag, rhs, scalar_type, upper_op, body_label, next_label)) {
					return false;
				}
			} else {
				if (!fast_backend_emit_compare_exprs_branch(emitter, ss->tag, expr, scalar_type, Token_CmpEq, body_label, next_label)) {
					return false;
				}
			}

			fast_backend_emit_label(emitter->file, emitter->plan, next_label);
		}
	}

	if (default_label >= 0) {
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, default_label);
	} else {
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);
	}

	for_array(i, ss->body->BlockStmt.stmts) {
		Ast *stmt = ss->body->BlockStmt.stmts[i];
		AstCaseClause *cc = &stmt->CaseClause;
		fast_backend_emit_label(emitter->file, emitter->plan, body_labels[i]);
		FastControlContext ctx = {};
		ctx.label = ss->label;
		ctx.break_label = done_label;
		ctx.break_scope_count = outer_scope_count;
		ctx.continue_label = -1;
		ctx.continue_scope_count = -1;
		ctx.fallthrough_label = i+1 < body_labels.count ? body_labels[i+1] : done_label;
		ctx.fallthrough_scope_count = switch_scope_count;
		array_add(&emitter->control_stack, ctx);
		defer (emitter->control_stack.count -= 1);
		if (!fast_backend_emit_enter_scope(emitter, cc->scope)) {
			return false;
		}
		if (!fast_backend_emit_stmt_list(emitter, cc->stmts)) {
			return false;
		}
		if (!fast_backend_emit_leave_scope(emitter, cc->scope)) {
			return false;
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return fast_backend_emit_leave_scope(emitter, ss->scope);
}

gb_internal bool fast_backend_emit_branch_stmt(FastLeafProcEmitter *emitter, AstBranchStmt *bs) {
	FastControlContext *ctx = fast_backend_find_control_context(emitter, bs->label, bs->token.kind);
	if (ctx == nullptr) {
		return false;
	}

	switch (bs->token.kind) {
	case Token_break:
		{
			isize deferred_count = emitter->deferred_stmts.count;
			if (!fast_backend_emit_scope_exit_defers(emitter, ctx->break_scope_count)) {
				return false;
			}
			emitter->deferred_stmts.count = deferred_count;
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, ctx->break_label);
		return true;
	case Token_continue:
		{
			isize deferred_count = emitter->deferred_stmts.count;
			if (!fast_backend_emit_scope_exit_defers(emitter, ctx->continue_scope_count)) {
				return false;
			}
			emitter->deferred_stmts.count = deferred_count;
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, ctx->continue_label);
		return true;
	case Token_fallthrough:
		{
			isize deferred_count = emitter->deferred_stmts.count;
			if (!fast_backend_emit_scope_exit_defers(emitter, ctx->fallthrough_scope_count)) {
				return false;
			}
			emitter->deferred_stmts.count = deferred_count;
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, ctx->fallthrough_label);
		return true;
	}
	return false;
}

gb_internal bool fast_backend_emit_expr_stmt(FastLeafProcEmitter *emitter, AstExprStmt *es) {
	if (es->expr == nullptr || es->expr->kind != Ast_CallExpr) {
		return false;
	}
	return fast_backend_emit_call_expr(emitter, &es->expr->CallExpr);
}

gb_internal bool fast_backend_emit_stmt(FastLeafProcEmitter *emitter, Ast *stmt) {
	if (stmt == nullptr) {
		return true;
	}

	switch (stmt->kind) {
	case Ast_EmptyStmt:
		return true;
	case Ast_BlockStmt:
		if (!fast_backend_emit_enter_scope(emitter, stmt->BlockStmt.scope)) {
			return false;
		}
		if (!fast_backend_emit_stmt_list(emitter, stmt->BlockStmt.stmts)) {
			return false;
		}
		return fast_backend_emit_leave_scope(emitter, stmt->BlockStmt.scope);
	case Ast_ValueDecl:
		return fast_backend_emit_value_decl(emitter, &stmt->ValueDecl);
	case Ast_AssignStmt:
		return fast_backend_emit_assign_stmt(emitter, &stmt->AssignStmt);
	case Ast_DeferStmt:
		return fast_backend_emit_defer_stmt(emitter, &stmt->DeferStmt);
	case Ast_ExprStmt:
		return fast_backend_emit_expr_stmt(emitter, &stmt->ExprStmt);
	case Ast_IfStmt:
		return fast_backend_emit_if_stmt(emitter, &stmt->IfStmt);
	case Ast_ForStmt:
		return fast_backend_emit_for_stmt(emitter, &stmt->ForStmt);
	case Ast_SwitchStmt:
		return fast_backend_emit_switch_stmt(emitter, &stmt->SwitchStmt);
	case Ast_BranchStmt:
		return fast_backend_emit_branch_stmt(emitter, &stmt->BranchStmt);
	case Ast_ReturnStmt:
		return fast_backend_emit_return_stmt(emitter, &stmt->ReturnStmt);
	}

	return false;
}

gb_internal void fast_backend_emit_leaf_prologue(FastLeafProcEmitter *emitter) {
	if (!emitter->use_frame) {
		return;
	}

	i32 spill_bytes = 8 * emitter->plan->spill_depth;
	i32 slot_bytes = emitter->plan->local_stack_size;
	i32 frame_size = align_formula(spill_bytes + slot_bytes, 16);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tpush rbp\n");
		gb_fprintf(emitter->file, "\tmov rbp, rsp\n");
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tsub rsp, %d\n", frame_size);
		}
	} else {
		gb_fprintf(emitter->file, "\tstp x29, x30, [sp, #-16]!\n");
		gb_fprintf(emitter->file, "\tmov x29, sp\n");
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tsub sp, sp, #%d\n", frame_size);
		}
	}
}

gb_internal bool fast_backend_emit_param_spills(FastLeafProcEmitter *emitter) {
	i32 param_offset = emitter->plan->return_by_pointer ? 1 : 0;

	if (emitter->plan->return_by_pointer) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, 0);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
			fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
		} else {
			char const *src = fast_backend_arm64_param_reg(0);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), src);
			fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
		}
	}

	for_array(i, emitter->plan->params) {
		Entity *param = emitter->plan->params[i];
		FastLocalSlot slot = {};
		if (!fast_backend_find_slot(emitter->plan, param, &slot)) {
			return false;
		}

		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, param_offset + cast(i32)i);
			if (src == nullptr) {
				return false;
			}
			if (slot.is_scalar) {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
				fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), slot.type);
				fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
			} else {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_tmp_reg()->r64, src->r64);
				if (!fast_backend_emit_address_of_storage_entity(emitter, param)) {
					return false;
				}
				fast_backend_emit_copy_bytes_between_addresses(emitter, slot.size);
			}
		} else {
			char const *src = fast_backend_arm64_param_reg(param_offset + cast(i32)i);
			if (src == nullptr) {
				return false;
			}
			if (slot.is_scalar) {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), src);
				fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), slot.type);
				fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
			} else {
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_tmp_reg(), src);
				if (!fast_backend_emit_address_of_storage_entity(emitter, param)) {
					return false;
				}
				fast_backend_emit_copy_bytes_between_addresses(emitter, slot.size);
			}
		}
	}

	if (emitter->plan->has_context_slot) {
		i32 param_index = param_offset + cast(i32)emitter->plan->params.count;
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, param_index);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
			fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
		} else {
			char const *src = fast_backend_arm64_param_reg(param_index);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), src);
			fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
		}
	}
	return true;
}

gb_internal void fast_backend_emit_leaf_epilogue(FastLeafProcEmitter *emitter) {
	if (!emitter->use_frame) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tret\n");
		} else {
			gb_fprintf(emitter->file, "\tret\n");
		}
		return;
	}

	i32 spill_bytes = 8 * emitter->plan->spill_depth;
	i32 slot_bytes = emitter->plan->local_stack_size;
	i32 frame_size = align_formula(spill_bytes + slot_bytes, 16);
	if (build_context.metrics.arch == TargetArch_amd64) {
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tadd rsp, %d\n", frame_size);
		}
		gb_fprintf(emitter->file, "\tpop rbp\n");
		gb_fprintf(emitter->file, "\tret\n");
	} else {
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tadd sp, sp, #%d\n", frame_size);
		}
		gb_fprintf(emitter->file, "\tldp x29, x30, [sp], #16\n");
		gb_fprintf(emitter->file, "\tret\n");
	}
}

gb_internal bool fast_backend_emit_leaf_procedure(gbFile *file, FastLeafProcPlan *plan) {
	String symbol = fast_backend_get_entity_name(plan->entity);
	String asm_name = fast_backend_mangle_asm_name(symbol);

	if (fast_backend_allow_external_symbol(plan->entity)) {
		gb_fprintf(file, ".globl \"%.*s\"\n", LIT(asm_name));
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(file, ".p2align 4, 0x90\n");
	} else {
		gb_fprintf(file, ".p2align 2\n");
	}

	gb_fprintf(file, "\"%.*s\":\n", LIT(asm_name));

	FastLeafProcEmitter emitter = {};
	emitter.file = file;
	emitter.plan = plan;
	emitter.control_stack = array_make<FastControlContext>(heap_allocator(), 0, 4);
	emitter.deferred_stmts = array_make<FastDeferredStmt>(heap_allocator(), 0, 4);
	emitter.scope_stack = array_make<FastScopeState>(heap_allocator(), 0, 4);
	emitter.current_spill_depth = 0;
	emitter.epilogue_label_index = 0;
	emitter.next_label_index = 1;
	emitter.use_frame = plan->spill_depth > 0 || plan->local_stack_size > 0;
	if (build_context.metrics.arch == TargetArch_arm64 && plan->has_calls) {
		emitter.use_frame = true;
	}

	fast_backend_emit_leaf_prologue(&emitter);
	if (!fast_backend_emit_param_spills(&emitter)) {
		error(plan->entity->token, "Fast backend failed to spill procedure parameters");
		return false;
	}

	if (!fast_backend_emit_enter_scope(&emitter, plan->body->BlockStmt.scope)) {
		error(plan->entity->token, "Fast backend failed to initialize the procedure scope");
		return false;
	}
	if (!fast_backend_emit_stmt_list(&emitter, plan->body->BlockStmt.stmts)) {
		error(plan->entity->token, "Fast backend failed to emit the procedure body");
		return false;
	}
	if (!fast_backend_emit_leave_scope(&emitter, plan->body->BlockStmt.scope)) {
		error(plan->entity->token, "Fast backend failed to emit deferred procedure statements");
		return false;
	}

	fast_backend_emit_label(file, plan, emitter.epilogue_label_index);
	fast_backend_emit_leaf_epilogue(&emitter);
	gb_fprintf(file, "\n");
	return true;
}

gb_internal void fast_backend_emit_global_section(gbFile *file, Entity *entity) {
	if (entity->Variable.link_section.len > 0) {
		gb_fprintf(file, ".section %.*s\n", LIT(entity->Variable.link_section));
		return;
	}

	if (entity->Variable.is_rodata) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, ".section __TEXT,__const\n");
		} else if (build_context.metrics.os == TargetOs_windows) {
			gb_fprintf(file, ".section .rdata,\"dr\"\n");
		} else {
			gb_fprintf(file, ".section .rodata,\"a\",@progbits\n");
		}
		return;
	}

	if (build_context.metrics.os == TargetOs_darwin) {
		gb_fprintf(file, ".section __DATA,__data\n");
	} else {
		gb_fprintf(file, ".data\n");
	}
}

gb_internal void fast_backend_emit_readonly_section(gbFile *file) {
	if (build_context.metrics.os == TargetOs_darwin) {
		gb_fprintf(file, ".section __TEXT,__const\n");
	} else if (build_context.metrics.os == TargetOs_windows) {
		gb_fprintf(file, ".section .rdata,\"dr\"\n");
	} else {
		gb_fprintf(file, ".section .rodata,\"a\",@progbits\n");
	}
}

gb_internal void fast_backend_emit_global_init_value(gbFile *file, FastGlobalVarPlan const &plan) {
	GB_ASSERT(plan.init_data != nullptr);
	for (i32 offset = 0; offset < plan.size; ) {
		i32 count = gb_min(16, plan.size-offset);
		gb_fprintf(file, "\t.byte ");
		for (i32 i = 0; i < count; i++) {
			if (i != 0) {
				gb_fprintf(file, ", ");
			}
			gb_fprintf(file, "%u", cast(unsigned)plan.init_data[offset+i]);
		}
		gb_fprintf(file, "\n");
		offset += count;
	}
}

gb_internal bool fast_backend_emit_global_string_literal_init(gbFile *file, FastGlobalVarPlan const &plan) {
	i32 blob_index = -1;
	i64 len = 0;
	if (!fast_backend_get_string_literal_blob(plan.gen, plan.value_type, plan.direct_init_expr, &blob_index, &len)) {
		return false;
	}

	if (blob_index >= 0) {
		gb_fprintf(file, "\t.quad ");
		fast_backend_emit_blob_label(file, blob_index);
		gb_fprintf(file, "\n");
	} else {
		gb_fprintf(file, "\t.quad 0\n");
	}
	gb_fprintf(file, "\t.quad %lld\n", cast(long long)len);
	return true;
}

gb_internal bool fast_backend_emit_global_var(gbFile *file, FastGlobalVarPlan const &plan) {
	Entity *entity = plan.entity;
	String asm_name = fast_backend_mangle_asm_name(fast_backend_get_entity_name(entity));

	fast_backend_emit_global_section(file, entity);
	if (fast_backend_allow_external_symbol(entity)) {
		gb_fprintf(file, ".globl \"%.*s\"\n", LIT(asm_name));
	}
	if (build_context.metrics.os != TargetOs_darwin && build_context.metrics.os != TargetOs_windows) {
		gb_fprintf(file, ".type \"%.*s\",@object\n", LIT(asm_name));
	}
	if (plan.align > 0 && is_power_of_two(plan.align)) {
		gb_fprintf(file, ".p2align %u, 0x0\n", cast(unsigned)floor_log2(cast(u64)plan.align));
	}
	gb_fprintf(file, "\"%.*s\":\n", LIT(asm_name));
	if (plan.has_init_data) {
		fast_backend_emit_global_init_value(file, plan);
	} else if (plan.direct_init_expr != nullptr) {
		if (!fast_backend_emit_global_string_literal_init(file, plan)) {
			return false;
		}
	} else {
		gb_fprintf(file, "\t.zero %d\n", plan.size);
	}
	if (build_context.metrics.os != TargetOs_darwin && build_context.metrics.os != TargetOs_windows) {
		gb_fprintf(file, ".size \"%.*s\", %d\n", LIT(asm_name), plan.size);
	}
	gb_fprintf(file, "\n");
	return true;
}

gb_internal void fast_backend_emit_literal_blobs(gbFile *file, FastGenerator *gen) {
	if (gen == nullptr || gen->literal_blobs.count == 0) {
		return;
	}

	fast_backend_emit_readonly_section(file);
	for_array(i, gen->literal_blobs) {
		FastLiteralBlob const &blob = gen->literal_blobs[i];
		if (blob.align > 0 && is_power_of_two(blob.align)) {
			gb_fprintf(file, ".p2align %u, 0x0\n", cast(unsigned)floor_log2(cast(u64)blob.align));
		}
		fast_backend_emit_blob_label(file, cast(i32)i);
		gb_fprintf(file, ":\n");
		if (blob.size == 0) {
			gb_fprintf(file, "\n");
			continue;
		}
		for (i32 offset = 0; offset < blob.size; ) {
			i32 count = gb_min(16, blob.size-offset);
			gb_fprintf(file, "\t.byte ");
			for (i32 j = 0; j < count; j++) {
				if (j != 0) {
					gb_fprintf(file, ", ");
				}
				gb_fprintf(file, "%u", cast(unsigned)blob.data[offset+j]);
			}
			gb_fprintf(file, "\n");
			offset += count;
		}
		gb_fprintf(file, "\n");
	}
}

gb_internal bool fast_backend_write_object_assembly(FastGenerator *gen, Array<FastGlobalVarPlan> const &globals, Array<FastLeafProcPlan> const &procedures, String asm_path) {
	gb_unused(gen);

	gbFile file = {};
	gbFileError err = gb_file_open_mode(&file, gbFileMode_Write, cast(char const *)asm_path.text);
	if (err != gbFileError_None) {
		gb_printf_err("Failed to open fast backend assembly output: %.*s\n", LIT(asm_path));
		return false;
	}
	defer (gb_file_close(&file));

	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(&file, ".intel_syntax noprefix\n");
	}

	for (auto const &plan : globals) {
		if (!fast_backend_emit_global_var(&file, plan)) {
			return false;
		}
	}

	gb_fprintf(&file, ".text\n");

	for (auto const &plan : procedures) {
		FastLeafProcPlan *proc_plan = const_cast<FastLeafProcPlan *>(&plan);
		i32 total_param_count = cast(i32)proc_plan->params.count + (proc_plan->has_context_slot ? 1 : 0) + (proc_plan->return_by_pointer ? 1 : 0);
		if (total_param_count > fast_backend_param_limit(proc_plan)) {
			error(plan.entity->token, "Fast backend does not yet support this many parameters");
			return false;
		}
		if (!fast_backend_emit_leaf_procedure(&file, proc_plan)) {
			return false;
		}
	}

	fast_backend_emit_literal_blobs(&file, gen);
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

gb_internal bool fast_generate_code(FastGenerator *gen) {
	GB_ASSERT(gen != nullptr);

	auto globals = array_make<FastGlobalVarPlan>(heap_allocator(), 0, gen->info->definitions.count);
	auto procedures = array_make<FastLeafProcPlan>(heap_allocator(), 0, gen->info->definitions.count);
	if (!fast_backend_collect_program(gen, &globals, &procedures)) {
		return false;
	}

	String asm_path = concatenate_strings(permanent_allocator(), gen->output_base, str_lit(".fast.S"));
	String obj_path = {};
	if (build_context.build_mode == BuildMode_Object) {
		obj_path = path_to_string(permanent_allocator(), build_context.build_paths[BuildPath_Output]);
	} else {
		String obj_ext = infer_object_extension_from_build_context();
		obj_path = concatenate3_strings(permanent_allocator(), gen->output_base, str_lit("."), obj_ext);
	}

	if (!fast_backend_write_object_assembly(gen, globals, procedures, asm_path)) {
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
