package main

dependency_leaf_01 :: proc() -> int { return 1 }
dependency_leaf_02 :: proc() -> int { return 2 }
dependency_leaf_03 :: proc() -> int { return 3 }
dependency_leaf_04 :: proc() -> int { return 4 }
dependency_leaf_05 :: proc() -> int { return 5 }
dependency_leaf_06 :: proc() -> int { return 6 }
dependency_leaf_07 :: proc() -> int { return 7 }
dependency_leaf_08 :: proc() -> int { return 8 }
dependency_leaf_09 :: proc() -> int { return 9 }
dependency_leaf_10 :: proc() -> int { return 10 }
dependency_leaf_11 :: proc() -> int { return 11 }
dependency_leaf_12 :: proc() -> int { return 12 }

dependency_tree_root :: proc() -> int {
	level_1_sibling_a :: proc() -> int { return dependency_leaf_01() }
	level_1_sibling_b :: proc() -> int { return dependency_leaf_02() }
	level_2 :: proc() -> int {
		level_2_sibling_a :: proc() -> int { return dependency_leaf_03() }
		level_2_sibling_b :: proc() -> int { return dependency_leaf_04() }
		level_3 :: proc() -> int {
			level_3_sibling_a :: proc() -> int { return dependency_leaf_05() }
			level_3_sibling_b :: proc() -> int { return dependency_leaf_06() }
			level_4 :: proc() -> int {
				level_4_sibling_a :: proc() -> int { return dependency_leaf_07() }
				level_4_sibling_b :: proc() -> int { return dependency_leaf_08() }
				level_5 :: proc() -> int {
					level_5_sibling_a :: proc() -> int { return dependency_leaf_09() }
					level_5_sibling_b :: proc() -> int { return dependency_leaf_10() }
					return level_5_sibling_a() + level_5_sibling_b()
				}
				return level_4_sibling_a() + level_4_sibling_b() + level_5()
			}
			return level_3_sibling_a() + level_3_sibling_b() + level_4()
		}
		return level_2_sibling_a() + level_2_sibling_b() + level_3()
	}
	level_3_sibling_a :: proc() -> int { return dependency_leaf_11() }
	level_3_sibling_b :: proc() -> int { return dependency_leaf_12() }
	return level_1_sibling_a() + level_1_sibling_b() + level_2() + level_3_sibling_a() + level_3_sibling_b()
}

main :: proc() {
	if dependency_tree_root() != 78 {
		panic("dependency tree scheduling lost child dependencies")
	}
}
