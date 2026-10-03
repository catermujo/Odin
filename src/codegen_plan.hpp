struct CodeGenProcedureImport {
	Entity *requester;
	Entity *procedure;
};

struct CodeGenGlobalPlan {
	Array<Entity *>                 constants;
	Array<Entity *>                 global_entities;
	Array<Entity *>                 initial_procedure_bodies;
	Array<CodeGenProcedureImport> procedure_imports;
};

gb_internal void codegen_build_global_plan(CodeGenGlobalPlan *plan, CheckerInfo *info, gbAllocator allocator);
gb_internal GB_COMPARE_PROC(codegen_global_entity_cmp);
gb_internal GB_COMPARE_PROC(codegen_procedure_import_cmp);
