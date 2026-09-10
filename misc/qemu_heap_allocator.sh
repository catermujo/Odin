#!/usr/bin/env bash
set -euo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ODIN_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
readonly QEMU_BIN_OVERRIDE="${QEMU_BIN:-}"
readonly ALL_TARGETS=(
    linux_i386 linux_amd64 linux_arm32 linux_arm64 linux_riscv64
    freebsd_i386 freebsd_amd64 freebsd_arm64
    netbsd_amd64 netbsd_arm64 openbsd_amd64
)
BUILD_DIR=""
VM_STARTED_BY_HARNESS=0
CLEANUP_VM_ON_EXIT=0

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

usage() {
    printf '%s\n' \
        'usage: misc/qemu_heap_allocator.sh <target|all> prepare|start|stop|config' \
        '       misc/qemu_heap_allocator.sh <target> bootstrap' \
        '       misc/qemu_heap_allocator.sh <target> ssh [command]' \
        '       misc/qemu_heap_allocator.sh <target|all> test [--long]' \
        '' \
        "targets: ${ALL_TARGETS[*]}" \
        '' \
        'bootstrap opens the one-time installation/configuration console.' \
        'test emits native objects on the host, links them in the guest, and runs the allocator suite.'
}

contains_target() {
    local candidate="$1"
    local supported
    for supported in "${ALL_TARGETS[@]}"; do
        [[ "${candidate}" == "${supported}" ]] && return 0
    done
    return 1
}

find_firmware() {
    local firmware_name="$1"
    shift
    local candidate
    if [[ -n "${QEMU_EFI:-}" ]]; then
        [[ -f "${QEMU_EFI}" ]] || die "QEMU_EFI does not exist: ${QEMU_EFI}"
        printf '%s\n' "${QEMU_EFI}"
        return
    fi
    for candidate in "$@"; do
        [[ ! -f "${candidate}" ]] || { printf '%s\n' "${candidate}"; return; }
    done
    die "missing ${firmware_name} firmware; set QEMU_EFI to its path"
}

