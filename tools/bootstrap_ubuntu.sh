#!/usr/bin/env bash
# System installation is explicit; no PMU, governor, capability or mitigation changes.
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
if [[ "${1:-}" != "--install" ]]; then
  cat <<'USAGE'
Usage: bash tools/bootstrap_ubuntu.sh --install
Installs Ubuntu build, diagnostic, perf and target tools through sudo apt-get,
then the repository's locked Python dependencies into .venv. Requires sudo.
Does not change perf_event_paranoid, capabilities or other host policy.
USAGE
  exit 0
fi
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build clang llvm lld \
  libclang-rt-dev python3-venv python3-pip gdb valgrind binutils time \
  linux-tools-common "linux-tools-$(uname -r)" \
  gcc-arm-none-eabi binutils-arm-none-eabi qemu-system-arm qemu-system-x86 \
  qemu-user gcc-multilib libc6-dev-i386 grub-pc-bin xorriso
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-lock.txt
.venv/bin/python -m cmastery doctor
