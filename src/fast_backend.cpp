struct FastBackendGenerator : LinkerData {
	CheckerInfo *info;
	Checker *checker;
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
	FastScalarType type;
	i32 index;
};

struct FastLeafProcPlan {
	Entity *entity;
	CheckerInfo *info;
	TypeProc *type;
	Array<Entity *> params;
	Array<FastLocalSlot> slots;
	Ast *body;
	FastScalarType return_type;
	FastLocalSlot context_slot;
	i32 spill_depth;
	i32 proc_index;
	bool has_context_slot;
	bool has_result;
};

struct FastGlobalVarPlan {
	Entity *entity;
	FastScalarType type;
	u64 init_value;
	i32 size;
	i32 align;
	bool has_init_value;
};

struct FastControlContext {
	Ast *label;
	i32 break_label;
	i32 continue_label;
	i32 fallthrough_label;
};

struct FastLeafProcEmitter {
	gbFile *file;
	FastLeafProcPlan *plan;
	Array<FastControlContext> control_stack;
	i32 current_spill_depth;
	i32 epilogue_label_index;
	i32 next_label_index;
	bool use_frame;
};

gb_internal bool fast_backend_find_slot(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *out);
gb_internal i32 fast_backend_slot_offset(FastLeafProcPlan *plan, FastLocalSlot const &slot);
gb_internal bool fast_backend_add_slot(FastLeafProcPlan *plan, Entity *entity, FastScalarType type);
gb_internal FastScalarType fast_backend_context_scalar_type(void);
gb_internal i32 fast_backend_param_limit_from_proc_type(TypeProc *pt);
gb_internal bool fast_backend_expr_scalar_type(Ast *expr, Type *expected_type, FastScalarType *out);
gb_internal bool fast_backend_leaf_expr_is_supported(FastLeafProcPlan *plan, Ast *expr, Type *expected_type);
gb_internal bool fast_backend_find_scalar_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, FastScalarType *type_, bool *is_global_);
gb_internal bool fast_backend_exact_value_as_u64(ExactValue value, FastScalarType type, u64 *out);

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

	if (is_type_integer(bt)) {
		out->kind = is_type_unsigned(bt) ? FastScalar_Unsigned : FastScalar_Signed;
		out->bit_size = 8*type_size_of(bt);
		return out->bit_size > 0 && out->bit_size <= 64;
	}

	if (is_type_pointer(type)) {
		out->kind = FastScalar_Pointer;
		out->bit_size = 8*build_context.metrics.ptr_size;
		return true;
	}

	return false;
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
		i32 left_depth  = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.left);
		i32 right_depth = fast_backend_leaf_expr_spill_depth(expr->BinaryExpr.right);
		return gb_max(left_depth, 1 + right_depth);
	}
	case Ast_CallExpr: {
		i32 depth = 0;
		for_array(i, expr->CallExpr.args) {
			depth = gb_max(depth, cast(i32)i + fast_backend_leaf_expr_spill_depth(expr->CallExpr.args[i]));
		}
		i32 total_arg_count = cast(i32)expr->CallExpr.args.count;
		Type *proc_type = base_type(type_of_expr(expr->CallExpr.proc));
		if (proc_type != nullptr && proc_type->kind == Type_Proc && proc_type->Proc.calling_convention == ProcCC_Odin) {
			total_arg_count += 1;
		}
		return gb_max(depth, total_arg_count);
	}
	}

	return 0;
}