configure_target() {
    TARGET="$1"
    contains_target "${TARGET}" || die "unsupported target: ${TARGET}"
    VM_STARTED_BY_HARNESS=0
    OS_NAME="${TARGET%%_*}"
    ARCH="${TARGET#*_}"
    if [[ "${QEMU_CACHE_DIR_EXACT:-0}" == 1 && -n "${QEMU_CACHE_DIR:-}" ]]; then
        CACHE_DIR="${QEMU_CACHE_DIR}"
    else
        CACHE_DIR="${QEMU_CACHE_DIR:-${XDG_CACHE_HOME:-${HOME}/.cache}/odin-qemu}/${TARGET}"
    fi
    SSH_KEY="${QEMU_SSH_KEY:-${CACHE_DIR}/ssh_key}"
    PID_FILE="${CACHE_DIR}/qemu.pid"
    SERIAL_LOG="${CACHE_DIR}/serial.log"
    QEMU_LOG="${CACHE_DIR}/qemu.log"
    IMAGE_ID_FILE="${CACHE_DIR}/image.url"
    DISK="${CACHE_DIR}/disk.qcow2"
    MEMORY="${QEMU_MEM:-4096}"
    CPUS="${QEMU_CPU:-4}"
    DISK_SIZE="${QEMU_DISK_SIZE:-16G}"
    INSTALL_MEDIA=""
    BASE_IMAGE=""
    IMAGE_FORMAT=""
    LINK_LIBRARIES=(-pthread -lm)

    case "${TARGET}" in
    linux_i386)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-i386}"; SSH_PORT="${QEMU_SSH_PORT:-2201}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/x86/alpine-standard-3.24.1-x86.iso}" ;;
    linux_amd64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-x86_64}"; SSH_PORT="${QEMU_SSH_PORT:-2202}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/x86_64/alpine-standard-3.24.1-x86_64.iso}" ;;
    linux_arm32)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-arm}"; SSH_PORT="${QEMU_SSH_PORT:-2203}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/armv7/alpine-standard-3.24.1-armv7.iso}" ;;
    linux_arm64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-aarch64}"; SSH_PORT="${QEMU_SSH_PORT:-2204}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/aarch64/alpine-standard-3.24.1-aarch64.iso}" ;;
    linux_riscv64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-riscv64}"; SSH_PORT="${QEMU_SSH_PORT:-2205}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://dl-cdn.alpinelinux.org/alpine/v3.24/releases/riscv64/alpine-standard-3.24.1-riscv64.iso}" ;;
    freebsd_i386)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-i386}"; SSH_PORT="${QEMU_SSH_PORT:-2211}"; IMAGE_FORMAT=qcow2
        IMAGE_URL="${QEMU_IMAGE_URL:-https://download.freebsd.org/releases/VM-IMAGES/14.3-RELEASE/i386/Latest/FreeBSD-14.3-RELEASE-i386-ufs.qcow2.xz}" ;;
    freebsd_amd64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-x86_64}"; SSH_PORT="${QEMU_SSH_PORT:-2212}"; IMAGE_FORMAT=qcow2
        IMAGE_URL="${QEMU_IMAGE_URL:-https://download.freebsd.org/releases/VM-IMAGES/14.3-RELEASE/amd64/Latest/FreeBSD-14.3-RELEASE-amd64-ufs.qcow2.xz}" ;;
    freebsd_arm64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-aarch64}"; SSH_PORT="${QEMU_SSH_PORT:-2213}"; IMAGE_FORMAT=qcow2
        IMAGE_URL="${QEMU_IMAGE_URL:-https://download.freebsd.org/releases/VM-IMAGES/14.3-RELEASE/aarch64/Latest/FreeBSD-14.3-RELEASE-arm64-aarch64-ufs.qcow2.xz}" ;;
    netbsd_amd64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-x86_64}"; SSH_PORT="${QEMU_SSH_PORT:-2221}"; IMAGE_FORMAT=raw
        IMAGE_URL="${QEMU_IMAGE_URL:-https://cdn.netbsd.org/pub/NetBSD/images/10.1/NetBSD-10.1-amd64-live.img.gz}" ;;
    netbsd_arm64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-aarch64}"; SSH_PORT="${QEMU_SSH_PORT:-2222}"; IMAGE_FORMAT=raw
        IMAGE_URL="${QEMU_IMAGE_URL:-https://cdn.netbsd.org/pub/NetBSD/NetBSD-10.1/evbarm-aarch64/binary/gzimg/arm64.img.gz}" ;;
    openbsd_amd64)
        QEMU_BIN="${QEMU_BIN_OVERRIDE:-qemu-system-x86_64}"; SSH_PORT="${QEMU_SSH_PORT:-2231}"
        IMAGE_URL="${QEMU_IMAGE_URL:-https://cdn.openbsd.org/pub/OpenBSD/7.7/amd64/install77.iso}" ;;
    esac

    case "${OS_NAME}" in
    freebsd) LINK_LIBRARIES=(-pthread -lm) ;;
    openbsd) LINK_LIBRARIES=(-pthread -lm -Wl,-z,nobtcfi) ;;
    esac

    case "${ARCH}" in
    i386|amd64)
        MACHINE_ARGS=(-machine pc -cpu max)
        NETWORK_ARGS=(-device e1000,netdev=net0)
        DISK_ARGS=(-drive "file=${DISK},format=qcow2,if=virtio") ;;
    arm32)
        FIRMWARE="$(find_firmware 'ARM EDK2' /opt/homebrew/share/qemu/edk2-arm-code.fd /usr/share/qemu/edk2-arm-code.fd /usr/share/qemu-efi-arm/QEMU_EFI.fd)"
        MACHINE_ARGS=(-machine virt -cpu cortex-a15 -bios "${FIRMWARE}")
        NETWORK_ARGS=(-device virtio-net-pci,netdev=net0)
        DISK_ARGS=(-drive "if=none,id=disk0,file=${DISK},format=qcow2" -device virtio-blk-pci,drive=disk0) ;;
    arm64)
        FIRMWARE="$(find_firmware 'AArch64 EDK2' /opt/homebrew/share/qemu/edk2-aarch64-code.fd /usr/share/qemu/edk2-aarch64-code.fd /usr/share/qemu-efi-aarch64/QEMU_EFI.fd /usr/share/AAVMF/AAVMF_CODE.fd)"
        MACHINE_ARGS=(-machine virt -cpu max -bios "${FIRMWARE}")
        NETWORK_ARGS=(-device virtio-net-pci,netdev=net0)
        DISK_ARGS=(-drive "if=none,id=disk0,file=${DISK},format=qcow2" -device virtio-blk-pci,drive=disk0) ;;
    riscv64)
        FIRMWARE="$(find_firmware 'RISC-V EDK2' /opt/homebrew/share/qemu/edk2-riscv-code.fd /usr/share/qemu/edk2-riscv-code.fd)"
        MACHINE_ARGS=(-machine virt -cpu max -bios "${FIRMWARE}")
        NETWORK_ARGS=(-device virtio-net-pci,netdev=net0)
        DISK_ARGS=(-drive "if=none,id=disk0,file=${DISK},format=qcow2" -device virtio-blk-pci,drive=disk0) ;;
    esac

    QEMU_ARGS=(
        "${MACHINE_ARGS[@]}" -m "${MEMORY}" -smp "${CPUS}"
        -netdev "user,id=net0,hostfwd=tcp:127.0.0.1:${SSH_PORT}-:22"
        "${NETWORK_ARGS[@]}" "${DISK_ARGS[@]}"
    )
}

