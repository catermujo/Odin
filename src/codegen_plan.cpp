gb_internal GB_COMPARE_PROC(codegen_global_entity_cmp) {
	Entity *x = *cast(Entity **)a;
	Entity *y = *cast(Entity **)b;
	if (x == y) {
		return 0;
	}
	if (x->kind != y->kind) {
		return cast(i32)(x->kind - y->kind);
	}
	return token_pos_cmp(x->token.pos, y->token.pos);
}

gb_internal void codegen_build_global_plan(CodeGenGlobalPlan *plan, CheckerInfo *info, gbAllocator allocator) {
	array_init(&plan->constants, allocator);
	array_init(&plan->global_entities, allocator);

	for (Entity *e : info->entities) {
		Scope *scope = e->scope;
		if ((scope->flags & ScopeFlag_File) == 0) {
			continue;
		}
		GB_ASSERT(scope->parent->flags & ScopeFlag_Pkg);

		if (e->kind == Entity_Variable || e->kind == Entity_ProcGroup) {
			continue;
		}

		if (e->kind != Entity_Constant && e->kind != Entity_TypeName && e->kind != Entity_Procedure) {
			continue;
		}

		if (e->kind == Entity_Constant) {
			array_add(&plan->constants, e);
			continue;
		}

		bool polymorphic_struct = false;
		if (e->type != nullptr && e->kind == Entity_TypeName) {
			Type *bt = base_type(e->type);
			if (bt->kind == Type_Struct) {
				polymorphic_struct = is_type_polymorphic(bt);
			}
		}

		if (!polymorphic_struct && e->min_dep_count.load(std::memory_order_relaxed) == 0) {
			continue;
		}

		Entity *codegen_entity = e;
		if (e->kind == Entity_Procedure) {
			codegen_entity = strip_entity_wrapping(e);
			if (codegen_entity == nullptr) {
				continue;
			}
			if (e->Procedure.is_foreign && e->Procedure.is_objc_impl_or_import) {
				continue;
			}
		}

		switch (e->kind) {
		case Entity_TypeName:
			array_add(&plan->global_entities, e);
			break;
		case Entity_Procedure:
			array_add(&plan->global_entities, codegen_entity);
			break;
		default:
			GB_PANIC("unexpected global entity kind");
		}
	}

}
