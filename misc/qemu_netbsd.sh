#!/usr/bin/env bash
set -euo pipefail

readonly RELEASE="10.0"
readonly ARCH="amd64"
readonly CACHE_DIR="${NETBSD_QEMU_CACHE_DIR:-${XDG_CACHE_HOME:-${HOME}/.cache}/netbsd-qemu}"
readonly IMAGE_GZ="${CACHE_DIR}/NetBSD-${RELEASE}-${ARCH}-live.img.gz"
readonly IMAGE="${CACHE_DIR}/NetBSD-${RELEASE}-${ARCH}-live.img"
readonly DISK="${CACHE_DIR}/netbsd-${RELEASE}.qcow2"
readonly SSH_KEY="${CACHE_DIR}/ssh_key"
readonly SSH_PORT="${QEMU_SSH_PORT:-2222}"
readonly MEMORY="${QEMU_MEM:-4096}"
readonly CPUS="${QEMU_CPU:-4}"
readonly PID_FILE="${CACHE_DIR}/qemu.pid"
readonly SERIAL_LOG="${CACHE_DIR}/serial.log"
readonly QEMU_LOG="${CACHE_DIR}/qemu.log"
readonly QEMU_BIN="${QEMU_BIN:-qemu-system-x86_64}"
readonly IMAGE_URL="https://cdn.netbsd.org/pub/NetBSD/images/${RELEASE}/NetBSD-${RELEASE}-${ARCH}-live.img.gz"
readonly QEMU_ARGS=(
    -machine pc
    -m "${MEMORY}"
    -smp "${CPUS}"
    -netdev "user,id=net0,hostfwd=tcp:127.0.0.1:${SSH_PORT}-:22"
    -device e1000,netdev=net0
    -drive "file=${DISK},format=qcow2,if=virtio"
)

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

require_commands() {
    local command_name
    for command_name in curl gzip qemu-img ssh ssh-keygen "${QEMU_BIN}"; do
        command -v "${command_name}" >/dev/null 2>&1 || die "missing command: ${command_name}"
    done
}

prepare_image() {
    mkdir -p "${CACHE_DIR}"

    if [[ ! -f "${IMAGE}" ]]; then
        printf 'Downloading NetBSD %s live image...\n' "${RELEASE}"
        curl -fL --retry 3 -o "${IMAGE_GZ}.part" "${IMAGE_URL}"
        gzip -t "${IMAGE_GZ}.part"
        gzip -dc "${IMAGE_GZ}.part" > "${IMAGE}.part"
        mv "${IMAGE_GZ}.part" "${IMAGE_GZ}"
        mv "${IMAGE}.part" "${IMAGE}"
    fi

    if [[ ! -f "${DISK}" ]]; then
        printf 'Creating writable NetBSD overlay...\n'
        qemu-img create -f qcow2 -F raw -b "${IMAGE}" "${DISK}"
    fi
}

prepare_key() {
    mkdir -p "${CACHE_DIR}"
    if [[ ! -f "${SSH_KEY}" ]]; then
        ssh-keygen -q -t ed25519 -N '' -f "${SSH_KEY}" -C netbsd-qemu
    elif [[ ! -f "${SSH_KEY}.pub" ]]; then
        ssh-keygen -y -f "${SSH_KEY}" > "${SSH_KEY}.pub"
    fi
}

vm_pid() {
    [[ -f "${PID_FILE}" ]] || return 1
    local pid
    pid="$(<"${PID_FILE}")"
    [[ "${pid}" =~ ^[0-9]+$ ]] || return 1
    kill -0 "${pid}" 2>/dev/null || return 1
    printf '%s\n' "${pid}"
}

wait_for_ssh() {
    local attempt
    for attempt in {1..60}; do
        if ssh \
            -i "${SSH_KEY}" \
            -o BatchMode=yes \
            -o ConnectTimeout=1 \
            -o IdentitiesOnly=yes \
            -o StrictHostKeyChecking=no \
            -o UserKnownHostsFile=/dev/null \
            -p "${SSH_PORT}" \
            root@127.0.0.1 true \
            >/dev/null 2>&1; then
            return 0
        fi
        sleep 1
    done
    return 1
}