require_commands() {
    local action="$1"
    local required_command
    local required_commands=()
    case "${action}" in
    prepare) required_commands=(curl qemu-img ssh-keygen) ;;
    bootstrap) required_commands=(curl qemu-img ps ssh-keygen "${QEMU_BIN}") ;;
    start) required_commands=(curl qemu-img ps ssh ssh-keygen "${QEMU_BIN}") ;;
    test) required_commands=(curl qemu-img ps ssh ssh-keygen tar "${QEMU_BIN}") ;;
    ssh) required_commands=(ssh ssh-keygen) ;;
    stop) required_commands=(ps) ;;
    config) return ;;
    *) die "unsupported command: ${action}" ;;
    esac

    for required_command in "${required_commands[@]}"; do
        command -v "${required_command}" >/dev/null 2>&1 || die "missing command: ${required_command}"
    done
    case "${action}" in
    prepare|bootstrap|start|test)
        [[ "${IMAGE_URL}" != *.gz ]] || command -v gzip >/dev/null 2>&1 || die 'missing command: gzip'
        [[ "${IMAGE_URL}" != *.xz ]] || command -v xz >/dev/null 2>&1 || die 'missing command: xz'
        ;;
    esac
}

download() {
    local destination="$1"
    [[ -f "${destination}" ]] && return
    printf 'Downloading %s...\n' "${IMAGE_URL}"
    curl -fL --retry 3 -o "${destination}.part" "${IMAGE_URL}"
    mv -- "${destination}.part" "${destination}"
}

check_image_identity() {
    [[ ! -f "${IMAGE_ID_FILE}" ]] && return
    local cached_url
    cached_url="$(<"${IMAGE_ID_FILE}")"
    [[ "${cached_url}" == "${IMAGE_URL}" ]] || die \
        "cached image belongs to ${cached_url}; use a target-specific QEMU_CACHE_DIR for ${IMAGE_URL}"
}

record_image_identity() {
    printf '%s\n' "${IMAGE_URL}" > "${IMAGE_ID_FILE}.part"
    mv -- "${IMAGE_ID_FILE}.part" "${IMAGE_ID_FILE}"
}