gb_internal bool fast_backend_leaf_expr_has_call(Ast *expr) {
	if (expr == nullptr) {
		return false;
	}

	switch (expr->kind) {
	case Ast_ParenExpr:
		return fast_backend_leaf_expr_has_call(expr->ParenExpr.expr);
	case Ast_TypeCast:
		return fast_backend_leaf_expr_has_call(expr->TypeCast.expr);
	case Ast_AutoCast:
		return fast_backend_leaf_expr_has_call(expr->AutoCast.expr);
	case Ast_UnaryExpr:
		return fast_backend_leaf_expr_has_call(expr->UnaryExpr.expr);
	case Ast_BinaryExpr:
		return fast_backend_leaf_expr_has_call(expr->BinaryExpr.left) ||
		       fast_backend_leaf_expr_has_call(expr->BinaryExpr.right);
	case Ast_CallExpr:
		return true;
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

gb_internal bool fast_backend_find_addressable_scalar_entity(FastLeafProcPlan *plan, Ast *expr, Entity **entity_, FastScalarType *type_) {
	expr = unparen_expr(expr);
	if (expr == nullptr || expr->kind != Ast_Ident) {
		return false;
	}

	Entity *entity = expr->Ident.entity.load();
	FastScalarType scalar_type = {};
	if (entity == nullptr || !fast_backend_find_scalar_storage(plan, entity, nullptr, &scalar_type, nullptr)) {
		return false;
	}

	if (entity_) *entity_ = entity;
	if (type_) *type_ = scalar_type;
	return true;
}

gb_internal bool fast_backend_deref_expr_is_supported(FastLeafProcPlan *plan, Ast *expr) {
	if (expr == nullptr || expr->kind != Ast_DerefExpr) {
		return false;
	}

	Type *pointer_type = base_type(type_of_expr(expr->DerefExpr.expr));
	if (pointer_type == nullptr || pointer_type->kind != Type_Pointer) {
		return false;
	}

	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(nullptr, type_of_expr(expr), &result_type)) {
		return false;
	}
	return fast_backend_leaf_expr_is_supported(plan, expr->DerefExpr.expr, type_of_expr(expr->DerefExpr.expr));
}

gb_internal bool fast_backend_cast_expr_is_supported(FastLeafProcPlan *plan, Ast *operand, Type *target_type, TokenKind cast_kind) {
	FastScalarType source_type = {};
	FastScalarType result_type = {};
	Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(operand).type);
	if (!fast_backend_expr_scalar_type(operand, operand_type, &source_type)) {
		return false;
	}
	if (!fast_backend_expr_scalar_type(nullptr, target_type, &result_type)) {
		return false;
	}
	if (!fast_backend_leaf_expr_is_supported(plan, operand, operand_type)) {
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

gb_internal bool fast_backend_switch_case_expr_is_supported(FastLeafProcPlan *plan, Ast *expr, Type *tag_type) {
	expr = unparen_expr(expr);
	if (is_ast_range(expr)) {
		FastScalarType scalar_type = {};
		if (!fast_backend_classify_scalar_type(tag_type, &scalar_type) ||
		    !fast_backend_scalar_supports_ordered_cmp(scalar_type)) {
			return false;
		}

		Ast *lhs = expr->BinaryExpr.left;
		Ast *rhs = expr->BinaryExpr.right;
		if (fast_backend_leaf_expr_has_call(lhs) || fast_backend_leaf_expr_has_call(rhs)) {
			return false;
		}
		return fast_backend_leaf_expr_is_supported(plan, lhs, tag_type) &&
		       fast_backend_leaf_expr_is_supported(plan, rhs, tag_type);
	}

	if (fast_backend_leaf_expr_has_call(expr)) {
		return false;
	}
	return fast_backend_leaf_expr_is_supported(plan, expr, tag_type);
}

gb_internal bool fast_backend_get_call_info(AstCallExpr *ce, TypeProc **proc_type_, Entity **proc_entity_, FastScalarType *result_type_, bool *has_result_) {
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
	if (pt->is_closure || pt->variadic || pt->c_vararg || pt->return_by_pointer || pt->diverging || pt->is_polymorphic || proc_entity->Procedure.generated_from_polymorphic) {
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
	bool has_result = false;
	if (pt->result_count > 1) {
		return false;
	}
	if (pt->result_count == 1) {
		Entity *result_entity = pt->results->Tuple.variables[0];
		if (result_entity == nullptr || !fast_backend_classify_scalar_type(result_entity->type, &result_type)) {
			return false;
		}
		has_result = true;
	}

	if (proc_type_) *proc_type_ = pt;
	if (proc_entity_) *proc_entity_ = proc_entity;
	if (result_type_) *result_type_ = result_type;
	if (has_result_) *has_result_ = has_result;
	return true;
}

gb_internal bool fast_backend_call_expr_is_supported(FastLeafProcPlan *plan, AstCallExpr *ce, bool allow_void_result) {
	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	FastScalarType result_type = {};
	bool has_result = false;
	if (!fast_backend_get_call_info(ce, &pt, &proc_entity, &result_type, &has_result)) {
		return false;
	}
	gb_unused(proc_entity);

	if (!allow_void_result && !has_result) {
		return false;
	}
	if (cast(i32)ce->args.count != pt->param_count) {
		return false;
	}

	i32 total_param_count = pt->param_count + (pt->calling_convention == ProcCC_Odin ? 1 : 0);
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
		if (!fast_backend_classify_scalar_type(param->type, &arg_type)) {
			return false;
		}
		if (!fast_backend_leaf_expr_is_supported(plan, ce->args[i], param->type)) {
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_leaf_expr_is_supported(FastLeafProcPlan *plan, Ast *expr, Type *expected_type) {
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
		return fast_backend_leaf_expr_is_supported(plan, expr->ParenExpr.expr, expected_type);

	case Ast_Implicit:
		return plan->has_context_slot && expr->Implicit.kind == Token_context;

	case Ast_Ident: {
		Entity *e = expr->Ident.entity.load();
		return e != nullptr && fast_backend_find_scalar_storage(plan, e, nullptr, nullptr, nullptr);
	}

	case Ast_DerefExpr:
		return fast_backend_deref_expr_is_supported(plan, expr);

	case Ast_CallExpr:
		return fast_backend_call_expr_is_supported(plan, &expr->CallExpr, false);

	case Ast_TypeCast: {
		Type *result_type = reduce_tuple_to_single_type(tv.type);
		return fast_backend_cast_expr_is_supported(plan, expr->TypeCast.expr, result_type, expr->TypeCast.token.kind);
	}

	case Ast_AutoCast: {
		Type *result_type = reduce_tuple_to_single_type(tv.type);
		return fast_backend_cast_expr_is_supported(plan, expr->AutoCast.expr, result_type, Token_cast);
	}

	case Ast_UnaryExpr: {
		Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->UnaryExpr.expr).type);
		FastScalarType operand_scalar = {};
		if (!fast_backend_classify_scalar_type(operand_type, &operand_scalar)) {
			return false;
		}
		if (!fast_backend_leaf_expr_is_supported(plan, expr->UnaryExpr.expr, operand_type)) {
			return false;
		}

		switch (expr->UnaryExpr.op.kind) {
		case Token_And:
			return fast_backend_find_addressable_scalar_entity(plan, expr->UnaryExpr.expr, nullptr, nullptr);
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
		Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->BinaryExpr.left).type);
		FastScalarType operand_scalar = {};
		if (!fast_backend_classify_scalar_type(operand_type, &operand_scalar)) {
			return false;
		}

		if (!fast_backend_leaf_expr_is_supported(plan, expr->BinaryExpr.left, operand_type) ||
		    !fast_backend_leaf_expr_is_supported(plan, expr->BinaryExpr.right, operand_type)) {
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

gb_internal bool fast_backend_plan_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, Ast *stmt);

gb_internal bool fast_backend_plan_stmt_list(FastBackendGenerator *gen, FastLeafProcPlan *plan, Slice<Ast *> const &stmts) {
	for (Ast *stmt : stmts) {
		if (!fast_backend_plan_stmt(gen, plan, stmt)) {
			return false;
		}
	}
	return true;
}

gb_internal bool fast_backend_plan_value_decl(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstValueDecl *vd) {
	gb_unused(gen);

	if (!vd->is_mutable) {
		return true;
	}
	if (vd->is_using) {
		error(vd->names[0], "Fast backend does not yet support 'using' value declarations");
		return false;
	}
	if (vd->values.count != 0 && vd->values.count != vd->names.count) {
		error(vd->names[0], "Fast backend currently only supports one-to-one scalar value declarations");
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

		FastScalarType scalar_type = {};
		if (!fast_backend_classify_scalar_type(entity->type, &scalar_type)) {
			error(name, "Fast backend currently only supports scalar local variable types");
			return false;
		}
		if (!fast_backend_add_slot(plan, entity, scalar_type)) {
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

		if (!fast_backend_leaf_expr_is_supported(plan, rhs, entity->type)) {
			error(rhs, "Fast backend does not yet support this value declaration expression");
			return false;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(rhs));
	}

	return true;
}

gb_internal bool fast_backend_plan_assign_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstAssignStmt *as) {
	gb_unused(gen);

	if (as->op.kind != Token_Eq) {
		error(as->op, "Fast backend currently only supports simple '=' assignments");
		return false;
	}
	if (as->lhs.count != as->rhs.count) {
		error(as->op, "Fast backend currently only supports one-to-one scalar assignments");
		return false;
	}

	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		Ast *rhs = as->rhs[i];
		if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
			if (!fast_backend_leaf_expr_is_supported(plan, rhs, type_and_value_of_expr(rhs).type)) {
				error(rhs, "Fast backend does not yet support this discard assignment expression");
				return false;
			}
			plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(rhs));
			continue;
		}

		Type *target_type = nullptr;
		if (lhs->kind == Ast_Ident) {
			Entity *entity = entity_of_node(lhs);
			if (entity == nullptr || !fast_backend_find_scalar_storage(plan, entity, nullptr, nullptr, nullptr)) {
				error(lhs, "Fast backend expected a scalar local, parameter, global, or pointer-dereference assignment target");
				return false;
			}
			target_type = entity->type;
		} else if (lhs->kind == Ast_DerefExpr) {
			if (!fast_backend_deref_expr_is_supported(plan, lhs)) {
				error(lhs, "Fast backend expected a scalar pointer-dereference assignment target");
				return false;
			}
			target_type = type_of_expr(lhs);
			plan->spill_depth = gb_max(plan->spill_depth, 1 + fast_backend_leaf_expr_spill_depth(lhs->DerefExpr.expr));
		} else {
			error(lhs, "Fast backend currently only supports identifier and scalar pointer-dereference assignment targets");
			return false;
		}

		if (!fast_backend_leaf_expr_is_supported(plan, rhs, target_type)) {
			error(rhs, "Fast backend does not yet support this assignment expression");
			return false;
		}
		plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(rhs));
	}

	return true;
}