start_vm() {
    prepare_image
    prepare_key

    if vm_pid >/dev/null; then
        printf 'NetBSD VM already running: ssh -p %s root@127.0.0.1\n' "${SSH_PORT}"
        return 0
    fi

    rm -f "${PID_FILE}"
    printf 'Starting headless NetBSD %s on SSH port %s...\n' "${RELEASE}" "${SSH_PORT}"
    "${QEMU_BIN}" \
        "${QEMU_ARGS[@]}" \
        -display none \
        -serial "file:${SERIAL_LOG}" \
        -monitor none \
        -D "${QEMU_LOG}" \
        -daemonize \
        -pidfile "${PID_FILE}"

    if ! wait_for_ssh; then
        printf 'VM did not accept SSH. Inspect %s and %s.\n' "${SERIAL_LOG}" "${QEMU_LOG}" >&2
        return 1
    fi

    printf 'SSH ready: ssh -i %s -p %s root@127.0.0.1\n' "${SSH_KEY}" "${SSH_PORT}"
}

bootstrap_vm() {
    prepare_image
    prepare_key

    if vm_pid >/dev/null; then
        die "VM already running; stop it first"
    fi

    printf '%s\n' \
        'NetBSD bootstrap:' \
        '1. Wait for the login prompt and log in as root.' \
        "2. Run each command below separately. Public key:"
    printf '%s\n' 'mkdir -p /root/.ssh'
    printf 'echo \''%s\'' > /root/.ssh/authorized_keys\n' "$(<"${SSH_KEY}.pub")"
    printf '%s\n' \
        'chmod 700 /root/.ssh' \
        'chmod 600 /root/.ssh/authorized_keys' \
        'echo PermitRootLogin yes >> /etc/ssh/sshd_config' \
        'echo PubkeyAuthentication yes >> /etc/ssh/sshd_config' \
        '/etc/rc.d/sshd restart' \
        'exit' \
        '3. Stop QEMU with Ctrl-A, then X.' \
        '4. This script starts the same overlay headlessly and verifies SSH.'

    "${QEMU_BIN}" \
        "${QEMU_ARGS[@]}" \
        -display curses \
        -serial none \
        -monitor none

    start_vm
}

ssh_vm() {
    prepare_key
    ssh \
        -i "${SSH_KEY}" \
        -o IdentitiesOnly=yes \
        -o StrictHostKeyChecking=no \
        -o UserKnownHostsFile=/dev/null \
        -p "${SSH_PORT}" \
        root@127.0.0.1 \
        "$@"
}

stop_vm() {
    local pid
    if ! pid="$(vm_pid)"; then
        printf 'NetBSD VM not running.\n'
        return 0
    fi

    kill "${pid}"
    for _ in {1..20}; do
        kill -0 "${pid}" 2>/dev/null || break
        sleep 1
    done
    if kill -0 "${pid}" 2>/dev/null; then
        die "VM did not stop; kill it manually: ${pid}"
    fi
    rm -f "${PID_FILE}"
    printf 'NetBSD VM stopped.\n'
}

usage() {
    printf '%s\n' \
        'usage: misc/qemu_netbsd.sh bootstrap' \
        '       misc/qemu_netbsd.sh start' \
        '       misc/qemu_netbsd.sh ssh [command]' \
        '       misc/qemu_netbsd.sh stop' \
        '' \
        'bootstrap: one-time curses console setup; creates local VM SSH key' \
        'start:     detached headless VM; waits until key SSH works' \
        'ssh:       connect to VM as root' \
        'stop:      stop detached VM'
}

require_commands
case "${1:-}" in
    bootstrap) bootstrap_vm ;;
    start) start_vm ;;
    ssh) shift; ssh_vm "$@" ;;
    stop) stop_vm ;;
    *) usage; exit 2 ;;
esac