prepare_image() {
    mkdir -p -- "${CACHE_DIR}"
    check_image_identity
    if [[ -z "${IMAGE_FORMAT}" ]]; then
        INSTALL_MEDIA="${CACHE_DIR}/$(basename -- "${IMAGE_URL}")"
        download "${INSTALL_MEDIA}"
        if [[ ! -f "${DISK}" ]]; then
            printf 'Creating blank %s disk...\n' "${DISK_SIZE}"
            qemu-img create -f qcow2 "${DISK}" "${DISK_SIZE}"
        fi
        record_image_identity
        return
    fi

    local compressed_image="${CACHE_DIR}/$(basename -- "${IMAGE_URL}")"
    BASE_IMAGE="${compressed_image%.gz}"
    BASE_IMAGE="${BASE_IMAGE%.xz}"
    download "${compressed_image}"
    if [[ ! -f "${BASE_IMAGE}" ]]; then
        printf 'Decompressing base image...\n'
        if [[ "${compressed_image}" == *.gz ]]; then
            gzip -t "${compressed_image}"
            gzip -dc "${compressed_image}" > "${BASE_IMAGE}.part"
        else
            xz -t "${compressed_image}"
            xz -dc "${compressed_image}" > "${BASE_IMAGE}.part"
        fi
        mv -- "${BASE_IMAGE}.part" "${BASE_IMAGE}"
    fi
    if [[ ! -f "${DISK}" ]]; then
        printf 'Creating writable overlay...\n'
        qemu-img create -f qcow2 -F "${IMAGE_FORMAT}" -b "${BASE_IMAGE}" "${DISK}"
    fi
    record_image_identity
}

prepare_key() {
    mkdir -p -- "${CACHE_DIR}" "$(dirname -- "${SSH_KEY}")"
    if [[ ! -f "${SSH_KEY}" ]]; then
        ssh-keygen -q -t ed25519 -N '' -f "${SSH_KEY}" -C "${TARGET}-qemu"
    elif [[ ! -f "${SSH_KEY}.pub" ]]; then
        ssh-keygen -y -f "${SSH_KEY}" > "${SSH_KEY}.pub"
    fi
    return 0
}

vm_pid() {
    [[ -f "${PID_FILE}" ]] || return 1
    local pid
    pid="$(<"${PID_FILE}")"
    [[ "${pid}" =~ ^[0-9]+$ ]] || return 1
    kill -0 "${pid}" 2>/dev/null || return 1
    local command_line
    command_line="$(ps -p "${pid}" -o command= 2>/dev/null)" || return 1
    [[ "${command_line}" == *"${QEMU_BIN}"* ]] || return 1
    printf '%s\n' "${pid}"
}

ssh_vm() {
    prepare_key
    ssh -i "${SSH_KEY}" -o BatchMode=yes -o ConnectTimeout=2 -o IdentitiesOnly=yes \
        -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -p "${SSH_PORT}" root@127.0.0.1 "$@"
}

wait_for_ssh() {
    local attempt
    for attempt in {1..120}; do
        ssh_vm true >/dev/null 2>&1 && return
        if [[ -f "${PID_FILE}" ]] && ! vm_pid >/dev/null; then
            return 1
        fi
        sleep 1
    done
    return 1
}

start_vm() {
    prepare_image
    prepare_key
    if vm_pid >/dev/null; then
        printf '%s VM already running on SSH port %s.\n' "${TARGET}" "${SSH_PORT}"
        return
    fi
    rm -f -- "${PID_FILE}"
    printf 'Starting %s headlessly on SSH port %s...\n' "${TARGET}" "${SSH_PORT}"
    "${QEMU_BIN}" "${QEMU_ARGS[@]}" -display none -serial "file:${SERIAL_LOG}" -monitor none \
        -D "${QEMU_LOG}" -daemonize -pidfile "${PID_FILE}"
    VM_STARTED_BY_HARNESS=1
    wait_for_ssh || die "VM did not accept SSH; inspect ${SERIAL_LOG} and ${QEMU_LOG}"
    printf 'SSH ready: %s %s ssh\n' "$0" "${TARGET}"
}

