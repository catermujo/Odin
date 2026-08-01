package main

import "core:os"
import "core:fmt"
import "core:path/filepath"

wait_like :: proc() -> (state: os.Process_State, err: os.Error) {
	state.pid = 7
	return
}

cleanup :: proc() -> os.Error {
	return nil
}

check_desc :: proc(desc: os.Process_Desc) {
	if desc.command[0] != "true" || desc.command[1] != "cg" {
		panic("CG process descriptor argument regression")
	}
}

empty :: proc() -> (state: os.Process_State, stdout, stderr: []byte, err: os.Error) {
	defer cleanup()
	state, err = wait_like()
	return
}

main :: proc() {
	dir, filename := os.split_path("/tmp/cg-result.odin")
	if dir != "/tmp" || filename != "cg-result.odin" {
		panic("CG split return regression")
	}
	temp_dir, temp_err := os.mkdir_temp("", "cg-process-result-*", context.temp_allocator)
	if temp_err != nil {
		panic("CG mkdir_temp result regression")
	}
	defer os.remove_all(temp_dir)
	nested, nested_err := filepath.join({temp_dir, "nested"}, context.temp_allocator)
	if nested_err != nil || os.make_directory_all(nested) != nil {
		panic("CG union nil comparison call regression")
	}

	state, stdout, stderr, err := empty()
	if state.pid != 7 || len(stdout) != 0 || len(stderr) != 0 || err != nil {
		panic("CG multi-result shared-nil regression")
	}

	command := make([]string, 2, context.temp_allocator)
	command[0] = "true"
	command[1] = "cg"
	if command[0] != "true" || command[1] != "cg" {
		panic("CG dynamic command storage regression")
	}
	desc := os.Process_Desc{command = command}
	if desc.command[0] != "true" || desc.command[1] != "cg" {
		panic("CG dynamic process descriptor regression")
	}
	check_desc(desc)
	runner := os.process_exec
	exec_state, exec_stdout, exec_stderr, exec_err := runner(desc, context.temp_allocator)
	if exec_err != nil {
		fmt.eprintf("CG dynamic process_exec error: %x\n", transmute(u64)exec_err)
		panic("CG dynamic process_exec error regression")
	}
	if len(exec_stdout) != 0 {
		panic("CG dynamic process_exec stdout regression")
	}
	if len(exec_stderr) != 0 {
		panic("CG dynamic process_exec stderr regression")
	}
	if !exec_state.success {
		panic("CG dynamic process_exec state regression")
	}
}
