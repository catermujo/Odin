#!/usr/bin/env bash
exec "$(dirname -- "$0")/qemu_heap_allocator.sh" openbsd_amd64 "$@"
