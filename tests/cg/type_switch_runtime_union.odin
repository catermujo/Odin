package main

import "base:runtime"

@(export)
classify :: proc(id: typeid) -> i32 {
	info := type_info_of(id)
	if info != nil {
		#partial switch _ in info.variant {
		case runtime.Type_Info_Named:                        return 1
		case runtime.Type_Info_Integer:                      return 2
		case runtime.Type_Info_Rune:                         return 3
		case runtime.Type_Info_Float:                        return 4
		case runtime.Type_Info_Complex:                      return 5
		case runtime.Type_Info_Quaternion:                   return 6
		case runtime.Type_Info_String:                       return 7
		case runtime.Type_Info_Boolean:                      return 8
		case runtime.Type_Info_Any:                          return 9
		case runtime.Type_Info_Type_Id:                      return 10
		case runtime.Type_Info_Pointer:                      return 11
		case runtime.Type_Info_Multi_Pointer:                return 12
		case runtime.Type_Info_Procedure:                    return 13
		case runtime.Type_Info_Array:                        return 14
		case runtime.Type_Info_Enumerated_Array:             return 15
		case runtime.Type_Info_Dynamic_Array:                return 16
		case runtime.Type_Info_Slice:                        return 17
		case runtime.Type_Info_Parameters:                   return 18
		case runtime.Type_Info_Struct:                       return 19
		case runtime.Type_Info_Union:                        return 20
		case runtime.Type_Info_Enum:                         return 21
		case runtime.Type_Info_Map:                          return 22
		case runtime.Type_Info_Bit_Set:                      return 23
		case runtime.Type_Info_Simd_Vector:                  return 24
		case runtime.Type_Info_Matrix:                       return 25
		case runtime.Type_Info_Soa_Pointer:                  return 26
		case runtime.Type_Info_Bit_Field:                    return 27
		case runtime.Type_Info_Fixed_Capacity_Dynamic_Array: return 28
		}
	}
	return 30
}

main :: proc() {
	_ = classify(i32)
}
