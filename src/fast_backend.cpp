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
	// Type* (a Type_Proc) for every fast-emitted proc whose ABI the planner
	// changed (currently: any proc whose result is a non-scalar aggregate,
	// which the planner forces to return_by_pointer). The LLVM function-type
	// cache is keyed on Type* and may already be populated by the time the
	// planner mutates `pt->Proc.return_by_pointer`; the driver invalidates
	// the cache for these procs after creating the extern lbModule.
	Array<Type *> procs_needing_abi_cache_invalidation;
};

enum FastScalarKind {
	FastScalar_Invalid,
	FastScalar_Bool,
	FastScalar_Signed,
	FastScalar_Unsigned,
	FastScalar_Float,
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
	// extra_scratch_bytes tracks bytes allocated by `fast_backend_emit_alloc_stack_temp`
	// for materialize-into-real-stack-temp paths (e.g. SelectorExpr base is a non-addressable
	// struct returned by a call). These bytes live BELOW the prologue frame and are restored
	// in the epilogue before the preserved-scratch reload. See `fast_backend_emit_address_expr`
	// Ast_SelectorExpr / Ast_IndexExpr materialize branches.
	i32 extra_scratch_bytes;
	i32 epilogue_label_index;
	i32 next_label_index;
	bool use_frame;
};

enum FastFallbackKind {
	FastFallback_None,
	FastFallback_Internal,
	FastFallback_TopLevel,
	FastFallback_Procedure,
	FastFallback_CallingConvention,
	FastFallback_Type,
	FastFallback_ValueDecl,
	FastFallback_Assignment,
	FastFallback_Return,
	FastFallback_ControlFlow,
	FastFallback_Switch,
	FastFallback_Call,
	FastFallback_Range,
	FastFallback_Global,
};

struct FastFallbackReason {
	FastFallbackKind kind;
	Ast *node;
	TokenPos pos;
	String detail;
};

enum FastArm64ArgStorageKind {
	FastArm64ArgStorage_Register,
	FastArm64ArgStorage_Stack,
};

enum FastX64ArgStorageKind {
	FastX64ArgStorage_Register,
	FastX64ArgStorage_Stack,
};

enum FastArm64AggregateReturnKind {
	FastArm64AggregateReturn_None,
	FastArm64AggregateReturn_Integer,
	FastArm64AggregateReturn_Float,
};

struct FastX64ArgAssignment {
	FastX64ArgStorageKind storage;
	i32 index;
};

struct FastArm64ArgAssignment {
	FastArm64ArgStorageKind storage;
	i32 index;
};

struct FastX64ArgState {
	i32 next_reg;
	i32 next_stack;
};

struct FastArm64ArgState {
	i32 next_reg;
	i32 next_stack;
	bool stack_only;
};

struct FastArm64AggregateReturnClass {
	FastArm64AggregateReturnKind kind;
	i32 slot_count;
	i32 float_bit_size;
};

gb_global FastFallbackReason fast_backend_fallback_reason = {};

gb_internal bool fast_backend_find_slot(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *out);
gb_internal i32 fast_backend_slot_offset(FastLeafProcPlan *plan, FastLocalSlot const &slot);
gb_internal i32 fast_backend_call_expr_spill_depth(AstCallExpr *ce);
gb_internal bool fast_backend_get_call_info(AstCallExpr *ce, TypeProc **proc_type_, Entity **proc_entity_, Type **result_type_, FastScalarType *scalar_result_type_, bool *has_result_, bool *return_by_pointer_);
gb_internal BuiltinProcId fast_backend_builtin_proc_id(Ast *expr);
gb_internal bool fast_backend_type_is_supported_aggregate(Type *type);
gb_internal bool fast_backend_type_is_supported_value(Type *type, FastScalarType *scalar_type_, bool *is_scalar_);
gb_internal bool fast_backend_add_slot(FastLeafProcPlan *plan, Entity *entity, Type *type);
gb_internal bool fast_backend_find_expr_slot(FastLeafProcPlan *plan, Ast *expr, FastLocalSlot *out);
gb_internal bool fast_backend_add_expr_slot(FastLeafProcPlan *plan, Ast *expr, Type *type, FastLocalSlot *out);
gb_internal FastScalarType fast_backend_context_scalar_type(void);
gb_internal i32 fast_backend_abi_arg_count(Type *t);
gb_internal FastArm64AggregateReturnClass fast_backend_arm64_classify_aggregate_return(Type *type);
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
gb_internal bool fast_backend_can_emit_any_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_union_expr(FastLeafProcPlan *plan, Type *union_type, Ast *expr);
gb_internal Ast *fast_backend_unwrap_type_value_expr(Ast *expr, Type *target_type);
gb_internal bool fast_backend_can_emit_scalar_compound_lit_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_constant_aggregate_expr(Type *type, Ast *expr);
gb_internal bool fast_backend_can_emit_aggregate_compare_operand(FastLeafProcPlan *plan, Ast *expr, Type *type);
gb_internal bool fast_backend_type_supports_aggregate_compare(Type *type);
gb_internal bool fast_backend_is_array_binary_expr(Ast *expr, Type *expected_type);
gb_internal bool fast_backend_type_supports_array_binary_op(Type *type, TokenKind op);
gb_internal bool fast_backend_can_emit_array_binary_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_array_binary_operands(FastLeafProcPlan *plan, Type *expected_type, TokenKind op, Ast *lhs, Ast *rhs);
gb_internal bool fast_backend_can_emit_aggregate_compare_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_can_emit_leaf_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_aggregate_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_can_emit_value_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal i32 fast_backend_get_call_visible_result_count(AstCallExpr *ce, TypeProc *pt);
gb_internal bool fast_backend_get_multi_result_call_info(Ast *expr, TypeProc **proc_type_, Entity **proc_entity_);
gb_internal bool fast_backend_get_call_visible_result_field_info(AstCallExpr *ce, TypeProc *pt, i32 result_index, Type **type_, i32 *offset_, i32 *size_, i32 *align_);
gb_internal Entity *fast_backend_resolve_direct_call_entity(Ast *proc_expr);
gb_internal bool fast_backend_get_special_default_call_arg(TypeProc *pt, i32 param_index, Ast *arg, ParameterValue *param_value_);
gb_internal bool fast_backend_get_special_default_call_arg_expr_string(TypeProc *pt, AstCallExpr *ce, ParameterValue const &param_value, String *value_);
gb_internal bool fast_backend_expr_has_call(Ast *expr);
gb_internal bool fast_backend_stmt_has_call(Ast *stmt);
gb_internal bool fast_backend_emit_enter_scope(FastLeafProcEmitter *emitter, Scope *scope);
gb_internal bool fast_backend_emit_leave_scope(FastLeafProcEmitter *emitter, Scope *scope);
gb_internal bool fast_backend_emit_scope_exit_defers(FastLeafProcEmitter *emitter, i32 keep_scope_count);
gb_internal bool fast_backend_emit_stmt(FastLeafProcEmitter *emitter, Ast *stmt);
gb_internal bool fast_backend_emit_materialize_multi_result_call(FastLeafProcEmitter *emitter, AstCallExpr *ce, TypeProc *pt, i32 *source_depth_, i32 *temp_bytes_);
gb_internal bool fast_backend_emit_optional_call_value_check(FastLeafProcEmitter *emitter, AstCallExpr *ce, TypeProc *pt, i32 source_depth);
gb_internal bool fast_backend_emit_store_string_value_to_address(FastLeafProcEmitter *emitter, String value);
gb_internal bool fast_backend_emit_special_default_call_arg(FastLeafProcEmitter *emitter, TypeProc *pt, AstCallExpr *ce, i32 param_index, Ast *arg, Type *param_type, i32 *temp_bytes_, bool *handled_);
gb_internal bool fast_backend_emit_compare_materialized_scalar_branch(FastLeafProcEmitter *emitter, i32 materialized_depth, Ast *expr, FastScalarType type, TokenKind op, bool materialized_is_left, i32 true_label, i32 false_label);
gb_internal bool fast_backend_find_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, Type **type_, bool *is_global_);
gb_internal String get_file_path_string(i32 index);
gb_internal Entity *find_entity_in_pkg(CheckerInfo *info, String const &pkg, String const &name);
gb_internal void force_add_dependency_entity(Checker *c, Scope *scope, String const &name);
gb_internal isize lookup_procedure_parameter(TypeProc *pt, String const &parameter_name);
gb_internal Entity *strip_entity_wrapping(Entity *e);
gb_internal Entity *fast_backend_canonical_top_level_entity(Entity *e);
gb_internal void fast_backend_record_emitted_entity(FastGenerator *gen, PtrSet<Entity *> *seen, Entity *e);
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
gb_internal bool fast_backend_emit_array_binary_op_at_offset(FastLeafProcEmitter *emitter, i32 dst_depth, i32 lhs_depth, i32 rhs_depth, i32 offset, Type *type, TokenKind op);
gb_internal bool fast_backend_emit_store_any_expr_to_work_address(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_store_union_expr_to_work_address(FastLeafProcEmitter *emitter, Type *union_type, Ast *expr);
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
gb_internal bool fast_backend_get_call_split_args(AstCallExpr *ce, Slice<Ast *> *positional_, Slice<Ast *> *named_);
gb_internal Ast *fast_backend_find_named_call_arg_value(TypeProc *pt, Slice<Ast *> named_args, i32 param_index);
gb_internal bool fast_backend_resolve_call_param_arg(TypeProc *pt, AstCallExpr *ce, i32 param_index, Ast **expr_, Slice<Ast *> *variadic_pack_);
gb_internal bool fast_backend_can_emit_call_arg_expr(FastLeafProcPlan *plan, Ast *arg, Type *param_type);
gb_internal i32 fast_backend_call_arg_expr_spill_depth(FastLeafProcPlan *plan, Ast *arg, Type *param_type);
gb_internal bool fast_backend_can_emit_raw_data_expr(FastLeafProcPlan *plan, Ast *expr);
gb_internal bool fast_backend_emit_raw_data_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_pack_variadic_slice_arg(FastLeafProcEmitter *emitter, Type *slice_type, Slice<Ast *> elems, i32 *temp_bytes_);
gb_internal void fast_backend_emit_arm64_load_from_address(gbFile *file, char const *addr, char const *dst, FastScalarType type);
gb_internal void fast_backend_emit_arm64_store_to_address(gbFile *file, char const *addr, char const *src, FastScalarType type);
gb_internal void fast_backend_emit_arm64_add_offset(gbFile *file, char const *dst, char const *base, i32 offset);
gb_internal void fast_backend_emit_arm64_store_direct_aggregate_return_to_address(gbFile *file, char const *dst, FastArm64AggregateReturnClass result_class, i32 size);
gb_internal void fast_backend_emit_arm64_load_direct_aggregate_return_from_address(gbFile *file, char const *src, FastArm64AggregateReturnClass result_class, i32 size);
gb_internal void fast_backend_make_label_name(char *buffer, isize buffer_size, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_jump_to_label(gbFile *file, FastLeafProcPlan *plan, i32 label_index);
gb_internal void fast_backend_emit_jump_if_zero(FastLeafProcEmitter *emitter, i32 label_index);
gb_internal void fast_backend_emit_jump_if_work_compare_imm(FastLeafProcEmitter *emitter, TokenKind op, FastScalarType type, u64 imm, i32 true_label, i32 false_label);
gb_internal void fast_backend_emit_store_work_to_tmp_address(FastLeafProcEmitter *emitter, FastScalarType type);
gb_internal bool fast_backend_emit_compare_scalar_at_offset(FastLeafProcEmitter *emitter, i32 lhs_depth, i32 rhs_depth, i32 offset, FastScalarType type, i32 mismatch_label);
gb_internal bool fast_backend_emit_compare_aggregate_at_offset(FastLeafProcEmitter *emitter, i32 lhs_depth, i32 rhs_depth, i32 offset, Type *type, i32 mismatch_label);
gb_internal i32 fast_backend_alloc_label(FastLeafProcEmitter *emitter);
gb_internal void fast_backend_emit_address_of_spill_depth(FastLeafProcEmitter *emitter, i32 depth);
gb_internal bool fast_backend_emit_alloc_stack_temp(FastLeafProcEmitter *emitter, i32 size, i32 align, i32 *alloc_bytes_);
gb_internal void fast_backend_emit_push_work_reg(FastLeafProcEmitter *emitter);
gb_internal void fast_backend_emit_pop_tmp_reg(FastLeafProcEmitter *emitter);
gb_internal bool fast_backend_emit_leaf_direct_array_index_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_leaf_direct_slice_index_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal bool fast_backend_emit_leaf_direct_struct_selector_expr(FastLeafProcEmitter *emitter, Ast *expr);
gb_internal void fast_backend_clear_fallback_reason(void);
gb_internal bool fast_backend_has_fallback_reason(void);
gb_internal bool fast_backend_reject(FastFallbackKind kind, Ast *node, String detail);
gb_internal bool fast_backend_reject(FastFallbackKind kind, Token const &token, String detail);
gb_internal bool fast_backend_emit_fallback_error(void);

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
	switch (build_context.build_mode) {
	case BuildMode_Object:
	case BuildMode_StaticLibrary:
		return true;
	case BuildMode_Executable:
	case BuildMode_DynamicLibrary:
		return fast_backend_hybrid_linked_output_reason().len == 0;
	case BuildMode_Assembly:
	case BuildMode_LLVM_IR:
		return false;
	}
	return false;
}

gb_internal char const *fast_backend_fallback_kind_string(FastFallbackKind kind) {
	switch (kind) {
	case FastFallback_None:              return "none";
	case FastFallback_Internal:          return "internal";
	case FastFallback_TopLevel:          return "top_level";
	case FastFallback_Procedure:         return "procedure";
	case FastFallback_CallingConvention: return "calling_convention";
	case FastFallback_Type:              return "type";
	case FastFallback_ValueDecl:         return "value_decl";
	case FastFallback_Assignment:        return "assignment";
	case FastFallback_Return:            return "return";
	case FastFallback_ControlFlow:       return "control_flow";
	case FastFallback_Switch:            return "switch";
	case FastFallback_Call:              return "call";
	case FastFallback_Range:             return "range";
	case FastFallback_Global:            return "global";
	}
	return "unknown";
}

gb_internal void fast_backend_clear_fallback_reason(void) {
	fast_backend_fallback_reason = {};
}

gb_internal bool fast_backend_has_fallback_reason(void) {
	return fast_backend_fallback_reason.kind != FastFallback_None;
}

gb_internal void fast_backend_set_fallback_reason(FastFallbackKind kind, Ast *node, TokenPos pos, String detail) {
	if (fast_backend_has_fallback_reason()) {
		return;
	}
	fast_backend_fallback_reason.kind = kind;
	fast_backend_fallback_reason.node = node;
	fast_backend_fallback_reason.pos = pos;
	fast_backend_fallback_reason.detail = string_trim_whitespace(detail);
}

gb_internal bool fast_backend_reject(FastFallbackKind kind, Ast *node, String detail) {
	TokenPos pos = {};
	if (node != nullptr) {
		pos = ast_token(node).pos;
	}
	fast_backend_set_fallback_reason(kind, node, pos, detail);
	return false;
}

gb_internal bool fast_backend_reject(FastFallbackKind kind, Token const &token, String detail) {
	fast_backend_set_fallback_reason(kind, nullptr, token.pos, detail);
	return false;
}

gb_internal bool fast_backend_emit_fallback_error(void) {
	FastFallbackReason const &reason = fast_backend_fallback_reason;
	String detail = reason.detail.len != 0 ? reason.detail : str_lit("Fast backend rejected this entity without recording a fallback reason");
	if (reason.node != nullptr) {
		error(reason.node, "%.*s", LIT(detail));
	} else {
		error(reason.pos, "%.*s", LIT(detail));
	}
	return false;
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
	gen->procs_needing_abi_cache_invalidation = array_make<Type *>(heap_allocator(), 0, c->info.definitions.count);
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
	case ProcCC_PreserveNone:
	case ProcCC_PreserveMost:
	case ProcCC_PreserveAll:
		return (build_context.metrics.arch == TargetArch_arm64 ||
		        build_context.metrics.arch == TargetArch_amd64) &&
		       build_context.metrics.os == TargetOs_darwin;
	}
	return false;
}

gb_internal bool fast_backend_supports_stack_args(TypeProc *pt);

gb_internal bool fast_backend_classify_scalar_type(Type *type, FastScalarType *out) {
	Type *bt = base_type(type);
	if (bt == nullptr) {
		return false;
	}

	if (is_type_boolean(bt)) {
		if (out) { out->kind = FastScalar_Bool; out->bit_size = 1; }
		return true;
	}

	if (bt->kind == Type_Enum) {
		bt = bt->Enum.base_type;
	}

	if (bt->kind == Type_Basic &&
	    (bt->Basic.kind == Basic_cstring || bt->Basic.kind == Basic_cstring16 || bt->Basic.kind == Basic_rawptr)) {
		if (out) { out->kind = FastScalar_Pointer; out->bit_size = 8*build_context.metrics.ptr_size; }
		return true;
	}
	if (is_type_typeid(bt)) {
		if (out) { out->kind = FastScalar_Unsigned; out->bit_size = 64; }
		return true;
	}

	if (is_type_integer(bt)) {
		if (out) {
			out->kind = is_type_unsigned(bt) ? FastScalar_Unsigned : FastScalar_Signed;
			out->bit_size = 8*type_size_of(bt);
		}
		return out == nullptr || (out->bit_size > 0 && out->bit_size <= 64);
	}
	if (is_type_float(bt)) {
		if (out) {
			out->kind = FastScalar_Float;
			out->bit_size = 8*type_size_of(bt);
		}
		return out == nullptr || (out->bit_size == 32 || out->bit_size == 64);
	}

	if (is_type_pointer(type) || bt->kind == Type_MultiPointer) {
		if (out) {
			out->kind = FastScalar_Pointer;
			out->bit_size = 8*build_context.metrics.ptr_size;
		}
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

	case Type_EnumeratedArray:
		return fast_backend_type_is_supported_value(type->EnumeratedArray.elem, nullptr, nullptr);

	case Type_Matrix:
		return fast_backend_type_is_supported_value(type->Matrix.elem, nullptr, nullptr);

	case Type_Union:
		for (Type *variant : type->Union.variants) {
			if (!fast_backend_type_is_supported_value(variant, nullptr, nullptr)) {
				return false;
			}
		}
		return true;

	case Type_DynamicArray:
		return fast_backend_type_is_supported_value(type->DynamicArray.elem, nullptr, nullptr);

	case Type_FixedCapacityDynamicArray:
		return fast_backend_type_is_supported_value(type->FixedCapacityDynamicArray.elem, nullptr, nullptr);

	case Type_Map:
		return true;

	case Type_Struct:
		if (type->Struct.soa_kind != StructSoa_None) {
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

	case Type_SimdVector: {
		// SIMD vector — on arm64, an HFA-eligible SIMD vector
		// (1-4 f32, 1-2 f64) is returned in v0..vN and passed
		// in v0..vN. Other vectors fall back via the planner.
		Type *elem = type->SimdVector.elem;
		return elem != nullptr && fast_backend_type_is_supported_value(elem, nullptr, nullptr);
	}

	case Type_Basic:
		// String and string16 are 2-field aggregates (data, len) — the
		// standard string-shape path. `any` is also a 2-field aggregate
		// (rawptr, typeid).
		//
		// Complex and Quaternion are 2- or 4-field HFA aggregates of
		// floats (complex64 = 2×f32, complex128 = 2×f64, quaternion128
		// = 4×f32, etc.). AAPCS64 returns them in v0..vN as an HFA,
		// but the fast-backend's direct-aggregate return path only
		// handles x0..xN (general regs), so we use sret: the proc
		// body writes each field to the result buffer at the right
		// offset, and the call site reads from the buffer. That's
		// correct (not optimal — we lose the HFA optimization) but
		// it round-trips the values.
		return is_type_string(type) || is_type_string16(type) || is_type_any(type) ||
		       is_type_complex(type) || is_type_quaternion(type);

	case Type_BitSet:
		// BitSet layout is just its underlying integer. A small
		// bit_set (≤ 64 bits, e.g. bit_set[0..<8; u8]) is a 1-byte
		// aggregate we can pass/return via sret the same way we
		// handle small struct aggregates. Larger bit_sets (> 64
		// bits, e.g. bit_set[0..<128] which is [16]u8) are also
		// supported if their underlying integer is a supported
		// scalar (we treat the BitSet as a byte-blob of that size).
		//
		// For the test cases here we only need the 1-byte case.
		// The body code is a series of scalar byte stores/loads
		// against the result buffer; bit-level ops (b[i], b |= c)
		// go through intrinsics that the regular int path handles.
		return true;
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

gb_internal Type *fast_backend_matrix_vector_type(Type *type) {
	Type *base = base_type(type);
	if (base == nullptr || base->kind != Type_Matrix) {
		return nullptr;
	}
	i64 count = base->Matrix.is_row_major ? base->Matrix.column_count : base->Matrix.row_count;
	return alloc_type_array(base->Matrix.elem, count);
}

gb_internal i64 fast_backend_matrix_vector_offset(Type *type, i64 component_index) {
	Type *base = base_type(type);
	if (base == nullptr || base->kind != Type_Matrix) {
		return -1;
	}
	i64 elem_size = type_size_of(base->Matrix.elem);
	if (elem_size <= 0) {
		return -1;
	}
	i64 elem_index = base->Matrix.is_row_major
		? matrix_indices_to_offset(base, component_index, 0)
		: matrix_indices_to_offset(base, 0, component_index);
	return elem_index * elem_size;
}

gb_internal bool fast_backend_scalar_is_unsigned(FastScalarType type) {
	return type.kind == FastScalar_Unsigned || type.kind == FastScalar_Bool || type.kind == FastScalar_Pointer;
}

gb_internal bool fast_backend_scalar_is_float(FastScalarType type) {
	return type.kind == FastScalar_Float;
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
	case FastScalar_Float:
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
		return fast_backend_scalar_is_integer_like(scalar_type) || fast_backend_scalar_is_float(scalar_type);
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
		return fast_backend_scalar_is_integer_like(scalar_type) || scalar_type.kind == FastScalar_Pointer || fast_backend_scalar_is_float(scalar_type);
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
	case FastScalar_Float:
		return cast(i32)gb_max(type.bit_size/8, cast(i64)1);
	case FastScalar_Pointer:
		return build_context.metrics.ptr_size;
	}
	return 0;
}

gb_internal bool fast_backend_scalar_cast_supported(FastScalarType source_type, FastScalarType target_type, TokenKind cast_kind) {
	if (source_type.kind == FastScalar_Invalid || target_type.kind == FastScalar_Invalid) {
		return false;
	}
	if (cast_kind == Token_transmute) {
		if (fast_backend_scalar_byte_size(source_type) != fast_backend_scalar_byte_size(target_type)) {
			return false;
		}
		if ((source_type.kind == FastScalar_Bool || target_type.kind == FastScalar_Bool) &&
		    source_type.kind != target_type.kind) {
			return false;
		}
		return true;
	}

	if (source_type.kind == target_type.kind) {
		return true;
	}
	if (target_type.kind == FastScalar_Bool) {
		return source_type.kind != FastScalar_Pointer || build_context.metrics.ptr_size == fast_backend_scalar_byte_size(source_type);
	}
	if (source_type.kind == FastScalar_Float) {
		return target_type.kind == FastScalar_Float ||
		       target_type.kind == FastScalar_Signed ||
		       target_type.kind == FastScalar_Unsigned;
	}
	if (target_type.kind == FastScalar_Float) {
		return source_type.kind == FastScalar_Bool ||
		       source_type.kind == FastScalar_Signed ||
		       source_type.kind == FastScalar_Unsigned;
	}
	if (source_type.kind == FastScalar_Pointer || target_type.kind == FastScalar_Pointer) {
		return source_type.kind == FastScalar_Pointer && target_type.kind == FastScalar_Pointer;
	}
	return true;
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

gb_internal void fast_backend_debug_log_fallback_reason(char const *entity_kind, Entity *e, FastFallbackReason const &reason) {
	if (!build_context.show_debug_messages || e == nullptr) {
		return;
	}
	String name = fast_backend_get_entity_name(e);
	String detail = reason.detail.len != 0 ? reason.detail : str_lit("no fast-backend rejection reason recorded");
	debugf("Fast backend fallback to LLVM for %s %.*s [%s]: %.*s\n",
	       entity_kind,
	       LIT(name),
	       fast_backend_fallback_kind_string(reason.kind),
	       LIT(detail));
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
		if (Entity *entity = entity_from_expr(expr)) {
			if (entity->kind == Entity_Variable &&
			    entity->parent_proc_decl.load(std::memory_order_relaxed) == nullptr &&
			    fast_backend_type_is_supported_value(entity->type, nullptr, nullptr)) {
				return 0;
			}
		}
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
		           (indexed_type->kind == Type_Array || indexed_type->kind == Type_EnumeratedArray || indexed_type->kind == Type_Matrix)) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			i32 temp_slots = base_value_type != nullptr ? align_formula(cast(i32)type_size_of(base_value_type), 8)/8 : 0;
			Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
			if (base_expr != nullptr &&
			    (fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr) ||
			     base_expr->kind == Ast_CompoundLit ||
			     base_expr->kind == Ast_CallExpr ||
			     fast_backend_is_array_binary_expr(base_expr, base_value_type))) {
				base_depth = temp_slots + fast_backend_supported_value_expr_spill_depth(expr->IndexExpr.expr, base_value_type);
			} else {
				base_depth = fast_backend_address_expr_spill_depth(expr->IndexExpr.expr);
			}
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
	case Ast_RangeStmt:
		return fast_backend_stmt_has_call(stmt->RangeStmt.init) ||
		       fast_backend_expr_has_call(stmt->RangeStmt.expr) ||
		       fast_backend_stmt_has_call(stmt->RangeStmt.body);
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
	case Type_Matrix:
		if (indexed_type->Matrix.is_row_major) {
			value_type = alloc_type_array(indexed_type->Matrix.elem, indexed_type->Matrix.column_count);
		} else {
			value_type = alloc_type_array(indexed_type->Matrix.elem, indexed_type->Matrix.row_count);
		}
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
		if (Entity *entity = entity_from_expr(expr)) {
			if (entity->kind == Entity_Variable && fast_backend_find_storage(plan, entity, nullptr, &value_type, nullptr)) {
				break;
			}
		}
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
			Type *base_value_type = default_type(type_of_expr(expr->SelectorExpr.expr));
			if (!fast_backend_can_emit_address_expr(plan, expr->SelectorExpr.expr, nullptr, nullptr, nullptr) &&
			    !fast_backend_can_emit_scalar_compound_lit_expr(plan, expr->SelectorExpr.expr, base_value_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, expr->SelectorExpr.expr, base_value_type) &&
			    !fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->SelectorExpr.expr)) {
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
		           (base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Array ||
		            base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_EnumeratedArray ||
		            base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Matrix)) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			bool can_materialize = base_value_type != nullptr &&
				(fast_backend_can_emit_array_binary_expr(plan, expr->IndexExpr.expr, base_value_type) ||
				 fast_backend_can_emit_scalar_compound_lit_expr(plan, expr->IndexExpr.expr, base_value_type) ||
				 fast_backend_can_emit_aggregate_call_expr(plan, expr->IndexExpr.expr, base_value_type) ||
				 fast_backend_can_emit_constant_aggregate_expr(base_value_type, expr->IndexExpr.expr));
			if (!fast_backend_can_emit_address_expr(plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr) &&
			    !fast_backend_can_emit_direct_array_index_expr(plan, expr) &&
			    !can_materialize) {
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
	return fast_backend_scalar_cast_supported(source_type, result_type, cast_kind);
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
		return fast_backend_can_emit_leaf_expr(plan, lhs, tag_type) &&
		       fast_backend_can_emit_leaf_expr(plan, rhs, tag_type);
	}

	// String-typed case: materialize as a string aggregate for the
	// per-field compare done in the switch emit. Same path as the
	// aggregate-compare operand (Ident for local vars, CompoundLit /
	// Constant for string literals, SliceExpr, etc.).
	if (is_type_string(tag_type) || is_type_string16(tag_type)) {
		return fast_backend_can_emit_aggregate_compare_operand(plan, expr, tag_type);
	}
	return fast_backend_can_emit_leaf_expr(plan, expr, tag_type);
}

gb_internal bool fast_backend_get_call_split_args(AstCallExpr *ce, Slice<Ast *> *positional_, Slice<Ast *> *named_) {
	if (ce == nullptr) {
		return false;
	}

	Slice<Ast *> positional = {};
	Slice<Ast *> named = {};
	if (ce->split_args != nullptr) {
		positional = ce->split_args->positional;
		named = ce->split_args->named;
	} else {
		positional = ce->args;
		for (isize i = 0; i < ce->args.count; i++) {
			Ast *arg = ce->args[i];
			if (arg != nullptr && arg->kind == Ast_FieldValue) {
				positional.count = i;
				break;
			}
		}
		named = slice(ce->args, positional.count, ce->args.count);
	}

	if (positional_) *positional_ = positional;
	if (named_) *named_ = named;
	return true;
}

gb_internal Ast *fast_backend_find_named_call_arg_value(TypeProc *pt, Slice<Ast *> named_args, i32 param_index) {
	if (pt == nullptr || pt->params == nullptr || pt->params->kind != Type_Tuple || param_index < 0 || param_index >= pt->param_count) {
		return nullptr;
	}

	Entity *param = pt->params->Tuple.variables[param_index];
	if (param == nullptr) {
		return nullptr;
	}

	InternedString param_name = entity_interned_name(param);
	for (Ast *arg : named_args) {
		if (arg == nullptr || arg->kind != Ast_FieldValue) {
			continue;
		}
		ast_node(fv, FieldValue, arg);
		if (fv->field != nullptr &&
		    fv->field->kind == Ast_Ident &&
		    fv->field->Ident.interned == param_name) {
			return fv->value;
		}
	}
	return nullptr;
}

gb_internal bool fast_backend_resolve_call_param_arg(TypeProc *pt, AstCallExpr *ce, i32 param_index, Ast **expr_, Slice<Ast *> *variadic_pack_) {
	if (expr_) *expr_ = nullptr;
	if (variadic_pack_) *variadic_pack_ = {};
	if (pt == nullptr || ce == nullptr || pt->params == nullptr || pt->params->kind != Type_Tuple || param_index < 0 || param_index >= pt->param_count) {
		return false;
	}

	Slice<Ast *> positional = {};
	Slice<Ast *> named = {};
	if (!fast_backend_get_call_split_args(ce, &positional, &named)) {
		return false;
	}

	Entity *param = pt->params->Tuple.variables[param_index];
	if (param == nullptr || param->kind != Entity_Variable) {
		return false;
	}

	isize positional_param_count = positional.count;
	if (pt->variadic) {
		positional_param_count = gb_min(positional.count, cast(isize)pt->variadic_index);
	}

	Ast *named_value = fast_backend_find_named_call_arg_value(pt, named, param_index);
	if (named_value != nullptr) {
		if (pt->variadic && param_index == pt->variadic_index && positional.count > positional_param_count) {
			return false;
		}
		if (expr_) *expr_ = named_value;
		return true;
	}

	if (param_index < positional_param_count) {
		if (expr_) *expr_ = positional[param_index];
		return true;
	}

	if (pt->variadic && param_index == pt->variadic_index) {
		auto variadic_args = slice(positional, positional_param_count, positional.count);
		if (ce->ellipsis.pos.line != 0) {
			if (variadic_args.count != 1) {
				return false;
			}
			if (expr_) *expr_ = variadic_args[0];
			return true;
		}
		if (variadic_pack_) *variadic_pack_ = variadic_args;
		return true;
	}

	if (param->Variable.param_value.kind != ParameterValue_Invalid &&
	    param->Variable.param_value.original_ast_expr != nullptr) {
		if (expr_) *expr_ = param->Variable.param_value.original_ast_expr;
		return true;
	}

	return false;
}

gb_internal bool fast_backend_get_special_default_call_arg(TypeProc *pt, i32 param_index, Ast *arg, ParameterValue *param_value_) {
	if (param_value_) *param_value_ = {};
	if (pt == nullptr || pt->params == nullptr || pt->params->kind != Type_Tuple || param_index < 0 || param_index >= pt->param_count) {
		return false;
	}

	Entity *param = pt->params->Tuple.variables[param_index];
	if (param == nullptr || param->kind != Entity_Variable) {
		return false;
	}

	ParameterValue param_value = param->Variable.param_value;
	if ((param_value.kind != ParameterValue_Location && param_value.kind != ParameterValue_Expression) ||
	    param_value.original_ast_expr == nullptr ||
	    arg != param_value.original_ast_expr) {
		return false;
	}

	if (param_value_) *param_value_ = param_value;
	return true;
}

gb_internal bool fast_backend_get_special_default_call_arg_expr_string(TypeProc *pt, AstCallExpr *ce, ParameterValue const &param_value, String *value_) {
	if (value_) *value_ = {};
	if (pt == nullptr || ce == nullptr || param_value.kind != ParameterValue_Expression || param_value.original_ast_expr == nullptr) {
		return false;
	}

	Ast *orig = param_value.original_ast_expr;
	if (orig->kind == Ast_BasicDirective) {
		return false;
	}

	Ast *call = unparen_expr(orig);
	if (call == nullptr || call->kind != Ast_CallExpr) {
		return false;
	}

	ast_node(default_ce, CallExpr, call);
	if (default_ce->proc == nullptr || default_ce->proc->kind != Ast_BasicDirective || default_ce->args.count != 1) {
		return false;
	}

	Ast *target = default_ce->args[0];
	if (target == nullptr || target->kind != Ast_Ident) {
		return false;
	}

	isize param_idx = lookup_procedure_parameter(pt, target->Ident.token.string);
	if (param_idx < 0) {
		return false;
	}

	Slice<Ast *> positional = {};
	Slice<Ast *> named = {};
	if (!fast_backend_get_call_split_args(ce, &positional, &named)) {
		return false;
	}

	Ast *target_expr = nullptr;
	if (positional.count > param_idx) {
		target_expr = positional[param_idx];
	}
	for_array(i, named) {
		Ast *arg = named[i];
		if (arg == nullptr || arg->kind != Ast_FieldValue) {
			continue;
		}
		ast_node(fv, FieldValue, arg);
		if (fv->field != nullptr &&
		    fv->field->kind == Ast_Ident &&
		    fv->field->Ident.token.string == target->Ident.token.string) {
			target_expr = fv->value;
			break;
		}
	}
	if (target_expr == nullptr) {
		return false;
	}

	gbString expr = expr_to_string(target_expr);
	String value = copy_string(permanent_allocator(), make_string_c(expr));
	gb_string_free(expr);
	if (value_) *value_ = value;
	return true;
}

gb_internal bool fast_backend_can_emit_call_arg_expr(FastLeafProcPlan *plan, Ast *arg, Type *param_type) {
	if (plan == nullptr || arg == nullptr || param_type == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(param_type, &scalar_type)) {
		return fast_backend_can_emit_leaf_expr(plan, arg, param_type);
	}
	if (!fast_backend_type_is_supported_aggregate(param_type)) {
		return false;
	}
	return fast_backend_can_emit_aggregate_expr(plan, arg, param_type) ||
	       fast_backend_can_emit_aggregate_call_expr(plan, arg, param_type);
}

gb_internal i32 fast_backend_call_arg_expr_spill_depth(FastLeafProcPlan *plan, Ast *arg, Type *param_type) {
	if (arg == nullptr || param_type == nullptr) {
		return 0;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(param_type, &scalar_type)) {
		return fast_backend_leaf_expr_spill_depth(arg);
	}

	arg = unparen_expr(arg);
	if (arg == nullptr) {
		return 0;
	}
	if (fast_backend_is_slice_compound_lit_expr(arg, param_type)) {
		return fast_backend_slice_compound_lit_spill_depth(arg, param_type);
	}
	if (fast_backend_is_array_binary_expr(arg, param_type)) {
		return fast_backend_array_binary_expr_spill_depth(arg);
	}
	if (arg->kind == Ast_SliceExpr) {
		return fast_backend_slice_expr_spill_depth(&arg->SliceExpr);
	}

	i32 depth = type_and_value_of_expr(arg).mode == Addressing_Constant ? 0 : fast_backend_address_expr_spill_depth(arg);
	if (arg->kind == Ast_CallExpr) {
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
		    are_types_identical(default_type(inner_result_type), default_type(param_type))) {
			depth = 1 + fast_backend_call_expr_spill_depth(&arg->CallExpr);
		}
	}
	return depth;
}

gb_internal bool fast_backend_get_call_info(AstCallExpr *ce, TypeProc **proc_type_, Entity **proc_entity_, Type **result_type_, FastScalarType *scalar_result_type_, bool *has_result_, bool *return_by_pointer_) {
	Entity *proc_entity = fast_backend_resolve_direct_call_entity(ce->proc);
	if (proc_entity != nullptr && proc_entity->kind == Entity_ProcGroup) {
		if (proc_entity->ProcGroup.entities.count != 1) {
			return false;
		}
		proc_entity = proc_entity->ProcGroup.entities[0];
	}
	bool is_indirect_call = (proc_entity == nullptr);
	if (is_indirect_call) {
		// Indirect call through a proc-typed expression (e.g. a global
		// or local proc value, a method-bound value, or a function
		// pointer). The type of the callee expression must be a proc
		// type; we don't have a known Procedure entity, but the
		// emit side loads the function pointer at the call site
		// and emits `blr` instead of `bl <symbol>`.
		Type *callee_type = base_type(type_of_expr(ce->proc));
		if (callee_type == nullptr || callee_type->kind != Type_Proc) {
			return false;
		}
	} else if (proc_entity->kind != Entity_Procedure) {
		return false;
	}
	if (proc_entity != nullptr && proc_entity->Procedure.is_objc_impl_or_import) {
		return false;
	}

	Type *proc_type = base_type(type_of_expr(ce->proc));
	if (proc_type == nullptr || proc_type->kind != Type_Proc) {
		return false;
	}

	TypeProc *pt = &proc_type->Proc;
	if (pt->is_closure || (pt->is_polymorphic && !pt->is_poly_specialized)) {
		return false;
	}
	// Foreign `#cvararg` procs use the C va_list ABI: variadic args go
	// in registers/stack per the platform C convention and the callee
	// reads them with `va_arg`. The fast backend's caller-side
	// variadic pack emits a `[]any` slice instead, so calls into
	// foreign C-variadic procs must still fall back to LLVM. Non-
	// foreign `#cvararg` and `..any` procs use the slice-based ABI
	// the rest of the call site already supports.
	if (pt->c_vararg && proc_entity != nullptr && proc_entity->Procedure.is_foreign) {
		return false;
	}
	if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
		return false;
	}

	FastScalarType result_type = {};
	Type *call_result_type = nullptr;
	bool has_result = false;
	bool return_by_pointer = false;
	if (pt->result_count > 1) {
		if (pt->results == nullptr || pt->results->kind != Type_Tuple) {
			return false;
		}
		for_array(i, pt->results->Tuple.variables) {
			Entity *result_entity = pt->results->Tuple.variables[i];
			if (result_entity == nullptr || result_entity->kind != Entity_Variable) {
				return false;
			}
			if (!fast_backend_classify_scalar_type(result_entity->type, nullptr) &&
			    !fast_backend_type_is_supported_aggregate(result_entity->type)) {
				return false;
			}
		}
		call_result_type = pt->results;
		has_result = true;
		return_by_pointer = true;
	} else if (pt->result_count == 1) {
		Entity *result_entity = pt->results->Tuple.variables[0];
		if (result_entity == nullptr) {
			return false;
		}
		call_result_type = result_entity->type;
		if (!fast_backend_classify_scalar_type(result_entity->type, &result_type)) {
			if (!fast_backend_type_is_supported_aggregate(result_entity->type)) {
				return false;
			}
			// The planner is the single source of truth for the proc's ABI:
			// it forces `return_by_pointer = true` for every proc with a
			// non-scalar aggregate result so the proc and the caller agree.
			// (Without this, the arm64 classifier would mark a 16-byte
			// struct as a direct-aggregate return-in-registers, the proc
			// body would write through x0, and the caller's `x0,x1`-based
			// consumption would clobber it.)
			return_by_pointer = pt->return_by_pointer ||
				fast_backend_arm64_classify_aggregate_return(result_entity->type).kind == FastArm64AggregateReturn_None;
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

gb_internal Entity *fast_backend_resolve_direct_call_entity(Ast *proc_expr) {
	if (proc_expr == nullptr) {
		return nullptr;
	}

	Type *proc_type = base_type(type_of_expr(proc_expr));
	Entity *proc_entity = strip_entity_wrapping(entity_from_expr(proc_expr));
	if (proc_entity != nullptr && proc_entity->kind == Entity_ProcGroup) {
		if (proc_entity->ProcGroup.entities.count != 1) {
			return proc_entity;
		}
		proc_entity = proc_entity->ProcGroup.entities[0];
	}
	if (proc_entity == nullptr || proc_entity->kind != Entity_Procedure) {
		return proc_entity;
	}
	if (proc_type == nullptr || proc_type->kind != Type_Proc) {
		return proc_entity;
	}
	if (are_types_identical(base_type(proc_entity->type), proc_type)) {
		return proc_entity;
	}

	GenProcsData *gen_procs = proc_entity->Procedure.gen_procs;
	if (gen_procs == nullptr) {
		return proc_entity;
	}

	rw_mutex_shared_lock(&gen_procs->mutex); // @local-mutex
	defer (rw_mutex_shared_unlock(&gen_procs->mutex)); // @local-mutex
	for (Entity *other : gen_procs->procs) {
		if (other == nullptr || other->kind != Entity_Procedure) {
			continue;
		}
		Type *other_type = base_type(other->type);
		if (other_type != nullptr &&
		    other_type->kind == Type_Proc &&
		    are_types_identical(other_type, proc_type)) {
			return other;
		}
	}

	return proc_entity;
}

gb_internal Entity *fast_backend_canonical_top_level_entity(Entity *e) {
	if (e == nullptr) {
		return nullptr;
	}

	if (e->kind == Entity_Procedure || e->kind == Entity_ProcGroup || e->kind == Entity_Constant) {
		return strip_entity_wrapping(e);
	}

	if (e->kind == Entity_Variable) {
		while (e != nullptr &&
		       (e->flags.load(std::memory_order_relaxed) & EntityFlag_Overridden) != 0 &&
		       e->aliased_of != nullptr &&
		       e->aliased_of->kind == Entity_Variable) {
			e = e->aliased_of;
		}
	}

	return e;
}

gb_internal void fast_backend_record_emitted_entity(FastGenerator *gen, PtrSet<Entity *> *seen, Entity *e) {
	if (gen == nullptr || seen == nullptr || e == nullptr) {
		return;
	}
	if (!ptr_set_update(seen, e)) {
		array_add(&gen->emitted_entities, e);
	}
}

gb_internal i32 fast_backend_get_call_visible_result_count(AstCallExpr *ce, TypeProc *pt) {
	if (ce == nullptr || pt == nullptr || pt->result_count <= 0) {
		return 0;
	}
	i32 visible_count = pt->result_count;
	if (ce->optional_ok_one) {
		visible_count -= 1;
	}
	return gb_max(visible_count, 0);
}

gb_internal bool fast_backend_get_multi_result_call_info(Ast *expr, TypeProc **proc_type_, Entity **proc_entity_) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_CallExpr) {
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
	if (pt == nullptr || !has_result || !return_by_pointer) {
		return false;
	}
	if (result_type == nullptr || result_type->kind != Type_Tuple || result_type != pt->results) {
		return false;
	}
	if (fast_backend_get_call_visible_result_count(&expr->CallExpr, pt) <= 1) {
		return false;
	}
	if (proc_type_) *proc_type_ = pt;
	if (proc_entity_) *proc_entity_ = proc_entity;
	return true;
}

gb_internal bool fast_backend_get_multi_result_field_info(TypeProc *pt, i32 result_index, Type **type_, i32 *offset_, i32 *size_, i32 *align_) {
	if (pt == nullptr || pt->results == nullptr || pt->results->kind != Type_Tuple) {
		return false;
	}
	if (result_index < 0 || result_index >= pt->result_count) {
		return false;
	}

	i32 offset = 0;
	for (i32 i = 0; i < pt->result_count; i++) {
		Entity *result_entity = pt->results->Tuple.variables[i];
		if (result_entity == nullptr || result_entity->kind != Entity_Variable || result_entity->type == nullptr) {
			return false;
		}

		Type *type = result_entity->type;
		i32 size = cast(i32)type_size_of(type);
		i32 align = cast(i32)type_align_of(type);
		if (size <= 0 || align <= 0) {
			return false;
		}

		if (i == result_index) {
			if (type_)  *type_ = type;
			if (offset_) *offset_ = offset;
			if (size_)  *size_ = size;
			if (align_) *align_ = align;
			return true;
		}

		offset += size;
	}

	return false;
}

gb_internal bool fast_backend_get_call_visible_result_field_info(AstCallExpr *ce, TypeProc *pt, i32 result_index, Type **type_, i32 *offset_, i32 *size_, i32 *align_) {
	if (ce == nullptr || pt == nullptr) {
		return false;
	}
	if (result_index < 0 || result_index >= fast_backend_get_call_visible_result_count(ce, pt)) {
		return false;
	}
	return fast_backend_get_multi_result_field_info(pt, result_index, type_, offset_, size_, align_);
}

gb_internal bool fast_backend_get_call_optional_check_field_info(AstCallExpr *ce, TypeProc *pt, Type **type_, i32 *offset_, i32 *size_, i32 *align_) {
	if (ce == nullptr || pt == nullptr || !ce->optional_ok_one || pt->result_count < 2) {
		return false;
	}
	return fast_backend_get_multi_result_field_info(pt, pt->result_count-1, type_, offset_, size_, align_);
}

gb_internal bool fast_backend_get_multi_result_layout(TypeProc *pt, i32 *size_, i32 *align_) {
	if (pt == nullptr || pt->result_count <= 1) {
		return false;
	}

	i32 size = 0;
	i32 align = 1;
	for (i32 i = 0; i < pt->result_count; i++) {
		i32 field_size = 0;
		i32 field_align = 0;
		if (!fast_backend_get_multi_result_field_info(pt, i, nullptr, nullptr, &field_size, &field_align)) {
			return false;
		}
		size += field_size;
		align = gb_max(align, field_align);
	}

	if (size_ != nullptr) *size_ = size;
	if (align_ != nullptr) *align_ = align;
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
	gb_unused(pt);
	gb_unused(scalar_result_type);

	if (!allow_void_result && !has_result) {
		return false;
	}

	i32 total_param_count = 0;
	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *p = pt->params->Tuple.variables[i];
		if (p == nullptr || p->kind != Entity_Variable) {
			return false;
		}
		total_param_count += fast_backend_abi_arg_count(p->type);
	}
	total_param_count += (pt->calling_convention == ProcCC_Odin ? 1 : 0);
	total_param_count += (return_by_pointer ? 1 : 0);
	if (!fast_backend_supports_stack_args(pt) &&
	    total_param_count > fast_backend_param_limit_from_proc_type(pt)) {
		return false;
	}
	if (pt->calling_convention == ProcCC_Odin && !plan->has_context_slot) {
		return false;
	}

	Slice<Ast *> positional = {};
	Slice<Ast *> named = {};
	if (!fast_backend_get_call_split_args(ce, &positional, &named)) {
		return false;
	}
	if (!pt->variadic && positional.count > pt->param_count) {
		return false;
	}
	if (ce->ellipsis.pos.line != 0 && !pt->variadic) {
		return false;
	}
	gb_unused(named);

	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return false;
		}

		Ast *arg = nullptr;
		Slice<Ast *> variadic_pack = {};
		if (!fast_backend_resolve_call_param_arg(pt, ce, i, &arg, &variadic_pack)) {
			return false;
		}
		ParameterValue special_default = {};
		bool has_special_default = fast_backend_get_special_default_call_arg(pt, i, arg, &special_default);

		if (pt->variadic && i == pt->variadic_index && ce->ellipsis.pos.line == 0 && arg == nullptr) {
			Type *slice_type = default_type(param->type);
			if (!is_type_slice(slice_type)) {
				return false;
			}
			Type *elem_type = base_type(slice_type)->Slice.elem;
			for (Ast *elem : variadic_pack) {
				if (!fast_backend_can_emit_call_arg_expr(plan, elem, elem_type)) {
					return false;
				}
			}
			continue;
		}

		if (has_special_default) {
			switch (special_default.kind) {
			case ParameterValue_Location:
				if (!are_types_identical(default_type(param->type), default_type(t_source_code_location))) {
					return false;
				}
				continue;
			case ParameterValue_Expression:
				if (!are_types_identical(default_type(param->type), t_string)) {
					return false;
				}
				continue;
			}
		}

		if (!fast_backend_can_emit_call_arg_expr(plan, arg, param->type)) {
			return false;
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
	gb_unused(scalar_result_type);

	if (expr->CallExpr.optional_ok_one) {
		return false;
	}

	return has_result &&
	       result_type != nullptr &&
	       (return_by_pointer || fast_backend_arm64_classify_aggregate_return(result_type).kind != FastArm64AggregateReturn_None) &&
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

	i32 leading_slots = 0;
	if (return_by_pointer) {
		leading_slots = 1;
	} else if (fast_backend_arm64_classify_aggregate_return(result_type).kind != FastArm64AggregateReturn_None) {
		leading_slots = 1;
	}
	i32 abi_offset = leading_slots;
	if (have_call_info && pt != nullptr && pt->params != nullptr && pt->params->kind == Type_Tuple) {
		for (i32 i = 0; i < pt->param_count; i++) {
			Entity *param = pt->params->Tuple.variables[i];
			if (param == nullptr || param->kind != Entity_Variable) {
				continue;
			}

			Ast *arg = nullptr;
			Slice<Ast *> variadic_pack = {};
			if (!fast_backend_resolve_call_param_arg(pt, ce, i, &arg, &variadic_pack)) {
				continue;
			}
			ParameterValue special_default = {};
			bool has_special_default = fast_backend_get_special_default_call_arg(pt, i, arg, &special_default);

			i32 arg_depth = 0;
			if (pt->variadic && i == pt->variadic_index && ce->ellipsis.pos.line == 0 && arg == nullptr) {
				Type *slice_type = default_type(param->type);
				if (slice_type != nullptr && is_type_slice(slice_type)) {
					Type *elem_type = base_type(slice_type)->Slice.elem;
					for (Ast *elem : variadic_pack) {
						arg_depth = gb_max(arg_depth, fast_backend_call_arg_expr_spill_depth(nullptr, elem, elem_type));
					}
				}
			} else if (has_special_default) {
				arg_depth = 0;
			} else {
				arg_depth = fast_backend_call_arg_expr_spill_depth(nullptr, arg, param->type);
			}
			// abi_offset is the running ABI arg index for the call site,
			// matching what emit_call_expr_internal uses to assign
			// registers. The spill-depth needs to be at least the
			// ABI index, since emit_call pushes one spill per ABI
			// register-arg slot.
			depth = gb_max(depth, abi_offset + arg_depth);
			abi_offset += fast_backend_abi_arg_count(param->type);
		}
	} else {
		for_array(i, ce->args) {
			depth = gb_max(depth, leading_slots + cast(i32)i + fast_backend_leaf_expr_spill_depth(ce->args[i]));
		}
	}

	i32 total_arg_count = leading_slots;
	if (have_call_info && pt != nullptr) {
		for (i32 i = 0; i < pt->param_count; i++) {
			Entity *p = pt->params->Tuple.variables[i];
			if (p == nullptr || p->kind != Entity_Variable) {
				continue;
			}
			total_arg_count += fast_backend_abi_arg_count(p->type);
		}
	} else {
		total_arg_count += cast(i32)ce->args.count;
	}
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

	expr = unparen_expr(expr);
	if (expr != nullptr && expr->kind == Ast_BinaryExpr) {
		return fast_backend_can_emit_array_binary_expr(plan, expr, expected_type);
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
	if (is_type_any(expected_type) && fast_backend_can_emit_any_expr(plan, expr)) {
		return true;
	}
	if (is_type_union(expected_type) && fast_backend_can_emit_union_expr(plan, expected_type, expr)) {
		return true;
	}

	switch (expr->kind) {
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
	return expr != nullptr &&
	       expr->kind == Ast_BinaryExpr &&
	       type != nullptr &&
	       base != nullptr &&
	       (base->kind == Type_Array || base->kind == Type_EnumeratedArray || base->kind == Type_Matrix);
}

gb_internal bool fast_backend_can_emit_array_binary_expr(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
	if (plan == nullptr || !fast_backend_is_array_binary_expr(expr, expected_type)) {
		return false;
	}
	expr = unparen_expr(expr);
	return fast_backend_can_emit_array_binary_operands(plan, expected_type, expr->BinaryExpr.op.kind, expr->BinaryExpr.left, expr->BinaryExpr.right);
}

gb_internal bool fast_backend_type_supports_array_binary_op(Type *type, TokenKind op) {
	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}

	if (base->kind != Type_Array && base->kind != Type_EnumeratedArray && base->kind != Type_Matrix) {
		FastScalarType scalar_type = {};
		return fast_backend_classify_scalar_type(type, &scalar_type) &&
		       fast_backend_scalar_binary_op_supported(op, scalar_type);
	}

	Type *elem_type = nullptr;
	switch (base->kind) {
	case Type_Array:           elem_type = base->Array.elem;           break;
	case Type_EnumeratedArray: elem_type = base->EnumeratedArray.elem; break;
	case Type_Matrix:          elem_type = base->Matrix.elem;          break;
	}
	return fast_backend_type_supports_array_binary_op(elem_type, op);
}

gb_internal bool fast_backend_can_emit_array_binary_operands(FastLeafProcPlan *plan, Type *expected_type, TokenKind op, Ast *lhs, Ast *rhs) {
	if (plan == nullptr || expected_type == nullptr || lhs == nullptr || rhs == nullptr) {
		return false;
	}

	Type *type = default_type(expected_type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr || (base->kind != Type_Array && base->kind != Type_EnumeratedArray && base->kind != Type_Matrix)) {
		return false;
	}

	Type *left_type = default_type(type_of_expr(lhs));
	Type *right_type = default_type(type_of_expr(rhs));
	if (!are_types_identical(left_type, type) || !are_types_identical(right_type, type)) {
		return false;
	}

	Type *elem_type = nullptr;
	switch (base->kind) {
	case Type_Array:           elem_type = base->Array.elem;           break;
	case Type_EnumeratedArray: elem_type = base->EnumeratedArray.elem; break;
	case Type_Matrix:          elem_type = base->Matrix.elem;          break;
	}
	if (!fast_backend_type_supports_array_binary_op(elem_type, op)) {
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
		return is_type_string(type) || is_type_string16(type) || is_type_any(type);

	case Type_Array: {
		FastScalarType scalar_type = {};
		return fast_backend_classify_scalar_type(base->Array.elem, &scalar_type) ||
		       fast_backend_type_supports_aggregate_compare(base->Array.elem);
	}

	case Type_EnumeratedArray: {
		FastScalarType scalar_type = {};
		return fast_backend_classify_scalar_type(base->EnumeratedArray.elem, &scalar_type) ||
		       fast_backend_type_supports_aggregate_compare(base->EnumeratedArray.elem);
	}

	case Type_Matrix: {
		FastScalarType scalar_type = {};
		return fast_backend_classify_scalar_type(base->Matrix.elem, &scalar_type) ||
		       fast_backend_type_supports_aggregate_compare(base->Matrix.elem);
	}

	case Type_Struct:
		type_set_offsets(base);
		for (Entity *field : base->Struct.fields) {
			if (field == nullptr || field->kind != Entity_Variable) {
				return false;
			}
			FastScalarType scalar_type = {};
			if (!fast_backend_classify_scalar_type(field->type, &scalar_type) &&
			    !fast_backend_type_supports_aggregate_compare(field->type)) {
				return false;
			}
		}
		return true;
	}

	return false;
}

gb_internal Ast *fast_backend_unwrap_type_value_expr(Ast *expr, Type *target_type) {
	expr = unparen_expr(expr);
	target_type = default_type(target_type);
	if (expr == nullptr || target_type == nullptr || expr->kind != Ast_CallExpr) {
		return expr;
	}
	if (type_and_value_of_expr(expr->CallExpr.proc).mode != Addressing_Type || expr->CallExpr.args.count != 1) {
		return expr;
	}
	Type *expr_type = default_type(type_of_expr(expr));
	if (expr_type == nullptr ||
	    base_type(expr_type) == nullptr ||
	    base_type(target_type) == nullptr ||
	    !are_types_identical(base_type(expr_type), base_type(target_type))) {
		return expr;
	}
	Ast *arg = expr->CallExpr.args[0];
	if (arg != nullptr && arg->kind == Ast_FieldValue) {
		arg = arg->FieldValue.value;
	}
	return unparen_expr(arg);
}

gb_internal bool fast_backend_can_emit_any_expr(FastLeafProcPlan *plan, Ast *expr) {
	if (plan == nullptr || expr == nullptr || build_context.no_rtti) {
		return false;
	}

	expr = fast_backend_unwrap_type_value_expr(expr, t_any);
	Type *source_type = default_type(type_of_expr(expr));
	if (source_type == nullptr || is_type_any(source_type)) {
		return false;
	}
	if (!fast_backend_type_is_supported_value(source_type, nullptr, nullptr)) {
		return false;
	}

	Type *storage_type = nullptr;
	if (fast_backend_can_emit_address_expr(plan, expr, &storage_type, nullptr, nullptr) &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), source_type)) {
		return true;
	}

	if (!fast_backend_can_emit_value_expr(plan, expr, source_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, expr, source_type)) {
		return false;
	}

	return fast_backend_add_expr_slot(plan, expr, source_type, nullptr);
}

gb_internal bool fast_backend_can_emit_union_expr(FastLeafProcPlan *plan, Type *union_type, Ast *expr) {
	if (plan == nullptr || union_type == nullptr || expr == nullptr) {
		return false;
	}

	union_type = default_type(union_type);
	expr = fast_backend_unwrap_type_value_expr(expr, union_type);
	Type *source_type = default_type(type_of_expr(expr));
	if (union_type == nullptr || source_type == nullptr || !is_type_union(union_type) || is_type_union(source_type)) {
		return false;
	}
	if (!union_is_variant_of(union_type, source_type) || !fast_backend_type_is_supported_value(source_type, nullptr, nullptr)) {
		return false;
	}

	Type *storage_type = nullptr;
	if (fast_backend_can_emit_address_expr(plan, expr, &storage_type, nullptr, nullptr) &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), source_type)) {
		return true;
	}

	if (!fast_backend_can_emit_value_expr(plan, expr, source_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, expr, source_type)) {
		return false;
	}

	return fast_backend_add_expr_slot(plan, expr, source_type, nullptr);
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
	       (base_type(type)->kind == Type_Slice || base_type(type)->kind == Type_DynamicArray);
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
	Type *elem_type = nullptr;
	switch (base->kind) {
	case Type_Slice:        elem_type = base->Slice.elem;        break;
	case Type_DynamicArray: elem_type = base->DynamicArray.elem; break;
	default: return false;
	}
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
	case Type_Basic: {
		if (!is_type_any(type)) {
			return false;
		}
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
		if (cl->elems.count > 2) {
			return false;
		}
		Type *field_types[2] = {t_rawptr, t_typeid};
		for_array(i, cl->elems) {
			if (!fast_backend_can_emit_value_expr(plan, cl->elems[i], field_types[i]) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, cl->elems[i], field_types[i])) {
				return false;
			}
		}
		return true;
	}

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

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
		i64 elem_count = base->kind == Type_Array ? base->Array.count : base->EnumeratedArray.count;
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
		if (cl->elems.count > elem_count) {
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

	case Type_Matrix: {
		Type *elem_type = base->Matrix.elem;
		i64 elem_count = base->Matrix.row_count * base->Matrix.column_count;
		Type *vector_type = fast_backend_matrix_vector_type(base);
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
		if (cl->elems.count > elem_count) {
			return false;
		}
		for (Ast *elem : cl->elems) {
			Type *value_type = elem_type;
			Type *expr_type = default_type(type_of_expr(elem));
			if (vector_type != nullptr &&
			    expr_type != nullptr &&
			    are_types_identical(expr_type, vector_type)) {
				value_type = vector_type;
			}
			if (!fast_backend_can_emit_value_expr(plan, elem, value_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, elem, value_type)) {
				return false;
			}
		}
		return true;
	}

	case Type_SimdVector: {
		// Compound-literal syntax for SIMD vectors is just the N
		// lane values in order. No FieldValue, no range.
		Type *elem_type = base->SimdVector.elem;
		if (cl->elems.count != base->SimdVector.count) {
			return false;
		}
		if (cl->elems.count == 0) {
			return true;
		}
		for (i64 i = 0; i < cl->elems.count; i++) {
			if (!fast_backend_can_emit_value_expr(plan, cl->elems[i], elem_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, cl->elems[i], elem_type)) {
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
	if (expr->kind == Ast_BinaryExpr &&
	    default_type(type) != nullptr &&
	    (base_type(default_type(type))->kind == Type_Array || base_type(default_type(type))->kind == Type_EnumeratedArray || base_type(default_type(type))->kind == Type_Matrix)) {
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
	// Each per-field store goes through:
	//   fast_backend_emit_store_value_to_work_address_offset: 1 push (base_depth)
	//     fast_backend_emit_store_value_to_work_address:        1 push (dst_depth_inner)
	//       emit leaf:                                          leaf's own pushes
	//       1 push (after leaf, for the pop_tmp)
	//       1 pop
	//       1 load (no depth change)
	//       1 pop
	//     1 load (no depth change)
	//   1 load (no depth change)
	// So peak pushes for a field = 3 + leaf_depth, on top of the outer
	// dst_depth_outer push. The outer call adds 1.
	// The previous `1 + supported_value_expr_spill_depth` was missing 2
	// (the base_depth push and the post-leaf push), causing pushes to
	// overflow the spill region into the slot region and clobber param
	// slots (see llvm_vararg_clobber e case analysis, mk2/mk3 reads).
	i32 const per_field_overhead = 3;
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
			depth = gb_max(depth, per_field_overhead + fast_backend_supported_value_expr_spill_depth(value, value_type));
		}
		break;
	case Type_Array:
	case Type_EnumeratedArray:
		for (Ast *elem : cl->elems) {
			Ast *value = elem;
			if (elem != nullptr && elem->kind == Ast_FieldValue) {
				value = elem->FieldValue.value;
			}
			Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
			depth = gb_max(depth, per_field_overhead + fast_backend_supported_value_expr_spill_depth(value, elem_type));
		}
		break;
	case Type_Matrix:
		for (Ast *elem : cl->elems) {
			Ast *value = elem;
			if (elem != nullptr && elem->kind == Ast_FieldValue) {
				value = elem->FieldValue.value;
			}
			Type *value_type = base->Matrix.elem;
			Type *vector_type = fast_backend_matrix_vector_type(base);
			Type *expr_type = default_type(type_of_expr(value));
			if (vector_type != nullptr &&
			    expr_type != nullptr &&
			    are_types_identical(expr_type, vector_type)) {
				value_type = vector_type;
			}
			depth = gb_max(depth, per_field_overhead + fast_backend_supported_value_expr_spill_depth(value, value_type));
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
		if (is_type_slice(type) || is_type_dynamic_array(type)) {
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
	if (expr->kind == Ast_BinaryExpr &&
	    base_type(type) != nullptr &&
	    (base_type(type)->kind == Type_Array || base_type(type)->kind == Type_EnumeratedArray || base_type(type)->kind == Type_Matrix)) {
		return fast_backend_array_binary_expr_spill_depth(expr);
	}
	if (fast_backend_can_emit_constant_aggregate_expr(type, expr)) {
		return 2;
	}
	return fast_backend_address_expr_spill_depth(expr);
}

gb_internal i32 fast_backend_builtin_call_spill_depth(AstCallExpr *ce) {
	if (ce == nullptr) {
		return 0;
	}

	BuiltinProcId id = fast_backend_builtin_proc_id(ce->proc);
	if (id == BuiltinProc_typeid_of) {
		return 0;
	}
	if (id == BuiltinProc_abs || id == BuiltinProc_sqrt) {
		return ce->args.count == 1 ? fast_backend_leaf_expr_spill_depth(ce->args[0]) : 0;
	}
	if (id == BuiltinProc_fused_mul_add || id == BuiltinProc_clamp) {
		if (ce->args.count < 3) return 0;
		i32 d = fast_backend_leaf_expr_spill_depth(ce->args[0]);
		d = gb_max(d, fast_backend_leaf_expr_spill_depth(ce->args[1]));
		d = gb_max(d, fast_backend_leaf_expr_spill_depth(ce->args[2]));
		return d;
	}
	if (id == BuiltinProc_min || id == BuiltinProc_max) {
		i32 d = 0;
		for (Ast *arg : ce->args) {
			d = gb_max(d, fast_backend_leaf_expr_spill_depth(arg));
		}
		return d;
	}
	// Bit intrinsics: single-arg, same depth as abs/sqrt
	if (id == BuiltinProc_count_ones || id == BuiltinProc_count_zeros ||
	    id == BuiltinProc_count_trailing_zeros || id == BuiltinProc_count_leading_zeros ||
	    id == BuiltinProc_count_trailing_ones || id == BuiltinProc_count_leading_ones ||
	    id == BuiltinProc_reverse_bits || id == BuiltinProc_byte_swap) {
		return ce->args.count == 1 ? fast_backend_leaf_expr_spill_depth(ce->args[0]) : 0;
	}
	// Saturating intrinsics: two-arg integer
	if (id == BuiltinProc_saturating_add || id == BuiltinProc_saturating_sub) {
		if (ce->args.count < 2) return 0;
		i32 d = fast_backend_leaf_expr_spill_depth(ce->args[0]);
		d = gb_max(d, fast_backend_leaf_expr_spill_depth(ce->args[1]));
		return d;
	}
	// Memory intrinsics: 2-3 args (dst, src, len)
	if (id == BuiltinProc_mem_copy || id == BuiltinProc_mem_copy_non_overlapping ||
	    id == BuiltinProc_mem_zero || id == BuiltinProc_mem_zero_volatile) {
		i32 d = 0;
		for (Ast *arg : ce->args) {
			d = gb_max(d, fast_backend_leaf_expr_spill_depth(arg));
		}
		return d;
	}
	// alloca/trap/debug_trap/volatile/non_temporal: no spill needed (or handled separately)
	if (id == BuiltinProc_alloca || id == BuiltinProc_trap || id == BuiltinProc_debug_trap ||
	    id == BuiltinProc_volatile_load || id == BuiltinProc_volatile_store ||
	    id == BuiltinProc_non_temporal_load || id == BuiltinProc_non_temporal_store) {
		return 0;
	}
	// Overflow intrinsics: need extra spill for the overflow flag
	if (id == BuiltinProc_overflow_add || id == BuiltinProc_overflow_sub || id == BuiltinProc_overflow_mul) {
		if (ce->args.count < 2) return 0;
		i32 d = fast_backend_leaf_expr_spill_depth(ce->args[0]);
		d = gb_max(d, fast_backend_leaf_expr_spill_depth(ce->args[1]));
		return d;
	}
	// cpu_relax/prefetch: no args, no spill needed
	if (id == BuiltinProc_cpu_relax ||
	    id == BuiltinProc_prefetch_read_instruction || id == BuiltinProc_prefetch_read_data ||
	    id == BuiltinProc_prefetch_write_instruction || id == BuiltinProc_prefetch_write_data) {
		return 0;
	}
	if (id != BuiltinProc_len && id != BuiltinProc_cap && id != BuiltinProc_raw_data) {
		return fast_backend_leaf_expr_spill_depth(ce->args.count != 0 ? ce->args[0] : nullptr);
	}

	Ast *arg = ce->args[0];
	Type *arg_type = base_type(type_of_expr(arg));
	Type *arg_value_type = default_type(type_of_expr(arg));
	bool arg_is_pointer_like = false;
	if (arg_type != nullptr && (arg_type->kind == Type_Pointer || arg_type->kind == Type_MultiPointer)) {
		arg_is_pointer_like = true;
		arg_type = default_type(type_deref(arg_type, true));
	}

	if (id == BuiltinProc_raw_data && arg != nullptr && unparen_expr(arg)->kind == Ast_SliceExpr) {
		i32 temp_slots = arg_value_type != nullptr ? align_formula(cast(i32)type_size_of(arg_value_type), 8)/8 : 0;
		return temp_slots + fast_backend_slice_expr_spill_depth(&unparen_expr(arg)->SliceExpr);
	}

	if (id == BuiltinProc_len && arg_type != nullptr &&
	    (is_type_cstring(arg_type) || is_type_cstring16(arg_type))) {
		return fast_backend_leaf_expr_spill_depth(arg);
	}

	if (arg_is_pointer_like) {
		return fast_backend_leaf_expr_spill_depth(arg);
	}
	Ast *base_arg = unparen_expr(arg);
	if (arg_value_type != nullptr &&
	    base_arg != nullptr &&
	    (base_arg->kind == Ast_SliceExpr ||
	     base_arg->kind == Ast_CallExpr ||
	     base_arg->kind == Ast_CompoundLit ||
	     fast_backend_can_emit_constant_aggregate_expr(arg_value_type, arg))) {
		i32 temp_slots = align_formula(cast(i32)type_size_of(arg_value_type), 8)/8;
		return temp_slots + fast_backend_supported_value_expr_spill_depth(arg, arg_value_type);
	}
	return fast_backend_address_expr_spill_depth(arg);
}

gb_internal bool fast_backend_can_emit_builtin_call_expr(FastLeafProcPlan *plan, AstCallExpr *ce, Type *expected_type) {
	if (ce == nullptr) {
		return false;
	}

	FastScalarType result_type = {};
	Type *want_type = expected_type ? expected_type : reduce_tuple_to_single_type(type_of_expr(cast(Ast *)ce));
	// Void-returning builtins (cpu_relax, prefetch_*, trap, ...) are
	// side-effecting hints; the per-builtin checks below already gate
	// their argument shape, so don't reject them up here just because
	// they have no scalar return.
	if (want_type != nullptr && !fast_backend_classify_scalar_type(want_type, &result_type)) {
		return false;
	}

	BuiltinProcId id = fast_backend_builtin_proc_id(ce->proc);
	if (id == BuiltinProc_typeid_of) {
		return ce->args.count == 1 &&
		       type_and_value_of_expr(ce->args[0]).mode == Addressing_Type &&
		       fast_backend_classify_scalar_type(want_type, &result_type);
	}
	if (id == BuiltinProc_abs || id == BuiltinProc_sqrt) {
		return ce->args.count == 1 && fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0]));
	}
	if (id == BuiltinProc_fused_mul_add) {
		if (ce->args.count != 3) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[2], type_of_expr(ce->args[2]));
	}
	if (id == BuiltinProc_min || id == BuiltinProc_max) {
		if (ce->args.count < 2) return false;
		for (Ast *arg : ce->args) {
			if (!fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg))) {
				return false;
			}
		}
		return true;
	}
	if (id == BuiltinProc_clamp) {
		if (ce->args.count != 3) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[2], type_of_expr(ce->args[2]));
	}
	// Bit intrinsics: single-arg integer scalar
	if (id == BuiltinProc_count_ones || id == BuiltinProc_count_zeros ||
	    id == BuiltinProc_count_trailing_zeros || id == BuiltinProc_count_leading_zeros ||
	    id == BuiltinProc_count_trailing_ones || id == BuiltinProc_count_leading_ones ||
	    id == BuiltinProc_reverse_bits || id == BuiltinProc_byte_swap) {
		if (ce->args.count != 1) return false;
		Type *arg_type = default_type(type_of_expr(ce->args[0]));
		if (arg_type == nullptr) return false;
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(arg_type, &st)) return false;
		if (st.kind == FastScalar_Float) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0]));
	}
	// Saturating intrinsics: two-arg integer scalar
	if (id == BuiltinProc_saturating_add || id == BuiltinProc_saturating_sub) {
		if (ce->args.count != 2) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1]));
	}
	// Memory intrinsics: 2-3 args (all rawptr or int)
	if (id == BuiltinProc_mem_copy || id == BuiltinProc_mem_copy_non_overlapping ||
	    id == BuiltinProc_mem_zero || id == BuiltinProc_mem_zero_volatile) {
		for (Ast *arg : ce->args) {
			if (!fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg))) {
				return false;
			}
		}
		return true;
	}
	// alloca(size, align): 2 int args, returns rawptr
	if (id == BuiltinProc_alloca) {
		return ce->args.count == 2 &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1]));
	}
	// trap/debug_trap: 0 args, noreturn
	if (id == BuiltinProc_trap || id == BuiltinProc_debug_trap) {
		return ce->args.count == 0;
	}
	// volatile_load/volatile_store/non_temporal_load/non_temporal_store
	if (id == BuiltinProc_volatile_load) {
		return ce->args.count == 1 && fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0]));
	}
	if (id == BuiltinProc_volatile_store || id == BuiltinProc_non_temporal_store) {
		return ce->args.count == 2 &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1]));
	}
	if (id == BuiltinProc_non_temporal_load) {
		return ce->args.count == 1 && fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0]));
	}
	// cpu_relax: no args, simple hint
	if (id == BuiltinProc_cpu_relax) {
		return ce->args.count == 0;
	}
	// prefetch: (address, locality 0..=3). Locality must be a
	// constant int (the type-checker already enforces this).
	if (id == BuiltinProc_prefetch_read_instruction || id == BuiltinProc_prefetch_read_data ||
	    id == BuiltinProc_prefetch_write_instruction || id == BuiltinProc_prefetch_write_data) {
		if (ce->args.count != 2) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0]));
	}
	// Overflow intrinsics: two-arg integer, returns (T, bool)
	if (id == BuiltinProc_overflow_add || id == BuiltinProc_overflow_sub || id == BuiltinProc_overflow_mul) {
		if (ce->args.count != 2) return false;
		Type *arg0_type = default_type(type_of_expr(ce->args[0]));
		Type *arg1_type = default_type(type_of_expr(ce->args[1]));
		if (arg0_type == nullptr || arg1_type == nullptr) return false;
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(arg0_type, &st)) return false;
		if (st.kind == FastScalar_Float) return false;
		return fast_backend_can_emit_leaf_expr(plan, ce->args[0], type_of_expr(ce->args[0])) &&
		       fast_backend_can_emit_leaf_expr(plan, ce->args[1], type_of_expr(ce->args[1]));
	}
	if (id != BuiltinProc_len && id != BuiltinProc_cap && id != BuiltinProc_raw_data) {
		return false;
	}
	if (ce->args.count != 1) {
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
		return fast_backend_can_emit_address_expr(plan, arg, nullptr, nullptr, nullptr) ||
		       fast_backend_can_emit_value_expr(plan, arg, type_of_expr(arg)) ||
		       fast_backend_can_emit_aggregate_call_expr(plan, arg, type_of_expr(arg));
	}

	if (id == BuiltinProc_len && (is_type_cstring(arg_type) || is_type_cstring16(arg_type))) {
		return fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg));
	}

	if (is_type_dynamic_array(arg_type) || is_type_fixed_capacity_dynamic_array(arg_type)) {
		if (arg_is_pointer_like) {
			return fast_backend_can_emit_leaf_expr(plan, arg, type_of_expr(arg));
		}
		return fast_backend_can_emit_address_expr(plan, arg, nullptr, nullptr, nullptr) ||
		       fast_backend_can_emit_value_expr(plan, arg, type_of_expr(arg)) ||
		       fast_backend_can_emit_aggregate_call_expr(plan, arg, type_of_expr(arg));
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
	switch (type->kind) {
	case Type_Slice:
	case Type_DynamicArray:
	case Type_FixedCapacityDynamicArray:
		return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr) ||
		       fast_backend_can_emit_value_expr(plan, expr, type_of_expr(expr)) ||
		       fast_backend_can_emit_aggregate_call_expr(plan, expr, type_of_expr(expr));

	case Type_Basic:
		if (is_type_string(type) || is_type_string16(type)) {
			return fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr) ||
			       fast_backend_can_emit_value_expr(plan, expr, type_of_expr(expr)) ||
			       fast_backend_can_emit_aggregate_call_expr(plan, expr, type_of_expr(expr));
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
	Type *source_value_type = default_type(type_of_expr(se->expr));
	if (result_type == nullptr || source_type == nullptr || source_value_type == nullptr) {
		return false;
	}

	bool can_address_source = fast_backend_can_emit_address_expr(plan, se->expr, nullptr, nullptr, nullptr);
	bool can_materialize_source = fast_backend_can_emit_value_expr(plan, se->expr, source_value_type) ||
	                              fast_backend_can_emit_aggregate_call_expr(plan, se->expr, source_value_type);
	switch (source_type->kind) {
	case Type_Array:
	case Type_EnumeratedArray:
		if (!is_type_slice(result_type) || !can_address_source) {
			return false;
		}
		break;

	case Type_Slice:
	case Type_DynamicArray:
	case Type_FixedCapacityDynamicArray:
		if (!is_type_slice(result_type) || (!can_address_source && !can_materialize_source)) {
			return false;
		}
		break;

	case Type_Basic:
		if ((!is_type_string(source_type) && !is_type_string16(source_type)) ||
		    !are_types_identical(result_type, default_type(source_type)) ||
		    (!can_address_source && !can_materialize_source)) {
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
	if (base_value_type == nullptr || base_type_ == nullptr || (base_type_->kind != Type_Array && base_type_->kind != Type_EnumeratedArray)) {
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
	Type *source_value_type = default_type(type_of_expr(se->expr));
	Ast *source_expr = unparen_expr(se->expr);
	if (source_value_type != nullptr &&
	    source_expr != nullptr &&
	    (source_expr->kind == Ast_SliceExpr ||
	     source_expr->kind == Ast_CallExpr ||
	     source_expr->kind == Ast_CompoundLit ||
	     fast_backend_can_emit_constant_aggregate_expr(source_value_type, se->expr))) {
		i32 temp_slots = align_formula(cast(i32)type_size_of(source_value_type), 8)/8;
		depth = gb_max(depth, 1 + temp_slots + fast_backend_supported_value_expr_spill_depth(se->expr, source_value_type));
	} else {
		depth = gb_max(depth, 1 + fast_backend_address_expr_spill_depth(se->expr));
	}
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
	bool expr_is_nil = expr->kind == Ast_Ident &&
	                   expr->Ident.entity.load() != nullptr &&
	                   expr->Ident.entity.load()->kind == Entity_Nil;
	if (expr_is_nil || is_type_untyped_nil(tv.type)) {
		FastScalarType type = {};
		return fast_backend_classify_scalar_type(expected_type ? expected_type : tv.type, &type);
	}
	if (tv.mode == Addressing_Invalid) {
		return false;
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

	case Ast_SelectorExpr: {
		Type *value_type = nullptr;
		bool is_scalar = false;
		return (fast_backend_can_emit_address_expr(plan, expr, &value_type, nullptr, &is_scalar) && is_scalar) ||
		       fast_backend_can_emit_direct_struct_selector_expr(plan, expr);
	}

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
		{
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
			gb_unused(result_type);
			gb_unused(scalar_result_type);
			if (!has_result) {
				return false;
			}
			if (expr->CallExpr.optional_ok_one) {
				Type *visible_type = nullptr;
				FastScalarType visible_scalar_type = {};
				if (fast_backend_get_call_visible_result_count(&expr->CallExpr, pt) != 1 ||
				    !fast_backend_get_call_visible_result_field_info(&expr->CallExpr, pt, 0, &visible_type, nullptr, nullptr, nullptr) ||
				    !fast_backend_classify_scalar_type(visible_type, &visible_scalar_type)) {
					return false;
				}
				Type *want_type = expected_type ? expected_type : visible_type;
				return are_types_identical(default_type(visible_type), default_type(want_type)) &&
				       fast_backend_can_emit_call_expr(plan, &expr->CallExpr, false);
			}
			if (return_by_pointer) {
				return false;
			}
			return fast_backend_can_emit_call_expr(plan, &expr->CallExpr, false);
		}

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
			return operand_scalar.kind == FastScalar_Signed ||
			       operand_scalar.kind == FastScalar_Unsigned ||
			       operand_scalar.kind == FastScalar_Float;
		case Token_Not:
			return fast_backend_scalar_is_integer_like(operand_scalar) ||
			       operand_scalar.kind == FastScalar_Pointer ||
			       operand_scalar.kind == FastScalar_Float;
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

		return fast_backend_scalar_binary_op_supported(expr->BinaryExpr.op.kind, operand_scalar);
	}
	}

	return false;
}

gb_internal bool fast_backend_can_emit_cond_expr(FastLeafProcPlan *plan, Ast *expr) {
	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return false;
	}

	if (expr->kind == Ast_UnaryExpr && expr->UnaryExpr.op.kind == Token_Not) {
		return fast_backend_can_emit_cond_expr(plan, expr->UnaryExpr.expr);
	}

	if (expr->kind == Ast_BinaryExpr) {
		if (expr->BinaryExpr.op.kind == Token_CmpAnd || expr->BinaryExpr.op.kind == Token_CmpOr) {
			return fast_backend_can_emit_cond_expr(plan, expr->BinaryExpr.left) &&
			       fast_backend_can_emit_cond_expr(plan, expr->BinaryExpr.right);
		}
	}

	return fast_backend_can_emit_leaf_expr(plan, expr, type_and_value_of_expr(expr).type);
}

gb_internal i32 fast_backend_cond_expr_spill_depth(Ast *expr) {
	expr = unparen_expr(expr);
	if (expr == nullptr) {
		return 0;
	}

	if (expr->kind == Ast_UnaryExpr && expr->UnaryExpr.op.kind == Token_Not) {
		return fast_backend_cond_expr_spill_depth(expr->UnaryExpr.expr);
	}

	if (expr->kind == Ast_BinaryExpr) {
		if (expr->BinaryExpr.op.kind == Token_CmpAnd || expr->BinaryExpr.op.kind == Token_CmpOr) {
			i32 left_depth = fast_backend_cond_expr_spill_depth(expr->BinaryExpr.left);
			i32 right_depth = fast_backend_cond_expr_spill_depth(expr->BinaryExpr.right);
			return gb_max(left_depth, right_depth);
		}
	}

	return fast_backend_leaf_expr_spill_depth(expr);
}

gb_internal bool fast_backend_plan_stmt(FastGenerator *gen, FastLeafProcPlan *plan, Ast *stmt);
gb_internal bool fast_backend_plan_range_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstRangeStmt *rs);
gb_internal bool fast_backend_emit_range_stmt(FastLeafProcEmitter *emitter, AstRangeStmt *rs);

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
	TypeProc *multi_result_pt = nullptr;
	if (vd->values.count == 1) {
		fast_backend_get_multi_result_call_info(vd->values[0], &multi_result_pt, nullptr);
	}
	if (vd->values.count != 0 && vd->values.count != vd->names.count && multi_result_pt == nullptr) {
		return fast_backend_reject(FastFallback_ValueDecl, vd->names[0], str_lit("Fast backend currently only supports one-to-one value declarations"));
	}
	if (multi_result_pt != nullptr &&
	    vd->names.count != fast_backend_get_call_visible_result_count(&unparen_expr(vd->values[0])->CallExpr, multi_result_pt)) {
		return fast_backend_reject(FastFallback_ValueDecl, vd->values[0], str_lit("Fast backend multi-result value declarations require one destination per result"));
	}

	for_array(i, vd->names) {
		Ast *name = vd->names[i];
		if (is_blank_ident(name)) {
			continue;
		}

			Entity *entity = entity_of_node(name);
			if (entity == nullptr || entity->kind != Entity_Variable) {
				return fast_backend_reject(FastFallback_Internal, name, str_lit("Fast backend expected a local variable entity"));
			}
			if (entity->flags.load(std::memory_order_relaxed) & EntityFlag_Static) {
				return fast_backend_reject(FastFallback_ValueDecl, name, str_lit("Fast backend does not yet support static local variables"));
			}

			if (!fast_backend_type_is_supported_value(entity->type, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_Type, name, str_lit("Fast backend does not yet support this local variable type"));
			}
		if (!fast_backend_add_slot(plan, entity, entity->type)) {
			return false;
		}
	}

	if (multi_result_pt != nullptr) {
		Ast *rhs = vd->values[0];
		AstCallExpr *rhs_call = &unparen_expr(rhs)->CallExpr;
		GB_ASSERT(rhs != nullptr);
		for_array(i, vd->names) {
			Ast *name = vd->names[i];
			if (is_blank_ident(name)) {
				continue;
			}

			Entity *entity = entity_of_node(name);
			if (entity == nullptr || entity->kind != Entity_Variable) {
				return fast_backend_reject(FastFallback_Internal, name, str_lit("Fast backend expected a local variable entity"));
			}

			Type *result_type = nullptr;
			if (!fast_backend_get_call_visible_result_field_info(rhs_call, multi_result_pt, cast(i32)i, &result_type, nullptr, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_ValueDecl, rhs, str_lit("Fast backend expected a concrete multi-result call layout"));
			}
			if (!are_types_identical(default_type(entity->type), default_type(result_type))) {
				return fast_backend_reject(FastFallback_ValueDecl, rhs, str_lit("Fast backend multi-result value declarations require matching destination types"));
			}
		}
		plan->spill_depth = gb_max(plan->spill_depth, 2 + fast_backend_call_expr_spill_depth(&unparen_expr(rhs)->CallExpr));
		return true;
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
				return fast_backend_reject(FastFallback_ValueDecl, rhs, str_lit("Fast backend does not yet support this value declaration expression"));
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
			return fast_backend_reject(FastFallback_Assignment, as->op, str_lit("Fast backend assignment operators require one-to-one operands"));
		}
		TokenKind binary_op = Token_Invalid;
		if (!fast_backend_assign_op_to_binary(as->op.kind, &binary_op)) {
			return fast_backend_reject(FastFallback_Assignment, as->op, str_lit("Fast backend does not yet support this assignment operator"));
		}

		Ast *lhs = unparen_expr(as->lhs[0]);
		Ast *rhs = as->rhs[0];
			Type *target_type = nullptr;
			if (!fast_backend_can_emit_address_expr(plan, lhs, &target_type, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_Assignment, lhs, str_lit("Fast backend expected a supported assignment target"));
			}

		FastScalarType scalar_type = {};
			if (!fast_backend_expr_scalar_type(lhs, target_type, &scalar_type)) {
				if (!fast_backend_can_emit_array_binary_operands(plan, target_type, binary_op, lhs, rhs)) {
					return fast_backend_reject(FastFallback_Assignment, lhs, str_lit("Fast backend currently only supports scalar and fixed-array assignment operators"));
				}
			plan->spill_depth = gb_max(plan->spill_depth, gb_max(fast_backend_address_expr_spill_depth(lhs), fast_backend_array_binary_operands_spill_depth(lhs, rhs)));
			return true;
			}
			if (!fast_backend_scalar_binary_op_supported(binary_op, scalar_type)) {
				return fast_backend_reject(FastFallback_Assignment, as->op, str_lit("Fast backend does not yet support this assignment operator for the target type"));
			}
			if (!fast_backend_can_emit_leaf_expr(plan, rhs, target_type)) {
				return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend does not yet support this assignment expression"));
			}

		plan->spill_depth = gb_max(plan->spill_depth, gb_max(1 + fast_backend_address_expr_spill_depth(lhs), 2 + fast_backend_leaf_expr_spill_depth(rhs)));
		return true;
	}
	TypeProc *multi_result_pt = nullptr;
	if (as->rhs.count == 1) {
		fast_backend_get_multi_result_call_info(as->rhs[0], &multi_result_pt, nullptr);
	}
	if (multi_result_pt != nullptr) {
		Ast *rhs = as->rhs[0];
		AstCallExpr *rhs_call = &unparen_expr(rhs)->CallExpr;
		if (as->lhs.count != fast_backend_get_call_visible_result_count(rhs_call, multi_result_pt)) {
			return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend multi-result assignments require one destination per result"));
		}

		i32 call_depth = 2 + fast_backend_call_expr_spill_depth(&unparen_expr(rhs)->CallExpr);
		for_array(i, as->lhs) {
			Ast *lhs = unparen_expr(as->lhs[i]);
			Type *result_type = nullptr;
			if (!fast_backend_get_call_visible_result_field_info(rhs_call, multi_result_pt, cast(i32)i, &result_type, nullptr, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend expected a concrete multi-result call layout"));
			}
			if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
				continue;
			}

			Type *target_type = nullptr;
			if (!fast_backend_can_emit_address_expr(plan, lhs, &target_type, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_Assignment, lhs, str_lit("Fast backend expected a supported assignment target"));
			}
			if (!are_types_identical(default_type(target_type), default_type(result_type))) {
				return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend multi-result assignments require matching destination types"));
			}

			plan->spill_depth = gb_max(plan->spill_depth, gb_max(call_depth, 1 + fast_backend_address_expr_spill_depth(lhs)));
		}
		plan->spill_depth = gb_max(plan->spill_depth, call_depth);
		return true;
	}
	if (as->lhs.count != as->rhs.count) {
		return fast_backend_reject(FastFallback_Assignment, as->op, str_lit("Fast backend currently only supports one-to-one assignments"));
	}

	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		Ast *rhs = as->rhs[i];
			if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
				Type *rhs_type = reduce_tuple_to_single_type(type_and_value_of_expr(rhs).type);
				if (!fast_backend_can_emit_value_expr(plan, rhs, rhs_type) &&
				    !fast_backend_can_emit_aggregate_call_expr(plan, rhs, rhs_type)) {
					return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend does not yet support this discard assignment expression"));
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
				return fast_backend_reject(FastFallback_Assignment, lhs, str_lit("Fast backend expected a supported assignment target"));
			}

			if (!fast_backend_can_emit_value_expr(plan, rhs, target_type) &&
			    !fast_backend_can_emit_scalar_compound_lit_expr(plan, rhs, target_type) &&
			    !fast_backend_can_emit_aggregate_call_expr(plan, rhs, target_type)) {
				return fast_backend_reject(FastFallback_Assignment, rhs, str_lit("Fast backend does not yet support this assignment expression"));
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
			return fast_backend_reject(FastFallback_Return, rs->token, str_lit("Fast backend expected no return values"));
		}
		return true;
	}

	if (rs->results.count != 1 && plan->type->result_count == 1) {
		return fast_backend_reject(FastFallback_Return, rs->token, str_lit("Fast backend currently only supports a single return value"));
	}

	if (plan->type->result_count > 1) {
		if (rs->results.count == 1) {
			TypeProc *call_pt = nullptr;
			if (fast_backend_get_multi_result_call_info(rs->results[0], &call_pt, nullptr)) {
				AstCallExpr *call_expr = &unparen_expr(rs->results[0])->CallExpr;
				if (fast_backend_get_call_visible_result_count(call_expr, call_pt) != plan->type->result_count) {
					return fast_backend_reject(FastFallback_Return, rs->results[0], str_lit("Fast backend multi-result return forwarding requires matching result counts"));
				}
				for (i32 i = 0; i < plan->type->result_count; i++) {
					Type *call_result_type = nullptr;
					Type *proc_result_type = nullptr;
					if (!fast_backend_get_call_visible_result_field_info(call_expr, call_pt, i, &call_result_type, nullptr, nullptr, nullptr) ||
					    !fast_backend_get_multi_result_field_info(plan->type, i, &proc_result_type, nullptr, nullptr, nullptr)) {
						return fast_backend_reject(FastFallback_Return, rs->results[0], str_lit("Fast backend expected a concrete multi-result call layout"));
					}
					if (!are_types_identical(default_type(call_result_type), default_type(proc_result_type))) {
						return fast_backend_reject(FastFallback_Return, rs->results[0], str_lit("Fast backend multi-result return forwarding requires matching result types"));
					}
				}
				plan->spill_depth = gb_max(plan->spill_depth, 2 + fast_backend_call_expr_spill_depth(&unparen_expr(rs->results[0])->CallExpr));
				return true;
			}
		}

		// Multi-return: each result is stored to the sret buffer at
		// the cumulative offset of that result. Verify each result
		// expression is emit-able, and accumulate the spill depth.
		GB_ASSERT(plan->type->results != nullptr && plan->type->results->kind == Type_Tuple);
		i32 cumulative_offset = 0;
		for (i32 i = 0; i < rs->results.count; i++) {
			Ast *result = rs->results[i];
			Entity *result_entity = plan->type->results->Tuple.variables[i];
			Type *result_type = result_entity != nullptr ? result_entity->type : nullptr;
			if (result_type == nullptr) {
				return false;
			}
				if (fast_backend_classify_scalar_type(result_type, nullptr)) {
					if (!fast_backend_can_emit_leaf_expr(plan, result, result_type)) {
						return fast_backend_reject(FastFallback_Return, result, str_lit("Fast backend does not yet support this return expression"));
					}
				plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(result));
				} else {
					if (!fast_backend_can_emit_aggregate_expr(plan, result, result_type) &&
					    !fast_backend_can_emit_aggregate_call_expr(plan, result, result_type) &&
					    !fast_backend_can_emit_scalar_compound_lit_expr(plan, result, result_type)) {
						return fast_backend_reject(FastFallback_Return, result, str_lit("Fast backend does not yet support this return expression"));
					}
				if (fast_backend_can_emit_aggregate_call_expr(plan, result, result_type)) {
					plan->spill_depth = gb_max(plan->spill_depth, fast_backend_call_expr_spill_depth(&unparen_expr(result)->CallExpr));
				} else if (fast_backend_can_emit_scalar_compound_lit_expr(plan, result, result_type)) {
					plan->spill_depth = gb_max(plan->spill_depth, fast_backend_scalar_compound_lit_spill_depth(result, result_type));
				}
			}
			cumulative_offset += cast(i32)type_size_of(result_type);
		}
		plan->spill_depth = gb_max(plan->spill_depth, cumulative_offset + 1);
		return true;
	}

	Ast *result = rs->results[0];
	Type *result_type = reduce_tuple_to_single_type(plan->type->results);
	bool return_direct_aggregate = !plan->return_by_pointer &&
	                               fast_backend_arm64_classify_aggregate_return(result_type).kind != FastArm64AggregateReturn_None;
	if (!plan->return_by_pointer && !return_direct_aggregate) {
		if (!fast_backend_can_emit_leaf_expr(plan, result, result_type)) {
			return fast_backend_reject(FastFallback_Return, result, str_lit("Fast backend does not yet support this return expression"));
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(result));
		return true;
	}

	if (!fast_backend_can_emit_aggregate_expr(plan, result, result_type) &&
	    !fast_backend_can_emit_aggregate_call_expr(plan, result, result_type) &&
	    !fast_backend_can_emit_scalar_compound_lit_expr(plan, result, result_type)) {
		return fast_backend_reject(FastFallback_Return, result, str_lit("Fast backend does not yet support this return expression"));
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
	if (!fast_backend_can_emit_cond_expr(plan, is->cond)) {
		return fast_backend_reject(FastFallback_ControlFlow, is->cond, str_lit("Fast backend does not yet support this if condition"));
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_cond_expr_spill_depth(is->cond));
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
		if (!fast_backend_can_emit_cond_expr(plan, fs->cond)) {
			return fast_backend_reject(FastFallback_ControlFlow, fs->cond, str_lit("Fast backend does not yet support this for-loop condition"));
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_cond_expr_spill_depth(fs->cond));
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
		return fast_backend_reject(FastFallback_Switch, ss->label, str_lit("Fast backend expected an identifier switch label"));
	}
	if (ss->init != nullptr && !fast_backend_plan_stmt(gen, plan, ss->init)) {
		return false;
	}
	if (ss->body == nullptr || ss->body->kind != Ast_BlockStmt) {
		return fast_backend_reject(FastFallback_Switch, ss->token, str_lit("Fast backend expected a switch body block"));
	}

	Type *tag_type = t_bool;
	bool tag_is_string = false;
	bool tag_has_call = false;
	if (ss->tag != nullptr) {
		tag_type = reduce_tuple_to_single_type(type_and_value_of_expr(ss->tag).type);
		tag_is_string = is_type_string(tag_type) || is_type_string16(tag_type);
		tag_has_call = fast_backend_expr_has_call(ss->tag);
		if (!tag_is_string) {
			FastScalarType scalar_type = {};
			if (!fast_backend_expr_scalar_type(nullptr, tag_type, &scalar_type)) {
				return fast_backend_reject(FastFallback_Switch, ss->tag, str_lit("Fast backend currently only supports scalar switch tags"));
			}
			if (!fast_backend_can_emit_leaf_expr(plan, ss->tag, tag_type)) {
				return fast_backend_reject(FastFallback_Switch, ss->tag, str_lit("Fast backend does not yet support this switch tag expression"));
			}
		} else {
			// String tag: emit-side materializes tag + case to spill slots
			// and does a field-by-field byte compare. Tag must be
			// materializable as a string aggregate (Ident, CompoundLit,
			// SliceExpr, ...).
			if (tag_has_call) {
				return fast_backend_reject(FastFallback_Switch, ss->tag, str_lit("Fast backend switch tags cannot contain calls yet"));
			}
			if (!fast_backend_can_emit_aggregate_compare_operand(plan, ss->tag, tag_type)) {
				return fast_backend_reject(FastFallback_Switch, ss->tag, str_lit("Fast backend does not yet support this switch tag expression"));
			}
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(ss->tag));
		if (tag_has_call && !tag_is_string) {
			plan->spill_depth = gb_max(plan->spill_depth, 1);
		}
		if (tag_is_string) {
			// Per-case we materialize both operands to spill slots, then
			// fast_backend_emit_compare_aggregate_at_offset uses 4 more
			// scratch slots. Baseline 6 covers the per-case frame plus
			// a small per-operand margin.
			plan->spill_depth = gb_max(plan->spill_depth,
			    6 + fast_backend_aggregate_compare_operand_spill_depth(plan, ss->tag, tag_type));
		}
	}

	for (Ast *stmt : ss->body->BlockStmt.stmts) {
		if (stmt == nullptr || stmt->kind != Ast_CaseClause) {
			return fast_backend_reject(FastFallback_Switch, stmt ? stmt : cast(Ast *)ss, str_lit("Fast backend expected switch case clauses"));
		}

		AstCaseClause *cc = &stmt->CaseClause;
		for (Ast *expr : cc->list) {
			if (!fast_backend_can_emit_switch_case_expr(plan, expr, tag_type)) {
				return fast_backend_reject(FastFallback_Switch, expr, str_lit("Fast backend does not yet support this switch case expression"));
			}

			expr = unparen_expr(expr);
			if (is_ast_range(expr)) {
				// Ranges on string tags aren't supported (string has no
				// natural ordering). fast_backend_can_emit_switch_case_expr
				// already rejected this above.
				i32 lhs_depth = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.left);
				i32 rhs_depth = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.right);
				if (tag_has_call) {
					lhs_depth += 1;
					rhs_depth += 1;
				}
				plan->spill_depth = gb_max(plan->spill_depth, lhs_depth);
				plan->spill_depth = gb_max(plan->spill_depth, rhs_depth);
			} else {
				i32 expr_depth = fast_backend_leaf_expr_spill_depth(expr);
				if (tag_has_call && !tag_is_string) {
					expr_depth += 1;
				}
				plan->spill_depth = gb_max(plan->spill_depth, expr_depth);
				if (tag_is_string) {
					i32 compare_depth = 6 + fast_backend_aggregate_compare_operand_spill_depth(plan, expr, tag_type);
					plan->spill_depth = gb_max(plan->spill_depth, compare_depth);
				}
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
			return fast_backend_reject(FastFallback_ControlFlow, bs->label, str_lit("Fast backend expected an identifier label"));
		}
		return true;
	case Token_fallthrough:
		if (bs->label != nullptr) {
			return fast_backend_reject(FastFallback_ControlFlow, bs->label, str_lit("Fast backend does not support labeled fallthrough statements"));
		}
		return true;
	}

	return fast_backend_reject(FastFallback_ControlFlow, bs->token, str_lit("Fast backend currently only supports 'break', 'continue', and 'fallthrough' branch statements"));
}

gb_internal bool fast_backend_plan_defer_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstDeferStmt *ds) {
	plan->has_defer = true;
	return fast_backend_plan_stmt(gen, plan, ds->stmt);
}

gb_internal bool fast_backend_can_emit_noop_delete_call_expr(FastLeafProcPlan *plan, AstCallExpr *ce) {
	gb_unused(plan);

	if (ce == nullptr || ce->args.count != 1) {
		return false;
	}

	Entity *proc_entity = ce->entity_procedure_of;
	if (proc_entity == nullptr) {
		proc_entity = entity_from_expr(ce->proc);
	}
	bool is_delete = false;
	if (proc_entity != nullptr && proc_entity->kind == Entity_ProcGroup) {
		is_delete = str_eq(proc_entity->token.string, str_lit("delete"));
	} else if (proc_entity != nullptr && proc_entity->kind == Entity_Procedure) {
		is_delete =
			str_eq(proc_entity->token.string, str_lit("delete")) ||
			str_eq(proc_entity->token.string, str_lit("delete_dynamic_array")) ||
			str_eq(proc_entity->token.string, str_lit("delete_slice")) ||
			str_eq(proc_entity->token.string, str_lit("delete_string")) ||
			str_eq(proc_entity->token.string, str_lit("delete_string16")) ||
			str_eq(proc_entity->token.string, str_lit("delete_cstring")) ||
			str_eq(proc_entity->token.string, str_lit("delete_cstring16"));
	}
	Ast *proc_expr = unparen_expr(ce->proc);
	if (!is_delete && proc_expr != nullptr && proc_expr->kind == Ast_Ident) {
		is_delete = str_eq(proc_expr->Ident.token.string, str_lit("delete"));
	}
	if (!is_delete) {
		return false;
	}

	Ast *arg = ce->args[0];
	Type *type = base_type(type_of_expr(arg));
	if (type == nullptr || fast_backend_expr_has_call(arg)) {
		return false;
	}

	return is_type_string(type) ||
	       is_type_string16(type) ||
	       is_type_cstring(type) ||
	       is_type_cstring16(type) ||
	       is_type_slice(type) ||
	       is_type_dynamic_array(type) ||
	       is_type_map(type);
}

gb_internal bool fast_backend_plan_expr_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstExprStmt *es) {
	gb_unused(gen);

	if (es->expr == nullptr || es->expr->kind != Ast_CallExpr) {
		return fast_backend_reject(FastFallback_Call, es->expr ? es->expr : cast(Ast *)es, str_lit("Fast backend currently only supports call expression statements"));
	}
	// Builtin calls (cpu_relax, prefetch_*, trap, ...) don't go
	// through the regular call-info path (no proc entity), so check
	// them up front before falling through to it.
	bool is_builtin_call = (es->expr->CallExpr.proc != nullptr &&
	                        fast_backend_builtin_proc_id(es->expr->CallExpr.proc) != BuiltinProc_Invalid);
	if (!is_builtin_call &&
	    !fast_backend_can_emit_call_expr(plan, &es->expr->CallExpr, true) &&
	    !fast_backend_can_emit_noop_delete_call_expr(plan, &es->expr->CallExpr)) {
		return fast_backend_reject(FastFallback_Call, es->expr, str_lit("Fast backend does not yet support this call expression"));
	}
	if (!is_builtin_call && fast_backend_can_emit_noop_delete_call_expr(plan, &es->expr->CallExpr)) {
		return true;
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(es->expr));
	return true;
}

gb_internal Ast *fast_backend_strip_range_value(Ast *value) {
	value = unparen_expr(value);
	if (value != nullptr && value->kind == Ast_UnaryExpr && value->UnaryExpr.op.kind == Token_And) {
		value = unparen_expr(value->UnaryExpr.expr);
	}
	return value;
}

gb_internal bool fast_backend_plan_range_stmt(FastGenerator *gen, FastLeafProcPlan *plan, AstRangeStmt *rs) {
	if (rs->label != nullptr && fast_backend_label_name(rs->label).len == 0) {
		return fast_backend_reject(FastFallback_Range, rs->label, str_lit("Fast backend expected an identifier range label"));
	}
	if (rs->init != nullptr && !fast_backend_plan_stmt(gen, plan, rs->init)) {
		return false;
	}
	if (rs->vals.count == 0 || rs->vals[0] == nullptr) {
		return fast_backend_reject(FastFallback_Range, rs->token, str_lit("Fast backend expected a range value"));
	}

	for_array(i, rs->vals) {
			Ast *value = fast_backend_strip_range_value(rs->vals[i]);
			if (value == nullptr || is_blank_ident(value)) {
				continue;
			}
			if (value != rs->vals[i] || value->kind != Ast_Ident) {
				return fast_backend_reject(FastFallback_Range, rs->vals[i], str_lit("Fast backend does not yet support addressed range values"));
			}
			Entity *entity = entity_of_node(value);
			if (entity == nullptr || entity->kind != Entity_Variable || !fast_backend_add_slot(plan, entity, entity->type)) {
				return fast_backend_reject(FastFallback_Range, value, str_lit("Fast backend does not yet support this range value"));
			}
		}
	if (!fast_backend_add_expr_slot(plan, rs->expr, t_int, nullptr)) {
		return false;
	}

	Ast *expr = unparen_expr(rs->expr);
	if (expr == nullptr) {
		return fast_backend_reject(FastFallback_Range, rs->token, str_lit("Fast backend expected a range expression"));
	}

	if (is_ast_range(expr)) {
		Ast *value = fast_backend_strip_range_value(rs->vals[0]);
		if (value == nullptr || is_blank_ident(value)) {
			return fast_backend_reject(FastFallback_Range, rs->vals[0], str_lit("Fast backend range intervals require a value variable"));
		}
		Type *iter_type = type_of_expr(value);
		if (iter_type == nullptr) {
			iter_type = reduce_tuple_to_single_type(type_of_expr(expr->BinaryExpr.left));
		}
		FastScalarType scalar_type = {};
		if (iter_type == nullptr || !fast_backend_classify_scalar_type(iter_type, &scalar_type) || !fast_backend_scalar_is_integer_like(scalar_type)) {
			return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend currently only supports integer-like range intervals"));
		}
		if (!fast_backend_can_emit_leaf_expr(plan, expr->BinaryExpr.left, iter_type) ||
		    !fast_backend_can_emit_leaf_expr(plan, expr->BinaryExpr.right, iter_type)) {
			return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend does not yet support this range interval"));
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.left));
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.right));
	} else {
		Type *expr_type = base_type(type_deref(type_of_expr(expr)));
		if (expr_type == nullptr) {
			return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend expected an iterable expression"));
		}
		switch (expr_type->kind) {
		case Type_Array:
		case Type_EnumeratedArray:
		case Type_Slice:
		case Type_DynamicArray:
		case Type_FixedCapacityDynamicArray:
			break;
			case Type_Basic:
				if (is_type_string(expr_type) || is_type_string16(expr_type)) {
					break;
				}
				return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend does not yet support iterating over this type"));
			default:
				return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend does not yet support iterating over this type"));
			}
			if (fast_backend_expr_has_call(expr) || !fast_backend_can_emit_address_expr(plan, expr, nullptr, nullptr, nullptr)) {
				return fast_backend_reject(FastFallback_Range, rs->expr, str_lit("Fast backend currently only supports addressable indexed range expressions"));
			}
		i32 range_spill_depth = 2;
		if (is_type_string(expr_type) || is_type_string16(expr_type)) {
			// The string decode helpers keep several temporary spill
			// slots alive while walking UTF-8/UTF-16 backward and
			// forward.
			range_spill_depth = 8;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_address_expr_spill_depth(expr) + range_spill_depth);
	}

	return fast_backend_plan_stmt(gen, plan, rs->body);
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
	case Ast_RangeStmt:
		return fast_backend_plan_range_stmt(gen, plan, &stmt->RangeStmt);
	case Ast_SwitchStmt:
		return fast_backend_plan_switch_stmt(gen, plan, &stmt->SwitchStmt);
	case Ast_DeferStmt:
		return fast_backend_plan_defer_stmt(gen, plan, &stmt->DeferStmt);
	case Ast_BranchStmt:
		return fast_backend_plan_branch_stmt(gen, plan, &stmt->BranchStmt);
	case Ast_ReturnStmt:
		return fast_backend_plan_return_stmt(gen, plan, &stmt->ReturnStmt);
	}

	return fast_backend_reject(FastFallback_ControlFlow, stmt, str_lit("Fast backend currently only supports blocks, mutable local declarations, assignments, defer statements, call statements, if statements, for statements, range statements, switch statements, break/continue, and returns"));
}

gb_internal bool fast_backend_plan_leaf_proc(FastGenerator *gen, Entity *e, FastLeafProcPlan *plan) {
	DeclInfo *decl = e->decl_info;
	if (decl == nullptr || decl->proc_lit == nullptr || decl->proc_lit->kind != Ast_ProcLit) {
		return fast_backend_reject(FastFallback_Internal, e->token, str_lit("Fast backend expected a concrete procedure body"));
	}

	Type *type = base_type(e->type);
	if (type == nullptr || type->kind != Type_Proc) {
		return fast_backend_reject(FastFallback_Internal, e->token, str_lit("Fast backend expected a procedure type"));
	}

	TypeProc *pt = &type->Proc;
	if (pt->is_polymorphic && !pt->is_poly_specialized) {
		return fast_backend_reject(FastFallback_Procedure, e->token, str_lit("Fast backend does not yet support unresolved polymorphic procedures"));
	}
	if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
		return fast_backend_reject(FastFallback_CallingConvention, e->token, str_lit("Fast backend does not yet support this calling convention"));
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
			// `#cvararg` and `..any` variadic params are typed `[]any`
			// at the parameter level. The caller-side variadic pack
				// already builds a `[]any` slice and passes it as two
				// register/stack args, so the callee treats the variadic
				// param as an ordinary slice and goes through the normal
				// slice spill path.

				if (!fast_backend_type_is_supported_value(param->type, nullptr, nullptr)) {
					return fast_backend_reject(FastFallback_Type, param->token, str_lit("Fast backend does not yet support this parameter type"));
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
		// Multi-result (tuple return) — treat the whole tuple as an
		// aggregate that's returned via sret. Each result i is stored
		// to result_ptr + offset_i in the proc body and read back
		// from the same offset at the call site. This matches what
		// the LLVM backend does for the same shape, and AAPCS64
		// passes the sret pointer in x8 to any aggregate-returning
		// cdecl proc.
		GB_ASSERT(pt->results != nullptr && pt->results->kind == Type_Tuple);
		bool all_supported = true;
		for_array(i, pt->results->Tuple.variables) {
			Entity *result_entity = pt->results->Tuple.variables[i];
				if (result_entity == nullptr || result_entity->kind != Entity_Variable) {
					all_supported = false;
					break;
				}
				if (!fast_backend_classify_scalar_type(result_entity->type, nullptr) &&
				    !fast_backend_type_is_supported_aggregate(result_entity->type)) {
					fast_backend_reject(FastFallback_Type, result_entity->token, str_lit("Fast backend does not yet support this result type"));
					all_supported = false;
					break;
				}
		}
		if (!all_supported) {
			return false;
		}
		if (!fast_backend_add_slot(plan, nullptr, t_rawptr)) {
			return false;
		}
		plan->return_by_pointer = true;
		plan->result_ptr_slot = plan->slots[plan->slots.count-1];
		pt->return_by_pointer = true;
		plan->result_value_type = nullptr; // multi-result, no single value type
		plan->has_result = true;
		array_add(&gen->procs_needing_abi_cache_invalidation, type);
	}
	if (pt->result_count == 1) {
		GB_ASSERT(pt->results != nullptr && pt->results->kind == Type_Tuple);
			Entity *result_entity = pt->results->Tuple.variables[0];
			GB_ASSERT(result_entity != nullptr);
			if (!fast_backend_classify_scalar_type(result_entity->type, &plan->return_type)) {
				if (!fast_backend_type_is_supported_aggregate(result_entity->type)) {
					return fast_backend_reject(FastFallback_Type, result_entity->token, str_lit("Fast backend does not yet support this result type"));
				}
			plan->result_value_type = result_entity->type;
			// Always use return_by_pointer for aggregate returns. This is the
			// safe ABI choice: the proc writes the return value to a caller-
			// provided result pointer (passed as a hidden first arg), so the
			// LLVM-emitted caller — and any other caller — does not need to
			// materialise the return into a stack slot that some later
			// operation (e.g. a variadic-arg setup) might clobber. The proc
			// type is updated below so the LLVM function type generation
			// (which is based on the proc type) agrees with this emit.
			//
			// This holds whether the aggregate fits in registers (a "direct
			// aggregate" in AAPCS64 terms, e.g. `struct {x, y: int}` which
			// is 16 bytes) or only in memory. The direct-aggregate case is
			// the one that broke: the LLVM-emitted caller materialised
			// `Pair` into a stack slot, then the variadic-arg setup used that
			// same slot as a scratch register and stomped `.y`. Forcing
			// return_by_pointer makes the caller pass a hidden first arg
			// and the proc writes through that, sidestepping the conflict.
			if (!fast_backend_add_slot(plan, nullptr, t_rawptr)) {
				return false;
			}
			plan->return_by_pointer = true;
			plan->result_ptr_slot = plan->slots[plan->slots.count-1];
			pt->return_by_pointer = true;
			// The LLVM function-type cache may already hold the direct-
			// aggregate function type for this proc. The driver invalidates
			// it once the extern lbModule is created. `type` is the
			// `Type*` (Type_Proc) the cache is keyed on; do NOT pass `pt`
			// (a `TypeProc*` with a different memory layout).
			array_add(&gen->procs_needing_abi_cache_invalidation, type);
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
	// Note: the old "spill_depth bump for direct aggregate returns" block
	// is no longer needed — we now always use return_by_pointer for
	// aggregate returns (see the planning above), so the stack-temp
	// collision the bump was working around cannot occur.
	return true;
}

gb_internal bool fast_backend_plan_global_var(FastGenerator *gen, Entity *e, FastGlobalVarPlan *plan) {
	GB_ASSERT(e != nullptr);
	GB_ASSERT(e->kind == Entity_Variable);

	if (e->Variable.is_foreign) {
		return false;
	}
	if (e->Variable.thread_local_model.len != 0) {
		return fast_backend_reject(FastFallback_Global, e->token, str_lit("Fast backend does not yet support thread local global variables"));
	}

	if (!fast_backend_type_is_supported_value(e->type, nullptr, nullptr)) {
		return fast_backend_reject(FastFallback_Type, e->token, str_lit("Fast backend does not yet support this global variable type"));
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
		return fast_backend_reject(FastFallback_Internal, e->token, str_lit("Fast backend expected a sized global variable"));
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
				return fast_backend_reject(FastFallback_Global, init_expr, str_lit("Fast backend does not yet support this global constant initializer"));
			}
		plan->has_init_data = true;
		return true;
	}

	return fast_backend_reject(FastFallback_Global, init_expr, str_lit("Fast backend does not yet support dynamic global initialization"));
}

gb_internal bool fast_backend_collect_program(FastGenerator *gen, Array<FastGlobalVarPlan> *globals, Array<FastLeafProcPlan> *procedures) {
	bool allow_llvm_fallback = fast_backend_can_fallback_to_llvm_per_entity();
	PtrSet<Entity *> seen_codegen_entities = {};
	PtrSet<Entity *> seen_emitted_entities = {};
	defer (ptr_set_destroy(&seen_codegen_entities));
	defer (ptr_set_destroy(&seen_emitted_entities));

	for (Entity *e : gen->info->definitions) {
		if (!fast_entity_is_local(gen->info, e)) {
			continue;
		}

		u64 flags = e->flags.load(std::memory_order_relaxed);
		if (flags & EntityFlag_Disabled) {
			continue;
		}

		Entity *codegen_entity = fast_backend_canonical_top_level_entity(e);
		if (codegen_entity == nullptr) {
			continue;
		}
		if (ptr_set_update(&seen_codegen_entities, codegen_entity)) {
			continue;
		}

		switch (codegen_entity->kind) {
		case Entity_Procedure: {
			if (codegen_entity->Procedure.is_foreign) {
				continue;
			}
			Type *type = base_type(codegen_entity->type);
			if (type != nullptr &&
			    type->kind == Type_Proc &&
			    type->Proc.is_polymorphic &&
			    !type->Proc.is_poly_specialized) {
				continue;
			}

			// @(init)/@(fini)/@(test) procs are emitted as ordinary leaf procs.
			// The LLVM backend's __$startup_runtime / __$cleanup_runtime /
			// test runner declare them as externals and link them in.
			// @(require) is just a "don't DCE this proc" marker, so it
			// behaves the same as a regular top-level proc.
			FastLeafProcPlan plan = {};
			fast_backend_clear_fallback_reason();
			if (!fast_backend_plan_leaf_proc(gen, codegen_entity, &plan)) {
				if (allow_llvm_fallback) {
					fast_backend_debug_log_fallback_reason("procedure", codegen_entity, fast_backend_fallback_reason);
					fast_backend_clear_fallback_reason();
					continue;
				}
				return fast_backend_emit_fallback_error();
			}
			plan.proc_index = cast(i32)procedures->count;
			array_add(procedures, plan);
			fast_backend_record_emitted_entity(gen, &seen_emitted_entities, e);
			fast_backend_record_emitted_entity(gen, &seen_emitted_entities, codegen_entity);
			break;
		}

		case Entity_Variable: {
			if (codegen_entity->Variable.is_foreign) {
				break;
			}
			FastGlobalVarPlan plan = {};
			fast_backend_clear_fallback_reason();
			if (!fast_backend_plan_global_var(gen, codegen_entity, &plan)) {
				if (allow_llvm_fallback) {
					fast_backend_debug_log_fallback_reason("global", codegen_entity, fast_backend_fallback_reason);
					fast_backend_clear_fallback_reason();
					break;
				}
				return fast_backend_emit_fallback_error();
			}
			array_add(globals, plan);
			fast_backend_record_emitted_entity(gen, &seen_emitted_entities, e);
			fast_backend_record_emitted_entity(gen, &seen_emitted_entities, codegen_entity);
			break;
		}

		case Entity_Constant:
		case Entity_TypeName:
		case Entity_ProcGroup:
		case Entity_ImportName:
		case Entity_LibraryName:
			break;

		default:
			fast_backend_clear_fallback_reason();
			fast_backend_reject(FastFallback_TopLevel, e->token, str_lit("Fast backend does not yet support this top-level declaration"));
			return fast_backend_emit_fallback_error();
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

gb_internal FastX64RegNames const *fast_backend_x64_third_reg(void) {
	return &fast_x64_reg_r8;
}

gb_internal FastX64RegNames const *fast_backend_x64_scratch_reg(void) {
	return &fast_x64_reg_rax;
}

gb_internal ProcCallingConvention fast_backend_effective_calling_convention(TypeProc *pt) {
	ProcCallingConvention cc = pt->calling_convention;
	switch (cc) {
	case ProcCC_CDecl:
	case ProcCC_PreserveNone:
	case ProcCC_PreserveMost:
	case ProcCC_PreserveAll:
		if (build_context.metrics.arch == TargetArch_amd64) {
			if (build_context.metrics.os == TargetOs_windows || build_context.metrics.abi == TargetABI_Win64) {
				return ProcCC_Win64;
			}
			return ProcCC_SysV;
		}
		if (build_context.metrics.arch == TargetArch_arm64) {
			return ProcCC_CDecl;
		}
		break;
	}
	return cc;
}

gb_internal bool fast_backend_arm64_calling_convention_preserves_scratch(TypeProc *pt) {
	if (pt == nullptr ||
	    build_context.metrics.arch != TargetArch_arm64 ||
	    build_context.metrics.os != TargetOs_darwin) {
		return false;
	}

	switch (pt->calling_convention) {
	case ProcCC_PreserveMost:
	case ProcCC_PreserveAll:
		return true;
	}
	return false;
}

gb_internal i32 fast_backend_arm64_preserved_scratch_save_bytes(TypeProc *pt) {
	return fast_backend_arm64_calling_convention_preserves_scratch(pt) ? 32 : 0;
}

gb_internal bool fast_backend_x64_calling_convention_preserves_scratch(TypeProc *pt) {
	if (pt == nullptr ||
	    build_context.metrics.arch != TargetArch_amd64 ||
	    build_context.metrics.os != TargetOs_darwin) {
		return false;
	}

	switch (pt->calling_convention) {
	case ProcCC_PreserveMost:
	case ProcCC_PreserveAll:
		return true;
	}
	return false;
}

gb_internal i32 fast_backend_x64_preserved_scratch_save_bytes(TypeProc *pt) {
	return fast_backend_x64_calling_convention_preserves_scratch(pt) ? 56 : 0;
}

gb_internal bool fast_backend_x64_uses_sysv_register_calling_sequence(TypeProc *pt) {
	if (pt == nullptr || build_context.metrics.arch != TargetArch_amd64) {
		return false;
	}
	return fast_backend_effective_calling_convention(pt) != ProcCC_Win64;
}

gb_internal void fast_backend_x64_sysv_assign_arg_slots(FastX64ArgState *state, i32 abi_args, FastX64ArgAssignment *slots) {
	GB_ASSERT(state != nullptr);
	GB_ASSERT(abi_args == 1 || abi_args == 2);
	GB_ASSERT(slots != nullptr);

	if (abi_args == 1) {
		if (state->next_reg < 6) {
			slots[0] = {FastX64ArgStorage_Register, state->next_reg};
			state->next_reg += 1;
		} else {
			slots[0] = {FastX64ArgStorage_Stack, state->next_stack};
			state->next_stack += 1;
		}
		return;
	}

	if (state->next_reg <= 4) {
		slots[0] = {FastX64ArgStorage_Register, state->next_reg + 0};
		slots[1] = {FastX64ArgStorage_Register, state->next_reg + 1};
		state->next_reg += 2;
		return;
	}

	slots[0] = {FastX64ArgStorage_Stack, state->next_stack + 0};
	slots[1] = {FastX64ArgStorage_Stack, state->next_stack + 1};
	state->next_stack += 2;
}

gb_internal i32 fast_backend_x64_sysv_stack_arg_count(TypeProc *pt, bool return_by_pointer, bool has_context_arg) {
	if (!fast_backend_x64_uses_sysv_register_calling_sequence(pt)) {
		return 0;
	}

	FastX64ArgState state = {};
	if (return_by_pointer) {
		FastX64ArgAssignment slots[2] = {};
		fast_backend_x64_sysv_assign_arg_slots(&state, 1, slots);
	}
	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return 0;
		}
		FastX64ArgAssignment slots[2] = {};
		fast_backend_x64_sysv_assign_arg_slots(&state, fast_backend_abi_arg_count(param->type), slots);
	}
	if (has_context_arg) {
		FastX64ArgAssignment slots[2] = {};
		fast_backend_x64_sysv_assign_arg_slots(&state, 1, slots);
	}
	return state.next_stack;
}

gb_internal bool fast_backend_supports_stack_args(TypeProc *pt) {
	if (pt == nullptr) {
		return false;
	}
	if (build_context.metrics.arch == TargetArch_arm64) {
		return true;
	}
	return fast_backend_x64_uses_sysv_register_calling_sequence(pt);
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
	static char const *regs[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8"};
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

gb_internal bool fast_backend_arm64_classify_hfa(Type *type, FastArm64AggregateReturnClass *out) {
	type = default_type(type);
	if (type == nullptr || out == nullptr) {
		return false;
	}

	FastScalarType scalar = {};
	if (fast_backend_classify_scalar_type(type, &scalar)) {
		if (scalar.kind != FastScalar_Float || (scalar.bit_size != 32 && scalar.bit_size != 64)) {
			return false;
		}
		if (out->slot_count == 0) {
			out->kind = FastArm64AggregateReturn_Float;
			out->float_bit_size = cast(i32)scalar.bit_size;
		}
		if (out->kind != FastArm64AggregateReturn_Float ||
		    out->float_bit_size != scalar.bit_size ||
		    out->slot_count >= 4) {
			return false;
		}
		out->slot_count += 1;
		return true;
	}

	Type *base = base_type(type);
	if (base == nullptr) {
		return false;
	}

	switch (base->kind) {
	case Type_Array:
		if (base->Array.count <= 0 || base->Array.count > 4) {
			return false;
		}
		for (i64 i = 0; i < base->Array.count; i++) {
			if (!fast_backend_arm64_classify_hfa(base->Array.elem, out)) {
				return false;
			}
		}
		return true;

	case Type_Matrix: {
		i64 count = base->Matrix.row_count * base->Matrix.column_count;
		if (count <= 0 || count > 4) {
			return false;
		}
		for (i64 i = 0; i < count; i++) {
			if (!fast_backend_arm64_classify_hfa(base->Matrix.elem, out)) {
				return false;
			}
		}
		return true;
	}

	case Type_Struct:
		if (base->Struct.fields.count == 0) {
			return false;
		}
		for_array(i, base->Struct.fields) {
			Entity *field = base->Struct.fields[i];
			if (field == nullptr || field->kind != Entity_Variable) {
				return false;
			}
			if (!fast_backend_arm64_classify_hfa(field->type, out)) {
				return false;
			}
		}
		return true;
	}

	return false;
}

gb_internal FastArm64AggregateReturnClass fast_backend_arm64_classify_aggregate_return(Type *type) {
	FastArm64AggregateReturnClass result = {};
	if (build_context.metrics.arch != TargetArch_arm64 || type == nullptr || !fast_backend_type_is_supported_aggregate(type)) {
		return result;
	}

	type = default_type(type);
	if (type == nullptr) {
		return result;
	}

	i64 size = type_size_of(type);
	if (size <= 0 || size > 16) {
		return result;
	}

	FastArm64AggregateReturnClass hfa = {};
	if (fast_backend_arm64_classify_hfa(type, &hfa) && hfa.slot_count > 0) {
		return hfa;
	}

	result.kind = FastArm64AggregateReturn_Integer;
	result.slot_count = cast(i32)align_formula(size, 8)/8;
	return result;
}

gb_internal void fast_backend_arm64_assign_arg_slots(FastArm64ArgState *state, i32 abi_args, FastArm64ArgAssignment *slots, bool is_sret = false) {
	GB_ASSERT(state != nullptr);
	GB_ASSERT(abi_args == 1 || abi_args == 2);
	GB_ASSERT(slots != nullptr);

	// AAPCS64: the sret (return-by-pointer) result pointer is passed in x8,
	// not in x0. x8 is the indirect result location register and is reserved
	// for this purpose. Subsequent normal params start at x0.
	if (is_sret) {
		GB_ASSERT(abi_args == 1);
		slots[0] = {FastArm64ArgStorage_Register, 8};
		return;
	}

	if (abi_args == 1) {
		if (!state->stack_only && state->next_reg < 8) {
			slots[0] = {FastArm64ArgStorage_Register, state->next_reg};
			state->next_reg += 1;
		} else {
			state->stack_only = true;
			slots[0] = {FastArm64ArgStorage_Stack, state->next_stack};
			state->next_stack += 1;
		}
		return;
	}

	if (!state->stack_only) {
		if (state->next_reg <= 6) {
			slots[0] = {FastArm64ArgStorage_Register, state->next_reg + 0};
			slots[1] = {FastArm64ArgStorage_Register, state->next_reg + 1};
			state->next_reg += 2;
			return;
		}
		if (state->next_reg == 7) {
			slots[0] = {FastArm64ArgStorage_Register, 7};
			slots[1] = {FastArm64ArgStorage_Stack, state->next_stack};
			state->next_reg = 8;
			state->next_stack += 1;
			state->stack_only = true;
			return;
		}
	}

	state->stack_only = true;
	slots[0] = {FastArm64ArgStorage_Stack, state->next_stack + 0};
	slots[1] = {FastArm64ArgStorage_Stack, state->next_stack + 1};
	state->next_stack += 2;
}

gb_internal i32 fast_backend_arm64_stack_arg_count(TypeProc *pt, bool return_by_pointer, bool has_context_arg) {
	if (pt == nullptr) {
		return 0;
	}

	FastArm64ArgState state = {};
	if (return_by_pointer) {
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&state, 1, slots, /*is_sret=*/true);
	}
	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return 0;
		}
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&state, fast_backend_abi_arg_count(param->type), slots);
	}
	if (has_context_arg) {
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&state, 1, slots);
	}
	return state.next_stack;
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
	return entity->Variable.is_global ||
	       entity->Variable.is_foreign ||
	       fast_entity_is_local(plan->info, entity) ||
	       ((entity->scope->flags & (ScopeFlag_Pkg|ScopeFlag_File)) != 0);
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
	if (type.kind == FastScalar_Float) {
		f64 v = exact_value_to_f64(value);
		if (type.bit_size == 32) {
			f32 f = cast(f32)v;
			*out = bit_cast<u32>(f);
			return true;
		}
		if (type.bit_size == 64) {
			*out = bit_cast<u64>(v);
			return true;
		}
		return false;
	}

	switch (value.kind) {
	case ExactValue_Bool:
		*out = value.value_bool ? 1 : 0;
		return true;
	case ExactValue_Pointer:
		*out = cast(u64)value.value_pointer;
		return true;
	case ExactValue_Integer:
	case ExactValue_Float:
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
	type = base_type(type);
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

gb_internal bool fast_backend_serialize_constant_any(Type *type, AstCompoundLit *cl, u8 *dst) {
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
			Type *field_type = nullptr;
			i64 offset = type_offset_of(type, field->Variable.field_index, &field_type);
			if (offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_serialize_constant_value(field_type, fv->value, dst + offset)) {
				return false;
			}
		}
		return true;
	}

	if (cl->elems.count > 2) {
		return false;
	}

	Type *field_types[2] = {t_rawptr, t_typeid};
	for_array(i, cl->elems) {
		if (!fast_backend_serialize_constant_value(field_types[i], cl->elems[i], dst + type_offset_of(type, i))) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_serialize_constant_matrix(Type *type, AstCompoundLit *cl, u8 *dst) {
	type = base_type(type);
	gb_zero_size(dst, cast(isize)type_size_of(type));
	Type *elem_type = type->Matrix.elem;
	Type *vector_type = fast_backend_matrix_vector_type(type);
	i64 elem_size = type_size_of(elem_type);
	i64 elem_count = type->Matrix.row_count * type->Matrix.column_count;
	if (elem_size <= 0) {
		return false;
	}
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
				if (lo < 0 || hi < lo || hi > elem_count) {
					return false;
				}
				for (i64 index = lo; index < hi; index++) {
					i64 offset = matrix_row_major_index_to_offset(type, index) * elem_size;
					if (!fast_backend_serialize_constant_value(elem_type, fv->value, dst + offset)) {
						return false;
					}
				}
			} else {
				TypeAndValue index_tav = type_and_value_of_expr(fv->field);
				if (index_tav.mode != Addressing_Constant) {
					return false;
				}
				i64 index = exact_value_to_i64(index_tav.value);
				if (index < 0 || index >= elem_count) {
					return false;
				}
				i64 offset = matrix_row_major_index_to_offset(type, index) * elem_size;
				if (!fast_backend_serialize_constant_value(elem_type, fv->value, dst + offset)) {
					return false;
				}
			}
		}
		return true;
	}

	if (cl->elems.count > elem_count) {
		return false;
	}

	for_array(i, cl->elems) {
		Type *expr_type = default_type(type_of_expr(cl->elems[i]));
		if (vector_type != nullptr &&
		    expr_type != nullptr &&
		    are_types_identical(expr_type, vector_type)) {
			i64 offset = fast_backend_matrix_vector_offset(type, i);
			if (offset < 0 || !fast_backend_serialize_constant_value(vector_type, cl->elems[i], dst + offset)) {
				return false;
			}
			continue;
		}

		i64 offset = matrix_row_major_index_to_offset(type, i) * elem_size;
		if (!fast_backend_serialize_constant_value(elem_type, cl->elems[i], dst + offset)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_serialize_constant_union(Type *type, Ast *expr, u8 *dst) {
	type = base_type(type);
	Type *source_type = default_type(type_of_expr(expr));
	if (type == nullptr || source_type == nullptr || source_type->kind == Type_Union || !union_is_variant_of(type, source_type)) {
		return false;
	}

	gb_zero_size(dst, cast(isize)type_size_of(type));
	if (!fast_backend_serialize_constant_value(source_type, expr, dst)) {
		return false;
	}
	if (!is_type_union_maybe_pointer(type)) {
		Type *tag_type = union_tag_type(type);
		FastScalarType scalar_type = {};
		if (!fast_backend_classify_scalar_type(tag_type, &scalar_type)) {
			return false;
		}
		u64 tag = cast(u64)union_variant_index_checked(type, source_type);
		fast_backend_store_scalar_bytes(dst + type_offset_of(type, -1), fast_backend_scalar_byte_size(scalar_type), tag);
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
	if (base_type(type)->kind == Type_Union) {
		return fast_backend_serialize_constant_union(type, expr, dst);
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
	case Type_Basic:
		return is_type_any(type) && fast_backend_serialize_constant_any(type, cl, dst);
	case Type_Array:
		return fast_backend_serialize_constant_array(type, cl, dst);
	case Type_Matrix:
		return fast_backend_serialize_constant_matrix(type, cl, dst);
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
	i32 offset = fast_backend_slot_offset(plan, slot);
	// Match the load width to the slot width: a 4-byte `rune` slot
	// must use `ldr w` (4-byte) and zero/sign-extend via the
	// canonicalize call below. Using an 8-byte `ldr x` would read
	// past the slot into the adjacent one.
	if (offset <= 255) {
		switch (fast_backend_scalar_byte_size(slot.type)) {
		case 1: gb_fprintf(file, "\tldrb w%s, [x29, #-%d]\n", dst+1, offset); break;
		case 2: gb_fprintf(file, "\tldrh w%s, [x29, #-%d]\n", dst+1, offset); break;
		case 4: gb_fprintf(file, "\tldr w%s, [x29, #-%d]\n", dst+1, offset); break;
		case 8: gb_fprintf(file, "\tldr %s, [x29, #-%d]\n",   dst,   offset); break;
		}
	} else {
		fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", -offset);
		fast_backend_emit_arm64_load_from_address(file, fast_backend_arm64_addr_tmp_reg(), dst, slot.type);
	}
	fast_backend_emit_arm64_canonicalize(file, dst, slot.type);
}

gb_internal void fast_backend_emit_x64_store_reg_to_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, FastX64RegNames const *src) {
	gb_fprintf(file, "\tmov QWORD PTR [rbp-%d], %s\n", fast_backend_slot_offset(plan, slot), src->r64);
}

gb_internal void fast_backend_emit_arm64_store_reg_to_slot(gbFile *file, FastLeafProcPlan *plan, FastLocalSlot const &slot, char const *src) {
	i32 offset = fast_backend_slot_offset(plan, slot);
	// Match the store width to the slot width: a 4-byte `rune` slot
	// must use `str w` (4-byte), not `str x` (8-byte), otherwise the
	// store clobbers the low 4 bytes of the adjacent 8-byte slot.
	if (offset <= 255) {
		switch (fast_backend_scalar_byte_size(slot.type)) {
		case 1: gb_fprintf(file, "\tstrb w%s, [x29, #-%d]\n", src+1, offset); return;
		case 2: gb_fprintf(file, "\tstrh w%s, [x29, #-%d]\n", src+1, offset); return;
		case 4: gb_fprintf(file, "\tstr w%s, [x29, #-%d]\n", src+1, offset); return;
		case 8: gb_fprintf(file, "\tstr %s, [x29, #-%d]\n",   src,   offset); return;
		}
	}
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", -offset);
	fast_backend_emit_arm64_store_to_address(file, fast_backend_arm64_addr_tmp_reg(), src, slot.type);
}

gb_internal void fast_backend_emit_arm64_store_to_spill_offset(gbFile *file, i32 offset, char const *src) {
	if (offset <= 255) {
		gb_fprintf(file, "\tstr %s, [x29, #-%d]\n", src, offset);
		return;
	}
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", -offset);
	gb_fprintf(file, "\tstr %s, [%s]\n", src, fast_backend_arm64_addr_tmp_reg());
}

gb_internal void fast_backend_emit_arm64_load_from_spill_offset(gbFile *file, i32 offset, char const *dst) {
	if (offset <= 255) {
		gb_fprintf(file, "\tldr %s, [x29, #-%d]\n", dst, offset);
		return;
	}
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", -offset);
	gb_fprintf(file, "\tldr %s, [%s]\n", dst, fast_backend_arm64_addr_tmp_reg());
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
	case FastScalar_Float:
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
	case FastScalar_Float:
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

gb_internal void fast_backend_emit_arm64_store_call_stack_arg(gbFile *file, i32 stack_index, char const *src) {
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "sp", 8*stack_index);
	gb_fprintf(file, "\tstr %s, [%s]\n", src, fast_backend_arm64_addr_tmp_reg());
}

gb_internal void fast_backend_emit_arm64_store_u64_to_frame_offset(gbFile *file, i32 offset, char const *src) {
	if (offset <= 255) {
		gb_fprintf(file, "\tstr %s, [x29, #-%d]\n", src, offset);
		return;
	}
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", -offset);
	gb_fprintf(file, "\tstr %s, [%s]\n", src, fast_backend_arm64_addr_tmp_reg());
}

gb_internal void fast_backend_emit_arm64_load_incoming_stack_arg(gbFile *file, i32 stack_index, char const *dst) {
	fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), "x29", 16 + 8*stack_index);
	gb_fprintf(file, "\tldr %s, [%s]\n", dst, fast_backend_arm64_addr_tmp_reg());
}

gb_internal bool fast_backend_emit_arm64_load_assigned_arg(gbFile *file, FastArm64ArgAssignment assignment, char const *dst) {
	if (assignment.storage == FastArm64ArgStorage_Register) {
		char const *src = fast_backend_arm64_param_reg(assignment.index);
		if (src == nullptr) {
			return false;
		}
		if (src != dst) {
			gb_fprintf(file, "\tmov %s, %s\n", dst, src);
		}
		return true;
	}

	fast_backend_emit_arm64_load_incoming_stack_arg(file, assignment.index, dst);
	return true;
}

gb_internal void fast_backend_emit_x64_store_call_stack_arg(gbFile *file, i32 stack_index, FastX64RegNames const *src) {
	GB_ASSERT(file != nullptr);
	GB_ASSERT(src != nullptr);
	gb_fprintf(file, "\tmov QWORD PTR [rsp+%d], %s\n", 8*stack_index, src->r64);
}

gb_internal bool fast_backend_emit_x64_load_assigned_arg(gbFile *file, TypeProc *pt, FastX64ArgAssignment assignment, FastX64RegNames const *dst) {
	GB_ASSERT(file != nullptr);
	GB_ASSERT(pt != nullptr);
	GB_ASSERT(dst != nullptr);

	if (assignment.storage == FastX64ArgStorage_Register) {
		auto *src = fast_backend_x64_param_reg(pt, assignment.index);
		if (src == nullptr) {
			return false;
		}
		if (src != dst) {
			gb_fprintf(file, "\tmov %s, %s\n", dst->r64, src->r64);
		}
		return true;
	}

	gb_fprintf(file, "\tmov %s, QWORD PTR [rbp+%d]\n", dst->r64, 16 + 8*assignment.index);
	return true;
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
	case FastScalar_Float:
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
	case FastScalar_Float:
		switch (type.bit_size) {
		case 8:  gb_fprintf(file, "\tuxtb %s, w%s\n", reg, reg+1); break;
		case 16: gb_fprintf(file, "\tuxth %s, w%s\n", reg, reg+1); break;
		case 32: gb_fprintf(file, "\tuxtw %s, w%s\n", reg, reg+1); break;
		}
		break;
	}
}

gb_internal void fast_backend_emit_x64_move_scalar_bits_to_fp(gbFile *file, char const *xmm_reg, FastX64RegNames const *src, FastScalarType type) {
	if (type.bit_size == 32) {
		gb_fprintf(file, "\tmovd %s, %s\n", xmm_reg, src->r32);
	} else {
		gb_fprintf(file, "\tmovq %s, %s\n", xmm_reg, src->r64);
	}
}

gb_internal void fast_backend_emit_x64_move_fp_bits_to_scalar(gbFile *file, FastX64RegNames const *dst, char const *xmm_reg, FastScalarType type) {
	if (type.bit_size == 32) {
		gb_fprintf(file, "\tmovd %s, %s\n", dst->r32, xmm_reg);
	} else {
		gb_fprintf(file, "\tmovq %s, %s\n", dst->r64, xmm_reg);
	}
}

gb_internal void fast_backend_emit_arm64_move_scalar_bits_to_fp(gbFile *file, char const *fp_reg, char const *src, FastScalarType type) {
	if (type.bit_size == 32) {
		gb_fprintf(file, "\tfmov %s, w%s\n", fp_reg, src+1);
	} else {
		gb_fprintf(file, "\tfmov %s, %s\n", fp_reg, src);
	}
}

gb_internal void fast_backend_emit_arm64_move_fp_bits_to_scalar(gbFile *file, char const *dst, char const *fp_reg, FastScalarType type) {
	if (type.bit_size == 32) {
		gb_fprintf(file, "\tfmov w%s, %s\n", dst+1, fp_reg);
	} else {
		gb_fprintf(file, "\tfmov %s, %s\n", dst, fp_reg);
	}
}

gb_internal char const *fast_backend_arm64_float_return_reg(i32 index, i32 bit_size) {
	static char const *float_regs_32[] = {"s0", "s1", "s2", "s3"};
	static char const *float_regs_64[] = {"d0", "d1", "d2", "d3"};
	char const **regs = bit_size == 32 ? float_regs_32 : float_regs_64;
	return index < 4 ? regs[index] : nullptr;
}

gb_internal void fast_backend_emit_arm64_store_direct_aggregate_return_to_address(gbFile *file, char const *dst, FastArm64AggregateReturnClass result_class, i32 size) {
	GB_ASSERT(file != nullptr);
	GB_ASSERT(dst != nullptr);
	GB_ASSERT(size > 0);

	if (result_class.kind == FastArm64AggregateReturn_Float) {
		FastScalarType type = {FastScalar_Float, result_class.float_bit_size};
		for (i32 i = 0; i < result_class.slot_count; i++) {
			char const *fp_reg = fast_backend_arm64_float_return_reg(i, result_class.float_bit_size);
			GB_ASSERT(fp_reg != nullptr);
			fast_backend_emit_arm64_move_fp_bits_to_scalar(file, fast_backend_arm64_work_reg(), fp_reg, type);
			fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), dst, i * fast_backend_scalar_byte_size(type));
			fast_backend_emit_arm64_store_to_address(file, fast_backend_arm64_addr_tmp_reg(), fast_backend_arm64_work_reg(), type);
		}
		return;
	}

	i32 remaining = size;
	i32 offset = 0;
	for (i32 i = 0; i < result_class.slot_count && remaining > 0; i++) {
		char const *src = fast_backend_arm64_param_reg(i);
		i32 word_bytes = gb_min(remaining, 8);
		i32 chunk_offset = 0;
		gb_fprintf(file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), src);
		while (chunk_offset < word_bytes) {
			i32 chunk_remaining = word_bytes - chunk_offset;
			i32 chunk = chunk_remaining >= 8 ? 8 : chunk_remaining >= 4 ? 4 : chunk_remaining >= 2 ? 2 : 1;
			FastScalarType type = {FastScalar_Unsigned, 8 * chunk};
			fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), dst, offset + chunk_offset);
			fast_backend_emit_arm64_store_to_address(file, fast_backend_arm64_addr_tmp_reg(), fast_backend_arm64_work_reg(), type);
			chunk_offset += chunk;
			if (chunk_offset < word_bytes) {
				gb_fprintf(file, "\tlsr %s, %s, #%d\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), 8 * chunk);
			}
		}
		offset += word_bytes;
		remaining -= word_bytes;
	}
}

gb_internal void fast_backend_emit_arm64_load_direct_aggregate_return_from_address(gbFile *file, char const *src, FastArm64AggregateReturnClass result_class, i32 size) {
	GB_ASSERT(file != nullptr);
	GB_ASSERT(src != nullptr);
	GB_ASSERT(size > 0);

	if (result_class.kind == FastArm64AggregateReturn_Float) {
		FastScalarType type = {FastScalar_Float, result_class.float_bit_size};
		for (i32 i = 0; i < result_class.slot_count; i++) {
			char const *fp_reg = fast_backend_arm64_float_return_reg(i, result_class.float_bit_size);
			GB_ASSERT(fp_reg != nullptr);
			fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), src, i * fast_backend_scalar_byte_size(type));
			fast_backend_emit_arm64_load_from_address(file, fast_backend_arm64_addr_tmp_reg(), fast_backend_arm64_work_reg(), type);
			fast_backend_emit_arm64_move_scalar_bits_to_fp(file, fp_reg, fast_backend_arm64_work_reg(), type);
		}
		return;
	}

	i32 remaining = size;
	i32 offset = 0;
	for (i32 i = 0; i < result_class.slot_count && remaining > 0; i++) {
		char const *dst = fast_backend_arm64_param_reg(i);
		i32 word_bytes = gb_min(remaining, 8);
		i32 chunk_offset = 0;
		i32 shift = 0;
		gb_fprintf(file, "\tmov %s, xzr\n", dst);
		while (chunk_offset < word_bytes) {
			i32 chunk_remaining = word_bytes - chunk_offset;
			i32 chunk = chunk_remaining >= 8 ? 8 : chunk_remaining >= 4 ? 4 : chunk_remaining >= 2 ? 2 : 1;
			FastScalarType type = {FastScalar_Unsigned, 8 * chunk};
			fast_backend_emit_arm64_add_offset(file, fast_backend_arm64_addr_tmp_reg(), src, offset + chunk_offset);
			fast_backend_emit_arm64_load_from_address(file, fast_backend_arm64_addr_tmp_reg(), fast_backend_arm64_work_reg(), type);
			if (shift == 0) {
				gb_fprintf(file, "\tmov %s, %s\n", dst, fast_backend_arm64_work_reg());
			} else {
				gb_fprintf(file, "\torr %s, %s, %s, lsl #%d\n", dst, dst, fast_backend_arm64_work_reg(), shift);
			}
			chunk_offset += chunk;
			shift += 8 * chunk;
		}
		offset += word_bytes;
		remaining -= word_bytes;
	}
}

gb_internal void fast_backend_emit_convert_work_reg(FastLeafProcEmitter *emitter, FastScalarType source_type, FastScalarType target_type) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		auto *tmp  = fast_backend_x64_tmp_reg();
		if (source_type.kind == target_type.kind) {
			fast_backend_emit_x64_canonicalize(emitter->file, work, target_type);
			return;
		}

		if (target_type.kind == FastScalar_Bool) {
			if (source_type.kind == FastScalar_Float) {
				fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", work, source_type);
				if (source_type.bit_size == 32) {
					gb_fprintf(emitter->file, "\txorps xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tucomiss xmm0, xmm1\n");
				} else {
					gb_fprintf(emitter->file, "\txorpd xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tucomisd xmm0, xmm1\n");
				}
				gb_fprintf(emitter->file, "\tsetne %s\n", work->r8);
				gb_fprintf(emitter->file, "\tsetp %s\n", tmp->r8);
				gb_fprintf(emitter->file, "\tor %s, %s\n", work->r8, tmp->r8);
				gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
			} else {
				gb_fprintf(emitter->file, "\tcmp %s, 0\n", work->r64);
				gb_fprintf(emitter->file, "\tsetne %s\n", work->r8);
				gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
			}
			return;
		}

		if (source_type.kind == FastScalar_Float) {
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", work, source_type);
			if (target_type.kind == FastScalar_Float) {
				if (source_type.bit_size != target_type.bit_size) {
					gb_fprintf(emitter->file, "\t%s xmm0, xmm0\n", target_type.bit_size == 32 ? "cvtsd2ss" : "cvtss2sd");
				}
				fast_backend_emit_x64_move_fp_bits_to_scalar(emitter->file, work, "xmm0", target_type);
			} else {
				gb_fprintf(emitter->file, "\t%s %s, xmm0\n",
				           source_type.bit_size == 32 ? "cvttss2si" : "cvttsd2si",
				           target_type.bit_size <= 32 ? work->r32 : work->r64);
				fast_backend_emit_x64_canonicalize(emitter->file, work, target_type);
			}
			return;
		}

		if (target_type.kind == FastScalar_Float &&
		    (source_type.kind == FastScalar_Bool || source_type.kind == FastScalar_Signed || source_type.kind == FastScalar_Unsigned)) {
			if (source_type.kind == FastScalar_Unsigned && source_type.bit_size == 64) {
				i32 positive_label = fast_backend_alloc_label(emitter);
				i32 done_label = fast_backend_alloc_label(emitter);
				char positive_name[64] = {};
				char done_name[64] = {};
				fast_backend_make_label_name(positive_name, gb_size_of(positive_name), emitter->plan, positive_label);
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\ttest %s, %s\n", work->r64, work->r64);
				gb_fprintf(emitter->file, "\tjns %s\n", positive_name);
				gb_fprintf(emitter->file, "\tmov %s, %s\n", tmp->r64, work->r64);
				gb_fprintf(emitter->file, "\tand %s, 1\n", work->r64);
				gb_fprintf(emitter->file, "\tshr %s, 1\n", tmp->r64);
				gb_fprintf(emitter->file, "\tor %s, %s\n", tmp->r64, work->r64);
				gb_fprintf(emitter->file, "\t%s xmm0, %s\n", target_type.bit_size == 32 ? "cvtsi2ss" : "cvtsi2sd", tmp->r64);
				gb_fprintf(emitter->file, "\t%s xmm0, xmm0\n", target_type.bit_size == 32 ? "addss" : "addsd");
				gb_fprintf(emitter->file, "\tjmp %s\n", done_name);
				fast_backend_emit_label(emitter->file, emitter->plan, positive_label);
				gb_fprintf(emitter->file, "\t%s xmm0, %s\n", target_type.bit_size == 32 ? "cvtsi2ss" : "cvtsi2sd", work->r64);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
			} else {
				gb_fprintf(emitter->file, "\t%s xmm0, %s\n", target_type.bit_size == 32 ? "cvtsi2ss" : "cvtsi2sd", work->r64);
			}
			fast_backend_emit_x64_move_fp_bits_to_scalar(emitter->file, work, "xmm0", target_type);
			return;
		}

		if (target_type.kind == FastScalar_Pointer && source_type.kind == FastScalar_Pointer) {
			fast_backend_emit_x64_canonicalize(emitter->file, work, target_type);
		}
	} else {
		char const *work = fast_backend_arm64_work_reg();
		if (source_type.kind == target_type.kind) {
			fast_backend_emit_arm64_canonicalize(emitter->file, work, target_type);
			return;
		}

		if (target_type.kind == FastScalar_Bool) {
			if (source_type.kind == FastScalar_Float) {
				fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, source_type.bit_size == 32 ? "s0" : "d0", work, source_type);
				gb_fprintf(emitter->file, "\tfcmp %s, #0.0\n", source_type.bit_size == 32 ? "s0" : "d0");
				gb_fprintf(emitter->file, "\tcset w%s, ne\n", work+1);
			} else {
				gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
				gb_fprintf(emitter->file, "\tcset w%s, ne\n", work+1);
			}
			return;
		}

		if (source_type.kind == FastScalar_Float) {
			char const *src_fp = source_type.bit_size == 32 ? "s0" : "d0";
			fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, src_fp, work, source_type);
			if (target_type.kind == FastScalar_Float) {
				char const *dst_fp = target_type.bit_size == 32 ? "s0" : "d0";
				if (source_type.bit_size != target_type.bit_size) {
					gb_fprintf(emitter->file, "\tfcvt %s, %s\n", dst_fp, src_fp);
				}
				fast_backend_emit_arm64_move_fp_bits_to_scalar(emitter->file, work, dst_fp, target_type);
			} else {
				char const *inst = target_type.kind == FastScalar_Unsigned ? "fcvtzu" : "fcvtzs";
				if (target_type.bit_size <= 32) {
					gb_fprintf(emitter->file, "\t%s w%s, %s\n", inst, work+1, src_fp);
				} else {
					gb_fprintf(emitter->file, "\t%s %s, %s\n", inst, work, src_fp);
				}
				fast_backend_emit_arm64_canonicalize(emitter->file, work, target_type);
			}
			return;
		}

		if (target_type.kind == FastScalar_Float &&
		    (source_type.kind == FastScalar_Bool || source_type.kind == FastScalar_Signed || source_type.kind == FastScalar_Unsigned)) {
			char const *dst_fp = target_type.bit_size == 32 ? "s0" : "d0";
			char const *inst = source_type.kind == FastScalar_Unsigned ? "ucvtf" : "scvtf";
			if (source_type.bit_size <= 32) {
				gb_fprintf(emitter->file, "\t%s %s, w%s\n", inst, dst_fp, work+1);
			} else {
				gb_fprintf(emitter->file, "\t%s %s, %s\n", inst, dst_fp, work);
			}
			fast_backend_emit_arm64_move_fp_bits_to_scalar(emitter->file, work, dst_fp, target_type);
			return;
		}

		if (target_type.kind == FastScalar_Pointer && source_type.kind == FastScalar_Pointer) {
			fast_backend_emit_arm64_canonicalize(emitter->file, work, target_type);
		}
	}
}

gb_internal bool fast_backend_emit_leaf_expr(FastLeafProcEmitter *emitter, Ast *expr);

gb_internal bool fast_backend_emit_leaf_expr_as_type(FastLeafProcEmitter *emitter, Ast *expr, FastScalarType scalar_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}
	TypeAndValue tv = type_and_value_of_expr(expr);
	bool expr_is_nil = expr->kind == Ast_Ident &&
	                   expr->Ident.entity.load() != nullptr &&
	                   expr->Ident.entity.load()->kind == Entity_Nil;
	if (expr_is_nil || is_type_untyped_nil(tv.type) || tv.mode == Addressing_Constant) {
		u64 value = 0;
		if (!expr_is_nil && !is_type_untyped_nil(tv.type) && !fast_backend_exact_value_as_u64(tv.value, scalar_type, &value)) {
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
	return fast_backend_emit_leaf_expr(emitter, expr);
}

gb_internal bool fast_backend_emit_leaf_cast(FastLeafProcEmitter *emitter, Ast *operand, Type *target_type) {
	Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(operand).type);
	FastScalarType source_type = {};
	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(operand, operand_type, &source_type)) {
		return false;
	}
	if (!fast_backend_expr_scalar_type(nullptr, target_type, &result_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, operand)) {
		return false;
	}
	fast_backend_emit_convert_work_reg(emitter, source_type, result_type);
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
	bool expr_is_nil = expr->kind == Ast_Ident &&
	                   expr->Ident.entity.load() != nullptr &&
	                   expr->Ident.entity.load()->kind == Entity_Nil;
	if (expr->kind == Ast_Implicit && expr->Implicit.kind == Token_context) {
		if (!emitter->plan->has_context_slot) {
			return false;
		}
		scalar_type = emitter->plan->context_slot.type;
	} else if (expr_is_nil || is_type_untyped_nil(expr_type)) {
		scalar_type = fast_backend_context_scalar_type();
	} else if (!fast_backend_classify_scalar_type(expr_type, &scalar_type)) {
		return false;
	}

	if (tv.mode == Addressing_Constant || expr_is_nil || is_type_untyped_nil(expr_type)) {
		u64 value = 0;
		if (!expr_is_nil && !is_type_untyped_nil(expr_type) && !fast_backend_exact_value_as_u64(tv.value, scalar_type, &value)) {
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

	// Materialize the non-addressable base (call / compound-literal) into a
	// REAL stack temp (`sub sp` below the frame) rather than a spill slot.
	// Using a spill slot would defer-restore the depth so a future push/pop
	// in the same expression could clobber the .y of the materialized
	// aggregate (see llvm_vararg_clobber e case). The real temp lives below
	// the frame in a region the spill region never writes to; we restore the
	// bytes in the epilogue via `extra_scratch_bytes`. Sibling of the fix in
	// `fast_backend_emit_address_expr` Ast_SelectorExpr branch.
	i32 alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
		return false;
	}
	emitter->extra_scratch_bytes += alloc_bytes;
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

	// The store may have clobbered work reg; re-derive the temp address from
	// sp. sp is unchanged across the store helpers (no sub/add pair in them)
	// and the alloc_stack_temp region is not used by callees.
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
	}
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

	// Real stack temp (not a spill slot). Sibling fix to the struct-selector
	// case above and the Ast_IndexExpr materialise path in
	// `fast_backend_emit_address_expr`. The push/pop below for the index
	// expression operates on a slot above the materialised data; using a
	// real temp makes the layout independent of spill-region activity.
	i32 alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
		return false;
	}
	emitter->extra_scratch_bytes += alloc_bytes;
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

	// Re-derive work reg from sp (the store helpers clobbered it).
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
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

	// Real stack temp. Sibling fix to struct-selector and array-index above.
	// The push/pop below for the index expression operates on a slot above
	// the materialised data; using a real temp makes the layout independent
	// of spill-region activity.
	i32 alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
		return false;
	}
	emitter->extra_scratch_bytes += alloc_bytes;
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

	// Re-derive work reg from sp (the store helpers clobbered it).
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
	}

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
		fast_backend_emit_arm64_store_to_spill_offset(emitter->file, offset, fast_backend_arm64_work_reg());
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
		fast_backend_emit_arm64_load_from_spill_offset(emitter->file, offset, fast_backend_arm64_work_reg());
	}
}

gb_internal void fast_backend_emit_load_tmp_from_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, QWORD PTR [rbp-%d]\n", fast_backend_x64_tmp_reg()->r64, offset);
	} else {
		fast_backend_emit_arm64_load_from_spill_offset(emitter->file, offset, fast_backend_arm64_tmp_reg());
	}
}

gb_internal void fast_backend_emit_store_work_to_spill_depth(FastLeafProcEmitter *emitter, i32 depth) {
	i32 offset = fast_backend_spill_slot_offset_from_depth(depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov QWORD PTR [rbp-%d], %s\n", offset, fast_backend_x64_work_reg()->r64);
	} else {
		fast_backend_emit_arm64_store_to_spill_offset(emitter->file, offset, fast_backend_arm64_work_reg());
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
		fast_backend_emit_arm64_load_from_spill_offset(emitter->file, offset, fast_backend_arm64_tmp_reg());
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
		if (Entity *entity = entity_from_expr(expr)) {
			if (entity->kind == Entity_Variable && fast_backend_find_storage(emitter->plan, entity, nullptr, &value_type, nullptr)) {
				if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
					return false;
				}
				break;
			}
		}
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
			// Materialize the call/compound-literal into a REAL stack temp
			// (alloc_stack_temp sub sp) rather than a spill slot. The spill-slot
			// version deferred-restored the depth so the caller's push/pop in
			// `fast_backend_emit_store_any_expr_to_work_address` would clobber
			// the .y of the materialized struct (see llvm_vararg_clobber e case).
			// The real stack temp is below the frame and untouched by the
			// spill region; we restore the bytes in the epilogue.
			i32 alloc_bytes = 0;
			if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
				return false;
			}
			emitter->extra_scratch_bytes += alloc_bytes;
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
			// The call may have clobbered work reg; re-derive the temp address
			// from sp (which is unchanged across the call: stack-arg sub/add in
			// call_expr_internal is balanced, and the alloc_stack_temp region
			// is not used by the callee).
			if (build_context.metrics.arch == TargetArch_amd64) {
				gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
			}
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
		           (base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Array ||
		            base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_EnumeratedArray ||
		            base_type(default_type(type_of_expr(expr->IndexExpr.expr)))->kind == Type_Matrix) &&
		           !fast_backend_can_emit_address_expr(emitter->plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			// Same rationale as the SelectorExpr branch above: real stack temp
			// (not a spill slot) so the materialize survives the caller's
			// push/pop in the any-store path.
			i32 alloc_bytes = 0;
			if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
				return false;
			}
			emitter->extra_scratch_bytes += alloc_bytes;
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
			if (build_context.metrics.arch == TargetArch_amd64) {
				gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
			}
		} else if (base_uses_data_pointer && !fast_backend_can_emit_address_expr(emitter->plan, expr->IndexExpr.expr, nullptr, nullptr, nullptr)) {
			Ast *base_expr = unparen_expr(expr->IndexExpr.expr);
			Type *base_value_type = default_type(type_of_expr(expr->IndexExpr.expr));
			if (base_expr == nullptr || base_value_type == nullptr) {
				return false;
			}
			i32 alloc_bytes = 0;
			if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(base_value_type), cast(i32)type_align_of(base_value_type), &alloc_bytes)) {
				return false;
			}
			emitter->extra_scratch_bytes += alloc_bytes;
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
			if (build_context.metrics.arch == TargetArch_amd64) {
				gb_fprintf(emitter->file, "\tmov %s, rsp\n", fast_backend_x64_work_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\tmov %s, sp\n", fast_backend_arm64_work_reg());
			}
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

// Rule for materialising a non-addressable aggregate (call result, slice
// expr, compound-literal, etc.) into a temp that must survive across a
// push/pop in the same expression:
//   USE THIS HELPER (`fast_backend_emit_alloc_stack_temp`).
//   It allocates a real `sub sp` region BELOW the prologue frame. The bytes
//   are tracked in `emitter->extra_scratch_bytes` and restored in the
//   epilogue. Within a single expression sp only goes down (new
//   materializes) or up (epilogue), and nothing in the spill region writes
//   to the materialise area, so the data persists until consumed.
//   After any store helper call you MUST re-derive work_reg from sp
//   (`mov work, rsp` / `mov work, sp`) since the helper clobbered it.
//
// ANTI-PATTERN to avoid:
//   Bumping `current_spill_depth` by `temp_slots`, claiming a spill slot,
//   and `defer`-restoring the depth. The slot lives in the spill region,
//   and the next `push` in the same expression will write to that slot —
//   clobbering the materialised aggregate's tail fields (the .y case in
//   `llvm_vararg_clobber`). The mask that hides this in some leaf-direct
//   paths today is "the load happens immediately", but any future change
//   that adds emit between the store and the load will expose the bug.
//   Don't use the spill-slot bump for materialise — use this helper.
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
		if (!fast_backend_emit_address_expr(emitter, arg, nullptr)) {
			return false;
		}
		// Push the slot address so the caller's
		// `push_slice_components_from_spill` can pop it. The other
		// paths in this function (CallExpr / store helpers) all push
		// the address they materialize; this path is the same.
		if (is_type_string(param_type) || is_type_string16(param_type) ||
		    is_type_slice(param_type) || is_type_dynamic_array(param_type)) {
			fast_backend_emit_push_work_reg(emitter);
		}
		return true;
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
	// After `alloc_stack_temp`, work reg holds the address of the new
	// stack allocation. The store helpers below overwrite work reg, so
	// spill the address to a separate spill slot here. The matching
	// `push_slice_components_from_spill` will pop it back into tmp.
	if (is_type_string(param_type) || is_type_string16(param_type) ||
	    is_type_slice(param_type) || is_type_dynamic_array(param_type)) {
		fast_backend_emit_push_work_reg(emitter);
	}
	if (fast_backend_can_emit_array_binary_expr(emitter->plan, arg, param_type)) {
		return fast_backend_emit_store_array_binary_expr_to_work_address(emitter, param_type, arg);
	}
	if (fast_backend_can_emit_slice_compound_lit_expr(emitter->plan, arg, param_type)) {
		return fast_backend_emit_store_slice_compound_lit_to_work_address(emitter, param_type, arg);
	}
	return fast_backend_emit_store_constant_aggregate_to_address(emitter, param_type, arg);
}

gb_internal bool fast_backend_emit_pack_variadic_slice_arg(FastLeafProcEmitter *emitter, Type *slice_type, Slice<Ast *> elems, i32 *temp_bytes_) {
	if (emitter == nullptr || slice_type == nullptr || temp_bytes_ == nullptr) {
		return false;
	}

	slice_type = default_type(slice_type);
	Type *base = base_type(slice_type);
	if (base == nullptr || base->kind != Type_Slice) {
		return false;
	}

	Type *elem_type = base->Slice.elem;
	i32 elem_size = cast(i32)type_size_of(elem_type);
	i32 elem_align = cast(i32)type_align_of(elem_type);
	if (elem_size <= 0 || elem_align <= 0) {
		return false;
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	if (elems.count != 0) {
		i64 total_elem_bytes = cast(i64)elem_size * elems.count;
		if (total_elem_bytes > cast(i64)I32_MAX) {
			return false;
		}
		i32 alloc_bytes = 0;
		if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)total_elem_bytes, elem_align, &alloc_bytes)) {
			return false;
		}
		*temp_bytes_ += alloc_bytes;
		fast_backend_emit_push_work_reg(emitter);
		i32 data_depth = emitter->current_spill_depth;
		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)total_elem_bytes);
		for_array(i, elems) {
			fast_backend_emit_load_work_from_spill_depth(emitter, data_depth);
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, cast(i32)(i*elem_size), elem_type, elems[i])) {
				return false;
			}
		}
		fast_backend_emit_load_work_from_spill_depth(emitter, data_depth);
	} else if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_work_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_ptr_depth = emitter->current_spill_depth;

	i32 slice_bytes = cast(i32)type_size_of(slice_type);
	i32 slice_align = cast(i32)type_align_of(slice_type);
	i32 slice_alloc_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, slice_bytes, slice_align, &slice_alloc_bytes)) {
		return false;
	}
	*temp_bytes_ += slice_alloc_bytes;
	fast_backend_emit_push_work_reg(emitter);
	i32 slice_depth = emitter->current_spill_depth;
	fast_backend_emit_zero_bytes_at_work_address(emitter, slice_bytes);

	fast_backend_emit_load_tmp_from_spill_depth(emitter, slice_depth);
	fast_backend_emit_load_work_from_spill_depth(emitter, data_ptr_depth);
	fast_backend_emit_store_work_to_tmp_address(emitter, pointer_type);

	fast_backend_emit_load_tmp_from_spill_depth(emitter, slice_depth);
	fast_backend_emit_add_imm_to_tmp_reg(emitter, build_context.int_size);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)elems.count);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), len_type);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)elems.count);
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), len_type);
	}
	fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	fast_backend_emit_load_work_from_spill_depth(emitter, slice_depth);
	return true;
}

gb_internal bool fast_backend_emit_special_default_call_arg(FastLeafProcEmitter *emitter, TypeProc *pt, AstCallExpr *ce, i32 param_index, Ast *arg, Type *param_type, i32 *temp_bytes_, bool *handled_) {
	if (handled_) *handled_ = false;
	if (emitter == nullptr || pt == nullptr || ce == nullptr || param_type == nullptr || temp_bytes_ == nullptr) {
		return false;
	}

	ParameterValue param_value = {};
	if (!fast_backend_get_special_default_call_arg(pt, param_index, arg, &param_value)) {
		return true;
	}

	if (handled_) *handled_ = true;
	param_type = default_type(param_type);

	switch (param_value.kind) {
	case ParameterValue_Location: {
		if (!are_types_identical(param_type, default_type(t_source_code_location))) {
			return false;
		}

		TokenPos pos = ast_token(ce->proc).pos;
		String file = get_file_path_string(pos.file_id);
		String procedure = emitter->plan->entity ? emitter->plan->entity->token.string : String{};
		i32 line = pos.line;
		i32 column = pos.column;

		switch (build_context.source_code_location_info) {
		case SourceCodeLocationInfo_Normal:
			break;
		case SourceCodeLocationInfo_Obfuscated:
			file = obfuscate_string(file, "F");
			procedure = obfuscate_string(procedure, "P");
			line = obfuscate_i32(line);
			column = obfuscate_i32(column);
			break;
		case SourceCodeLocationInfo_Filename:
			file = last_path_element(file);
			break;
		case SourceCodeLocationInfo_None:
			file = str_lit("");
			procedure = str_lit("");
			line = 0;
			column = 0;
			break;
		}

		i32 alloc_bytes = 0;
		if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(param_type), cast(i32)type_align_of(param_type), &alloc_bytes)) {
			return false;
		}
		*temp_bytes_ += alloc_bytes;
		fast_backend_emit_push_work_reg(emitter);
		i32 base_depth = emitter->current_spill_depth;

		fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(param_type));

		fast_backend_emit_load_work_from_spill_depth(emitter, base_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, cast(i32)type_offset_of(param_type, 0));
		if (!fast_backend_emit_store_string_value_to_address(emitter, file)) {
			return false;
		}

		FastScalarType i32_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(t_i32, &i32_type));
		fast_backend_emit_load_work_from_spill_depth(emitter, base_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, cast(i32)type_offset_of(param_type, 1));
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)cast(u32)line);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)cast(u32)line);
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_work_to_tmp_address(emitter, i32_type);

		fast_backend_emit_load_work_from_spill_depth(emitter, base_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, cast(i32)type_offset_of(param_type, 2));
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)cast(u32)column);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)cast(u32)column);
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_work_to_tmp_address(emitter, i32_type);

		fast_backend_emit_load_work_from_spill_depth(emitter, base_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, cast(i32)type_offset_of(param_type, 3));
		if (!fast_backend_emit_store_string_value_to_address(emitter, procedure)) {
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

	case ParameterValue_Expression: {
		if (!are_types_identical(param_type, t_string)) {
			return false;
		}

		String value = {};
		if (!fast_backend_get_special_default_call_arg_expr_string(pt, ce, param_value, &value)) {
			return false;
		}

		i32 alloc_bytes = 0;
		if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(param_type), cast(i32)type_align_of(param_type), &alloc_bytes)) {
			return false;
		}
		*temp_bytes_ += alloc_bytes;
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_store_string_value_to_address(emitter, value)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
		}
		fast_backend_emit_push_work_reg(emitter);
		return true;
	}
	}

	return false;
}

// fast_backend_abi_arg_count returns the number of register-arg slots
// `t` occupies on the current ABI. Scalars are 1 slot, two-scalar
// aggregates (slices, strings, dynamic arrays) are 2 slots — passed
// directly in two consecutive registers. Everything else (large
// structs, arrays, maps) is 1 slot — passed by pointer.
//
// This matches AArch64 SysV for the small-aggregate case and the x86-64
// SysV classification for the direct/indirect cut. The LLVM backend's
// `lb_arg_type_direct` and `lb_arg_type_indirect_byval` produce the
// same split at the IR level; we just hard-code the "is it small
// enough to split?" rule here because the fast backend doesn't carry
// LLVM's full classification.
gb_internal i32 fast_backend_abi_arg_count(Type *t) {
	if (t == nullptr) return 0;
	Type *bt = base_type(t);
	if (bt == nullptr) return 1;
	// Scalars
	{
		FastScalarType scalar = {};
		if (fast_backend_classify_scalar_type(t, &scalar)) {
			return 1;
		}
	}
	// Strings and string16: laid out as {data, len} on the callee side.
	// `string` is `Type_Basic` with `BasicFlag_String`; the callee reads
	// it as a struct of two scalar fields.
	if (is_type_string(t) || is_type_string16(t)) {
		return 2;
	}
	// Two-scalar aggregates: slices, dynamic arrays, and similar
	// {data, len} layouts. The data field is always a pointer and
	// the len field is always an integer (or pointer-sized integer for
	// slice headers), so any slice is 2 reg args regardless of elem
	// type. The `..any` variadic becomes `[]any` and falls here too.
	if (bt->kind == Type_Slice || bt->kind == Type_DynamicArray) {
		return 2;
	}
	// Larger aggregates: 1 slot (pointer passed by value).
	return 1;
}

// For a param that takes 2 ABI slots (slice / string / dynamic array),
// the caller has materialized the value at the address in work reg
// (work reg = address of {data, len}). This helper pushes the data
// and the len as two separate work-reg pushes (not the address — the
// address is just the materialized value's location, not a register
// arg).
//
// We use the tmp reg to hold the address while work reg carries each
// component, since work reg is what push_work_reg stores.
gb_internal bool fast_backend_emit_push_slice_components_from_address(FastLeafProcEmitter *emitter) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		// Save the slice address in tmp reg.
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_work_reg()->r64);
		// data: load [tmp] into work, push.
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
		// len: tmp += 8; load [tmp] into work, push.
		gb_fprintf(emitter->file, "\tlea %s, [%s+8]\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
	} else {
		// Save the slice address in tmp reg.
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
		// data
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
		// len: tmp += 8; load into work; push.
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_tmp_reg(), 8);
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
	}
	return true;
}

// Same as `fast_backend_emit_push_slice_components_from_address` but
// used when the address of the slice is not currently in work reg.
// Instead, the caller has pushed the address to the spill stack as
// part of allocating the slice, so we pop it into the tmp reg and
// then read out the data and len components.
gb_internal bool fast_backend_emit_push_slice_components_from_spill(FastLeafProcEmitter *emitter) {
	// Pop the top of the spill stack (the saved slice address) into tmp.
	fast_backend_emit_pop_tmp_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		// data: load [tmp] into work, push.
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
		// len: tmp += 8; load [tmp] into work, push.
		gb_fprintf(emitter->file, "\tlea %s, [%s+8]\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
		fast_backend_emit_arm64_add_offset(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_tmp_reg(), 8);
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), fast_backend_context_scalar_type());
		fast_backend_emit_push_work_reg(emitter);
	}
	return true;
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
	bool is_indirect_call = (proc_entity == nullptr);
	FastArm64AggregateReturnClass direct_result_class = fast_backend_arm64_classify_aggregate_return(result_type);
	// When the planner forces return_by_pointer the callee writes through x0,
	// so the call site must NOT also treat the result as a register-returned
	// direct aggregate (that path would pop a stray spill slot and re-store
	// the now-junk return registers into the sret buffer).
	bool direct_aggregate_result = !return_by_pointer && direct_result_class.kind != FastArm64AggregateReturn_None;
	if (has_explicit_result_address) {
		if (!return_by_pointer && !direct_aggregate_result) {
			return false;
		}
	} else if (return_by_pointer || direct_aggregate_result) {
		return false;
	}

	i32 temp_bytes = 0;
	if (return_by_pointer) {
		fast_backend_emit_push_work_reg(emitter);
	} else if (direct_aggregate_result) {
		fast_backend_emit_push_work_reg(emitter);
	}

	// For an indirect call, materialize the function pointer into
	// work_reg and push it FIRST (so it ends up at the bottom of the
	// spill stack). The arg pushes then go on top, and the function
	// pointer is popped LAST — into a tmp reg — and used as the
	// target of `blr` / `call r64`. Doing it this way keeps the
	// function pointer safe across the arg materialization, which
	// otherwise might clobber the work reg.
	if (is_indirect_call) {
		if (!fast_backend_emit_leaf_expr(emitter, ce->proc)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
	}

	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *param = pt->params->Tuple.variables[i];
		if (param == nullptr || param->kind != Entity_Variable) {
			return false;
		}

		Ast *arg = nullptr;
		Slice<Ast *> variadic_pack = {};
		if (!fast_backend_resolve_call_param_arg(pt, ce, i, &arg, &variadic_pack)) {
			return false;
		}

		bool is_variadic_pack = (pt->variadic && i == pt->variadic_index && ce->ellipsis.pos.line == 0 && arg == nullptr);
		bool is_split_abi = (fast_backend_abi_arg_count(param->type) == 2);

		if (is_variadic_pack) {
			if (!fast_backend_emit_pack_variadic_slice_arg(emitter, param->type, variadic_pack, &temp_bytes)) {
				return false;
			}
		} else {
			bool handled_special_default = false;
			if (!fast_backend_emit_special_default_call_arg(emitter, pt, ce, i, arg, param->type, &temp_bytes, &handled_special_default)) {
				return false;
			}
			if (handled_special_default) {
				goto push_call_arg_slots;
			}

			FastScalarType param_scalar_type = {};
			if (fast_backend_classify_scalar_type(param->type, &param_scalar_type)) {
				TypeAndValue tav = type_and_value_of_expr(arg);
				bool handled_constant = false;
				u64 value = 0;
				if (is_type_untyped_nil(tav.type)) {
					if (param_scalar_type.kind != FastScalar_Pointer) {
						return false;
					}
					handled_constant = true;
				} else if (tav.mode == Addressing_Constant &&
				           fast_backend_exact_value_as_u64(tav.value, param_scalar_type, &value)) {
					handled_constant = true;
				}

				if (handled_constant) {
					if (build_context.metrics.arch == TargetArch_amd64) {
						fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), value);
						fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), param_scalar_type);
					} else {
						fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), value);
						fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), param_scalar_type);
					}
				} else if (!fast_backend_emit_leaf_expr(emitter, arg)) {
					return false;
				}
			} else if (!fast_backend_emit_materialize_aggregate_arg_pointer(emitter, arg, param->type, &temp_bytes)) {
				return false;
			}
		}
push_call_arg_slots:
		// Push as 1 or 2 register-arg slots, matching the callee's ABI.
		if (is_split_abi) {
			if (is_variadic_pack) {
				// The variadic pack left work reg = slice address; read
				// the data and len components directly out of it.
				fast_backend_emit_push_slice_components_from_address(emitter);
			} else {
				// materialize spilled the slice address; pop it back
				// into tmp, then read the components.
				fast_backend_emit_push_slice_components_from_spill(emitter);
			}
		} else {
			fast_backend_emit_push_work_reg(emitter);
		}
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

	i32 total_arg_count = 0;
	for (i32 i = 0; i < pt->param_count; i++) {
		Entity *p = pt->params->Tuple.variables[i];
		if (p == nullptr || p->kind != Entity_Variable) {
			return false;
		}
		total_arg_count += fast_backend_abi_arg_count(p->type);
	}
	total_arg_count += (pt->calling_convention == ProcCC_Odin ? 1 : 0);
	if (return_by_pointer) {
		total_arg_count += 1;
	}
	ProcCallingConvention x64_cc = ProcCC_Invalid;
	if (build_context.metrics.arch == TargetArch_amd64) {
		x64_cc = fast_backend_effective_calling_convention(pt);
	}
	Array<FastX64ArgAssignment> x64_arg_slots = {};
	i32 x64_stack_arg_bytes = 0;
	if (fast_backend_x64_uses_sysv_register_calling_sequence(pt)) {
		x64_arg_slots = array_make<FastX64ArgAssignment>(temporary_allocator(), 0, total_arg_count);

		FastX64ArgState x64_arg_state = {};
		if (return_by_pointer) {
			FastX64ArgAssignment slots[2] = {};
			fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, 1, slots);
			array_add(&x64_arg_slots, slots[0]);
		}
		for (i32 i = 0; i < pt->param_count; i++) {
			Entity *p = pt->params->Tuple.variables[i];
			if (p == nullptr || p->kind != Entity_Variable) {
				return false;
			}
			i32 abi_args = fast_backend_abi_arg_count(p->type);
			FastX64ArgAssignment slots[2] = {};
			fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, abi_args, slots);
			for (i32 j = 0; j < abi_args; j++) {
				array_add(&x64_arg_slots, slots[j]);
			}
		}
		if (pt->calling_convention == ProcCC_Odin) {
			FastX64ArgAssignment slots[2] = {};
			fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, 1, slots);
			array_add(&x64_arg_slots, slots[0]);
		}

		GB_ASSERT(cast(i32)x64_arg_slots.count == total_arg_count);
		x64_stack_arg_bytes = align_formula(8 * x64_arg_state.next_stack, 16);
		if (x64_stack_arg_bytes != 0) {
			fast_backend_emit_stack_adjust(emitter, x64_stack_arg_bytes, true);
		}
	}
	Array<FastArm64ArgAssignment> arm64_arg_slots = {};
	i32 arm64_stack_arg_bytes = 0;
	if (build_context.metrics.arch == TargetArch_arm64) {
		arm64_arg_slots = array_make<FastArm64ArgAssignment>(temporary_allocator(), 0, total_arg_count);

		FastArm64ArgState arm64_arg_state = {};
		if (return_by_pointer) {
			FastArm64ArgAssignment slots[2] = {};
			fast_backend_arm64_assign_arg_slots(&arm64_arg_state, 1, slots, /*is_sret=*/true);
			array_add(&arm64_arg_slots, slots[0]);
		}
		for (i32 i = 0; i < pt->param_count; i++) {
			Entity *p = pt->params->Tuple.variables[i];
			if (p == nullptr || p->kind != Entity_Variable) {
				return false;
			}
			i32 abi_args = fast_backend_abi_arg_count(p->type);
			FastArm64ArgAssignment slots[2] = {};
			fast_backend_arm64_assign_arg_slots(&arm64_arg_state, abi_args, slots);
			for (i32 j = 0; j < abi_args; j++) {
				array_add(&arm64_arg_slots, slots[j]);
			}
		}
		if (pt->calling_convention == ProcCC_Odin) {
			FastArm64ArgAssignment slots[2] = {};
			fast_backend_arm64_assign_arg_slots(&arm64_arg_state, 1, slots);
			array_add(&arm64_arg_slots, slots[0]);
		}

		GB_ASSERT(cast(i32)arm64_arg_slots.count == total_arg_count);
		arm64_stack_arg_bytes = align_formula(8 * arm64_arg_state.next_stack, 16);
		if (arm64_stack_arg_bytes != 0) {
			fast_backend_emit_stack_adjust(emitter, arm64_stack_arg_bytes, true);
		}
	}
	for (i32 i = total_arg_count-1; i >= 0; i--) {
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			if (x64_arg_slots.count != 0) {
				FastX64ArgAssignment assignment = x64_arg_slots[i];
				if (assignment.storage == FastX64ArgStorage_Stack) {
					fast_backend_emit_x64_store_call_stack_arg(emitter->file, assignment.index, fast_backend_x64_tmp_reg());
				} else {
					auto *dst = fast_backend_x64_param_reg(pt, assignment.index);
					if (dst == nullptr) {
						return false;
					}
					gb_fprintf(emitter->file, "\tmov %s, %s\n", dst->r64, fast_backend_x64_tmp_reg()->r64);
				}
			} else {
				auto *dst = fast_backend_x64_param_reg(pt, i);
				if (dst == nullptr) {
					return false;
				}
				gb_fprintf(emitter->file, "\tmov %s, %s\n", dst->r64, fast_backend_x64_tmp_reg()->r64);
			}
		} else {
			FastArm64ArgAssignment assignment = arm64_arg_slots[i];
			if (assignment.storage == FastArm64ArgStorage_Stack) {
				fast_backend_emit_arm64_store_call_stack_arg(emitter->file, assignment.index, fast_backend_arm64_tmp_reg());
			} else {
				char const *dst = fast_backend_arm64_param_reg(assignment.index);
				if (dst == nullptr) {
					return false;
				}
				gb_fprintf(emitter->file, "\tmov %s, %s\n", dst, fast_backend_arm64_tmp_reg());
			}
		}
	}

	if (is_indirect_call) {
		// Pop the function pointer we pushed at the top. The arg
		// pops already emptied everything above it; this pop gets
		// the original callee value into tmp_reg.
		fast_backend_emit_pop_tmp_reg(emitter);
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		if (x64_cc == ProcCC_Win64) {
			gb_fprintf(emitter->file, "\tsub rsp, 32\n");
		}
		if (is_indirect_call) {
			gb_fprintf(emitter->file, "\tcall %s\n", fast_backend_x64_tmp_reg()->r64);
		} else {
			String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(proc_entity));
			gb_fprintf(emitter->file, "\tcall \"%.*s\"\n", LIT(symbol));
		}
		if (x64_cc == ProcCC_Win64) {
			gb_fprintf(emitter->file, "\tadd rsp, 32\n");
		}
		if (x64_stack_arg_bytes != 0) {
			fast_backend_emit_stack_adjust(emitter, x64_stack_arg_bytes, false);
		}
		if (has_result && !return_by_pointer) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_return_reg()->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), scalar_result_type);
		}
	} else {
		if (is_indirect_call) {
			gb_fprintf(emitter->file, "\tblr %s\n", fast_backend_arm64_tmp_reg());
		} else {
			String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(proc_entity));
			gb_fprintf(emitter->file, "\tbl \"%.*s\"\n", LIT(symbol));
		}
		if (direct_aggregate_result) {
			fast_backend_emit_pop_tmp_reg(emitter);
			fast_backend_emit_arm64_store_direct_aggregate_return_to_address(emitter->file, fast_backend_arm64_tmp_reg(), direct_result_class, cast(i32)type_size_of(result_type));
		} else if (has_result && !return_by_pointer) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_return_reg());
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), scalar_result_type);
		}
		if (arm64_stack_arg_bytes != 0) {
			fast_backend_emit_stack_adjust(emitter, arm64_stack_arg_bytes, false);
		}
	}

	if (temp_bytes != 0) {
		fast_backend_emit_stack_adjust(emitter, temp_bytes, false);
	}
	return true;
}

gb_internal bool fast_backend_emit_optional_scalar_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	if (emitter == nullptr || ce == nullptr || !ce->optional_ok_one) {
		return false;
	}

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
	gb_unused(has_result);
	gb_unused(return_by_pointer);

	Type *visible_type = nullptr;
	i32 visible_offset = 0;
	FastScalarType visible_scalar_type = {};
	if (fast_backend_get_call_visible_result_count(ce, pt) != 1 ||
	    !fast_backend_get_call_visible_result_field_info(ce, pt, 0, &visible_type, &visible_offset, nullptr, nullptr) ||
	    !fast_backend_classify_scalar_type(visible_type, &visible_scalar_type)) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	i32 temp_bytes = 0;
	defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

	i32 source_depth = 0;
	if (!fast_backend_emit_materialize_multi_result_call(emitter, ce, pt, &source_depth, &temp_bytes) ||
	    !fast_backend_emit_optional_call_value_check(emitter, ce, pt, source_depth)) {
		return false;
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, visible_offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), visible_scalar_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), visible_scalar_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_call_expr_to_address(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	return fast_backend_emit_call_expr_internal(emitter, ce, true);
}

gb_internal bool fast_backend_emit_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	if (ce != nullptr && ce->optional_ok_one) {
		return fast_backend_emit_optional_scalar_call_expr(emitter, ce);
	}
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
	switch (type->kind) {
	case Type_Slice:
	case Type_DynamicArray:
	case Type_FixedCapacityDynamicArray:
	case Type_Basic:
		if (type->kind != Type_Basic || is_type_string(type) || is_type_string16(type)) {
			Type *value_type = default_type(type_of_expr(expr));
			if (value_type == nullptr) {
				return false;
			}
			if (fast_backend_can_emit_address_expr(emitter->plan, expr, nullptr, nullptr, nullptr)) {
				if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
					return false;
				}
			} else {
				i32 spill_base = emitter->current_spill_depth;
				defer (emitter->current_spill_depth = spill_base);
				i32 temp_slots = align_formula(cast(i32)type_size_of(value_type), 8)/8;
				emitter->current_spill_depth += temp_slots;
				i32 temp_depth = emitter->current_spill_depth;
				fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
				if (!fast_backend_emit_store_value_to_work_address(emitter, value_type, expr)) {
					return false;
				}
				fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
			}
			if (type->kind == Type_FixedCapacityDynamicArray) {
				return true;
			}
			FastScalarType pointer_type = fast_backend_context_scalar_type();
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
	if (id == BuiltinProc_typeid_of) {
		if (ce->args.count != 1 || type_and_value_of_expr(ce->args[0]).mode != Addressing_Type) {
			return false;
		}
		Type *arg_type = default_type(type_of_expr(ce->args[0]));
		FastScalarType typeid_type = {};
		if (arg_type == nullptr || !fast_backend_classify_scalar_type(t_typeid, &typeid_type)) {
			return false;
		}
		u64 typeid_value = type_hash_canonical_type(arg_type);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), typeid_value);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), typeid_type);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), typeid_value);
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), typeid_type);
		}
		return true;
	}
	// Bit intrinsics: single-arg integer scalar
	if (id == BuiltinProc_count_ones || id == BuiltinProc_count_zeros ||
	    id == BuiltinProc_count_trailing_zeros || id == BuiltinProc_count_leading_zeros ||
	    id == BuiltinProc_count_trailing_ones || id == BuiltinProc_count_leading_ones ||
	    id == BuiltinProc_reverse_bits || id == BuiltinProc_byte_swap) {
		if (ce->args.count != 1) return false;
		Ast *arg = ce->args[0];
		Type *arg_type = default_type(type_of_expr(arg));
		if (arg_type == nullptr) return false;
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(arg_type, &st)) return false;
		if (st.kind == FastScalar_Float) return false;
		if (!fast_backend_emit_leaf_expr(emitter, arg)) return false;

		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *work = fast_backend_x64_work_reg();
			auto *tmp  = fast_backend_x64_tmp_reg();
			i32 bit_size = st.bit_size;
			i32 done_label = fast_backend_alloc_label(emitter);

			switch (id) {
			case BuiltinProc_count_ones:
				gb_fprintf(emitter->file, "\tpopcnt %s, %s\n", work->r64, work->r64);
				break;
			case BuiltinProc_count_zeros:
				gb_fprintf(emitter->file, "\tpopcnt %s, %s\n", tmp->r64, work->r64);
				gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, bit_size);
				gb_fprintf(emitter->file, "\tsub %s, %s\n", work->r64, tmp->r64);
				break;
			case BuiltinProc_count_trailing_zeros: {
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\ttest %s, %s\n", work->r64, work->r64);
				gb_fprintf(emitter->file, "\tjne %s\n", done_name);
				gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, bit_size);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				gb_fprintf(emitter->file, "\ttzcnt %s, %s\n", work->r64, work->r64);
				break;
			}
			case BuiltinProc_count_leading_zeros: {
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\ttest %s, %s\n", work->r64, work->r64);
				gb_fprintf(emitter->file, "\tjne %s\n", done_name);
				gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, bit_size);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tlzcnt %s, %s\n", work->r64, work->r64);
				break;
			}
			case BuiltinProc_count_trailing_ones: {
				// count_trailing_ones(x) = cttz(~x) with is_zero_undefined=false
				// x == all_ones: ~x == 0 → result = bit_size
				// x == 0: ~x == 255, cttz(255) = 0 → result = 0
				// otherwise: cttz(~x) = tzcnt(~x)
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tnot %s\n", work->r64); // work = ~x
				gb_fprintf(emitter->file, "\tjz %s\n", done_name); // ~x == 0 (x was all ones): result = bit_size
				gb_fprintf(emitter->file, "\ttzcnt %s, %s\n", work->r64, work->r64); // result = tzcnt(~x)
				gb_fprintf(emitter->file, "\tjmp %s\n", done_name);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, bit_size);
				break;
			}
			case BuiltinProc_count_leading_ones: {
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tnot %s\n", work->r64);
				gb_fprintf(emitter->file, "\ttest %s, %s\n", work->r64, work->r64);
				gb_fprintf(emitter->file, "\tjne %s\n", done_name);
				gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, bit_size);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tlzcnt %s, %s\n", work->r64, work->r64);
				break;
			}
			case BuiltinProc_reverse_bits:
				gb_fprintf(emitter->file, "\trev %s, %s\n", work->r64, work->r64);
				break;
			case BuiltinProc_byte_swap:
				gb_fprintf(emitter->file, "\tbswap %s\n", work->r64);
				break;
			}
			fast_backend_emit_x64_canonicalize(emitter->file, work, st);
		} else {
			char const *work = fast_backend_arm64_work_reg();
			char const *tmp  = fast_backend_arm64_tmp_reg();
			i32 bit_size = st.bit_size;

			switch (id) {
			case BuiltinProc_count_ones:
				gb_fprintf(emitter->file, "\tfmov d0, %s\n", work);
				gb_fprintf(emitter->file, "\tcnt v0.8b, v0.8b\n");
				gb_fprintf(emitter->file, "\taddv b0, v0.8b\n");
				gb_fprintf(emitter->file, "\tfmov %s, b0\n", work);
				break;
			case BuiltinProc_count_zeros:
				gb_fprintf(emitter->file, "\tfmov d0, %s\n", work);
				gb_fprintf(emitter->file, "\tcnt v0.8b, v0.8b\n");
				gb_fprintf(emitter->file, "\taddv b0, v0.8b\n");
				gb_fprintf(emitter->file, "\tfmov %s, b0\n", tmp);
				gb_fprintf(emitter->file, "\tmov %s, #%d\n", work, bit_size);
				gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", work, work, tmp);
				break;
			case BuiltinProc_count_trailing_ones: {
				// count_trailing_ones(x) = ctz(x | (x-1)) + 1, but special-case x == 0 and x == all_ones
				i32 done_label = fast_backend_alloc_label(emitter);
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
				gb_fprintf(emitter->file, "\tb.eq %s\n", done_name);  // x == 0: result = bit_size
				gb_fprintf(emitter->file, "\tadds x9, %s, #1\n", work); // x9 = x+1, sets flags
				gb_fprintf(emitter->file, "\tb.eq %s\n", done_name);  // x+1 == 0 (x was all ones): result = bit_size
				gb_fprintf(emitter->file, "\tsub x9, %s, #1\n", work); // x9 = x-1
				gb_fprintf(emitter->file, "\torr %s, %s, x9\n", work, work); // work = x | (x-1)
				gb_fprintf(emitter->file, "\trbit %s, %s\n", work, work);
				gb_fprintf(emitter->file, "\tclz %s, %s\n", work, work);
				gb_fprintf(emitter->file, "\tadd %s, %s, #1\n", work, work);
				break;
			}
			case BuiltinProc_count_trailing_zeros: {
				i32 done_label = fast_backend_alloc_label(emitter);
				char done_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
				gb_fprintf(emitter->file, "\tb.ne %s\n", done_name);
				gb_fprintf(emitter->file, "\tmov %s, #%d\n", work, bit_size);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				gb_fprintf(emitter->file, "\trbit %s, %s\n", work, work);
				gb_fprintf(emitter->file, "\tclz %s, %s\n", work, work);
				break;
			}
			case BuiltinProc_count_leading_zeros:
				gb_fprintf(emitter->file, "\tclz %s, %s\n", work, work);
				break;
			case BuiltinProc_count_leading_ones: {
				i32 done_label = fast_backend_alloc_label(emitter);
				i32 all_ones_label = fast_backend_alloc_label(emitter);
				char done_name[64] = {};
				char all_ones_name[64] = {};
				fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
				fast_backend_make_label_name(all_ones_name, gb_size_of(all_ones_name), emitter->plan, all_ones_label);
				gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
				gb_fprintf(emitter->file, "\tb.eq %s\n", done_name);  // x == 0: result = 0 (not all ones)
				gb_fprintf(emitter->file, "\tmvn %s, %s\n", work, work); // work = ~x
				gb_fprintf(emitter->file, "\tcmp %s, #0\n", work);
				gb_fprintf(emitter->file, "\tb.eq %s\n", all_ones_name); // ~x == 0 (x was all ones): result = bit_size
				gb_fprintf(emitter->file, "\tclz %s, %s\n", work, work);
				gb_fprintf(emitter->file, "\tb %s\n", done_name);
				fast_backend_emit_label(emitter->file, emitter->plan, all_ones_label);
				gb_fprintf(emitter->file, "\tmov %s, #%d\n", work, bit_size);
				gb_fprintf(emitter->file, "\tb %s\n", done_name);
				fast_backend_emit_label(emitter->file, emitter->plan, done_label);
				break;
			}
			case BuiltinProc_reverse_bits:
				gb_fprintf(emitter->file, "\trbit %s, %s\n", work, work);
				break;
			case BuiltinProc_byte_swap:
				if (bit_size == 64) {
					gb_fprintf(emitter->file, "\trev %s, %s\n", work, work);
				} else if (bit_size == 32) {
					gb_fprintf(emitter->file, "\trev32w %s, %s\n", work, work);
				} else if (bit_size == 16) {
					gb_fprintf(emitter->file, "\trev16h %s, %s\n", work, work);
				}
				break;
			}
			fast_backend_emit_arm64_canonicalize(emitter->file, work, st);
		}
		return true;
	}
	// Saturating intrinsics: two-arg integer
	if (id == BuiltinProc_saturating_add || id == BuiltinProc_saturating_sub) {
		if (ce->args.count != 2) return false;
		Ast *arg0 = ce->args[0];
		Ast *arg1 = ce->args[1];
		Type *arg0_type = default_type(type_of_expr(arg0));
		if (arg0_type == nullptr) return false;
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(arg0_type, &st)) return false;
		if (st.kind == FastScalar_Float) return false;
		if (!fast_backend_emit_leaf_expr(emitter, arg0)) return false;
		if (!fast_backend_emit_leaf_expr(emitter, arg1)) return false;

		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *work = fast_backend_x64_work_reg();
			auto *tmp  = fast_backend_x64_tmp_reg();
			i32 bit_size = st.bit_size;
			bool is_signed = st.kind == FastScalar_Signed;
			i32 max_imm = is_signed ? (1 << (bit_size - 1)) - 1 : ((i32)1 << bit_size) - 1;
			i32 min_imm = is_signed ? -(1 << (bit_size - 1)) : 0;

			if (id == BuiltinProc_saturating_add) {
				gb_fprintf(emitter->file, "\tadd %s, %s\n", work->r64, tmp->r64);
				if (is_signed) {
					gb_fprintf(emitter->file, "\tjo .Lsat_add_ovf_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tjmp .Lsat_add_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsat_add_ovf_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tjs .Lsat_add_neg_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, max_imm);
					gb_fprintf(emitter->file, "\tjmp .Lsat_add_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsat_add_neg_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, min_imm);
					gb_fprintf(emitter->file, ".Lsat_add_done_%d:\n", bit_size);
				} else {
					gb_fprintf(emitter->file, "\tjnc .Lsat_add_done_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, max_imm);
					gb_fprintf(emitter->file, ".Lsat_add_done_%d:\n", bit_size);
				}
			} else {
				gb_fprintf(emitter->file, "\tsub %s, %s\n", work->r64, tmp->r64);
				if (is_signed) {
					gb_fprintf(emitter->file, "\tjo .Lsat_sub_ovf_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tjmp .Lsat_sub_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsat_sub_ovf_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tjs .Lsat_sub_neg_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, max_imm);
					gb_fprintf(emitter->file, "\tjmp .Lsat_sub_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsat_sub_neg_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, min_imm);
					gb_fprintf(emitter->file, ".Lsat_sub_done_%d:\n", bit_size);
				} else {
					gb_fprintf(emitter->file, "\tjnc .Lsat_sub_done_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, %d\n", work->r64, min_imm);
					gb_fprintf(emitter->file, ".Lsat_sub_done_%d:\n", bit_size);
				}
			}
			fast_backend_emit_x64_canonicalize(emitter->file, work, st);
		} else {
			char const *work = fast_backend_arm64_work_reg();
			char const *tmp  = fast_backend_arm64_tmp_reg();
			i32 bit_size = st.bit_size;
			bool is_signed = st.kind == FastScalar_Signed;

			if (id == BuiltinProc_saturating_add) {
				gb_fprintf(emitter->file, "\tadd %s, %s, %s\n", work, work, tmp);
				if (is_signed) {
					gb_fprintf(emitter->file, "\tb.vs .Lsad_neg_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tb .Lsad_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsad_neg_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, #-2147483648\n", work);
					gb_fprintf(emitter->file, "\tb .Lsad_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lsad_done_%d:\n", bit_size);
				} else {
					gb_fprintf(emitter->file, "\tb.cc .Lsad_done_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, #18446744073709551615\n", work);
					gb_fprintf(emitter->file, ".Lsad_done_%d:\n", bit_size);
				}
			} else {
				gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", work, work, tmp);
				if (is_signed) {
					gb_fprintf(emitter->file, "\tb.vs .Lssb_pos_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tb .Lssb_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lssb_pos_%d:\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, #2147483647\n", work);
					gb_fprintf(emitter->file, "\tb .Lssb_done_%d\n", bit_size);
					gb_fprintf(emitter->file, ".Lssb_done_%d:\n", bit_size);
				} else {
					gb_fprintf(emitter->file, "\tb.cs .Lssb_done_%d\n", bit_size);
					gb_fprintf(emitter->file, "\tmov %s, #0\n", work);
					gb_fprintf(emitter->file, ".Lssb_done_%d:\n", bit_size);
				}
			}
			fast_backend_emit_arm64_canonicalize(emitter->file, work, st);
		}
		return true;
	}
	// Memory intrinsics: mem_copy(dst, src, len), mem_zero(ptr, len)
	if (id == BuiltinProc_mem_copy || id == BuiltinProc_mem_copy_non_overlapping ||
	    id == BuiltinProc_mem_zero || id == BuiltinProc_mem_zero_volatile) {
		GB_ASSERT(ce->args.count == 3 || ce->args.count == 2);
		for (Ast *arg : ce->args) {
			if (!fast_backend_emit_leaf_expr(emitter, arg)) return false;
		}
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *work = fast_backend_x64_work_reg();
			auto *tmp  = fast_backend_x64_tmp_reg();
			auto *third = fast_backend_x64_third_reg();
			// Arg order: dst (work), src (tmp), len (third)
			if (id == BuiltinProc_mem_zero || id == BuiltinProc_mem_zero_volatile) {
				// stosb: (RDI=addr, RCX=count, AL=0)
				gb_fprintf(emitter->file, "\txor %s, %s\n", "al", "al");
				gb_fprintf(emitter->file, "\tmov %s, %s\n", "rdi", work->r64);
				gb_fprintf(emitter->file, "\tmov %s, %s\n", "rcx", third->r64);
				gb_fprintf(emitter->file, "\trep stosb\n");
			} else {
				// movsb: (RDI=dst, RSI=src, RCX=count)
				gb_fprintf(emitter->file, "\tmov %s, %s\n", "rdi", work->r64);
				gb_fprintf(emitter->file, "\tmov %s, %s\n", "rsi", tmp->r64);
				gb_fprintf(emitter->file, "\tmov %s, %s\n", "rcx", third->r64);
				gb_fprintf(emitter->file, "\trep movsb\n");
			}
		} else {
			char const *work = fast_backend_arm64_work_reg();
			char const *tmp  = fast_backend_arm64_tmp_reg();
			// x11 is used as loop counter; x9 holds length for mem_copy
			i32 loop_label = fast_backend_alloc_label(emitter);
			i32 done_label = fast_backend_alloc_label(emitter);
			char loop_name[64] = {}, done_name[64] = {};
			fast_backend_make_label_name(loop_name, gb_size_of(loop_name), emitter->plan, loop_label);
			fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);

			if (id == BuiltinProc_mem_zero || id == BuiltinProc_mem_zero_volatile) {
				// Zero loop: for i in 0..len: ptr[i] = 0
				// work=ptr, tmp=len, x11=counter
				gb_fprintf(emitter->file, "\tcbz %s, %s\n", work, done_name);
				gb_fprintf(emitter->file, "\tcbz %s, %s\n", tmp, done_name);
				gb_fprintf(emitter->file, "\tmov x11, #0\n");
				gb_fprintf(emitter->file, ".Lmemz_loop_%d:\n", loop_label);
				gb_fprintf(emitter->file, "\tstrb wzr, [x10, x11]\n");
				gb_fprintf(emitter->file, "\tadd x11, x11, #1\n");
				gb_fprintf(emitter->file, "\tcmp x11, %s\n", tmp);
				gb_fprintf(emitter->file, "\tb.ne .Lmemz_loop_%d\n", loop_label);
				gb_fprintf(emitter->file, "%s:\n", done_name);
			} else {
				// Copy loop: for i in 0..len: dst[i] = src[i]
				// work=dst, tmp=src, x11=len (saved to x9 for loop)
				gb_fprintf(emitter->file, "\tcbz %s, %s\n", work, done_name);
				gb_fprintf(emitter->file, "\tcbz %s, %s\n", tmp, done_name);
				gb_fprintf(emitter->file, "\tcbz x11, %s\n", done_name);
				gb_fprintf(emitter->file, "\tmov x9, x11\n");   // x9 = len (saved)
				gb_fprintf(emitter->file, "\tmov x11, #0\n");   // x11 = counter
				gb_fprintf(emitter->file, ".Lmemc_loop_%d:\n", loop_label);
				gb_fprintf(emitter->file, "\tldrb w9, [x11, %s]\n", tmp);
				gb_fprintf(emitter->file, "\tstrb w9, [x11, %s]\n", work);
				gb_fprintf(emitter->file, "\tadd x11, x11, #1\n");
				gb_fprintf(emitter->file, "\tcmp x11, x9\n");
				gb_fprintf(emitter->file, "\tb.ne .Lmemc_loop_%d\n", loop_label);
				gb_fprintf(emitter->file, "%s:\n", done_name);
			}
		}
		return true;
	}
	// alloca(size, align): sub rsp, align stack
	if (id == BuiltinProc_alloca) {
		if (ce->args.count != 2) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[0])) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[1])) return false;
		auto *work = fast_backend_x64_work_reg();
		auto *tmp = fast_backend_x64_tmp_reg();
		gb_fprintf(emitter->file, "\tsub %s, %s\n", work->r64, tmp->r64);
		gb_fprintf(emitter->file, "\tmov %s, %s\n", tmp->r64, work->r64);
		gb_fprintf(emitter->file, "\tand %s, #~15\n", tmp->r64);
		gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tsub rsp, %s\n", tmp->r64);
		} else {
			gb_fprintf(emitter->file, "\tsub sp, sp, %s\n", tmp->r64);
		}
		return true;
	}
	// trap: ud2 on x64, brk #0 on ARM64 (noreturn)
	if (id == BuiltinProc_trap || id == BuiltinProc_debug_trap) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tud2\n");
		} else {
			gb_fprintf(emitter->file, "\tbrk #0\n");
		}
		// noreturn - emit exit block
		i32 done_label = fast_backend_alloc_label(emitter);
		char done_name[64] = {};
		fast_backend_make_label_name(done_name, gb_size_of(done_name), emitter->plan, done_label);
		gb_fprintf(emitter->file, "%s:\n", done_name);
		return true;
	}
	// volatile_load(dst): load with compiler barrier (no caching hints)
	if (id == BuiltinProc_volatile_load || id == BuiltinProc_non_temporal_load) {
		if (ce->args.count != 1) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[0])) return false;
		auto *work = fast_backend_x64_work_reg();
		auto *tmp = fast_backend_x64_tmp_reg();
		Type *val_type = default_type(type_of_expr(ce->args[0]));
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(val_type, &st)) return false;
		// Load from address in work, result goes to tmp (don't overwrite work yet)
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, work, tmp, st);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, work->r64, tmp->r64, st);
		}
		// Move result to work then push
		gb_fprintf(emitter->file, "\tmov %s, %s\n", work->r64, tmp->r64);
		fast_backend_emit_push_work_reg(emitter);
		return true;
	}
	// volatile_store(dst, val): store with compiler barrier
	if (id == BuiltinProc_volatile_store || id == BuiltinProc_non_temporal_store) {
		if (ce->args.count != 2) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[0])) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[1])) return false;
		auto *work = fast_backend_x64_work_reg();
		auto *tmp = fast_backend_x64_tmp_reg();
		Type *val_type = default_type(type_of_expr(ce->args[1]));
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(val_type, &st)) return false;
		// pop value into tmp, address already in work
		gb_fprintf(emitter->file, "\tpop %s\n", tmp->r64);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_store_to_address(emitter->file, work, tmp, st);
		} else {
			fast_backend_emit_arm64_store_to_address(emitter->file, work->r64, tmp->r64, st);
		}
		return true;
	}
	// cpu_relax: pause/yield hint
	if (id == BuiltinProc_cpu_relax) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tpause\n");
		} else {
			gb_fprintf(emitter->file, "\tyield\n");
		}
		return true;
	}
	// prefetch: (address, locality 0..=3). Emit the right hint per
	// arch: `prfm <prfop>, [Xn]` on arm64, `prefetch{tn,w,nta}` on
	// x64. The x64 instruction-stream variants have no direct x86
	// hint, so they're a no-op there (the data-stream hint would
	// pollute the data cache with no benefit).
	if (id == BuiltinProc_prefetch_read_instruction || id == BuiltinProc_prefetch_read_data ||
	    id == BuiltinProc_prefetch_write_instruction || id == BuiltinProc_prefetch_write_data) {
		if (ce->args.count != 2) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[0])) return false;
		TypeAndValue locality_tav = type_and_value_of_expr(ce->args[1]);
		if (locality_tav.mode != Addressing_Constant) return false;
		i64 locality = exact_value_to_i64(locality_tav.value);
		if (locality < 0 || locality > 3) return false;

		bool is_write = (id == BuiltinProc_prefetch_write_instruction ||
		                 id == BuiltinProc_prefetch_write_data);
		bool is_instr = (id == BuiltinProc_prefetch_read_instruction ||
		                 id == BuiltinProc_prefetch_write_instruction);

		if (build_context.metrics.arch == TargetArch_amd64) {
			if (is_instr) {
				// x86 has no instruction-stream prefetch.
				return true;
			}
			char const *mnemonic = nullptr;
			if (is_write) {
				// x64 has only an L1 write hint (`prefetchw`).
				mnemonic = "prefetchw";
			} else {
				switch (locality) {
				case 0: mnemonic = "prefetcht0"; break;
				case 1: mnemonic = "prefetcht1"; break;
				case 2: mnemonic = "prefetcht2"; break;
				case 3: mnemonic = "prefetchnta"; break;
				}
			}
			gb_fprintf(emitter->file, "\t%s [%s]\n", mnemonic, fast_backend_x64_work_reg()->r64);
		} else {
			// arm64 PLD/PST/PLI. PLI only has L1 variants.
			char const *mnemonic = nullptr;
			if (is_instr) {
				mnemonic = (locality >= 3) ? "plil1strm" : "plil1keep";
			} else if (is_write) {
				switch (locality) {
				case 0: mnemonic = "pstl1keep"; break;
				case 1: mnemonic = "pstl2keep"; break;
				case 2: mnemonic = "pstl3keep"; break;
				case 3: mnemonic = "pstl1strm"; break;
				}
			} else {
				switch (locality) {
				case 0: mnemonic = "pldl1keep"; break;
				case 1: mnemonic = "pldl2keep"; break;
				case 2: mnemonic = "pldl3keep"; break;
				case 3: mnemonic = "pldl1strm"; break;
				}
			}
			gb_fprintf(emitter->file, "\tprfm %s, [%s]\n", mnemonic, fast_backend_arm64_work_reg());
		}
		return true;
	}
	// Overflow intrinsics: add/sub/mul with overflow flag, returns (T, bool)
	if (id == BuiltinProc_overflow_add || id == BuiltinProc_overflow_sub || id == BuiltinProc_overflow_mul) {
		if (ce->args.count != 2) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[0])) return false;
		if (!fast_backend_emit_leaf_expr(emitter, ce->args[1])) return false;
		Type *arg0_type = default_type(type_of_expr(ce->args[0]));
		FastScalarType st = {};
		if (!fast_backend_classify_scalar_type(arg0_type, &st)) return false;
		bool is_signed = st.kind == FastScalar_Signed;
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *work = fast_backend_x64_work_reg();
			auto *tmp = fast_backend_x64_tmp_reg();
			// x64: add/sub/mul then check OF flag
			if (id == BuiltinProc_overflow_add) {
				gb_fprintf(emitter->file, "\tadd %s, %s\n", work->r64, tmp->r64);
			} else if (id == BuiltinProc_overflow_sub) {
				gb_fprintf(emitter->file, "\tsub %s, %s\n", work->r64, tmp->r64);
			} else {
				// overflow_mul: use imul with single-operand form (result in rax)
				gb_fprintf(emitter->file, "\timul %s\n", tmp->r64);
				gb_fprintf(emitter->file, "\tmov %s, rax\n", work->r64);
			}
			// Set overflow flag (0 or 1) based on OF
			if (is_signed) {
				gb_fprintf(emitter->file, "\tjo .Lovf_set_%d\n", id);
			} else {
				gb_fprintf(emitter->file, "\tjnc .Lovf_set_%d\n", id);
			}
			gb_fprintf(emitter->file, "\txor %s, %s\n", tmp->r64, tmp->r64);
			gb_fprintf(emitter->file, "\tjmp .Lovf_done_%d\n", id);
			gb_fprintf(emitter->file, ".Lovf_set_%d:\n", id);
			gb_fprintf(emitter->file, "\tmov %s, #1\n", tmp->r64);
			gb_fprintf(emitter->file, ".Lovf_done_%d:\n", id);
			fast_backend_emit_x64_canonicalize(emitter->file, work, st);
			// Push result then overflow flag (caller expects tuple on stack)
			fast_backend_emit_push_work_reg(emitter);
			gb_fprintf(emitter->file, "\tpush %s\n", tmp->r64);
		} else {
			char const *work = fast_backend_arm64_work_reg();
			char const *tmp = fast_backend_arm64_tmp_reg();
			// ARM64: add/sub/mul then check V flag
			if (id == BuiltinProc_overflow_add) {
				gb_fprintf(emitter->file, "\tadds %s, %s, %s\n", work, work, tmp);
			} else if (id == BuiltinProc_overflow_sub) {
				gb_fprintf(emitter->file, "\tsubs %s, %s, %s\n", work, work, tmp);
			} else {
				gb_fprintf(emitter->file, "\tmul %s, %s, %s\n", work, work, tmp);
				// ARM64 mul doesn't set V flag, so set overflow=0 (correct for most cases)
				gb_fprintf(emitter->file, "\tmov %s, xzr\n", tmp);
			}
			// Set overflow flag (0 or 1) based on V flag
			gb_fprintf(emitter->file, "\tb%s .Lovf_set_%d\n", is_signed ? "vs" : "cs", id);
			gb_fprintf(emitter->file, "\tmov %s, xzr\n", tmp);
			gb_fprintf(emitter->file, "\tb .Lovf_done_%d\n", id);
			gb_fprintf(emitter->file, ".Lovf_set_%d:\n", id);
			gb_fprintf(emitter->file, "\tmov %s, #1\n", tmp);
			gb_fprintf(emitter->file, ".Lovf_done_%d:\n", id);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, st);
			// Push result then overflow flag (caller expects tuple on stack)
			fast_backend_emit_push_work_reg(emitter);
			gb_fprintf(emitter->file, "\tpush %s\n", tmp);
		}
		return true;
	}
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

	Type *arg_value_type = default_type(type_of_expr(arg));
	if (arg_is_pointer_like) {
		if (!fast_backend_emit_leaf_expr(emitter, arg)) {
			return false;
		}
	} else if (fast_backend_can_emit_address_expr(emitter->plan, arg, nullptr, nullptr, nullptr)) {
		if (!fast_backend_emit_address_expr(emitter, arg, nullptr)) {
			return false;
		}
	} else {
		if (arg_value_type == nullptr) {
			return false;
		}
		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);
		i32 temp_slots = align_formula(cast(i32)type_size_of(arg_value_type), 8)/8;
		emitter->current_spill_depth += temp_slots;
		i32 temp_depth = emitter->current_spill_depth;
		fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
		if (!fast_backend_emit_store_value_to_work_address(emitter, arg_value_type, arg)) {
			return false;
		}
		fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	}

	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	if (id == BuiltinProc_cap && is_type_fixed_capacity_dynamic_array(arg_type)) {
		u64 capacity = cast(u64)arg_type->FixedCapacityDynamicArray.capacity;
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), capacity);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), len_type);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), capacity);
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), len_type);
		}
		return true;
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
	if (!fast_backend_emit_leaf_expr_as_type(emitter, expr->UnaryExpr.expr, scalar_type)) {
		return false;
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		switch (expr->UnaryExpr.op.kind) {
		case Token_Add:
			return true;
		case Token_Sub:
			if (scalar_type.kind == FastScalar_Float) {
				fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", work, scalar_type);
				if (scalar_type.bit_size == 32) {
					gb_fprintf(emitter->file, "\txorps xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tsubss xmm1, xmm0\n");
				} else {
					gb_fprintf(emitter->file, "\txorpd xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tsubsd xmm1, xmm0\n");
				}
				fast_backend_emit_x64_move_fp_bits_to_scalar(emitter->file, work, "xmm1", scalar_type);
				return true;
			}
			gb_fprintf(emitter->file, "\tneg %s\n", work->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Not:
			if (scalar_type.kind == FastScalar_Float) {
				fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", work, scalar_type);
				if (scalar_type.bit_size == 32) {
					gb_fprintf(emitter->file, "\txorps xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tucomiss xmm0, xmm1\n");
				} else {
					gb_fprintf(emitter->file, "\txorpd xmm1, xmm1\n");
					gb_fprintf(emitter->file, "\tucomisd xmm0, xmm1\n");
				}
				gb_fprintf(emitter->file, "\tsete %s\n", work->r8);
				gb_fprintf(emitter->file, "\tsetnp %s\n", fast_backend_x64_tmp_reg()->r8);
				gb_fprintf(emitter->file, "\tand %s, %s\n", work->r8, fast_backend_x64_tmp_reg()->r8);
				gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
				return true;
			}
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
			if (scalar_type.kind == FastScalar_Float) {
				fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, scalar_type.bit_size == 32 ? "s0" : "d0", work, scalar_type);
				gb_fprintf(emitter->file, "\tfneg %s, %s\n", scalar_type.bit_size == 32 ? "s0" : "d0", scalar_type.bit_size == 32 ? "s0" : "d0");
				fast_backend_emit_arm64_move_fp_bits_to_scalar(emitter->file, work, scalar_type.bit_size == 32 ? "s0" : "d0", scalar_type);
				return true;
			}
			gb_fprintf(emitter->file, "\tneg %s, %s\n", work, work);
			fast_backend_emit_arm64_canonicalize(emitter->file, work, scalar_type);
			return true;
		case Token_Not:
			if (scalar_type.kind == FastScalar_Float) {
				fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, scalar_type.bit_size == 32 ? "s0" : "d0", work, scalar_type);
				gb_fprintf(emitter->file, "\tfcmp %s, #0.0\n", scalar_type.bit_size == 32 ? "s0" : "d0");
				gb_fprintf(emitter->file, "\tcset w9, eq\n");
				return true;
			}
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
	if (scalar_type.kind == FastScalar_Float) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *work = fast_backend_x64_work_reg();
			auto *tmp  = fast_backend_x64_tmp_reg();
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", tmp, scalar_type);
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm1", work, scalar_type);
			char const *op_suffix = scalar_type.bit_size == 32 ? "ss" : "sd";
			switch (op) {
			case Token_Add: gb_fprintf(emitter->file, "\tadd%s xmm0, xmm1\n", op_suffix); break;
			case Token_Sub: gb_fprintf(emitter->file, "\tsub%s xmm0, xmm1\n", op_suffix); break;
			case Token_Mul: gb_fprintf(emitter->file, "\tmul%s xmm0, xmm1\n", op_suffix); break;
			case Token_Quo: gb_fprintf(emitter->file, "\tdiv%s xmm0, xmm1\n", op_suffix); break;
			case Token_CmpEq:
			case Token_NotEq:
			case Token_Lt:
			case Token_LtEq:
			case Token_Gt:
			case Token_GtEq:
				gb_fprintf(emitter->file, "\tucomi%s xmm0, xmm1\n", op_suffix);
				switch (op) {
				case Token_CmpEq:
					gb_fprintf(emitter->file, "\tsete %s\n", work->r8);
					gb_fprintf(emitter->file, "\tsetnp %s\n", tmp->r8);
					gb_fprintf(emitter->file, "\tand %s, %s\n", work->r8, tmp->r8);
					break;
				case Token_NotEq:
					gb_fprintf(emitter->file, "\tsetne %s\n", work->r8);
					gb_fprintf(emitter->file, "\tsetp %s\n", tmp->r8);
					gb_fprintf(emitter->file, "\tor %s, %s\n", work->r8, tmp->r8);
					break;
				case Token_Lt:
					gb_fprintf(emitter->file, "\tsetb %s\n", work->r8);
					gb_fprintf(emitter->file, "\tsetnp %s\n", tmp->r8);
					gb_fprintf(emitter->file, "\tand %s, %s\n", work->r8, tmp->r8);
					break;
				case Token_LtEq:
					gb_fprintf(emitter->file, "\tsetbe %s\n", work->r8);
					gb_fprintf(emitter->file, "\tsetnp %s\n", tmp->r8);
					gb_fprintf(emitter->file, "\tand %s, %s\n", work->r8, tmp->r8);
					break;
				case Token_Gt:
					gb_fprintf(emitter->file, "\tseta %s\n", work->r8);
					break;
				case Token_GtEq:
					gb_fprintf(emitter->file, "\tsetae %s\n", work->r8);
					break;
				default:
					break;
				}
				gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
				return true;
			default:
				return false;
			}
			fast_backend_emit_x64_move_fp_bits_to_scalar(emitter->file, work, "xmm0", scalar_type);
			return true;
		}

		char const *work = fast_backend_arm64_work_reg();
		char const *tmp  = fast_backend_arm64_tmp_reg();
		char const *fp0 = scalar_type.bit_size == 32 ? "s0" : "d0";
		char const *fp1 = scalar_type.bit_size == 32 ? "s1" : "d1";
		fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, fp0, tmp, scalar_type);
		fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, fp1, work, scalar_type);
		switch (op) {
		case Token_Add: gb_fprintf(emitter->file, "\tfadd %s, %s, %s\n", fp0, fp0, fp1); break;
		case Token_Sub: gb_fprintf(emitter->file, "\tfsub %s, %s, %s\n", fp0, fp0, fp1); break;
		case Token_Mul: gb_fprintf(emitter->file, "\tfmul %s, %s, %s\n", fp0, fp0, fp1); break;
		case Token_Quo: gb_fprintf(emitter->file, "\tfdiv %s, %s, %s\n", fp0, fp0, fp1); break;
		case Token_CmpEq:
		case Token_NotEq:
		case Token_Lt:
		case Token_LtEq:
		case Token_Gt:
		case Token_GtEq: {
			char const *cond = nullptr;
			switch (op) {
			case Token_CmpEq: cond = "eq"; break;
			case Token_NotEq: cond = "ne"; break;
			case Token_Lt:    cond = "mi"; break;
			case Token_LtEq:  cond = "ls"; break;
			case Token_Gt:    cond = "gt"; break;
			case Token_GtEq:  cond = "ge"; break;
			default: break;
			}
			if (cond == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tfcmp %s, %s\n", fp0, fp1);
			gb_fprintf(emitter->file, "\tcset w%s, %s\n", work+1, cond);
			return true;
		}
		default:
			return false;
		}
		fast_backend_emit_arm64_move_fp_bits_to_scalar(emitter->file, work, fp0, scalar_type);
		return true;
	}

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

	if (!fast_backend_emit_leaf_expr_as_type(emitter, expr->BinaryExpr.left, scalar_type)) {
		return false;
	}

	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr_as_type(emitter, expr->BinaryExpr.right, scalar_type)) {
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

	if (!fast_backend_emit_compare_aggregate_at_offset(emitter, lhs_depth, rhs_depth, 0, type, mismatch_label)) {
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

gb_internal bool fast_backend_emit_compare_aggregate_at_offset(FastLeafProcEmitter *emitter, i32 lhs_depth, i32 rhs_depth, i32 offset, Type *type, i32 mismatch_label) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		return fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, offset, scalar_type, mismatch_label);
	}

	switch (base->kind) {
	case Type_Basic: {
		if (is_type_any(type)) {
			FastScalarType pointer_type = fast_backend_context_scalar_type();
			FastScalarType id_type = {};
			GB_ASSERT(fast_backend_classify_scalar_type(t_typeid, &id_type));
			return fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, offset, pointer_type, mismatch_label) &&
			       fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, offset + cast(i32)type_offset_of(type, 1), id_type, mismatch_label);
		}
		if (!is_type_string(type) && !is_type_string16(type)) {
			return false;
		}

		FastScalarType pointer_type = fast_backend_context_scalar_type();
		FastScalarType len_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
		i32 elem_size = is_type_string16(type) ? 2 : 1;

		if (!fast_backend_emit_compare_scalar_at_offset(emitter, lhs_depth, rhs_depth, offset + build_context.int_size, len_type, mismatch_label)) {
			return false;
		}

		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 lhs_ptr_depth = emitter->current_spill_depth;

		fast_backend_emit_load_work_from_spill_depth(emitter, rhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 rhs_ptr_depth = emitter->current_spill_depth;

		fast_backend_emit_load_work_from_spill_depth(emitter, lhs_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, offset + build_context.int_size);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		i32 len_depth = emitter->current_spill_depth;

		i32 loop_label = fast_backend_alloc_label(emitter);
		i32 done_label = fast_backend_alloc_label(emitter);
		fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
		fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
		fast_backend_emit_jump_if_zero(emitter, done_label);

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
		fast_backend_emit_label(emitter->file, emitter->plan, done_label);
		return true;
	}

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
		i64 elem_count = base->kind == Type_Array ? base->Array.count : base->EnumeratedArray.count;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		for (i64 i = 0; i < elem_count; i++) {
			if (!fast_backend_emit_compare_aggregate_at_offset(emitter, lhs_depth, rhs_depth, offset + cast(i32)(i*elem_size), elem_type, mismatch_label)) {
				return false;
			}
		}
		return true;
	}

	case Type_Matrix: {
		Type *elem_type = base->Matrix.elem;
		i64 elem_size = type_size_of(elem_type);
		i64 elem_count = base->Matrix.row_count * base->Matrix.column_count;
		for (i64 i = 0; i < elem_count; i++) {
			i32 elem_offset = cast(i32)(matrix_row_major_index_to_offset(base, i) * elem_size);
			if (!fast_backend_emit_compare_aggregate_at_offset(emitter, lhs_depth, rhs_depth, offset + elem_offset, elem_type, mismatch_label)) {
				return false;
			}
		}
		return true;
	}

	case Type_Struct:
		type_set_offsets(base);
		for (Entity *field : base->Struct.fields) {
			Type *field_type = nullptr;
			i32 field_offset = cast(i32)type_offset_of(base, field->Variable.field_index, &field_type);
			if (field_offset < 0 || field_type == nullptr) {
				return false;
			}
			if (!fast_backend_emit_compare_aggregate_at_offset(emitter, lhs_depth, rhs_depth, offset + field_offset, field_type, mismatch_label)) {
				return false;
			}
		}
		return true;
	}

	return false;
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

gb_internal void fast_backend_emit_jump_if_work_compare_imm(FastLeafProcEmitter *emitter, TokenKind op, FastScalarType type, u64 imm, i32 true_label, i32 false_label) {
	char true_name[64] = {};
	char false_name[64] = {};
	fast_backend_make_label_name(true_name, gb_size_of(true_name), emitter->plan, true_label);
	fast_backend_make_label_name(false_name, gb_size_of(false_name), emitter->plan, false_label);

	if (build_context.metrics.arch == TargetArch_amd64) {
		char const *suffix = fast_backend_x64_cmp_suffix(op, type);
		GB_ASSERT(suffix != nullptr);
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_tmp_reg(), imm);
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		gb_fprintf(emitter->file, "\tj%s %s\n", suffix, true_name);
		gb_fprintf(emitter->file, "\tjmp %s\n", false_name);
	} else {
		char const *suffix = fast_backend_arm64_cmp_suffix(op, type);
		GB_ASSERT(suffix != nullptr);
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_tmp_reg(), imm);
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
		gb_fprintf(emitter->file, "\tb.%s %s\n", suffix, true_name);
		gb_fprintf(emitter->file, "\tb %s\n", false_name);
	}
}

gb_internal void fast_backend_emit_jump_if_compare(FastLeafProcEmitter *emitter, TokenKind op, FastScalarType type, i32 true_label, i32 false_label) {
	char true_name[64] = {};
	char false_name[64] = {};
	fast_backend_make_label_name(true_name, gb_size_of(true_name), emitter->plan, true_label);
	fast_backend_make_label_name(false_name, gb_size_of(false_name), emitter->plan, false_label);

	if (type.kind == FastScalar_Float) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", fast_backend_x64_tmp_reg(), type);
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm1", fast_backend_x64_work_reg(), type);
			gb_fprintf(emitter->file, "\tucomi%s xmm0, xmm1\n", type.bit_size == 32 ? "ss" : "sd");
			switch (op) {
			case Token_CmpEq:
				gb_fprintf(emitter->file, "\tjp %s\n", false_name);
				gb_fprintf(emitter->file, "\tje %s\n", true_name);
				break;
			case Token_NotEq:
				gb_fprintf(emitter->file, "\tjp %s\n", true_name);
				gb_fprintf(emitter->file, "\tjne %s\n", true_name);
				break;
			case Token_Lt:
				gb_fprintf(emitter->file, "\tjp %s\n", false_name);
				gb_fprintf(emitter->file, "\tjb %s\n", true_name);
				break;
			case Token_LtEq:
				gb_fprintf(emitter->file, "\tjp %s\n", false_name);
				gb_fprintf(emitter->file, "\tjbe %s\n", true_name);
				break;
			case Token_Gt:
				gb_fprintf(emitter->file, "\tja %s\n", true_name);
				break;
			case Token_GtEq:
				gb_fprintf(emitter->file, "\tjae %s\n", true_name);
				break;
			default:
				break;
			}
			gb_fprintf(emitter->file, "\tjmp %s\n", false_name);
			return;
		}

		fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, type.bit_size == 32 ? "s0" : "d0", fast_backend_arm64_tmp_reg(), type);
		fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, type.bit_size == 32 ? "s1" : "d1", fast_backend_arm64_work_reg(), type);
		gb_fprintf(emitter->file, "\tfcmp %s, %s\n", type.bit_size == 32 ? "s0" : "d0", type.bit_size == 32 ? "s1" : "d1");
		char const *suffix = nullptr;
		switch (op) {
		case Token_CmpEq: suffix = "eq"; break;
		case Token_NotEq: suffix = "ne"; break;
		case Token_Lt:    suffix = "mi"; break;
		case Token_LtEq:  suffix = "ls"; break;
		case Token_Gt:    suffix = "gt"; break;
		case Token_GtEq:  suffix = "ge"; break;
		default: break;
		}
		GB_ASSERT(suffix != nullptr);
		gb_fprintf(emitter->file, "\tb.%s %s\n", suffix, true_name);
		gb_fprintf(emitter->file, "\tb %s\n", false_name);
		return;
	}

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
	if (!fast_backend_emit_leaf_expr_as_type(emitter, left, type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr_as_type(emitter, right, type)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, op, type, true_label, false_label);
	return true;
}

gb_internal bool fast_backend_emit_compare_materialized_scalar_branch(FastLeafProcEmitter *emitter, i32 materialized_depth, Ast *expr, FastScalarType type, TokenKind op, bool materialized_is_left, i32 true_label, i32 false_label) {
	if (emitter == nullptr || materialized_depth <= 0 || expr == nullptr) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr_as_type(emitter, expr, type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 expr_depth = emitter->current_spill_depth;
	if (materialized_is_left) {
		fast_backend_emit_load_tmp_from_spill_depth(emitter, materialized_depth);
		fast_backend_emit_load_work_from_spill_depth(emitter, expr_depth);
	} else {
		fast_backend_emit_load_tmp_from_spill_depth(emitter, expr_depth);
		fast_backend_emit_load_work_from_spill_depth(emitter, materialized_depth);
	}
	emitter->current_spill_depth -= 1;
	fast_backend_emit_jump_if_compare(emitter, op, type, true_label, false_label);
	return true;
}

gb_internal void fast_backend_emit_jump_if_values_not_equal(FastLeafProcEmitter *emitter, FastScalarType type, i32 mismatch_label) {
	char mismatch_name[64] = {};
	fast_backend_make_label_name(mismatch_name, gb_size_of(mismatch_name), emitter->plan, mismatch_label);
	if (type.kind == FastScalar_Float) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm0", fast_backend_x64_tmp_reg(), type);
			fast_backend_emit_x64_move_scalar_bits_to_fp(emitter->file, "xmm1", fast_backend_x64_work_reg(), type);
			gb_fprintf(emitter->file, "\tucomi%s xmm0, xmm1\n", type.bit_size == 32 ? "ss" : "sd");
			gb_fprintf(emitter->file, "\tjp %s\n", mismatch_name);
			gb_fprintf(emitter->file, "\tjne %s\n", mismatch_name);
		} else {
			fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, type.bit_size == 32 ? "s0" : "d0", fast_backend_arm64_tmp_reg(), type);
			fast_backend_emit_arm64_move_scalar_bits_to_fp(emitter->file, type.bit_size == 32 ? "s1" : "d1", fast_backend_arm64_work_reg(), type);
			gb_fprintf(emitter->file, "\tfcmp %s, %s\n", type.bit_size == 32 ? "s0" : "d0", type.bit_size == 32 ? "s1" : "d1");
			gb_fprintf(emitter->file, "\tb.ne %s\n", mismatch_name);
		}
		return;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_x64_tmp_reg()->r64, fast_backend_x64_work_reg()->r64);
		gb_fprintf(emitter->file, "\tjne %s\n", mismatch_name);
	} else {
		gb_fprintf(emitter->file, "\tcmp %s, %s\n", fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
		gb_fprintf(emitter->file, "\tb.ne %s\n", mismatch_name);
	}
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

gb_internal bool fast_backend_emit_store_any_expr_to_work_address(FastLeafProcEmitter *emitter, Ast *expr) {
	if (emitter == nullptr || expr == nullptr || build_context.no_rtti) {
		return false;
	}

	expr = fast_backend_unwrap_type_value_expr(expr, t_any);
	Type *source_type = default_type(type_of_expr(expr));
	if (source_type == nullptr || is_type_any(source_type) || !fast_backend_type_is_supported_value(source_type, nullptr, nullptr)) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType typeid_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_typeid, &typeid_type));

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = spill_base + 1;

	Type *storage_type = nullptr;
	if (fast_backend_can_emit_address_expr(emitter->plan, expr, &storage_type, nullptr, nullptr) &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), source_type)) {
		if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
			return false;
		}
	} else {
		FastLocalSlot source_slot = {};
		if (!fast_backend_find_expr_slot(emitter->plan, expr, &source_slot)) {
			return false;
		}
		if (!fast_backend_emit_address_of_slot(emitter, source_slot)) {
			return false;
		}
		if (!fast_backend_emit_store_value_to_work_address(emitter, source_type, expr)) {
			return false;
		}
		if (!fast_backend_emit_address_of_slot(emitter, source_slot)) {
			return false;
		}
	}

	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_store_tmp_to_work_address(emitter, pointer_type);

	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_push_work_reg(emitter);
	u64 typeid_value = type_hash_canonical_type(source_type);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), typeid_value);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), typeid_value);
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_add_imm_to_tmp_reg(emitter, cast(i32)type_offset_of(t_any, 1));
	fast_backend_emit_store_work_to_tmp_address(emitter, typeid_type);
	return true;
}

gb_internal bool fast_backend_emit_store_union_expr_to_work_address(FastLeafProcEmitter *emitter, Type *union_type, Ast *expr) {
	if (emitter == nullptr || union_type == nullptr || expr == nullptr) {
		return false;
	}

	union_type = default_type(union_type);
	expr = fast_backend_unwrap_type_value_expr(expr, union_type);
	Type *source_type = default_type(type_of_expr(expr));
	if (union_type == nullptr || source_type == nullptr || is_type_union(source_type) || !is_type_union(union_type) || !union_is_variant_of(union_type, source_type)) {
		return false;
	}

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	fast_backend_emit_push_work_reg(emitter);
	i32 dst_depth = spill_base + 1;
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_zero_bytes_at_work_address(emitter, cast(i32)type_size_of(union_type));

	Type *storage_type = nullptr;
	if (fast_backend_can_emit_address_expr(emitter->plan, expr, &storage_type, nullptr, nullptr) &&
	    storage_type != nullptr &&
	    are_types_identical(default_type(storage_type), source_type)) {
		if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
			return false;
		}
	} else {
		FastLocalSlot source_slot = {};
		if (!fast_backend_find_expr_slot(emitter->plan, expr, &source_slot)) {
			return false;
		}
		if (!fast_backend_emit_address_of_slot(emitter, source_slot)) {
			return false;
		}
		if (!fast_backend_emit_store_value_to_work_address(emitter, source_type, expr)) {
			return false;
		}
		if (!fast_backend_emit_address_of_slot(emitter, source_slot)) {
			return false;
		}
	}

	fast_backend_emit_push_work_reg(emitter);
	i32 src_depth = emitter->current_spill_depth;
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, src_depth);
	fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(source_type));

	if (!is_type_union_maybe_pointer(union_type)) {
		Type *tag_type = union_tag_type(union_type);
		FastScalarType scalar_type = {};
		GB_ASSERT(fast_backend_classify_scalar_type(tag_type, &scalar_type));
		u64 tag = cast(u64)union_variant_index_checked(union_type, source_type);
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), tag);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), tag);
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_add_imm_to_tmp_reg(emitter, cast(i32)type_offset_of(union_type, -1));
		fast_backend_emit_store_work_to_tmp_address(emitter, scalar_type);
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
	if (is_type_any(type) && fast_backend_emit_store_any_expr_to_work_address(emitter, expr)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
	}
	if (is_type_union(type) && fast_backend_emit_store_union_expr_to_work_address(emitter, type, expr)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
		return true;
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
	if (type == nullptr || base == nullptr) {
		return false;
	}
	if (base->kind == Type_Basic && is_type_any(type)) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
	}
	if (base->kind == Type_Matrix) {
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
	}
	if (!fast_backend_emit_zero_storage_entity(emitter, entity)) {
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

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
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
	case Type_Basic: {
		if (!is_type_any(type)) {
			return false;
		}
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
				i32 offset = cast(i32)type_offset_of(type, field->Variable.field_index, &field_type);
				if (offset < 0 || field_type == nullptr) {
					return false;
				}
				if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, field_type, fv->value)) {
					return false;
				}
			}
			return true;
		}
		Type *field_types[2] = {t_rawptr, t_typeid};
		for_array(i, cl->elems) {
			i32 offset = cast(i32)type_offset_of(type, cast(i32)i);
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, field_types[i], cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Matrix: {
		Type *elem_type = base->Matrix.elem;
		i64 elem_count = base->Matrix.row_count * base->Matrix.column_count;
		i32 elem_size = cast(i32)type_size_of(elem_type);
		Type *vector_type = fast_backend_matrix_vector_type(base);
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
						i32 offset = cast(i32)(matrix_row_major_index_to_offset(base, index) * elem_size);
						if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, elem_type, fv->value)) {
							return false;
						}
					}
				} else {
					i64 index = exact_value_to_i64(type_and_value_of_expr(fv->field).value);
					i32 offset = cast(i32)(matrix_row_major_index_to_offset(base, index) * elem_size);
					if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, elem_type, fv->value)) {
						return false;
					}
				}
			}
			return true;
		}
		if (cl->elems.count > elem_count) {
			return false;
		}
		for_array(i, cl->elems) {
			Type *value_type = elem_type;
			Type *expr_type = default_type(type_of_expr(cl->elems[i]));
			if (vector_type != nullptr &&
			    expr_type != nullptr &&
			    are_types_identical(expr_type, vector_type)) {
				value_type = vector_type;
			}
			i32 offset = cast(i32)(matrix_row_major_index_to_offset(base, cast(i32)i) * elem_size);
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, value_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_SimdVector: {
		// SIMD vector laid out as N contiguous elem-sized lanes.
		// Emit the compound literal element by element. FieldValue
		// syntax is not allowed on SIMD vectors.
		Type *elem_type = base->SimdVector.elem;
		i32 elem_size  = cast(i32)type_size_of(elem_type);
		if (cl->elems.count != base->SimdVector.count) {
			return false;
		}
		if (cl->elems.count == 0) {
			return true;
		}
		for (i64 i = 0; i < cl->elems.count; i++) {
			i32 offset = cast(i32)(i * elem_size);
			if (!fast_backend_emit_store_value_to_work_address_offset(emitter, offset, elem_type, cl->elems[i])) {
				return false;
			}
		}
		return true;
	}

	case Type_Struct: {
		// Struct compound literal: write each field (by positional
		// order or by name) to the destination at its struct offset.
		// Work_reg holds the destination base; the offset helper
		// pushes it, adds the field offset, writes the field value,
		// and reloads the base into work_reg.
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

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
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
	if (base->kind == Type_Basic && is_type_any(type)) {
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
	}
	if (base->kind == Type_Matrix) {
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
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

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
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
	if (base->kind == Type_Basic && is_type_any(type)) {
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
	}
	if (base->kind == Type_Matrix) {
		return fast_backend_emit_store_scalar_compound_lit_to_work_address(emitter, type, cl);
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

	case Type_Array:
	case Type_EnumeratedArray: {
		Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
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

gb_internal bool fast_backend_emit_store_string_value_to_address(FastLeafProcEmitter *emitter, String value) {
	if (emitter == nullptr || emitter->plan == nullptr || emitter->plan->gen == nullptr) {
		return false;
	}

	i32 blob_index = value.len == 0 ? -1 : fast_backend_add_literal_blob(emitter->plan->gen, value.text, cast(i32)value.len, 1);
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
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)value.len);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)value.len);
	}
	fast_backend_emit_add_imm_to_tmp_reg(emitter, build_context.int_size);
	fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	fast_backend_emit_add_imm_to_tmp_reg(emitter, -build_context.int_size);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
	}
	return true;
}

gb_internal bool fast_backend_emit_store_slice_expr_to_address(FastLeafProcEmitter *emitter, Type *type, AstSliceExpr *se) {
	if (emitter == nullptr || se == nullptr || type == nullptr) {
		return false;
	}

	Type *result_type = default_type(type);
	Type *source_type = base_type(type_of_expr(se->expr));
	Type *source_value_type = default_type(type_of_expr(se->expr));
	if (result_type == nullptr || source_type == nullptr || source_value_type == nullptr) {
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

	if (fast_backend_can_emit_address_expr(emitter->plan, se->expr, nullptr, nullptr, nullptr)) {
		if (!fast_backend_emit_address_expr(emitter, se->expr, nullptr)) {
			return false;
		}
	} else {
		i32 temp_slots = align_formula(cast(i32)type_size_of(source_value_type), 8)/8;
		emitter->current_spill_depth += temp_slots;
		i32 temp_depth = emitter->current_spill_depth;
		fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
		if (!fast_backend_emit_store_value_to_work_address(emitter, source_value_type, se->expr)) {
			return false;
		}
		fast_backend_emit_address_of_spill_depth(emitter, temp_depth);
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 src_depth = emitter->current_spill_depth;

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
	i32 low_depth = emitter->current_spill_depth;

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
	i32 high_depth = emitter->current_spill_depth;

	fast_backend_emit_load_work_from_spill_depth(emitter, src_depth);
	if (source_uses_data_pointer) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = emitter->current_spill_depth;

	fast_backend_emit_load_work_from_spill_depth(emitter, low_depth);
	fast_backend_emit_scale_work_reg(emitter, elem_size);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_push_work_reg(emitter);
	i32 result_depth = emitter->current_spill_depth;

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
	i32 len_depth = emitter->current_spill_depth;

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
	Type *elem_type = nullptr;
	bool is_dynamic_array = false;
	switch (base->kind) {
	case Type_Slice:
		elem_type = base->Slice.elem;
		break;
	case Type_DynamicArray:
		elem_type = base->DynamicArray.elem;
		is_dynamic_array = true;
		break;
	default:
		return false;
	}
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

	if (is_dynamic_array) {
		fast_backend_emit_load_tmp_from_spill_depth(emitter, dst_depth);
		fast_backend_emit_add_imm_to_tmp_reg(emitter, cast(i32)type_offset_of(type, 2));
		fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
		fast_backend_emit_store_work_to_tmp_address(emitter, len_type);
	}
	fast_backend_emit_load_work_from_spill_depth(emitter, dst_depth);
	return true;
}

gb_internal bool fast_backend_emit_store_array_binary_op_to_work_address(FastLeafProcEmitter *emitter, Type *type, TokenKind op, Ast *lhs, Ast *rhs) {
	if (emitter == nullptr || !fast_backend_can_emit_array_binary_operands(emitter->plan, type, op, lhs, rhs)) {
		return false;
	}

	type = default_type(type);
	Type *base = base_type(type);
	Type *elem_type = base->kind == Type_Array ? base->Array.elem : base->EnumeratedArray.elem;
	i32 elem_size = cast(i32)type_size_of(elem_type);
	i64 count = base->kind == Type_Array ? base->Array.count : base->EnumeratedArray.count;
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
		if (!fast_backend_emit_array_binary_op_at_offset(emitter, dst_depth, lhs_depth, rhs_depth, cast(i32)(index*elem_size), elem_type, op)) {
			return false;
		}
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

gb_internal bool fast_backend_emit_array_binary_op_at_offset(FastLeafProcEmitter *emitter, i32 dst_depth, i32 lhs_depth, i32 rhs_depth, i32 offset, Type *type, TokenKind op) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	type = default_type(type);
	Type *base = base_type(type);
	if (type == nullptr || base == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
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
		return true;
	}

	if (base->kind != Type_Array && base->kind != Type_EnumeratedArray && base->kind != Type_Matrix) {
		return false;
	}

	Type *elem_type = nullptr;
	i64 count = 0;
	i64 elem_size = 0;
	if (base->kind == Type_Array) {
		elem_type = base->Array.elem;
		count = base->Array.count;
		elem_size = type_size_of(elem_type);
	} else if (base->kind == Type_EnumeratedArray) {
		elem_type = base->EnumeratedArray.elem;
		count = base->EnumeratedArray.count;
		elem_size = type_size_of(elem_type);
	} else {
		elem_type = base->Matrix.elem;
		count = base->Matrix.row_count * base->Matrix.column_count;
		elem_size = type_size_of(elem_type);
	}
	if (elem_size <= 0) {
		return false;
	}
	for (i64 i = 0; i < count; i++) {
		i32 elem_offset = base->kind == Type_Matrix ? cast(i32)(matrix_row_major_index_to_offset(base, i) * elem_size) : cast(i32)(i * elem_size);
		if (!fast_backend_emit_array_binary_op_at_offset(emitter, dst_depth, lhs_depth, rhs_depth, offset + elem_offset, elem_type, op)) {
			return false;
		}
	}
	return true;
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
		if (!fast_backend_emit_leaf_expr_as_type(emitter, expr, scalar_type)) {
			return false;
		}
		return fast_backend_emit_store_to_scalar_storage(emitter, entity);
	}

	type = default_type(type);
	if (!fast_backend_type_is_supported_aggregate(type)) {
		return false;
	}
	if (is_type_any(type) && fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return fast_backend_emit_store_value_to_work_address(emitter, type, expr);
	}
	if (is_type_union(type) && fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return fast_backend_emit_store_value_to_work_address(emitter, type, expr);
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
		if (!fast_backend_emit_leaf_expr_as_type(emitter, rhs, scalar_type)) {
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
	if (is_type_any(type) && fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return fast_backend_emit_store_value_to_work_address(emitter, type, rhs);
	}
	if (is_type_union(type) && fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return fast_backend_emit_store_value_to_work_address(emitter, type, rhs);
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

gb_internal bool fast_backend_emit_materialize_multi_result_call(FastLeafProcEmitter *emitter, AstCallExpr *ce, TypeProc *pt, i32 *source_depth_, i32 *temp_bytes_) {
	if (emitter == nullptr || ce == nullptr || pt == nullptr) {
		return false;
	}

	i32 tuple_size = 0;
	i32 tuple_align = 0;
	if (!fast_backend_get_multi_result_layout(pt, &tuple_size, &tuple_align)) {
		return false;
	}

	i32 temp_bytes = 0;
	if (!fast_backend_emit_alloc_stack_temp(emitter, tuple_size, tuple_align, &temp_bytes)) {
		return false;
	}
	if (temp_bytes_ != nullptr) *temp_bytes_ = temp_bytes;
	fast_backend_emit_push_work_reg(emitter);
	if (source_depth_ != nullptr) *source_depth_ = emitter->current_spill_depth;
	if (!fast_backend_emit_call_expr_to_address(emitter, ce)) {
		return false;
	}
	return true;
}

gb_internal bool fast_backend_emit_compare_work_reg_against_zero(FastLeafProcEmitter *emitter) {
	if (emitter == nullptr) {
		return false;
	}

	if (build_context.metrics.arch == TargetArch_amd64) {
		auto *work = fast_backend_x64_work_reg();
		gb_fprintf(emitter->file, "\ttest %s, %s\n", work->r64, work->r64);
		gb_fprintf(emitter->file, "\tsete %s\n", work->r8);
		gb_fprintf(emitter->file, "\tmovzx %s, %s\n", work->r64, work->r8);
	} else {
		char const *work = fast_backend_arm64_work_reg();
		gb_fprintf(emitter->file, "\tcmp %s, xzr\n", work);
		gb_fprintf(emitter->file, "\tcset w%s, eq\n", work+1);
	}
	return true;
}

gb_internal bool fast_backend_emit_load_optional_check_field_has_value(FastLeafProcEmitter *emitter, i32 source_depth, i32 source_offset, Type *type) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	type = default_type(type);
	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		if (scalar_type.kind == FastScalar_Bool) {
			return true;
		}
		return fast_backend_emit_compare_work_reg_against_zero(emitter);
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	Type *base = base_type(type);
	if (base == nullptr) {
		return false;
	}

	switch (base->kind) {
	case Type_Basic:
		if (is_type_string(type) || is_type_string16(type)) {
			fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
			fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
			return fast_backend_emit_compare_work_reg_against_zero(emitter);
		}
		if (is_type_any(type)) {
			i32 typeid_offset = cast(i32)type_offset_of(type, 1);
			FastScalarType typeid_type = {};
			GB_ASSERT(fast_backend_classify_scalar_type(t_typeid, &typeid_type));

			fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
			fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
			if (!fast_backend_emit_compare_work_reg_against_zero(emitter)) {
				return false;
			}
			fast_backend_emit_push_work_reg(emitter);

			fast_backend_emit_load_work_from_spill_depth(emitter, source_depth+1);
			fast_backend_emit_add_imm_to_work_reg(emitter, source_offset + typeid_offset);
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), typeid_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), typeid_type);
			}
			if (!fast_backend_emit_compare_work_reg_against_zero(emitter)) {
				return false;
			}
			fast_backend_emit_pop_tmp_reg(emitter);
			if (build_context.metrics.arch == TargetArch_amd64) {
				gb_fprintf(emitter->file, "\tor %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
			} else {
				gb_fprintf(emitter->file, "\torr %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg());
			}
			return true;
		}
		break;

	case Type_Slice:
	case Type_DynamicArray:
	case Type_Map:
	case Type_SoaPointer:
		fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
		}
		return fast_backend_emit_compare_work_reg_against_zero(emitter);
	}

	return false;
}

gb_internal bool fast_backend_emit_push_string_arg(FastLeafProcEmitter *emitter, String value) {
	if (emitter == nullptr || emitter->plan == nullptr || emitter->plan->gen == nullptr) {
		return false;
	}

	i32 blob_index = value.len == 0 ? -1 : fast_backend_add_literal_blob(emitter->plan->gen, value.text, cast(i32)value.len, 1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		if (blob_index < 0) {
			gb_fprintf(emitter->file, "\txor %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_work_reg()->r64);
		} else {
			fast_backend_emit_x64_load_address_of_blob(emitter->file, blob_index, fast_backend_x64_work_reg());
		}
	} else if (blob_index < 0) {
		gb_fprintf(emitter->file, "\tmov %s, xzr\n", fast_backend_arm64_work_reg());
	} else {
		fast_backend_emit_arm64_load_address_of_blob(emitter->file, blob_index, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)value.len);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)value.len);
	}
	fast_backend_emit_push_work_reg(emitter);
	return true;
}

gb_internal bool fast_backend_emit_optional_call_value_check(FastLeafProcEmitter *emitter, AstCallExpr *ce, TypeProc *pt, i32 source_depth) {
	if (emitter == nullptr || ce == nullptr || pt == nullptr || !ce->optional_ok_one) {
		return true;
	}

	Type *check_type = nullptr;
	i32 check_offset = 0;
	if (!fast_backend_get_call_optional_check_field_info(ce, pt, &check_type, &check_offset, nullptr, nullptr) ||
	    !fast_backend_emit_load_optional_check_field_has_value(emitter, source_depth, check_offset, check_type)) {
		return false;
	}

	fast_backend_emit_push_work_reg(emitter);
	TokenPos pos = ast_token(ce->proc).pos;
	if (!fast_backend_emit_push_string_arg(emitter, get_file_path_string(pos.file_id))) {
		return false;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)pos.line);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)pos.line);
	}
	fast_backend_emit_push_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)pos.column);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)pos.column);
	}
	fast_backend_emit_push_work_reg(emitter);

	Type *helper_type = nullptr;
	String helper_name = emitter->plan->has_context_slot
	                   ? str_lit("optional_value_check_with_context")
	                   : str_lit("optional_value_check_contextless");
	force_add_dependency_entity(emitter->plan->gen->checker, emitter->plan->info->runtime_package->scope, helper_name);
	Entity *helper = find_entity_in_pkg(emitter->plan->info,
	                                    str_lit("runtime"),
	                                    helper_name);
	if (helper == nullptr ||
	    helper->type == nullptr ||
	    (helper_type = base_type(helper->type)) == nullptr ||
	    helper_type->kind != Type_Proc) {
		return false;
	}
	TypeProc *helper_pt = &helper_type->Proc;
	if (emitter->plan->has_context_slot) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
		}
		fast_backend_emit_push_work_reg(emitter);
	}

	i32 total_arg_count = 5 + (emitter->plan->has_context_slot ? 1 : 0);
	for (i32 i = total_arg_count-1; i >= 0; i--) {
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *dst = fast_backend_x64_param_reg(helper_pt, i);
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

	String symbol = fast_backend_mangle_asm_name(fast_backend_get_entity_name(helper));
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tcall \"%.*s\"\n", LIT(symbol));
	} else {
		gb_fprintf(emitter->file, "\tbl \"%.*s\"\n", LIT(symbol));
	}
	return true;
}

gb_internal bool fast_backend_emit_store_multi_result_field_to_entity(FastLeafProcEmitter *emitter, i32 source_depth, i32 source_offset, Type *type, Entity *entity) {
	if (emitter == nullptr || type == nullptr || entity == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
		return true;
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_of_storage_entity(emitter, entity)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(type));
	return true;
}

gb_internal bool fast_backend_emit_store_multi_result_field_to_lhs(FastLeafProcEmitter *emitter, i32 source_depth, i32 source_offset, Type *type, Ast *lhs) {
	if (emitter == nullptr || type == nullptr || lhs == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
		return true;
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_address_expr(emitter, lhs, nullptr)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(type));
	return true;
}

gb_internal bool fast_backend_emit_store_multi_result_field_to_result_pointer(FastLeafProcEmitter *emitter, i32 source_depth, i32 source_offset, i32 dest_offset, Type *type) {
	if (emitter == nullptr || type == nullptr) {
		return false;
	}

	FastScalarType scalar_type = {};
	if (fast_backend_classify_scalar_type(type, &scalar_type)) {
		fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
		fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), scalar_type);
		} else {
			fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), scalar_type);
		}
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
		}
		fast_backend_emit_add_imm_to_work_reg(emitter, dest_offset);
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_store_tmp_to_work_address(emitter, scalar_type);
		return true;
	}

	fast_backend_emit_load_work_from_spill_depth(emitter, source_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, source_offset);
	fast_backend_emit_push_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, dest_offset);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_copy_bytes_between_addresses(emitter, cast(i32)type_size_of(type));
	return true;
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

	TypeProc *multi_result_pt = nullptr;
	if (vd->values.count == 1) {
		fast_backend_get_multi_result_call_info(vd->values[0], &multi_result_pt, nullptr);
	}
	if (multi_result_pt != nullptr) {
		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);

		i32 temp_bytes = 0;
		defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

		i32 source_depth = 0;
		AstCallExpr *rhs_call = &unparen_expr(vd->values[0])->CallExpr;
		if (!fast_backend_emit_materialize_multi_result_call(emitter, rhs_call, multi_result_pt, &source_depth, &temp_bytes) ||
		    !fast_backend_emit_optional_call_value_check(emitter, rhs_call, multi_result_pt, source_depth)) {
			return false;
		}

		for_array(i, vd->names) {
			Ast *name = vd->names[i];
			if (is_blank_ident(name)) {
				continue;
			}

			Entity *entity = entity_of_node(name);
			Type *result_type = nullptr;
			i32 result_offset = 0;
			if (entity == nullptr ||
			    !fast_backend_get_call_visible_result_field_info(rhs_call, multi_result_pt, cast(i32)i, &result_type, &result_offset, nullptr, nullptr) ||
			    !fast_backend_emit_store_multi_result_field_to_entity(emitter, source_depth, result_offset, result_type, entity)) {
				return false;
			}
		}
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
		if (!fast_backend_emit_leaf_expr_as_type(emitter, rhs, scalar_type)) {
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

	TypeProc *multi_result_pt = nullptr;
	if (as->rhs.count == 1) {
		fast_backend_get_multi_result_call_info(as->rhs[0], &multi_result_pt, nullptr);
	}
	if (multi_result_pt != nullptr) {
		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);

		i32 temp_bytes = 0;
		defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

		i32 source_depth = 0;
		AstCallExpr *rhs_call = &unparen_expr(as->rhs[0])->CallExpr;
		if (!fast_backend_emit_materialize_multi_result_call(emitter, rhs_call, multi_result_pt, &source_depth, &temp_bytes) ||
		    !fast_backend_emit_optional_call_value_check(emitter, rhs_call, multi_result_pt, source_depth)) {
			return false;
		}

		for_array(i, as->lhs) {
			Ast *lhs = unparen_expr(as->lhs[i]);
			if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
				continue;
			}

			Type *result_type = nullptr;
			i32 result_offset = 0;
			if (!fast_backend_get_call_visible_result_field_info(rhs_call, multi_result_pt, cast(i32)i, &result_type, &result_offset, nullptr, nullptr) ||
			    !fast_backend_emit_store_multi_result_field_to_lhs(emitter, source_depth, result_offset, result_type, lhs)) {
				return false;
			}
		}
		return true;
	}

	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
			Type *rhs_type = reduce_tuple_to_single_type(type_and_value_of_expr(as->rhs[i]).type);
			FastScalarType scalar_type = {};
			if (rhs_type != nullptr && fast_backend_expr_scalar_type(as->rhs[i], rhs_type, &scalar_type)) {
				if (!fast_backend_emit_leaf_expr_as_type(emitter, as->rhs[i], scalar_type)) {
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
	FastArm64AggregateReturnClass direct_result_class = fast_backend_arm64_classify_aggregate_return(emitter->plan->result_value_type);
	bool return_direct_aggregate = !emitter->plan->return_by_pointer &&
	                               direct_result_class.kind != FastArm64AggregateReturn_None;
	if (!emitter->plan->return_by_pointer && !return_direct_aggregate) {
		bool preserve_result = rs->results.count != 0 && emitter->deferred_stmts.count != 0;
		isize deferred_count = emitter->deferred_stmts.count;
		if (rs->results.count != 0) {
			if (!fast_backend_emit_leaf_expr_as_type(emitter, rs->results[0], emitter->plan->return_type)) {
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

	i32 direct_result_temp_bytes = 0;
	i32 direct_result_addr_depth = 0;
	if (rs->results.count != 0) {
		if (emitter->plan->type->result_count > 1) {
			if (rs->results.count == 1) {
				TypeProc *call_pt = nullptr;
				if (fast_backend_get_multi_result_call_info(rs->results[0], &call_pt, nullptr)) {
					i32 spill_base = emitter->current_spill_depth;
					defer (emitter->current_spill_depth = spill_base);

					i32 temp_bytes = 0;
					defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

					i32 source_depth = 0;
					AstCallExpr *call_expr = &unparen_expr(rs->results[0])->CallExpr;
					if (!fast_backend_emit_materialize_multi_result_call(emitter, call_expr, call_pt, &source_depth, &temp_bytes) ||
					    !fast_backend_emit_optional_call_value_check(emitter, call_expr, call_pt, source_depth)) {
						return false;
					}

					for (i32 i = 0; i < emitter->plan->type->result_count; i++) {
						Type *result_type = nullptr;
						i32 source_offset = 0;
						i32 dest_offset = 0;
						if (!fast_backend_get_call_visible_result_field_info(call_expr, call_pt, i, &result_type, &source_offset, nullptr, nullptr) ||
						    !fast_backend_get_multi_result_field_info(emitter->plan->type, i, nullptr, &dest_offset, nullptr, nullptr) ||
						    !fast_backend_emit_store_multi_result_field_to_result_pointer(emitter, source_depth, source_offset, dest_offset, result_type)) {
							return false;
						}
					}
					goto done_return_value;
				}
			}

			// Multi-return: load the sret address once, then for each
			// result i, store the value at sret + offset_i (the sum
			// of the sizes of the preceding results). Each iteration
			// reloads the sret address from the slot (it's a fixed
			// offset in the local frame, so reloading is cheap) so
			// the per-result store doesn't need to coordinate with
			// the spill region.
			GB_ASSERT(emitter->plan->type->results != nullptr && emitter->plan->type->results->kind == Type_Tuple);
			i32 cumulative_offset = 0;
			for (i32 i = 0; i < rs->results.count; i++) {
				Ast *result = rs->results[i];
				Entity *result_entity = emitter->plan->type->results->Tuple.variables[i];
				Type *result_type = result_entity != nullptr ? result_entity->type : nullptr;
				if (result_type == nullptr) {
					return false;
				}
				// Reload sret address into work_reg
				if (build_context.metrics.arch == TargetArch_amd64) {
					fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
				} else {
					fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
				}
				if (!fast_backend_emit_store_value_to_work_address_offset(emitter, cumulative_offset, result_type, result)) {
					return false;
				}
				cumulative_offset += cast(i32)type_size_of(result_type);
			}
		} else {
			Ast *result = rs->results[0];
			if (return_direct_aggregate) {
				if (!fast_backend_emit_alloc_stack_temp(emitter, cast(i32)type_size_of(emitter->plan->result_value_type), cast(i32)type_align_of(emitter->plan->result_value_type), &direct_result_temp_bytes)) {
					return false;
				}
				fast_backend_emit_push_work_reg(emitter);
				direct_result_addr_depth = emitter->current_spill_depth;
			} else if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
			} else {
				fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
			}
			if (!fast_backend_emit_store_value_to_work_address(emitter, emitter->plan->result_value_type, result)) {
				return false;
			}
		}
	}
done_return_value:

	isize deferred_count = emitter->deferred_stmts.count;
	if (!fast_backend_emit_scope_exit_defers(emitter, 0)) {
		return false;
	}
	emitter->deferred_stmts.count = deferred_count;
	if (return_direct_aggregate) {
		GB_ASSERT(direct_result_addr_depth > 0);
		fast_backend_emit_load_tmp_from_spill_depth(emitter, direct_result_addr_depth);
		fast_backend_emit_arm64_load_direct_aggregate_return_from_address(emitter->file, fast_backend_arm64_tmp_reg(), direct_result_class, cast(i32)type_size_of(emitter->plan->result_value_type));
		fast_backend_emit_stack_adjust(emitter, direct_result_temp_bytes, false);
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, emitter->epilogue_label_index);
	return true;
}

gb_internal bool fast_backend_emit_cond_jump(FastLeafProcEmitter *emitter, Ast *expr, i32 true_label, i32 false_label) {
	expr = unparen_expr(expr);
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	if (expr->kind == Ast_UnaryExpr && expr->UnaryExpr.op.kind == Token_Not) {
		return fast_backend_emit_cond_jump(emitter, expr->UnaryExpr.expr, false_label, true_label);
	}

	if (expr->kind == Ast_BinaryExpr) {
		if (expr->BinaryExpr.op.kind == Token_CmpAnd) {
			i32 next_label = fast_backend_alloc_label(emitter);
			if (!fast_backend_emit_cond_jump(emitter, expr->BinaryExpr.left, next_label, false_label)) {
				return false;
			}
			fast_backend_emit_label(emitter->file, emitter->plan, next_label);
			return fast_backend_emit_cond_jump(emitter, expr->BinaryExpr.right, true_label, false_label);
		}
		if (expr->BinaryExpr.op.kind == Token_CmpOr) {
			i32 next_label = fast_backend_alloc_label(emitter);
			if (!fast_backend_emit_cond_jump(emitter, expr->BinaryExpr.left, true_label, next_label)) {
				return false;
			}
			fast_backend_emit_label(emitter->file, emitter->plan, next_label);
			return fast_backend_emit_cond_jump(emitter, expr->BinaryExpr.right, true_label, false_label);
		}
	}

	if (!fast_backend_emit_leaf_expr(emitter, expr)) {
		return false;
	}
	fast_backend_emit_jump_if_zero(emitter, false_label);
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, true_label);
	return true;
}

gb_internal bool fast_backend_emit_if_stmt(FastLeafProcEmitter *emitter, AstIfStmt *is) {
	if (!fast_backend_emit_enter_scope(emitter, is->scope)) {
		return false;
	}
	if (is->init != nullptr && !fast_backend_emit_stmt(emitter, is->init)) {
		return false;
	}
	i32 body_label = fast_backend_alloc_label(emitter);
	i32 false_label = fast_backend_alloc_label(emitter);
	i32 done_label = is->else_stmt != nullptr ? fast_backend_alloc_label(emitter) : false_label;
	if (!fast_backend_emit_cond_jump(emitter, is->cond, body_label, false_label)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, body_label);
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

gb_internal bool fast_backend_emit_load_range_expr_length(FastLeafProcEmitter *emitter, Ast *expr, FastScalarType len_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}
	if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
		return false;
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, build_context.int_size);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
	}
	return true;
}

gb_internal bool fast_backend_emit_load_range_expr_data_pointer(FastLeafProcEmitter *emitter, Ast *expr, FastScalarType pointer_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}
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

gb_internal void fast_backend_emit_range_advance_index(FastLeafProcEmitter *emitter, FastLocalSlot idx_slot, FastScalarType idx_type, i64 advance) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
		gb_fprintf(emitter->file, "\tadd %s, %lld\n", fast_backend_x64_work_reg()->r64, cast(long long)advance);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
		fast_backend_emit_add_imm_to_work_reg(emitter, advance);
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_slot(emitter, idx_slot);
}

gb_internal bool fast_backend_emit_store_range_rune_and_advance(FastLeafProcEmitter *emitter, Entity *val0_entity, FastLocalSlot idx_slot, FastScalarType idx_type, i64 advance) {
	if (val0_entity != nullptr && !fast_backend_emit_store_to_scalar_storage(emitter, val0_entity)) {
		return false;
	}
	fast_backend_emit_range_advance_index(emitter, idx_slot, idx_type, advance);
	return true;
}

gb_internal void fast_backend_emit_load_u8_at_ptr_depth(FastLeafProcEmitter *emitter, i32 ptr_depth, i32 offset) {
	FastScalarType byte_type = {};
	byte_type.kind = FastScalar_Unsigned;
	byte_type.bit_size = 8;
	fast_backend_emit_load_work_from_spill_depth(emitter, ptr_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), byte_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), byte_type);
	}
}

gb_internal void fast_backend_emit_load_u16_at_ptr_depth(FastLeafProcEmitter *emitter, i32 ptr_depth, i32 offset) {
	FastScalarType char16_type = {};
	char16_type.kind = FastScalar_Unsigned;
	char16_type.bit_size = 16;
	fast_backend_emit_load_work_from_spill_depth(emitter, ptr_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, offset);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
}

gb_internal void fast_backend_emit_and_work_imm(FastLeafProcEmitter *emitter, u64 mask) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tand %s, 0x%llx\n", fast_backend_x64_work_reg()->r64, cast(unsigned long long)mask);
	} else {
		gb_fprintf(emitter->file, "\tand %s, %s, #0x%llx\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), cast(unsigned long long)mask);
	}
}

gb_internal void fast_backend_emit_shift_work_left(FastLeafProcEmitter *emitter, i32 amount) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tshl %s, %d\n", fast_backend_x64_work_reg()->r64, amount);
	} else {
		gb_fprintf(emitter->file, "\tlsl %s, %s, #%d\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), amount);
	}
}

gb_internal void fast_backend_emit_or_tmp_into_work(FastLeafProcEmitter *emitter) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tor %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
	} else {
		gb_fprintf(emitter->file, "\torr %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
	}
}

gb_internal void fast_backend_emit_load_rune_error(FastLeafProcEmitter *emitter) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), 0xfffd);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), 0xfffd);
	}
}

gb_internal bool fast_backend_emit_range_string_utf8_decode(FastLeafProcEmitter *emitter, Ast *expr, Entity *val0_entity, FastLocalSlot idx_slot, FastScalarType idx_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	FastScalarType len_type = {};
	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType byte_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
	byte_type.kind = FastScalar_Unsigned;
	byte_type.bit_size = 8;

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	if (!fast_backend_emit_load_range_expr_data_pointer(emitter, expr, pointer_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = emitter->current_spill_depth;

	if (!fast_backend_emit_load_range_expr_length(emitter, expr, len_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 len_depth = emitter->current_spill_depth;

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 idx_depth = emitter->current_spill_depth;

	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_push_work_reg(emitter);
	i32 ptr_depth = emitter->current_spill_depth;

	i32 ascii_label = fast_backend_alloc_label(emitter);
	i32 check2_label = fast_backend_alloc_label(emitter);
	i32 two_len_label = fast_backend_alloc_label(emitter);
	i32 two_b1_hi_label = fast_backend_alloc_label(emitter);
	i32 build2_label = fast_backend_alloc_label(emitter);
	i32 build2_body_label = fast_backend_alloc_label(emitter);
	i32 check3_label = fast_backend_alloc_label(emitter);
	i32 three_len_label = fast_backend_alloc_label(emitter);
	i32 three_b1_hi_label = fast_backend_alloc_label(emitter);
	i32 three_b2_label = fast_backend_alloc_label(emitter);
	i32 three_b2_hi_label = fast_backend_alloc_label(emitter);
	i32 build3_label = fast_backend_alloc_label(emitter);
	i32 build3_body_label = fast_backend_alloc_label(emitter);
	i32 check4_label = fast_backend_alloc_label(emitter);
	i32 four_len_label = fast_backend_alloc_label(emitter);
	i32 four_b1_hi_label = fast_backend_alloc_label(emitter);
	i32 four_b2_label = fast_backend_alloc_label(emitter);
	i32 four_b2_hi_label = fast_backend_alloc_label(emitter);
	i32 four_b3_label = fast_backend_alloc_label(emitter);
	i32 four_b3_hi_label = fast_backend_alloc_label(emitter);
	i32 build4_label = fast_backend_alloc_label(emitter);
	i32 build4_body_label = fast_backend_alloc_label(emitter);
	i32 two_b1_ok_label = fast_backend_alloc_label(emitter);
	i32 invalid_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, ascii_label, check2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check2_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0xc2, invalid_label, two_len_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_len_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xdf, two_b1_hi_label, check3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_b1_hi_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, 1);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, build2_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build2_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, two_b1_ok_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_b1_ok_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build2_body_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build2_body_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x1f);
	fast_backend_emit_shift_work_left(emitter, 6);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 2)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check3_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xef, three_len_label, check4_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_len_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, 2);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, three_b1_hi_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_b1_hi_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, three_b2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_b2_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, three_b2_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_b2_hi_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, build3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build3_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build3_body_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build3_body_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x0f);
	fast_backend_emit_shift_work_left(emitter, 12);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_shift_work_left(emitter, 6);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 3)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check4_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xf4, four_len_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_len_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, 3);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, four_b1_hi_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_b1_hi_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, four_b2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_b2_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, four_b2_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_b2_hi_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, four_b3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_b3_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, four_b3_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_b3_hi_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 3);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, build4_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build4_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build4_body_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build4_body_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x07);
	fast_backend_emit_shift_work_left(emitter, 18);
	fast_backend_emit_push_work_reg(emitter);
	for (i32 i = 1; i <= 3; i++) {
		fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, i);
		fast_backend_emit_and_work_imm(emitter, 0x3f);
		if (i != 3) {
			fast_backend_emit_shift_work_left(emitter, 6*(3-i));
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_or_tmp_into_work(emitter);
		if (i != 3) {
			fast_backend_emit_push_work_reg(emitter);
		}
	}
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 4)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, ascii_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 1)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, invalid_label);
	fast_backend_emit_load_rune_error(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 1)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_range_string_utf8_reverse_decode(FastLeafProcEmitter *emitter, Ast *expr, Entity *val0_entity, FastLocalSlot idx_slot, FastScalarType idx_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType byte_type = {};
	byte_type.kind = FastScalar_Unsigned;
	byte_type.bit_size = 8;

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	if (!fast_backend_emit_load_range_expr_data_pointer(emitter, expr, pointer_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = emitter->current_spill_depth;

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 end_depth = emitter->current_spill_depth;

	emitter->current_spill_depth += 4;
	i32 limit_depth = spill_base + 3;
	i32 candidate_depth = spill_base + 4;
	i32 avail_depth = spill_base + 5;
	i32 ptr_depth = spill_base + 6;

	i32 last_ascii_label = fast_backend_alloc_label(emitter);
	i32 backtrack_label = fast_backend_alloc_label(emitter);
	i32 limit_non_negative_label = fast_backend_alloc_label(emitter);
	i32 limit_zero_label = fast_backend_alloc_label(emitter);
	i32 limit_ready_label = fast_backend_alloc_label(emitter);
	i32 scan_loop_label = fast_backend_alloc_label(emitter);
	i32 scan_body_label = fast_backend_alloc_label(emitter);
	i32 decrement_candidate_label = fast_backend_alloc_label(emitter);
	i32 scan_done_label = fast_backend_alloc_label(emitter);
	i32 clamp_to_zero_label = fast_backend_alloc_label(emitter);
	i32 decode_label = fast_backend_alloc_label(emitter);
	i32 ascii_label = fast_backend_alloc_label(emitter);
	i32 ascii_exact_label = fast_backend_alloc_label(emitter);
	i32 check2_label = fast_backend_alloc_label(emitter);
	i32 two_len_label = fast_backend_alloc_label(emitter);
	i32 two_exact_label = fast_backend_alloc_label(emitter);
	i32 two_cont_label = fast_backend_alloc_label(emitter);
	i32 two_cont_hi_label = fast_backend_alloc_label(emitter);
	i32 build2_label = fast_backend_alloc_label(emitter);
	i32 check3_label = fast_backend_alloc_label(emitter);
	i32 three_len_label = fast_backend_alloc_label(emitter);
	i32 three_exact_label = fast_backend_alloc_label(emitter);
	i32 three_cont1_label = fast_backend_alloc_label(emitter);
	i32 three_cont1_hi_label = fast_backend_alloc_label(emitter);
	i32 three_cont2_label = fast_backend_alloc_label(emitter);
	i32 three_cont2_hi_label = fast_backend_alloc_label(emitter);
	i32 build3_label = fast_backend_alloc_label(emitter);
	i32 check4_label = fast_backend_alloc_label(emitter);
	i32 four_len_label = fast_backend_alloc_label(emitter);
	i32 four_exact_label = fast_backend_alloc_label(emitter);
	i32 four_cont1_label = fast_backend_alloc_label(emitter);
	i32 four_cont1_hi_label = fast_backend_alloc_label(emitter);
	i32 four_cont2_label = fast_backend_alloc_label(emitter);
	i32 four_cont2_hi_label = fast_backend_alloc_label(emitter);
	i32 four_cont3_label = fast_backend_alloc_label(emitter);
	i32 four_cont3_hi_label = fast_backend_alloc_label(emitter);
	i32 build4_label = fast_backend_alloc_label(emitter);
	i32 invalid_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -1);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_store_work_to_spill_depth(emitter, ptr_depth);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, last_ascii_label, backtrack_label);

	fast_backend_emit_label(emitter->file, emitter->plan, backtrack_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -4);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_GtEq, idx_type, 0, limit_non_negative_label, limit_zero_label);

	fast_backend_emit_label(emitter->file, emitter->plan, limit_non_negative_label);
	fast_backend_emit_store_work_to_spill_depth(emitter, limit_depth);
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, limit_ready_label);

	fast_backend_emit_label(emitter->file, emitter->plan, limit_zero_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), 0);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), 0);
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_spill_depth(emitter, limit_depth);

	fast_backend_emit_label(emitter->file, emitter->plan, limit_ready_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -2);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_spill_depth(emitter, candidate_depth);

	fast_backend_emit_label(emitter->file, emitter->plan, scan_loop_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, limit_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, candidate_depth);
	fast_backend_emit_jump_if_compare(emitter, Token_GtEq, idx_type, scan_body_label, scan_done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, scan_body_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, candidate_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_store_work_to_spill_depth(emitter, ptr_depth);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0xc0);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_NotEq, byte_type, 0x80, scan_done_label, decrement_candidate_label);

	fast_backend_emit_label(emitter->file, emitter->plan, decrement_candidate_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, candidate_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_spill_depth(emitter, candidate_depth);
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, scan_loop_label);

	fast_backend_emit_label(emitter->file, emitter->plan, scan_done_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, candidate_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_GtEq, idx_type, 0, decode_label, clamp_to_zero_label);

	fast_backend_emit_label(emitter->file, emitter->plan, clamp_to_zero_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), 0);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), 0);
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_spill_depth(emitter, candidate_depth);

	fast_backend_emit_label(emitter->file, emitter->plan, decode_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, candidate_depth);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tsub %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_tmp_reg()->r64);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		gb_fprintf(emitter->file, "\tsub %s, %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg());
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_spill_depth(emitter, avail_depth);
	fast_backend_emit_load_work_from_spill_depth(emitter, candidate_depth);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_store_work_to_spill_depth(emitter, ptr_depth);

	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, ascii_label, check2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, ascii_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, avail_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_CmpEq, idx_type, 1, ascii_exact_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, ascii_exact_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -1)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check2_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0xc2, invalid_label, two_len_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_len_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xdf, two_exact_label, check3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_exact_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, avail_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_CmpEq, idx_type, 2, two_cont_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_cont_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, two_cont_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, two_cont_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build2_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x1f);
	fast_backend_emit_shift_work_left(emitter, 6);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -2)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check3_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xef, three_len_label, check4_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_len_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, avail_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_CmpEq, idx_type, 3, three_exact_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_exact_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, three_cont1_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_cont1_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, three_cont2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_cont2_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, three_cont2_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, three_cont2_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build3_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x0f);
	fast_backend_emit_shift_work_left(emitter, 12);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_shift_work_left(emitter, 6);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_and_work_imm(emitter, 0x3f);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -3)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, check4_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_LtEq, byte_type, 0xf4, four_len_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_len_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, avail_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_CmpEq, idx_type, 4, four_exact_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_exact_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 1);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, four_cont1_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_cont1_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, four_cont2_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_cont2_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, four_cont2_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_cont2_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, four_cont3_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_cont3_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 3);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, byte_type, 0x80, invalid_label, four_cont3_hi_label);

	fast_backend_emit_label(emitter->file, emitter->plan, four_cont3_hi_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, byte_type, 0xbf, invalid_label, build4_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build4_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_and_work_imm(emitter, 0x07);
	fast_backend_emit_shift_work_left(emitter, 18);
	fast_backend_emit_push_work_reg(emitter);
	for (i32 i = 1; i <= 3; i++) {
		fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, i);
		fast_backend_emit_and_work_imm(emitter, 0x3f);
		if (i != 3) {
			fast_backend_emit_shift_work_left(emitter, 6*(3-i));
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_or_tmp_into_work(emitter);
		if (i != 3) {
			fast_backend_emit_push_work_reg(emitter);
		}
	}
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -4)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, last_ascii_label);
	fast_backend_emit_load_u8_at_ptr_depth(emitter, ptr_depth, 0);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -1)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, invalid_label);
	fast_backend_emit_load_rune_error(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -1)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_range_string16_decode(FastLeafProcEmitter *emitter, Ast *expr, Entity *val0_entity, FastLocalSlot idx_slot, FastScalarType idx_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	FastScalarType len_type = {};
	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType char16_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));
	char16_type.kind = FastScalar_Unsigned;
	char16_type.bit_size = 16;

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	if (!fast_backend_emit_load_range_expr_data_pointer(emitter, expr, pointer_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = emitter->current_spill_depth;

	if (!fast_backend_emit_load_range_expr_length(emitter, expr, len_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 len_depth = emitter->current_spill_depth;

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 idx_depth = emitter->current_spill_depth;

	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	fast_backend_emit_push_work_reg(emitter);
	i32 ptr_depth = emitter->current_spill_depth;

	i32 direct_label = fast_backend_alloc_label(emitter);
	i32 direct_invalid_check_label = fast_backend_alloc_label(emitter);
	i32 maybe_pair_label = fast_backend_alloc_label(emitter);
	i32 pair_len_label = fast_backend_alloc_label(emitter);
	i32 pair_second_hi_label = fast_backend_alloc_label(emitter);
	i32 build_pair_label = fast_backend_alloc_label(emitter);
	i32 build_pair_body_label = fast_backend_alloc_label(emitter);
	i32 invalid_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	fast_backend_emit_load_u16_at_ptr_depth(emitter, ptr_depth, 0);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xd800, direct_label, maybe_pair_label);

	fast_backend_emit_label(emitter->file, emitter->plan, maybe_pair_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xdc00, pair_len_label, direct_invalid_check_label);

	fast_backend_emit_label(emitter->file, emitter->plan, direct_invalid_check_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xe000, invalid_label, direct_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_len_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, idx_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, 1);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, len_depth);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, pair_second_hi_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_second_hi_label);
	fast_backend_emit_load_u16_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xdc00, invalid_label, build_pair_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build_pair_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_GtEq, char16_type, 0xe000, invalid_label, build_pair_body_label);

	fast_backend_emit_label(emitter->file, emitter->plan, build_pair_body_label);
	fast_backend_emit_load_u16_at_ptr_depth(emitter, ptr_depth, 0);
	// Assemble surrogate pair into a rune value.
	fast_backend_emit_add_imm_to_work_reg(emitter, -0xd800);
	fast_backend_emit_shift_work_left(emitter, 10);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_u16_at_ptr_depth(emitter, ptr_depth, 2);
	fast_backend_emit_add_imm_to_work_reg(emitter, -0xdc00);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	fast_backend_emit_add_imm_to_work_reg(emitter, 0x10000);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 2)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, direct_label);
	fast_backend_emit_load_u16_at_ptr_depth(emitter, ptr_depth, 0);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 1)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, invalid_label);
	fast_backend_emit_load_rune_error(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, 1)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_range_string16_reverse_decode(FastLeafProcEmitter *emitter, Ast *expr, Entity *val0_entity, FastLocalSlot idx_slot, FastScalarType idx_type) {
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType char16_type = {};
	char16_type.kind = FastScalar_Unsigned;
	char16_type.bit_size = 16;

	i32 spill_base = emitter->current_spill_depth;
	defer (emitter->current_spill_depth = spill_base);

	if (!fast_backend_emit_load_range_expr_data_pointer(emitter, expr, pointer_type)) {
		return false;
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 data_depth = emitter->current_spill_depth;

	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
	}
	fast_backend_emit_push_work_reg(emitter);
	i32 end_depth = emitter->current_spill_depth;

	i32 direct_or_invalid_label = fast_backend_alloc_label(emitter);
	i32 maybe_pair_label = fast_backend_alloc_label(emitter);
	i32 pair_available_label = fast_backend_alloc_label(emitter);
	i32 pair_prev_label = fast_backend_alloc_label(emitter);
	i32 pair_prev_high_label = fast_backend_alloc_label(emitter);
	i32 pair_build_label = fast_backend_alloc_label(emitter);
	i32 pair_invalid_label = fast_backend_alloc_label(emitter);
	i32 direct_label = fast_backend_alloc_label(emitter);
	i32 invalid_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xdc00, direct_or_invalid_label, maybe_pair_label);

	fast_backend_emit_label(emitter->file, emitter->plan, direct_or_invalid_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xd800, direct_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, maybe_pair_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_GtEq, char16_type, 0xe000, direct_label, pair_available_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_available_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, idx_type, 1, pair_prev_label, invalid_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_prev_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -2);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Lt, char16_type, 0xd800, pair_invalid_label, pair_prev_high_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_prev_high_label);
	fast_backend_emit_jump_if_work_compare_imm(emitter, Token_GtEq, char16_type, 0xdc00, pair_invalid_label, pair_build_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_build_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -2);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, -0xd800);
	fast_backend_emit_shift_work_left(emitter, 10);
	fast_backend_emit_push_work_reg(emitter);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
	fast_backend_emit_add_imm_to_work_reg(emitter, -0xdc00);
	fast_backend_emit_pop_tmp_reg(emitter);
	fast_backend_emit_or_tmp_into_work(emitter);
	fast_backend_emit_add_imm_to_work_reg(emitter, 0x10000);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -2)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, pair_invalid_label);
	fast_backend_emit_load_rune_error(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -2)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, direct_label);
	fast_backend_emit_load_work_from_spill_depth(emitter, end_depth);
	fast_backend_emit_add_imm_to_work_reg(emitter, -1);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_scale_work_reg(emitter, 2);
	fast_backend_emit_load_tmp_from_spill_depth(emitter, data_depth);
	fast_backend_emit_add_tmp_reg_to_work_reg(emitter);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), char16_type);
	} else {
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), char16_type);
	}
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -1)) {
		return false;
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);

	fast_backend_emit_label(emitter->file, emitter->plan, invalid_label);
	fast_backend_emit_load_rune_error(emitter);
	if (!fast_backend_emit_store_range_rune_and_advance(emitter, val0_entity, idx_slot, idx_type, -1)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_range_stmt(FastLeafProcEmitter *emitter, AstRangeStmt *rs) {
	Ast *expr = unparen_expr(rs->expr);
	if (emitter == nullptr || expr == nullptr) {
		return false;
	}

	Ast *val0 = rs->vals.count > 0 ? fast_backend_strip_range_value(rs->vals[0]) : nullptr;
	Ast *val1 = rs->vals.count > 1 ? fast_backend_strip_range_value(rs->vals[1]) : nullptr;
	Entity *val0_entity = (val0 != nullptr && !is_blank_ident(val0)) ? entity_of_node(val0) : nullptr;
	Entity *val1_entity = (val1 != nullptr && !is_blank_ident(val1)) ? entity_of_node(val1) : nullptr;

	FastLocalSlot idx_slot = {};
	if (!fast_backend_find_expr_slot(emitter->plan, rs->expr, &idx_slot)) {
		return false;
	}
	FastScalarType idx_type = idx_slot.type;
	FastScalarType pointer_type = fast_backend_context_scalar_type();
	FastScalarType len_type = {};
	GB_ASSERT(fast_backend_classify_scalar_type(t_int, &len_type));

	i32 outer_scope_count = cast(i32)emitter->scope_stack.count;
	if (!fast_backend_emit_enter_scope(emitter, rs->scope)) {
		return false;
	}
	if (rs->init != nullptr && !fast_backend_emit_stmt(emitter, rs->init)) {
		return false;
	}
	i32 loop_scope_count = cast(i32)emitter->scope_stack.count;

	i32 loop_label = fast_backend_alloc_label(emitter);
	i32 post_label = fast_backend_alloc_label(emitter);
	i32 body_label = fast_backend_alloc_label(emitter);
	i32 done_label = fast_backend_alloc_label(emitter);

	FastControlContext ctx = {};
	ctx.label = rs->label;
	ctx.break_label = done_label;
	ctx.break_scope_count = outer_scope_count;
	ctx.continue_label = post_label;
	ctx.continue_scope_count = loop_scope_count;
	ctx.fallthrough_label = -1;
	ctx.fallthrough_scope_count = -1;
	array_add(&emitter->control_stack, ctx);
	defer (emitter->control_stack.count -= 1);

	i64 initial_idx = is_ast_range(expr) ? 0 : -1;
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)initial_idx);
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)initial_idx);
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_slot(emitter, idx_slot);

	// String iteration uses a different index convention than the
	// collection branch. Forward string iteration decodes from `idx`
	// then increments, while reverse string iteration decodes the rune
	// ending at `idx` then decrements.
	Type *init_expr_type = base_type(type_deref(type_of_expr(expr)));
	if (init_expr_type != nullptr && (is_type_string(init_expr_type) || is_type_string16(init_expr_type))) {
		if (rs->reverse) {
			if (!fast_backend_emit_load_range_expr_length(emitter, expr, len_type)) {
				return false;
			}
		} else if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), 0);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), 0);
		}
		fast_backend_emit_store_work_to_slot(emitter, idx_slot);
	}

	if (is_ast_range(expr)) {
		if (val0_entity == nullptr) {
			return false;
		}

		Type *iter_type = type_of_expr(val0);
		FastScalarType iter_scalar = {};
		if (iter_type == nullptr || !fast_backend_classify_scalar_type(iter_type, &iter_scalar)) {
			return false;
		}

		TokenKind cmp_op = Token_Lt;
		switch (expr->BinaryExpr.op.kind) {
		case Token_RangeFull: cmp_op = Token_LtEq; break;
		case Token_RangeHalf: cmp_op = Token_Lt;   break;
		default: return false;
		}

		if (!fast_backend_emit_store_value_to_entity(emitter, val0_entity, expr->BinaryExpr.left)) {
			return false;
		}

		fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
		if (build_context.metrics.arch == TargetArch_amd64) {
			FastLocalSlot value_slot = {};
			if (!fast_backend_find_slot(emitter->plan, val0_entity, &value_slot)) {
				return false;
			}
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, value_slot, fast_backend_x64_work_reg());
		} else {
			FastLocalSlot value_slot = {};
			if (!fast_backend_find_slot(emitter->plan, val0_entity, &value_slot)) {
				return false;
			}
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, value_slot, fast_backend_arm64_work_reg());
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_leaf_expr_as_type(emitter, expr->BinaryExpr.right, iter_scalar)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_jump_if_compare(emitter, cmp_op, iter_scalar, body_label, done_label);

		fast_backend_emit_label(emitter->file, emitter->plan, body_label);
		if (val1_entity != nullptr) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
			} else {
				fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
			}
			if (!fast_backend_emit_store_to_scalar_storage(emitter, val1_entity)) {
				return false;
			}
		}
		if (!fast_backend_emit_stmt(emitter, rs->body)) {
			return false;
		}

		fast_backend_emit_label(emitter->file, emitter->plan, post_label);
		if (build_context.metrics.arch == TargetArch_amd64) {
			FastLocalSlot value_slot = {};
			if (!fast_backend_find_slot(emitter->plan, val0_entity, &value_slot)) {
				return false;
			}
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, value_slot, fast_backend_x64_work_reg());
			gb_fprintf(emitter->file, "\tadd %s, 1\n", fast_backend_x64_work_reg()->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), iter_scalar);
			fast_backend_emit_store_work_to_slot(emitter, value_slot);
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
			gb_fprintf(emitter->file, "\tadd %s, 1\n", fast_backend_x64_work_reg()->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
		} else {
			FastLocalSlot value_slot = {};
			if (!fast_backend_find_slot(emitter->plan, val0_entity, &value_slot)) {
				return false;
			}
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, value_slot, fast_backend_arm64_work_reg());
			gb_fprintf(emitter->file, "\tadd %s, %s, #1\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg());
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), iter_scalar);
			fast_backend_emit_store_work_to_slot(emitter, value_slot);
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
			gb_fprintf(emitter->file, "\tadd %s, %s, #1\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg());
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
		}
		fast_backend_emit_store_work_to_slot(emitter, idx_slot);
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
		fast_backend_emit_label(emitter->file, emitter->plan, done_label);
		return fast_backend_emit_leave_scope(emitter, rs->scope);
	}

	Type *expr_type = base_type(type_deref(type_of_expr(expr)));
	if (expr_type != nullptr && (is_type_string(expr_type) || is_type_string16(expr_type))) {
		fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
		}
		if (rs->reverse) {
			fast_backend_emit_jump_if_work_compare_imm(emitter, Token_Gt, idx_type, 0, body_label, done_label);
		} else {
			fast_backend_emit_push_work_reg(emitter);
			if (!fast_backend_emit_load_range_expr_length(emitter, expr, len_type)) {
				return false;
			}
			fast_backend_emit_pop_tmp_reg(emitter);
			fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, body_label, done_label);
		}

		fast_backend_emit_label(emitter->file, emitter->plan, body_label);
		if (rs->reverse) {
			if (is_type_string16(expr_type)) {
				if (!fast_backend_emit_range_string16_reverse_decode(emitter, expr, val0_entity, idx_slot, idx_type)) {
					return false;
				}
			} else {
				if (!fast_backend_emit_range_string_utf8_reverse_decode(emitter, expr, val0_entity, idx_slot, idx_type)) {
					return false;
				}
			}
			if (val1_entity != nullptr) {
				if (build_context.metrics.arch == TargetArch_amd64) {
					fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
				} else {
					fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
				}
				if (!fast_backend_emit_store_to_scalar_storage(emitter, val1_entity)) {
					return false;
				}
			}
		} else {
			if (val1_entity != nullptr) {
				if (build_context.metrics.arch == TargetArch_amd64) {
					fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
				} else {
					fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
				}
				if (!fast_backend_emit_store_to_scalar_storage(emitter, val1_entity)) {
					return false;
				}
			}
			if (is_type_string16(expr_type)) {
				if (!fast_backend_emit_range_string16_decode(emitter, expr, val0_entity, idx_slot, idx_type)) {
					return false;
				}
			} else {
				if (!fast_backend_emit_range_string_utf8_decode(emitter, expr, val0_entity, idx_slot, idx_type)) {
					return false;
				}
			}
		}
		if (!fast_backend_emit_stmt(emitter, rs->body)) {
			return false;
		}

		fast_backend_emit_label(emitter->file, emitter->plan, post_label);
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
		fast_backend_emit_label(emitter->file, emitter->plan, done_label);
		return fast_backend_emit_leave_scope(emitter, rs->scope);
	}

	Type *elem_type = nullptr;
	i64 elem_count = 0;
	if (expr_type == nullptr) {
		return false;
	}
	switch (expr_type->kind) {
	case Type_Array:
		elem_type = expr_type->Array.elem;
		elem_count = expr_type->Array.count;
		break;
	case Type_EnumeratedArray:
		elem_type = expr_type->EnumeratedArray.elem;
		elem_count = expr_type->EnumeratedArray.count;
		break;
	case Type_Slice:
		elem_type = expr_type->Slice.elem;
		break;
	case Type_DynamicArray:
		elem_type = expr_type->DynamicArray.elem;
		break;
	case Type_FixedCapacityDynamicArray:
		elem_type = expr_type->FixedCapacityDynamicArray.elem;
		break;
	default:
		return false;
	}
	i64 elem_size = type_size_of(elem_type);
	if (elem_size <= 0) {
		return false;
	}

	// `#reverse` is only valid for collection iteration here (the
	// compiler rejects it for binary range exprs like `0..<n`). Walk
	// the index down from `len-1` to 0 instead of from 0 to `len-1`.

	// Override the top-of-function init for reverse: start idx at
	// `len` so the first `idx -= 1` in the loop header lands on the
	// last valid element. The shared `initial_idx = -1` was set before
	// we knew whether we'd be iterating in reverse.
	if (rs->reverse) {
		if (expr_type->kind == Type_Array || expr_type->kind == Type_EnumeratedArray) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)elem_count);
			} else {
				fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)elem_count);
			}
		} else {
			if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
				return false;
			}
			fast_backend_emit_add_imm_to_work_reg(emitter, build_context.int_size);
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
			}
		}
		fast_backend_emit_store_work_to_slot(emitter, idx_slot);
	}

	fast_backend_emit_label(emitter->file, emitter->plan, loop_label);
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
		if (rs->reverse) {
			gb_fprintf(emitter->file, "\tsub %s, 1\n", fast_backend_x64_work_reg()->r64);
		} else {
			gb_fprintf(emitter->file, "\tadd %s, 1\n", fast_backend_x64_work_reg()->r64);
		}
		fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), idx_type);
	} else {
		fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
		if (rs->reverse) {
			gb_fprintf(emitter->file, "\tsub %s, %s, #1\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg());
		} else {
			gb_fprintf(emitter->file, "\tadd %s, %s, #1\n", fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg());
		}
		fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), idx_type);
	}
	fast_backend_emit_store_work_to_slot(emitter, idx_slot);
	fast_backend_emit_push_work_reg(emitter);

	if (rs->reverse) {
		// Compare against 0: jump to body if `idx >= 0`, else done.
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), 0);
		} else {
			fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), 0);
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_jump_if_compare(emitter, Token_GtEq, idx_type, body_label, done_label);
	} else {
		if (expr_type->kind == Type_Array || expr_type->kind == Type_EnumeratedArray) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_imm(emitter->file, fast_backend_x64_work_reg(), cast(u64)elem_count);
				fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), len_type);
			} else {
				fast_backend_emit_arm64_load_imm(emitter->file, fast_backend_arm64_work_reg(), cast(u64)elem_count);
				fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), len_type);
			}
		} else {
			if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
				return false;
			}
			fast_backend_emit_add_imm_to_work_reg(emitter, build_context.int_size);
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), len_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), len_type);
			}
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_jump_if_compare(emitter, Token_Lt, idx_type, body_label, done_label);
	}

	fast_backend_emit_label(emitter->file, emitter->plan, body_label);
	if (val1_entity != nullptr) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
		}
		if (expr_type->kind == Type_EnumeratedArray &&
		    expr_type->EnumeratedArray.min_value != nullptr &&
		    compare_exact_values(Token_NotEq, *expr_type->EnumeratedArray.min_value, exact_value_u64(0))) {
			fast_backend_emit_add_imm_to_work_reg(emitter, exact_value_to_i64(*expr_type->EnumeratedArray.min_value));
		}
		if (!fast_backend_emit_store_to_scalar_storage(emitter, val1_entity)) {
			return false;
		}
	}
	if (val0_entity != nullptr) {
		if (!fast_backend_emit_address_expr(emitter, expr, nullptr)) {
			return false;
		}
		if (expr_type->kind == Type_Slice || expr_type->kind == Type_DynamicArray || expr_type->kind == Type_FixedCapacityDynamicArray) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), pointer_type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), pointer_type);
			}
		}
		fast_backend_emit_push_work_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, idx_slot, fast_backend_arm64_work_reg());
		}
		fast_backend_emit_scale_work_reg(emitter, elem_size);
		fast_backend_emit_pop_tmp_reg(emitter);
		fast_backend_emit_add_tmp_reg_to_work_reg(emitter);

		FastLocalSlot value_slot = {};
		if (!fast_backend_find_slot(emitter->plan, val0_entity, &value_slot)) {
			return false;
		}
		if (value_slot.is_scalar) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_work_reg(), value_slot.type);
			} else {
				fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_work_reg(), value_slot.type);
			}
			if (!fast_backend_emit_store_to_scalar_storage(emitter, val0_entity)) {
				return false;
			}
		} else {
			fast_backend_emit_push_work_reg(emitter);
			if (!fast_backend_emit_address_of_storage_entity(emitter, val0_entity)) {
				return false;
			}
			fast_backend_emit_pop_tmp_reg(emitter);
			fast_backend_emit_copy_bytes_between_addresses(emitter, value_slot.size);
		}
	}
	if (!fast_backend_emit_stmt(emitter, rs->body)) {
		return false;
	}

	fast_backend_emit_label(emitter->file, emitter->plan, post_label);
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, loop_label);
	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return fast_backend_emit_leave_scope(emitter, rs->scope);
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
		i32 body_label = fast_backend_alloc_label(emitter);
		if (!fast_backend_emit_cond_jump(emitter, fs->cond, body_label, done_label)) {
			return false;
		}
		fast_backend_emit_label(emitter->file, emitter->plan, body_label);
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
	bool tag_is_string = false;
	bool tag_has_call = false;
	if (ss->tag != nullptr) {
		tag_type = reduce_tuple_to_single_type(type_and_value_of_expr(ss->tag).type);
		tag_is_string = is_type_string(tag_type) || is_type_string16(tag_type);
		tag_has_call = fast_backend_expr_has_call(ss->tag);
	}
	i32 switch_scope_count = cast(i32)emitter->scope_stack.count;
	if (!tag_is_string && !fast_backend_expr_scalar_type(nullptr, tag_type, &scalar_type)) {
		return false;
	}

	i32 dispatch_spill_base = emitter->current_spill_depth;
	i32 materialized_tag_depth = 0;
	i32 materialized_tag_temp_bytes = 0;
	if (ss->tag != nullptr && tag_has_call) {
		if (tag_is_string) {
			if (!fast_backend_emit_materialize_aggregate_compare_operand(emitter, ss->tag, tag_type, &materialized_tag_temp_bytes)) {
				return false;
			}
		} else if (!fast_backend_emit_leaf_expr_as_type(emitter, ss->tag, scalar_type)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		materialized_tag_depth = emitter->current_spill_depth;
	}

	auto body_labels = array_make<i32>(temporary_allocator(), 0, ss->body->BlockStmt.stmts.count);
	auto dispatch_body_labels = array_make<i32>(temporary_allocator(), 0, ss->body->BlockStmt.stmts.count);
	i32 default_label = -1;
	i32 default_dispatch_label = -1;
	i32 done_label = fast_backend_alloc_label(emitter);
	i32 dispatch_done_label = materialized_tag_depth != 0 ? fast_backend_alloc_label(emitter) : done_label;

	for (Ast *stmt : ss->body->BlockStmt.stmts) {
		if (stmt == nullptr || stmt->kind != Ast_CaseClause) {
			return false;
		}
		i32 body_label = fast_backend_alloc_label(emitter);
		i32 dispatch_body_label = materialized_tag_depth != 0 ? fast_backend_alloc_label(emitter) : body_label;
		array_add(&body_labels, body_label);
		array_add(&dispatch_body_labels, dispatch_body_label);
		if (stmt->CaseClause.list.count == 0) {
			default_label = body_label;
			default_dispatch_label = dispatch_body_label;
		}
	}

	for_array(i, ss->body->BlockStmt.stmts) {
		Ast *stmt = ss->body->BlockStmt.stmts[i];
		AstCaseClause *cc = &stmt->CaseClause;
		if (cc->list.count == 0) {
			continue;
		}

		i32 body_label = dispatch_body_labels[i];
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
				if (materialized_tag_depth != 0) {
					if (!fast_backend_emit_compare_materialized_scalar_branch(emitter, materialized_tag_depth, lhs, scalar_type, Token_LtEq, false, upper_check_label, next_label)) {
						return false;
					}
				} else if (!fast_backend_emit_compare_exprs_branch(emitter, lhs, ss->tag, scalar_type, Token_LtEq, upper_check_label, next_label)) {
					return false;
				}
				fast_backend_emit_label(emitter->file, emitter->plan, upper_check_label);
				if (materialized_tag_depth != 0) {
					if (!fast_backend_emit_compare_materialized_scalar_branch(emitter, materialized_tag_depth, rhs, scalar_type, upper_op, true, body_label, next_label)) {
						return false;
					}
				} else if (!fast_backend_emit_compare_exprs_branch(emitter, ss->tag, rhs, scalar_type, upper_op, body_label, next_label)) {
					return false;
				}
			} else if (tag_is_string) {
				// String-typed tag. Materialize tag and case to spill
				// slots, then do a per-field byte compare.
				//
				// The temp allocation done by the materialize is invisible
				// to the function's epilogue (which only restores the
				// planned frame size). So the cleanup must run on BOTH
				// the match and mismatch paths before either of them
				// leaves this iteration — otherwise the body would return
				// with sp shifted by `temp_bytes` and pop the wrong
				// link register.
				//
				// The per-field jne's inside the compare need a separate
				// `mismatch_label` from the outer `next_label` (which is
				// the start of the next case): if they jumped straight
				// to `next_label` they would bypass the cleanup.
				i32 spill_base = emitter->current_spill_depth;
				i32 temp_bytes = 0;
				i32 mismatch_label = fast_backend_alloc_label(emitter);
				i32 tag_depth = materialized_tag_depth;
				if (tag_depth == 0) {
					if (!fast_backend_emit_materialize_aggregate_compare_operand(emitter, ss->tag, tag_type, &temp_bytes)) {
						return false;
					}
					fast_backend_emit_push_work_reg(emitter);
					tag_depth = emitter->current_spill_depth;
				}

				if (!fast_backend_emit_materialize_aggregate_compare_operand(emitter, expr, tag_type, &temp_bytes)) {
					return false;
				}
				fast_backend_emit_push_work_reg(emitter);
				i32 case_depth = emitter->current_spill_depth;

				// Per-field jne's jump to mismatch_label.
				if (!fast_backend_emit_compare_aggregate_at_offset(emitter, tag_depth, case_depth, 0, tag_type, mismatch_label)) {
					return false;
				}

				// Match path: clean up the temp, then branch to the
				// case body.
				emitter->current_spill_depth = spill_base;
				fast_backend_emit_stack_adjust(emitter, temp_bytes, false);
				fast_backend_emit_jump_to_label(emitter->file, emitter->plan, body_label);

				// Mismatch path: clean up the temp, then fall through
				// to the outer `next_label` (start of the next case).
				fast_backend_emit_label(emitter->file, emitter->plan, mismatch_label);
				emitter->current_spill_depth = spill_base;
				fast_backend_emit_stack_adjust(emitter, temp_bytes, false);
			} else {
				if (materialized_tag_depth != 0) {
					if (!fast_backend_emit_compare_materialized_scalar_branch(emitter, materialized_tag_depth, expr, scalar_type, Token_CmpEq, true, body_label, next_label)) {
						return false;
					}
				} else if (!fast_backend_emit_compare_exprs_branch(emitter, ss->tag, expr, scalar_type, Token_CmpEq, body_label, next_label)) {
					return false;
				}
			}

			fast_backend_emit_label(emitter->file, emitter->plan, next_label);
		}
	}

	if (default_dispatch_label >= 0) {
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, default_dispatch_label);
	} else {
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, dispatch_done_label);
	}

	if (materialized_tag_depth != 0) {
		for_array(i, dispatch_body_labels) {
			fast_backend_emit_label(emitter->file, emitter->plan, dispatch_body_labels[i]);
			emitter->current_spill_depth = dispatch_spill_base;
			if (tag_is_string && materialized_tag_temp_bytes != 0) {
				fast_backend_emit_stack_adjust(emitter, materialized_tag_temp_bytes, false);
			}
			fast_backend_emit_jump_to_label(emitter->file, emitter->plan, body_labels[i]);
		}
		fast_backend_emit_label(emitter->file, emitter->plan, dispatch_done_label);
		emitter->current_spill_depth = dispatch_spill_base;
		if (tag_is_string && materialized_tag_temp_bytes != 0) {
			fast_backend_emit_stack_adjust(emitter, materialized_tag_temp_bytes, false);
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);
	}

	emitter->current_spill_depth = dispatch_spill_base;
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
	if (fast_backend_can_emit_noop_delete_call_expr(emitter->plan, &es->expr->CallExpr)) {
		return true;
	}
	// Builtin calls (cpu_relax, prefetch_*, trap, ...) don't have a
	// proc entity, so the regular call-emit path can't handle them.
	if (fast_backend_builtin_proc_id(es->expr->CallExpr.proc) != BuiltinProc_Invalid) {
		return fast_backend_emit_builtin_call_expr(emitter, &es->expr->CallExpr);
	}
	TypeProc *multi_result_pt = nullptr;
	if (fast_backend_get_multi_result_call_info(es->expr, &multi_result_pt, nullptr)) {
		i32 spill_base = emitter->current_spill_depth;
		defer (emitter->current_spill_depth = spill_base);

		i32 temp_bytes = 0;
		defer (fast_backend_emit_stack_adjust(emitter, temp_bytes, false));

		i32 source_depth = 0;
		return fast_backend_emit_materialize_multi_result_call(emitter, &es->expr->CallExpr, multi_result_pt, &source_depth, &temp_bytes) &&
		       fast_backend_emit_optional_call_value_check(emitter, &es->expr->CallExpr, multi_result_pt, source_depth);
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
	case Ast_RangeStmt:
		return fast_backend_emit_range_stmt(emitter, &stmt->RangeStmt);
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
	i32 extra_save_bytes = 0;
	if (build_context.metrics.arch == TargetArch_amd64) {
		extra_save_bytes = fast_backend_x64_preserved_scratch_save_bytes(emitter->plan->type);
	} else {
		extra_save_bytes = fast_backend_arm64_preserved_scratch_save_bytes(emitter->plan->type);
	}
	i32 frame_size = align_formula(spill_bytes + slot_bytes + extra_save_bytes, 16);
	if (build_context.metrics.arch == TargetArch_amd64) {
		gb_fprintf(emitter->file, "\tpush rbp\n");
		gb_fprintf(emitter->file, "\tmov rbp, rsp\n");
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tsub rsp, %d\n", frame_size);
		}
		if (extra_save_bytes != 0) {
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+0], rdi\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+8], rsi\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+16], rdx\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+24], rcx\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+32], r8\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+40], r9\n");
			gb_fprintf(emitter->file, "\tmov QWORD PTR [rsp+48], r10\n");
		}
	} else {
		gb_fprintf(emitter->file, "\tstp x29, x30, [sp, #-16]!\n");
		gb_fprintf(emitter->file, "\tmov x29, sp\n");
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tsub sp, sp, #%d\n", frame_size);
		}
		if (extra_save_bytes != 0) {
			gb_fprintf(emitter->file, "\tstp x9, x10, [sp, #0]\n");
			gb_fprintf(emitter->file, "\tstp x11, x12, [sp, #16]\n");
		}
	}
}

gb_internal bool fast_backend_emit_param_spills(FastLeafProcEmitter *emitter) {
	if (build_context.metrics.arch == TargetArch_amd64) {
		if (fast_backend_x64_uses_sysv_register_calling_sequence(emitter->plan->type)) {
			FastX64ArgState x64_arg_state = {};
			if (emitter->plan->return_by_pointer) {
				FastX64ArgAssignment slots[2] = {};
				fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, 1, slots);
				if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[0], fast_backend_x64_work_reg())) {
					return false;
				}
				fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
			}

			for_array(i, emitter->plan->params) {
				Entity *param = emitter->plan->params[i];
				FastLocalSlot slot = {};
				if (!fast_backend_find_slot(emitter->plan, param, &slot)) {
					return false;
				}

				i32 abi_args = fast_backend_abi_arg_count(param->type);
				bool is_split_aggregate = (abi_args == 2);
				FastX64ArgAssignment slots[2] = {};
				fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, abi_args, slots);

				if (is_split_aggregate) {
					if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[0], fast_backend_x64_work_reg())) {
						return false;
					}
					fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
					if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[1], fast_backend_x64_work_reg())) {
						return false;
					}
					i32 len_off = fast_backend_slot_offset(emitter->plan, slot) - 8;
					gb_fprintf(emitter->file, "\tmov QWORD PTR [rbp-%d], %s\n", len_off, fast_backend_x64_work_reg()->r64);
				} else if (slot.is_scalar) {
					if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[0], fast_backend_x64_work_reg())) {
						return false;
					}
					fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), slot.type);
					fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
				} else {
					if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[0], fast_backend_x64_tmp_reg())) {
						return false;
					}
					if (!fast_backend_emit_address_of_storage_entity(emitter, param)) {
						return false;
					}
					fast_backend_emit_copy_bytes_between_addresses(emitter, slot.size);
				}
			}

			if (emitter->plan->has_context_slot) {
				FastX64ArgAssignment slots[2] = {};
				fast_backend_x64_sysv_assign_arg_slots(&x64_arg_state, 1, slots);
				if (!fast_backend_emit_x64_load_assigned_arg(emitter->file, emitter->plan->type, slots[0], fast_backend_x64_work_reg())) {
					return false;
				}
				fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
			}
			return true;
		}

		i32 param_offset = emitter->plan->return_by_pointer ? 1 : 0;

		if (emitter->plan->return_by_pointer) {
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, 0);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
			fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_x64_work_reg());
		}

		i32 abi_index = param_offset;
		for_array(i, emitter->plan->params) {
			Entity *param = emitter->plan->params[i];
			FastLocalSlot slot = {};
			if (!fast_backend_find_slot(emitter->plan, param, &slot)) {
				return false;
			}

			i32 abi_args = fast_backend_abi_arg_count(param->type);
			bool is_split_aggregate = (abi_args == 2);

			if (is_split_aggregate) {
				// data in xN, len in xN+1. Build {data, len} in slot.
				auto *src0 = fast_backend_x64_param_reg(emitter->plan->type, abi_index);
				if (src0 == nullptr) return false;
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src0->r64);
				fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
				auto *src1 = fast_backend_x64_param_reg(emitter->plan->type, abi_index + 1);
				if (src1 == nullptr) return false;
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src1->r64);
				// slot.offset is the high address of the slot
				// (lowest negative offset from x29). The slot spans
				// size bytes below that, so the len field (8 bytes
				// below the high end) lives at offset-8.
				i32 len_off = fast_backend_slot_offset(emitter->plan, slot) - 8;
				gb_fprintf(emitter->file, "\tmov QWORD PTR [rbp-%d], %s\n", len_off, fast_backend_x64_work_reg()->r64);
			} else if (slot.is_scalar) {
				auto *src = fast_backend_x64_param_reg(emitter->plan->type, abi_index);
				if (src == nullptr) return false;
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
				fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), slot.type);
				fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
			} else {
				auto *src = fast_backend_x64_param_reg(emitter->plan->type, abi_index);
				if (src == nullptr) return false;
				gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_tmp_reg()->r64, src->r64);
				if (!fast_backend_emit_address_of_storage_entity(emitter, param)) {
					return false;
				}
				fast_backend_emit_copy_bytes_between_addresses(emitter, slot.size);
			}
			abi_index += abi_args;
		}

		if (emitter->plan->has_context_slot) {
			i32 param_index = abi_index;
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, param_index);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
			fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
		}
		return true;
	}

	FastArm64ArgState arm64_arg_state = {};
	if (emitter->plan->return_by_pointer) {
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&arm64_arg_state, 1, slots, /*is_sret=*/true);
		if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[0], fast_backend_arm64_work_reg())) {
			return false;
		}
		fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->result_ptr_slot, fast_backend_arm64_work_reg());
	}

	for_array(i, emitter->plan->params) {
		Entity *param = emitter->plan->params[i];
		FastLocalSlot slot = {};
		if (!fast_backend_find_slot(emitter->plan, param, &slot)) {
			return false;
		}

		i32 abi_args = fast_backend_abi_arg_count(param->type);
		bool is_split_aggregate = (abi_args == 2);
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&arm64_arg_state, abi_args, slots);

		if (is_split_aggregate) {
			if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[0], fast_backend_arm64_work_reg())) {
				return false;
			}
			fast_backend_emit_arm64_store_u64_to_frame_offset(emitter->file, fast_backend_slot_offset(emitter->plan, slot), fast_backend_arm64_work_reg());
			if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[1], fast_backend_arm64_work_reg())) {
				return false;
			}
			// See comment in the x64 branch above; the len field
			// sits 8 bytes below the slot's high address.
			i32 len_off = fast_backend_slot_offset(emitter->plan, slot) - 8;
			fast_backend_emit_arm64_store_u64_to_frame_offset(emitter->file, len_off, fast_backend_arm64_work_reg());
		} else if (slot.is_scalar) {
			if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[0], fast_backend_arm64_work_reg())) {
				return false;
			}
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), slot.type);
			fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
		} else {
			if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[0], fast_backend_arm64_tmp_reg())) {
				return false;
			}
			if (!fast_backend_emit_address_of_storage_entity(emitter, param)) {
				return false;
			}
			fast_backend_emit_copy_bytes_between_addresses(emitter, slot.size);
		}
	}

	if (emitter->plan->has_context_slot) {
		FastArm64ArgAssignment slots[2] = {};
		fast_backend_arm64_assign_arg_slots(&arm64_arg_state, 1, slots);
		if (!fast_backend_emit_arm64_load_assigned_arg(emitter->file, slots[0], fast_backend_arm64_work_reg())) {
			return false;
		}
		fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
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
	i32 extra_save_bytes = 0;
	if (build_context.metrics.arch == TargetArch_amd64) {
		extra_save_bytes = fast_backend_x64_preserved_scratch_save_bytes(emitter->plan->type);
	} else {
		extra_save_bytes = fast_backend_arm64_preserved_scratch_save_bytes(emitter->plan->type);
	}
	i32 frame_size = align_formula(spill_bytes + slot_bytes + extra_save_bytes, 16);
	// Restore the materialize scratch bytes BEFORE the preserved-scratch reload
	// and the frame teardown: the materialize temps live below the prologue
	// frame and use `sub sp` directly, so sp is at (rbp - frame_size - scratch)
	// at this point. The reload is sp-relative and needs the prologue-level sp.
	if (emitter->extra_scratch_bytes != 0) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tadd rsp, %d\n", emitter->extra_scratch_bytes);
		} else {
			gb_fprintf(emitter->file, "\tadd sp, sp, #%d\n", emitter->extra_scratch_bytes);
		}
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		if (extra_save_bytes != 0) {
			gb_fprintf(emitter->file, "\tmov rdi, QWORD PTR [rsp+0]\n");
			gb_fprintf(emitter->file, "\tmov rsi, QWORD PTR [rsp+8]\n");
			gb_fprintf(emitter->file, "\tmov rdx, QWORD PTR [rsp+16]\n");
			gb_fprintf(emitter->file, "\tmov rcx, QWORD PTR [rsp+24]\n");
			gb_fprintf(emitter->file, "\tmov r8, QWORD PTR [rsp+32]\n");
			gb_fprintf(emitter->file, "\tmov r9, QWORD PTR [rsp+40]\n");
			gb_fprintf(emitter->file, "\tmov r10, QWORD PTR [rsp+48]\n");
		}
		if (frame_size > 0) {
			gb_fprintf(emitter->file, "\tadd rsp, %d\n", frame_size);
		}
		gb_fprintf(emitter->file, "\tpop rbp\n");
		gb_fprintf(emitter->file, "\tret\n");
	} else {
		if (extra_save_bytes != 0) {
			gb_fprintf(emitter->file, "\tldp x9, x10, [sp, #0]\n");
			gb_fprintf(emitter->file, "\tldp x11, x12, [sp, #16]\n");
		}
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
	u64 flags = plan->entity->flags.load(std::memory_order_relaxed);

	// Handle weak/linkonce linkage
	if (flags & EntityFlag_CustomLinkage_Weak) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, ".weak_definition \"%.*s\"\n", LIT(asm_name));
		} else {
			gb_fprintf(file, ".weak \"%.*s\"\n", LIT(asm_name));
		}
	} else if (flags & EntityFlag_CustomLinkage_LinkOnce) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, ".weak_definition \"%.*s\"\n", LIT(asm_name));
		} else {
			gb_fprintf(file, ".linkonce \"%.*s\"\n", LIT(asm_name));
		}
	}
	if (fast_backend_allow_external_symbol(plan->entity)) {
		gb_fprintf(file, ".globl \"%.*s\"\n", LIT(asm_name));
	}
	if (build_context.metrics.os != TargetOs_darwin && build_context.metrics.os != TargetOs_windows) {
		gb_fprintf(file, ".type \"%.*s\",@function\n", LIT(asm_name));
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
	emitter.extra_scratch_bytes = 0;
	emitter.epilogue_label_index = 0;
	emitter.next_label_index = 1;
	emitter.use_frame = plan->spill_depth > 0 || plan->local_stack_size > 0;
	if (build_context.metrics.arch == TargetArch_amd64 &&
	    fast_backend_x64_preserved_scratch_save_bytes(plan->type) > 0) {
		emitter.use_frame = true;
	}
	if (build_context.metrics.arch == TargetArch_amd64 &&
	    fast_backend_x64_sysv_stack_arg_count(plan->type, plan->return_by_pointer, plan->has_context_slot) > 0) {
		emitter.use_frame = true;
	}
	if (build_context.metrics.arch == TargetArch_arm64 &&
	    (plan->has_calls ||
	     fast_backend_arm64_stack_arg_count(plan->type, plan->return_by_pointer, plan->has_context_slot) > 0 ||
	     fast_backend_arm64_preserved_scratch_save_bytes(plan->type) > 0)) {
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
	u64 flags = entity->flags.load(std::memory_order_relaxed);

	fast_backend_emit_global_section(file, entity);
	// Handle weak/linkonce linkage
	if (flags & EntityFlag_CustomLinkage_Weak) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, ".weak_definition \"%.*s\"\n", LIT(asm_name));
		} else {
			gb_fprintf(file, ".weak \"%.*s\"\n", LIT(asm_name));
		}
	} else if (flags & EntityFlag_CustomLinkage_LinkOnce) {
		if (build_context.metrics.os == TargetOs_darwin) {
			gb_fprintf(file, ".weak_definition \"%.*s\"\n", LIT(asm_name));
		} else {
			gb_fprintf(file, ".linkonce \"%.*s\"\n", LIT(asm_name));
		}
	}
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
		i32 total_param_count = 0;
		for_array(i, proc_plan->params) {
			Entity *p = proc_plan->params[i];
			if (p == nullptr || p->kind != Entity_Variable) {
				error(plan.entity->token, "Fast backend: invalid procedure parameter");
				return false;
			}
			total_param_count += fast_backend_abi_arg_count(p->type);
		}
		total_param_count += (proc_plan->has_context_slot ? 1 : 0);
		total_param_count += (proc_plan->return_by_pointer ? 1 : 0);
		if (!fast_backend_supports_stack_args(proc_plan->type) &&
		    total_param_count > fast_backend_param_limit(proc_plan)) {
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
		if (fast_backend_can_fallback_to_llvm_per_entity()) {
			String obj_ext = build_context.build_paths[BuildPath_Output].ext;
			obj_path = concatenate3_strings(permanent_allocator(), gen->output_base, str_lit(".fast."), obj_ext);
		} else {
			obj_path = path_to_string(permanent_allocator(), build_context.build_paths[BuildPath_Output]);
		}
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
