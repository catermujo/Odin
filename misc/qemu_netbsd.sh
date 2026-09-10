#!/usr/bin/env bash
set -euo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly NETBSD_ARCH="${QEMU_ARCH:-amd64}"
readonly NETBSD_TARGET="netbsd_${NETBSD_ARCH}"
readonly NETBSD_CACHE_DEFAULT="${XDG_CACHE_HOME:-${HOME}/.cache}/netbsd-qemu"
if [[ -n "${NETBSD_QEMU_CACHE_DIR:-}" ]]; then
    export QEMU_CACHE_DIR="${NETBSD_QEMU_CACHE_DIR}"
elif [[ -n "${QEMU_CACHE_DIR:-}" ]]; then
    export QEMU_CACHE_DIR="${QEMU_CACHE_DIR}"
elif [[ -n "${QEMU_ARCH:-}" ]]; then
    export QEMU_CACHE_DIR="${NETBSD_CACHE_DEFAULT}/${NETBSD_ARCH}"
    export QEMU_CACHE_DIR_EXACT=1
else
    export QEMU_CACHE_DIR="${NETBSD_CACHE_DEFAULT}"
    export QEMU_CACHE_DIR_EXACT=1
fi
export QEMU_SSH_PORT="${QEMU_SSH_PORT:-2222}"
exec "${SCRIPT_DIR}/qemu_heap_allocator.sh" "${NETBSD_TARGET}" "$@"