gb_internal bool fast_backend_plan_return_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstReturnStmt *rs) {
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
	if (!fast_backend_leaf_expr_is_supported(plan, result, reduce_tuple_to_single_type(plan->type->results))) {
		error(result, "Fast backend does not yet support this return expression");
		return false;
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(result));
	return true;
}

gb_internal bool fast_backend_plan_if_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstIfStmt *is) {
	if (is->init != nullptr && !fast_backend_plan_stmt(gen, plan, is->init)) {
		return false;
	}
	if (!fast_backend_leaf_expr_is_supported(plan, is->cond, type_and_value_of_expr(is->cond).type)) {
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

gb_internal bool fast_backend_plan_for_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstForStmt *fs) {
	if (fs->init != nullptr && !fast_backend_plan_stmt(gen, plan, fs->init)) {
		return false;
	}
	if (fs->cond != nullptr) {
		if (!fast_backend_leaf_expr_is_supported(plan, fs->cond, type_and_value_of_expr(fs->cond).type)) {
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

gb_internal bool fast_backend_plan_switch_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstSwitchStmt *ss) {
	if (ss->partial) {
		error(ss->token, "Fast backend does not yet support #partial switch statements");
		return false;
	}
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
		if (fast_backend_leaf_expr_has_call(ss->tag)) {
			error(ss->tag, "Fast backend switch tags cannot contain calls yet");
			return false;
		}
		if (!fast_backend_leaf_expr_is_supported(plan, ss->tag, tag_type)) {
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
			if (!fast_backend_switch_case_expr_is_supported(plan, expr, tag_type)) {
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

gb_internal bool fast_backend_plan_branch_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstBranchStmt *bs) {
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

gb_internal bool fast_backend_plan_expr_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, AstExprStmt *es) {
	gb_unused(gen);

	if (es->expr == nullptr || es->expr->kind != Ast_CallExpr) {
		error(es->expr ? es->expr : cast(Ast *)es, "Fast backend currently only supports call expression statements");
		return false;
	}
	if (!fast_backend_call_expr_is_supported(plan, &es->expr->CallExpr, true)) {
		error(es->expr, "Fast backend does not yet support this call expression");
		return false;
	}
	plan->spill_depth = gb_max(plan->spill_depth, fast_backend_leaf_expr_spill_depth(es->expr));
	return true;
}

gb_internal bool fast_backend_plan_stmt(FastBackendGenerator *gen, FastLeafProcPlan *plan, Ast *stmt) {
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
	case Ast_BranchStmt:
		return fast_backend_plan_branch_stmt(gen, plan, &stmt->BranchStmt);
	case Ast_ReturnStmt:
		return fast_backend_plan_return_stmt(gen, plan, &stmt->ReturnStmt);
	}

	error(stmt, "Fast backend currently only supports blocks, mutable local declarations, assignments, call statements, if statements, for statements, switch statements, break/continue, and returns");
	return false;
}

gb_internal bool fast_backend_plan_leaf_proc(FastBackendGenerator *gen, Entity *e, FastLeafProcPlan *plan) {
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
	if (pt->variadic || pt->return_by_pointer || pt->diverging || pt->is_polymorphic || e->Procedure.generated_from_polymorphic) {
		error(e->token, "Fast backend does not yet support this procedure kind");
		return false;
	}
	if (!fast_backend_supported_calling_convention(pt->calling_convention)) {
		error(e->token, "Fast backend does not yet support this calling convention");
		return false;
	}

	plan->entity = e;
	plan->info = gen->info;
	plan->type = pt;
	plan->body = decl->proc_lit->ProcLit.body;
	plan->has_result = false;
	plan->return_type = {};
	plan->context_slot = {};
	plan->spill_depth = 0;
	plan->proc_index = 0;
	plan->has_context_slot = false;
	plan->params = array_make<Entity *>(heap_allocator(), 0, pt->param_count);
	plan->slots = array_make<FastLocalSlot>(heap_allocator(), 0, pt->param_count);

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

			FastScalarType param_type = {};
			if (!fast_backend_classify_scalar_type(param->type, &param_type)) {
				error(param->token, "Fast backend currently only supports scalar parameter types");
				return false;
			}
			array_add(&plan->params, param);
			fast_backend_add_slot(plan, param, param_type);
		}
	}

	if (pt->calling_convention == ProcCC_Odin) {
		plan->has_context_slot = true;
		plan->context_slot.entity = nullptr;
		plan->context_slot.type = fast_backend_context_scalar_type();
		plan->context_slot.index = cast(i32)plan->slots.count;
		array_add(&plan->slots, plan->context_slot);
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
			error(result_entity->token, "Fast backend currently only supports scalar result types");
			return false;
		}
		plan->has_result = true;
	}

	ast_node(bs, BlockStmt, decl->proc_lit->ProcLit.body);
	return fast_backend_plan_stmt_list(gen, plan, bs->stmts);
}

gb_internal bool fast_backend_plan_global_var(Entity *e, FastGlobalVarPlan *plan) {
	GB_ASSERT(e != nullptr);
	GB_ASSERT(e->kind == Entity_Variable);

	if (e->Variable.is_foreign) {
		return false;
	}
	if (e->Variable.thread_local_model.len != 0) {
		error(e->token, "Fast backend does not yet support thread local global variables");
		return false;
	}

	FastScalarType scalar_type = {};
	if (!fast_backend_classify_scalar_type(e->type, &scalar_type)) {
		error(e->token, "Fast backend currently only supports scalar global variable types");
		return false;
	}

	DeclInfo *decl = decl_info_of_entity(e);
	Ast *init_expr = decl != nullptr ? decl->init_expr : e->Variable.init_expr;

	plan->entity = e;
	plan->type = scalar_type;
	plan->init_value = 0;
	plan->size = cast(i32)type_size_of(e->type);
	plan->align = cast(i32)type_align_of(e->type);
	plan->has_init_value = false;

	if (plan->size <= 0 || plan->align <= 0) {
		error(e->token, "Fast backend expected a sized scalar global variable");
		return false;
	}
	if (init_expr == nullptr) {
		return true;
	}

	TypeAndValue tv = type_and_value_of_expr(init_expr);
	if (tv.mode == Addressing_Invalid) {
		return false;
	}
	if (tv.mode == Addressing_Constant) {
		if (!fast_backend_exact_value_as_u64(tv.value, scalar_type, &plan->init_value)) {
			if (!(scalar_type.kind == FastScalar_Pointer && is_type_untyped_nil(tv.type))) {
				error(init_expr, "Fast backend does not yet support this global constant initializer");
				return false;
			}
			plan->init_value = 0;
		}
		plan->has_init_value = true;
		return true;
	}
	if (scalar_type.kind == FastScalar_Pointer && is_type_untyped_nil(tv.type)) {
		plan->has_init_value = true;
		return true;
	}

	error(init_expr, "Fast backend does not yet support dynamic global initialization");
	return false;
}

gb_internal bool fast_backend_collect_program(FastBackendGenerator *gen, Array<FastGlobalVarPlan> *globals, Array<FastLeafProcPlan> *procedures) {
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

			FastLeafProcPlan plan = {};
			if (!fast_backend_plan_leaf_proc(gen, e, &plan)) {
				return false;
			}
			plan.proc_index = cast(i32)procedures->count;
			array_add(procedures, plan);
			break;
		}

		case Entity_Variable: {
			if (e->Variable.is_foreign) {
				break;
			}
			if (flags & (EntityFlag_Overridden|
			             EntityFlag_CustomLinkage_Weak|
			             EntityFlag_CustomLinkage_LinkOnce)) {
				error(e->token, "Fast backend does not yet support this global variable linkage");
				return false;
			}
			FastGlobalVarPlan plan = {};
			if (!fast_backend_plan_global_var(e, &plan)) {
				return false;
			}
			array_add(globals, plan);
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

gb_internal i32 fast_backend_slot_offset(FastLeafProcPlan *plan, FastLocalSlot const &slot) {
	return 8 * (plan->spill_depth + slot.index + 1);
}

gb_internal bool fast_backend_add_slot(FastLeafProcPlan *plan, Entity *entity, FastScalarType type) {
	if (fast_backend_find_slot(plan, entity, nullptr)) {
		return true;
	}

	FastLocalSlot slot = {};
	slot.entity = entity;
	slot.type = type;
	slot.index = cast(i32)plan->slots.count;
	array_add(&plan->slots, slot);
	return true;
}

gb_internal bool fast_backend_entity_is_scalar_global(FastLeafProcPlan *plan, Entity *entity, FastScalarType *type_) {
	if (entity == nullptr || entity->kind != Entity_Variable) {
		return false;
	}
	if (entity->parent_proc_decl.load(std::memory_order_relaxed) != nullptr) {
		return false;
	}

	FastScalarType type = {};
	if (!fast_backend_classify_scalar_type(entity->type, &type)) {
		return false;
	}

	if (type_) *type_ = type;
	return entity->Variable.is_global || entity->Variable.is_foreign || fast_backend_is_local_entity(plan->info, entity);
}

gb_internal bool fast_backend_find_scalar_storage(FastLeafProcPlan *plan, Entity *entity, FastLocalSlot *slot_, FastScalarType *type_, bool *is_global_) {
	FastLocalSlot slot = {};
	if (fast_backend_find_slot(plan, entity, &slot)) {
		if (slot_) *slot_ = slot;
		if (type_) *type_ = slot.type;
		if (is_global_) *is_global_ = false;
		return true;
	}

	FastScalarType type = {};
	if (!fast_backend_entity_is_scalar_global(plan, entity, &type)) {
		return false;
	}

	if (slot_) *slot_ = {};
	if (type_) *type_ = type;
	if (is_global_) *is_global_ = true;
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

gb_internal bool fast_backend_entity_uses_external_symbol(FastLeafProcPlan *plan, Entity *entity) {
	if (entity == nullptr || entity->kind != Entity_Variable) {
		return true;
	}
	if (entity->Variable.is_foreign) {
		return true;
	}
	return !fast_backend_is_local_entity(plan->info, entity);
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

gb_internal bool fast_backend_emit_address_of_scalar_entity(FastLeafProcEmitter *emitter, Entity *entity) {
	FastLocalSlot slot = {};
	FastScalarType scalar_type = {};
	bool is_global = false;
	if (!fast_backend_find_scalar_storage(emitter->plan, entity, &slot, &scalar_type, &is_global)) {
		return false;
	}
	gb_unused(scalar_type);

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
		gb_fprintf(emitter->file, "\tsub %s, x29, #%d\n", fast_backend_arm64_work_reg(), offset);
	}
	return true;
}

gb_internal bool fast_backend_emit_leaf_deref(FastLeafProcEmitter *emitter, Ast *expr) {
	FastScalarType result_type = {};
	if (!fast_backend_expr_scalar_type(nullptr, type_of_expr(expr), &result_type)) {
		return false;
	}
	if (!fast_backend_emit_leaf_expr(emitter, expr->DerefExpr.expr)) {
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

	if (expr->kind != Ast_Ident) {
		if (expr->kind == Ast_Implicit && expr->Implicit.kind == Token_context && emitter->plan->has_context_slot) {
			if (build_context.metrics.arch == TargetArch_amd64) {
				fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_x64_work_reg());
			} else {
				fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, emitter->plan->context_slot, fast_backend_arm64_work_reg());
			}
			return true;
		}
		return false;
	}

	Entity *entity = expr->Ident.entity.load();
	FastLocalSlot slot = {};
	if (entity == nullptr) {
		return false;
	}

	if (fast_backend_find_slot(emitter->plan, entity, &slot)) {
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_load_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
		} else {
			fast_backend_emit_arm64_load_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
		}
		return true;
	}

	if (!fast_backend_entity_is_scalar_global(emitter->plan, entity, &scalar_type)) {
		return false;
	}
	if (build_context.metrics.arch == TargetArch_amd64) {
		fast_backend_emit_x64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_x64_tmp_reg());
		fast_backend_emit_x64_load_from_address(emitter->file, fast_backend_x64_tmp_reg(), fast_backend_x64_work_reg(), scalar_type);
	} else {
		fast_backend_emit_arm64_load_address_of_entity(emitter->file, emitter->plan, entity, fast_backend_arm64_tmp_reg());
		fast_backend_emit_arm64_load_from_address(emitter->file, fast_backend_arm64_tmp_reg(), fast_backend_arm64_work_reg(), scalar_type);
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

gb_internal bool fast_backend_emit_call_expr(FastLeafProcEmitter *emitter, AstCallExpr *ce) {
	TypeProc *pt = nullptr;
	Entity *proc_entity = nullptr;
	FastScalarType result_type = {};
	bool has_result = false;
	if (!fast_backend_get_call_info(ce, &pt, &proc_entity, &result_type, &has_result)) {
		return false;
	}

	for (Ast *arg : ce->args) {
		if (!fast_backend_emit_leaf_expr(emitter, arg)) {
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
		if (has_result) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, fast_backend_x64_return_reg()->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), result_type);
		}
	} else {
		gb_fprintf(emitter->file, "\tbl \"%.*s\"\n", LIT(symbol));
		if (has_result) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), fast_backend_arm64_return_reg());
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), result_type);
		}
	}

	return true;
}