print_bootstrap_commands() {
    local public_key="$(<"${SSH_KEY}.pub")"
    printf '%s\n' \
        "Configure ${TARGET}, networking, and a root account in the console."
    case "${OS_NAME}" in
    linux)
        printf '%s\n' \
            'For Alpine Linux, run setup-alpine, then install the test prerequisites:' \
            'apk add build-base openssh'
        ;;
    freebsd) printf '%s\n' 'The FreeBSD VM image is preinstalled; log in as root.' ;;
    netbsd) printf '%s\n' 'The NetBSD live image provides a root console; configure SSH and the compiler as needed.' ;;
    openbsd) printf '%s\n' 'Run the OpenBSD installer and install the base system on the writable disk.' ;;
    esac
    printf '%s\n' 'Then run these commands as root:' 'mkdir -p /root/.ssh'
    printf 'printf "%%s\\n" %q > /root/.ssh/authorized_keys\n' "${public_key}"
    printf '%s\n' 'chmod 700 /root/.ssh' 'chmod 600 /root/.ssh/authorized_keys'
    case "${OS_NAME}" in
    linux) printf '%s\n' 'rc-update add sshd' 'rc-service sshd restart' ;;
    freebsd) printf '%s\n' 'sysrc sshd_enable=YES' 'service sshd restart' ;;
    netbsd) printf '%s\n' \
        'echo PermitRootLogin yes >> /etc/ssh/sshd_config' \
        'echo PubkeyAuthentication yes >> /etc/ssh/sshd_config' \
        'echo sshd=YES >> /etc/rc.conf' \
        '/etc/rc.d/sshd restart' ;;
    openbsd) printf '%s\n' 'rcctl enable sshd' 'rcctl restart sshd' ;;
    esac
    printf '%s\n' 'Shut down the guest, then leave QEMU with Ctrl-A followed by X.' "Run: $0 ${TARGET} start"
}

bootstrap_vm() {
    prepare_image
    prepare_key
    vm_pid >/dev/null && die 'VM already running; stop it first'
    local installer_args=()
    if [[ -n "${INSTALL_MEDIA}" ]]; then
        if [[ "${ARCH}" == i386 || "${ARCH}" == amd64 ]]; then
            installer_args=(-drive "file=${INSTALL_MEDIA},format=raw,media=cdrom,readonly=on" -boot order=d)
        else
            installer_args=(-drive "if=none,id=install0,file=${INSTALL_MEDIA},format=raw,readonly=on" -device virtio-blk-pci,drive=install0 -boot order=d)
        fi
    fi
    print_bootstrap_commands
    if [[ "${ARCH}" == i386 || "${ARCH}" == amd64 ]]; then
        "${QEMU_BIN}" "${QEMU_ARGS[@]}" "${installer_args[@]}" -display curses -serial none -monitor none
    else
        "${QEMU_BIN}" "${QEMU_ARGS[@]}" "${installer_args[@]}" -nographic
    fi
}

build_test_objects() {
    BUILD_DIR="$(mktemp -d "${CACHE_DIR}/heap-test.XXXXXX")"
    "${ODIN_ROOT}/odin" build "${ODIN_ROOT}/tests/heap_allocator" \
        -target:"${TARGET}" -build-mode:object -out:"${BUILD_DIR}/heap_allocator" \
        -vet -vet-tabs -strict-style -vet-style -warnings-as-errors -disallow-do -debug
}

cleanup_build_dir() {
    [[ -n "${BUILD_DIR}" ]] || return 0
    rm -rf -- "${BUILD_DIR}"
    BUILD_DIR=""
}

check_guest_compiler() {
    if ssh_vm 'command -v cc >/dev/null 2>&1'; then
        return
    fi
    die "guest ${TARGET} has no usable cc; run $0 ${TARGET} bootstrap and install its C compiler"
}

tar_create_flag_supported() {
    local flag="$1"
    COPYFILE_DISABLE=1 tar "${flag}" -cf /dev/null -T /dev/null >/dev/null 2>&1
}

create_test_archive() {
    local candidate
    local tar_args=()
    for candidate in --no-xattrs --no-mac-metadata; do
        tar_create_flag_supported "${candidate}" && tar_args+=("${candidate}")
    done
    COPYFILE_DISABLE=1 tar "${tar_args[@]}" -C "${BUILD_DIR}" -cf - .
}

