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

gb_internal GB_COMPARE_PROC(codegen_procedure_import_cmp) {
	CodeGenProcedureImport *x = cast(CodeGenProcedureImport *)a;
	CodeGenProcedureImport *y = cast(CodeGenProcedureImport *)b;

	i32 result = codegen_global_entity_cmp(&x->requester, &y->requester);
	if (result != 0) {
		return result;
	}
	return codegen_global_entity_cmp(&x->procedure, &y->procedure);
}

gb_internal bool codegen_is_top_level_named_procedure(Entity *e) {
	return e != nullptr &&
	       e->kind == Entity_Procedure &&
	       e->token.string.len != 0 &&
	       e->scope != nullptr &&
	       (e->scope->flags & ScopeFlag_File) != 0;
}

gb_internal void codegen_build_global_plan(CodeGenGlobalPlan *plan, CheckerInfo *info, gbAllocator allocator) {
	array_init(&plan->constants, allocator);
	array_init(&plan->global_entities, allocator);
	array_init(&plan->initial_procedure_bodies, allocator);
	array_init(&plan->procedure_imports, allocator);

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
			array_add(&plan->initial_procedure_bodies, codegen_entity);
			break;
		default:
			GB_PANIC("unexpected global entity kind");
		}
	}
	array_sort(plan->initial_procedure_bodies, codegen_global_entity_cmp);

	for (Entity *requester : plan->global_entities) {
		if (!codegen_is_top_level_named_procedure(requester)) {
			continue;
		}

		DeclInfo *decl = decl_info_of_entity(requester);
		if (decl == nullptr) {
			continue;
		}

		rw_mutex_shared_lock(&decl->deps_mutex);
		FOR_PTR_SET(dep, decl->deps) {
			Entity *procedure = strip_entity_wrapping(dep);
			if (!codegen_is_top_level_named_procedure(procedure)) {
				continue;
			}
			array_add(&plan->procedure_imports, {requester, procedure});
		}
		rw_mutex_shared_unlock(&decl->deps_mutex);
	}

	array_sort(plan->procedure_imports, codegen_procedure_import_cmp);
	isize import_count = 0;
	for (CodeGenProcedureImport import : plan->procedure_imports) {
		if (import_count > 0) {
			CodeGenProcedureImport previous = plan->procedure_imports[import_count-1];
			if (previous.requester == import.requester && previous.procedure == import.procedure) {
				continue;
			}
		}
		plan->procedure_imports[import_count++] = import;
	}
	array_resize(&plan->procedure_imports, import_count);
}