gb_internal bool fast_backend_emit_leaf_unary(FastLeafProcEmitter *emitter, Ast *expr) {
	if (expr->UnaryExpr.op.kind == Token_And) {
		Entity *entity = nullptr;
		if (!fast_backend_find_addressable_scalar_entity(emitter->plan, expr->UnaryExpr.expr, &entity, nullptr)) {
			return false;
		}
		return fast_backend_emit_address_of_scalar_entity(emitter, entity);
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

gb_internal bool fast_backend_emit_leaf_binary(FastLeafProcEmitter *emitter, Ast *expr) {
	Type *operand_type = reduce_tuple_to_single_type(type_and_value_of_expr(expr->BinaryExpr.left).type);
	FastScalarType scalar_type = {};
	if (!fast_backend_classify_scalar_type(operand_type, &scalar_type)) {
		return false;
	}

	TokenKind op = expr->BinaryExpr.op.kind;
	if (!fast_backend_emit_leaf_expr(emitter, expr->BinaryExpr.left)) {
		return false;
	}

	fast_backend_emit_push_work_reg(emitter);
	if (!fast_backend_emit_leaf_expr(emitter, expr->BinaryExpr.right)) {
		return false;
	}
	fast_backend_emit_pop_tmp_reg(emitter);

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
			if (type_and_value_of_expr(expr->BinaryExpr.right).mode == Addressing_Constant) {
				ExactValue shift_value = type_and_value_of_expr(expr->BinaryExpr.right).value;
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

gb_internal bool fast_backend_emit_leaf_expr(FastLeafProcEmitter *emitter, Ast *expr) {
	TypeAndValue tv = type_and_value_of_expr(expr);
	if (tv.mode == Addressing_Constant || expr->kind == Ast_Ident || expr->kind == Ast_Implicit) {
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
		return fast_backend_emit_leaf_binary(emitter, expr);
	case Ast_CallExpr:
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
			fast_backend_emit_zero_slot(emitter, slot);
			continue;
		}

		if (!fast_backend_emit_leaf_expr(emitter, vd->values[i])) {
			return false;
		}
		if (!fast_backend_emit_store_to_scalar_storage(emitter, entity)) {
			return false;
		}
	}

	return true;
}

gb_internal bool fast_backend_emit_assign_stmt(FastLeafProcEmitter *emitter, AstAssignStmt *as) {
	for_array(i, as->lhs) {
		Ast *lhs = unparen_expr(as->lhs[i]);
		if (lhs->kind == Ast_Ident && is_blank_ident(lhs)) {
			if (!fast_backend_emit_leaf_expr(emitter, as->rhs[i])) {
				return false;
			}
			continue;
		}

		if (!fast_backend_emit_leaf_expr(emitter, as->rhs[i])) {
			return false;
		}

		if (lhs->kind == Ast_Ident) {
			Entity *entity = entity_of_node(lhs);
			if (entity == nullptr || !fast_backend_find_scalar_storage(emitter->plan, entity, nullptr, nullptr, nullptr)) {
				return false;
			}
			if (!fast_backend_emit_store_to_scalar_storage(emitter, entity)) {
				return false;
			}
			continue;
		}

		if (lhs->kind != Ast_DerefExpr) {
			return false;
		}

		FastScalarType result_type = {};
		if (!fast_backend_expr_scalar_type(nullptr, type_of_expr(lhs), &result_type)) {
			return false;
		}
		fast_backend_emit_push_work_reg(emitter);
		if (!fast_backend_emit_leaf_expr(emitter, lhs->DerefExpr.expr)) {
			return false;
		}
		fast_backend_emit_pop_tmp_reg(emitter);
		if (build_context.metrics.arch == TargetArch_amd64) {
			fast_backend_emit_x64_store_to_address(emitter->file, fast_backend_x64_work_reg(), fast_backend_x64_tmp_reg(), result_type);
		} else {
			fast_backend_emit_arm64_store_to_address(emitter->file, fast_backend_arm64_work_reg(), fast_backend_arm64_tmp_reg(), result_type);
		}
	}

	return true;
}

gb_internal bool fast_backend_emit_return_stmt(FastLeafProcEmitter *emitter, AstReturnStmt *rs) {
	if (rs->results.count != 0) {
		if (!fast_backend_emit_leaf_expr(emitter, rs->results[0])) {
			return false;
		}
		if (build_context.metrics.arch == TargetArch_amd64) {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_return_reg()->r64, fast_backend_x64_work_reg()->r64);
		} else {
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_return_reg(), fast_backend_arm64_work_reg());
		}
	}
	fast_backend_emit_jump_to_label(emitter->file, emitter->plan, emitter->epilogue_label_index);
	return true;
}

gb_internal bool fast_backend_emit_if_stmt(FastLeafProcEmitter *emitter, AstIfStmt *is) {
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

	return true;
}

gb_internal bool fast_backend_emit_for_stmt(FastLeafProcEmitter *emitter, AstForStmt *fs) {
	if (fs->init != nullptr && !fast_backend_emit_stmt(emitter, fs->init)) {
		return false;
	}

	i32 loop_label = fast_backend_alloc_label(emitter);
	i32 post_label = fs->post != nullptr ? fast_backend_alloc_label(emitter) : loop_label;
	i32 done_label = fast_backend_alloc_label(emitter);

	FastControlContext ctx = {};
	ctx.label = fs->label;
	ctx.break_label = done_label;
	ctx.continue_label = post_label;
	ctx.fallthrough_label = -1;
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
	return true;
}

gb_internal bool fast_backend_emit_switch_stmt(FastLeafProcEmitter *emitter, AstSwitchStmt *ss) {
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
		ctx.continue_label = -1;
		ctx.fallthrough_label = i+1 < body_labels.count ? body_labels[i+1] : done_label;
		array_add(&emitter->control_stack, ctx);
		defer (emitter->control_stack.count -= 1);
		if (!fast_backend_emit_stmt_list(emitter, cc->stmts)) {
			return false;
		}
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, done_label);
	}

	fast_backend_emit_label(emitter->file, emitter->plan, done_label);
	return true;
}

