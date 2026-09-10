#!/usr/bin/env bash
exec "$(dirname -- "$0")/qemu_heap_allocator.sh" linux_riscv64 "$@"
