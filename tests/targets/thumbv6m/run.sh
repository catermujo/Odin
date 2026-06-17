#!/usr/bin/env bash
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$SCRIPT_DIR"

mkdir -p build
pushd build >/dev/null

ODIN=../../../../odin
COMMON="-target:freestanding_thumbv6m -microarch:cortex-m0plus -no-crt -default-to-nil-allocator -no-thread-local -no-rtti -vet -vet-tabs -strict-style -vet-style -warnings-as-errors -disallow-do"
RP2040_COMMON="-target:freestanding_thumbv6m -subtarget:rp2040 -microarch:cortex-m0plus -default-to-nil-allocator -no-thread-local -no-rtti -vet -vet-tabs -strict-style -vet-style -warnings-as-errors -disallow-do"
UNSUPPORTED_MSG="'-target:freestanding_thumbv6m' does not support this build mode/command in Odin v1"
EXE_REQUIRES_NO_CRT_MSG="'-target:freestanding_thumbv6m' executable builds currently require -no-crt"
MAIN_EXPORT_FLAGS="$PWD/startup.o -Wl,-T,$SCRIPT_DIR/main_export/link.ld -Wl,--gc-sections"

set -x

$ODIN build ../smoke $COMMON -build-mode:object -out:thumbv6m_smoke_object
$ODIN build ../smoke $COMMON -build-mode:assembly -out:thumbv6m_smoke_asm
$ODIN build ../smoke $COMMON -build-mode:llvm-ir -out:thumbv6m_smoke_llvm
$ODIN build ../main_export $COMMON -no-entry-point -build-mode:object -out:thumbv6m_main_export_object
$ODIN build ../rp2040_main $RP2040_COMMON -build-mode:exe -out:thumbv6m_rp2040_exe
clang --target=thumbv6m-none-eabi -c ../main_export/startup.S -o startup.o
$ODIN build ../main_export $COMMON -no-entry-point -build-mode:exe -linker:lld -extra-linker-flags:"$MAIN_EXPORT_FLAGS" -out:thumbv6m_main_export_exe

check_unsupported_failure() {
	local mode="$1"
	if $ODIN build ../smoke $COMMON -build-mode:"$mode" 2>&1 >/dev/null | grep -Fq "$UNSUPPORTED_MSG"; then
		echo "SUCCESSFUL $mode"
	else
		echo "FAILED $mode"
		exit 1
	fi
}

check_object_only_command_failure() {
	local command="$1"
	if $ODIN "$command" ../smoke $COMMON 2>&1 >/dev/null | grep -Fq "$UNSUPPORTED_MSG"; then
		echo "SUCCESSFUL $command"
	else
		echo "FAILED $command"
		exit 1
	fi
}

check_exe_requires_no_crt_failure() {
	if $ODIN build ../main_export -target:freestanding_thumbv6m -microarch:cortex-m0plus -default-to-nil-allocator -no-thread-local -no-rtti -vet -vet-tabs -strict-style -vet-style -warnings-as-errors -disallow-do -no-entry-point -build-mode:exe -linker:lld -extra-linker-flags:"$MAIN_EXPORT_FLAGS" 2>&1 >/dev/null | grep -Fq "$EXE_REQUIRES_NO_CRT_MSG"; then
		echo "SUCCESSFUL exe-no-crt-check"
	else
		echo "FAILED exe-no-crt-check"
		exit 1
	fi
}

check_exe_requires_no_crt_failure
check_unsupported_failure static
check_unsupported_failure dynamic
check_object_only_command_failure run
check_object_only_command_failure test

set +x

popd >/dev/null
rm -rf build