gb_internal bool fast_backend_emit_branch_stmt(FastLeafProcEmitter *emitter, AstBranchStmt *bs) {
	FastControlContext *ctx = fast_backend_find_control_context(emitter, bs->label, bs->token.kind);
	if (ctx == nullptr) {
		return false;
	}

	switch (bs->token.kind) {
	case Token_break:
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, ctx->break_label);
		return true;
	case Token_continue:
		fast_backend_emit_jump_to_label(emitter->file, emitter->plan, ctx->continue_label);
		return true;
	case Token_fallthrough:
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
		return fast_backend_emit_stmt_list(emitter, stmt->BlockStmt.stmts);
	case Ast_ValueDecl:
		return fast_backend_emit_value_decl(emitter, &stmt->ValueDecl);
	case Ast_AssignStmt:
		return fast_backend_emit_assign_stmt(emitter, &stmt->AssignStmt);
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
	i32 slot_bytes = 8 * cast(i32)emitter->plan->slots.count;
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
	for_array(i, emitter->plan->params) {
		Entity *param = emitter->plan->params[i];
		FastLocalSlot slot = {};
		if (!fast_backend_find_slot(emitter->plan, param, &slot)) {
			return false;
		}

		if (build_context.metrics.arch == TargetArch_amd64) {
			auto *src = fast_backend_x64_param_reg(emitter->plan->type, cast(i32)i);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_x64_work_reg()->r64, src->r64);
			fast_backend_emit_x64_canonicalize(emitter->file, fast_backend_x64_work_reg(), slot.type);
			fast_backend_emit_x64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_x64_work_reg());
		} else {
			char const *src = fast_backend_arm64_param_reg(cast(i32)i);
			if (src == nullptr) {
				return false;
			}
			gb_fprintf(emitter->file, "\tmov %s, %s\n", fast_backend_arm64_work_reg(), src);
			fast_backend_emit_arm64_canonicalize(emitter->file, fast_backend_arm64_work_reg(), slot.type);
			fast_backend_emit_arm64_store_reg_to_slot(emitter->file, emitter->plan, slot, fast_backend_arm64_work_reg());
		}
	}

	if (emitter->plan->has_context_slot) {
		i32 param_index = cast(i32)emitter->plan->params.count;
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
	i32 slot_bytes = 8 * cast(i32)emitter->plan->slots.count;
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
	emitter.current_spill_depth = 0;
	emitter.epilogue_label_index = 0;
	emitter.next_label_index = 1;
	emitter.use_frame = plan->spill_depth > 0 || plan->slots.count > 0;

	fast_backend_emit_leaf_prologue(&emitter);
	if (!fast_backend_emit_param_spills(&emitter)) {
		error(plan->entity->token, "Fast backend failed to spill procedure parameters");
		return false;
	}

	if (!fast_backend_emit_stmt_list(&emitter, plan->body->BlockStmt.stmts)) {
		error(plan->entity->token, "Fast backend failed to emit the procedure body");
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

gb_internal void fast_backend_emit_global_init_value(gbFile *file, FastGlobalVarPlan const &plan) {
	switch (plan.size) {
	case 1:
		gb_fprintf(file, "\t.byte %llu\n", cast(unsigned long long)(plan.init_value & 0xff));
		return;
	case 2:
		gb_fprintf(file, "\t.short %llu\n", cast(unsigned long long)(plan.init_value & 0xffff));
		return;
	case 4:
		gb_fprintf(file, "\t.long %llu\n", cast(unsigned long long)(plan.init_value & 0xffffffffull));
		return;
	case 8:
		gb_fprintf(file, "\t.quad %llu\n", cast(unsigned long long)plan.init_value);
		return;
	}

	gb_fprintf(file, "\t.zero %d\n", plan.size);
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
	if (plan.has_init_value) {
		fast_backend_emit_global_init_value(file, plan);
	} else {
		gb_fprintf(file, "\t.zero %d\n", plan.size);
	}
	if (build_context.metrics.os != TargetOs_darwin && build_context.metrics.os != TargetOs_windows) {
		gb_fprintf(file, ".size \"%.*s\", %d\n", LIT(asm_name), plan.size);
	}
	gb_fprintf(file, "\n");
	return true;
}

gb_internal bool fast_backend_write_object_assembly(FastBackendGenerator *gen, Array<FastGlobalVarPlan> const &globals, Array<FastLeafProcPlan> const &procedures, String asm_path) {
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
		i32 total_param_count = cast(i32)proc_plan->params.count + (proc_plan->has_context_slot ? 1 : 0);
		if (total_param_count > fast_backend_param_limit(proc_plan)) {
			error(plan.entity->token, "Fast backend does not yet support this many parameters");
			return false;
		}
		if (!fast_backend_emit_leaf_procedure(&file, proc_plan)) {
			return false;
		}
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

	auto globals = array_make<FastGlobalVarPlan>(heap_allocator(), 0, gen->info->definitions.count);
	auto procedures = array_make<FastLeafProcPlan>(heap_allocator(), 0, gen->info->definitions.count);
	if (!fast_backend_collect_program(gen, &globals, &procedures)) {
		return false;
	}

	String obj_path = path_to_string(permanent_allocator(), build_context.build_paths[BuildPath_Output]);
	String asm_path = concatenate_strings(permanent_allocator(), gen->output_base, str_lit(".fast.S"));

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