test_vm() {
    local long_mode="$1"
    CLEANUP_VM_ON_EXIT=1
    start_vm
    build_test_objects
    check_guest_compiler
    printf 'Transferring and linking %s allocator test...\n' "${TARGET}"
    create_test_archive | ssh_vm 'rm -rf /root/odin-heap-test && mkdir -p /root/odin-heap-test && tar -C /root/odin-heap-test -xf -'
    ssh_vm "cd /root/odin-heap-test && cc ./*.o -o heap_allocator ${LINK_LIBRARIES[*]}"
    local test_args=(-allocator=feoramalloc -vmem-tests -serial-tests -parallel-tests)
    [[ "${long_mode}" == 1 ]] && test_args+=(-long)
    ssh_vm /root/odin-heap-test/heap_allocator "${test_args[@]}"
    cleanup_build_dir
    cleanup_started_vm
}

stop_vm() {
    local pid
    if ! pid="$(vm_pid)"; then
        printf '%s VM not running.\n' "${TARGET}"
        return
    fi
    kill "${pid}"
    for _ in {1..20}; do kill -0 "${pid}" 2>/dev/null || break; sleep 1; done
    kill -0 "${pid}" 2>/dev/null && die "VM did not stop; kill it manually: ${pid}"
    rm -f -- "${PID_FILE}"
    printf '%s VM stopped.\n' "${TARGET}"
}

cleanup_started_vm() {
    [[ "${VM_STARTED_BY_HARNESS}" == 1 ]] || return 0
    VM_STARTED_BY_HARNESS=0
    stop_vm || true
}

cleanup_test_resources() {
    local exit_status="$?"
    trap - EXIT INT TERM
    if [[ "${CLEANUP_VM_ON_EXIT}" == 1 ]]; then
        cleanup_started_vm
    fi
    cleanup_build_dir
    exit "${exit_status}"
}

print_config() {
    printf 'target=%s\nqemu=%s\nssh_port=%s\ncache=%s\nimage=%s\n' \
        "${TARGET}" "${QEMU_BIN}" "${SSH_PORT}" "${CACHE_DIR}" "${IMAGE_URL}"
    printf 'qemu_args='; printf ' %q' "${QEMU_ARGS[@]}"; printf '\n'
}

run_one() {
    local target_name="$1"
    local command_name="$2"
    shift 2
    configure_target "${target_name}"
    require_commands "${command_name}"
    case "${command_name}" in
    prepare) prepare_image; prepare_key ;;
    bootstrap) bootstrap_vm ;;
    start) start_vm ;;
    ssh) ssh_vm "$@" ;;
    test)
        case "$#" in
        0) test_vm 0 ;;
        1) [[ "$1" == --long ]] || die 'test accepts only --long'; test_vm 1 ;;
        *) die 'test accepts only --long' ;;
        esac
        ;;
    stop) stop_vm ;;
    config) print_config ;;
    *) usage; exit 2 ;;
    esac
}

run_all_tests() {
    CLEANUP_VM_ON_EXIT=1
    trap 'exit 130' INT
    trap 'exit 143' TERM
    local target_name
    local exit_status
    for target_name in "${ALL_TARGETS[@]}"; do
        if run_one "${target_name}" test "$@"; then
            cleanup_started_vm
        else
            exit_status="$?"
            cleanup_started_vm
            return "${exit_status}"
        fi
    done
}

TARGET_ARGUMENT="${1:-}"
COMMAND_ARGUMENT="${2:-}"
[[ -n "${TARGET_ARGUMENT}" && -n "${COMMAND_ARGUMENT}" ]] || { usage; exit 2; }
shift 2
trap cleanup_test_resources EXIT
if [[ "${TARGET_ARGUMENT}" == all ]]; then
    [[ "${COMMAND_ARGUMENT}" != bootstrap && "${COMMAND_ARGUMENT}" != ssh ]] || die "${COMMAND_ARGUMENT} requires one target"
    if [[ "${COMMAND_ARGUMENT}" == test ]]; then
        run_all_tests "$@"
    else
        for target_name in "${ALL_TARGETS[@]}"; do run_one "${target_name}" "${COMMAND_ARGUMENT}" "$@"; done
    fi
else
    run_one "${TARGET_ARGUMENT}" "${COMMAND_ARGUMENT}" "$@"
fi
