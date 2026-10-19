# T48 successor laboratory

This package adds the exact PR-T48-02 through PR-T48-14 tasks and their original
minimum-seven-day variants. The separate `boot/` package covers PR-T48-01.
Read the original project, its fixture and `criteria.json`, then `CONTRACTS.md`.
The reference implementation and assessor derivations are deliberately separate.
Public `contracts/contracts.c`, `device/device.c` and `scaffold/kernel.c` are
incomplete. Passing a supplied probe is author or learner evidence only for the
specific mechanism and inputs it exercises.

The language baseline is C17. Hosted models use the host C library; target code
uses owned byte routines and explicit providers. GCC/Clang, Python, binutils,
GRUB, xorriso and QEMU are build/observation tools, not target runtime dependencies.
The actual target is QEMU10.2.1 `pc-i440fx-10.2`, `qemu64`, one TCG CPU,128MiB.
Only new ordinary4MiB image files are writable; no networking, passthrough, KVM,
host policy changes, mounting or privileged setup is required.

From a copied package preserving the exported `labs/` paths:

```sh
python3 labs/kernel_followthrough/run.py contracts --build /tmp/my-kf-contracts
python3 labs/kernel_followthrough/run.py boundaries --build /tmp/my-kf-boundaries
python3 labs/kernel_followthrough/run.py device --build /tmp/my-kf-device
python3 labs/kernel_followthrough/run.py runtime --cvm /absolute/my/cvm.c --build /tmp/my-kf-runtime
python3 labs/kernel_followthrough/build_guest.py --build /tmp/my-kf-guest
```

The default host stubs compile and fail their semantic checks. The default guest
builds an intentionally incomplete image. Supply `--source` and, for contracts,
`--model` to the host runner. The guest builder accepts explicit `--kernel`,
`--boot`, `--interrupts`, `--model`, `--contracts` and `--device` sources; it never
searches for an assessor directory. `--run` gives an implemented image a bounded
owned TCG run. Keep output directories fresh. Reference access is a separate
assessor action, not a missing-dependency fallback.

Construct in this order: checked tags/reservations; frame and mapping ownership;
trap frame/context switching; isolated user ELF and copy boundaries; queue and
block ownership; console lifecycle; explicit VM adapter; durable service chain.
Before each run, predict state transitions and the first refusal. Keep raw
logs, input events, compiler commands, ELF/map/disassembly, source/image hashes
and cleanup. Retain failed attempts as failed. Do not promote a hosted model
pass to a guest-device claim, or a clean emulator restart to physical power loss.

For delayed assessment, an assessor chooses the unchanged original variant only
after at least7 days, withholds private implementation/analysis and records the
learner's independent prediction before revealing any reference output. A dated
author run cannot fill that learner gate. Physical CPU, DMA, timing and power-loss
claims require their own selected hardware and observations.
