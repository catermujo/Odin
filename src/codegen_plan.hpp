struct CodeGenGlobalPlan {
	Array<Entity *> constants;
	Array<Entity *> global_entities;
};

gb_internal void codegen_build_global_plan(CodeGenGlobalPlan *plan, CheckerInfo *info, gbAllocator allocator);
gb_internal GB_COMPARE_PROC(codegen_global_entity_cmp);
