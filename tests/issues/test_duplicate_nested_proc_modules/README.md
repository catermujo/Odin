# Duplicate nested procedure definitions across split modules

DUMBAI: Keep the four Odin inputs byte-identical to the minimized baseline reproducer; small source-size changes alter module splitting and can hide the duplicate-definition defect.

Run from the `vendor/odin` root with `./odin build tests/issues/test_duplicate_nested_proc_modules -o:minimal`. The baseline compiler emits duplicate symbols for nested `run_chunk`; the fixed compiler must build and run successfully.
