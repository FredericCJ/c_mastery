# CMK: a bounded x86-64 operating-system laboratory

This is a learner workspace with intentionally incomplete sources. The supplied
GRUB packaging, linker layouts, ABI declarations and hostile user programs are
infrastructure. Implement the boot trampoline, kernel and portable contract
functions yourself. Read one hint stage only after committing a failed attempt.
The separately selected assessor implementation is a review aid; opening it
changes an attempt from independent construction to worked-example study.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The initial build succeeds. The contract tests deliberately fail, and the guest
stops in its unimplemented bootstrap. To inspect instructor controls, explicitly
configure a different build with
`-DCMASTERY_ASSESSOR_ROOT=/absolute/path/to/assessor/kernel`.
The directory must be the kernel assessor itself, not its parent.

Hosted tests require a Linux GCC/Clang C17 compiler, CMake and Python 3. Guest
tests additionally require GNU ld, GRUB (`grub-file`, `grub-mkrescue`), xorriso
and QEMU x86. The exercised configuration is QEMU 10.2.1, machine
`pc-i440fx-10.2`, `qemu64`, TCG, one CPU and 128 MiB RAM. An absent guest
toolchain leaves guest objectives pending even if hosted tests pass. The guest
runner uses ordinary build-directory disk files; it never mounts or passes
through a host device. `build/guest/build.json`, map, ELF, archive and serial
logs identify the actual artifacts. CTest's `kernel_guest` performs write,
reboot/recovery and interactive-shell boots. These are finite acceptance
campaigns, not evidence that every kernel path has been exercised.

## Implementation boundaries

GRUB supplies boot loading and the Multiboot2 information structure. The small
32-bit trampoline creates supervisor identity mappings, enables long mode and
calls original 64-bit C. It is part of the x86-64 boot route, not another ISA
track. The original kernel owns serial I/O, GDT/TSS/IDT and normalized exception
frames, physical-frame allocation, separate user page tables, timer scheduling,
ELF64 loading, argument validation, process handles and modern virtio-blk.
The kernel is nonpreemptible; timer-driven preemption occurs in user mode.
User code and kernel are built without red-zone or floating-point/SIMD use.

The process window is `[0x40000000,0x40200000)`. Static ELF segments occupy the
first MiB; dynamically allocated pages begin at the next MiB. The upper 128 KiB
is user stack, with the preceding page unmapped as a guard. Each process has
an 8 KiB kernel stack. Six process slots, one thread each, are statically bounded.
The larger user stack accommodates the current caller-owned store's format
workspace and verifier scratch; it is a design choice to measure and improve,
not an asserted universal sufficient stack bound. Read generated stack reports
and inspect permitted call paths when changing either component.

The teaching scheduler admits processes at boot from the read-only CMKA1
archive, then cycles through runnable processes. Exit revokes the process handle
and reclaims its user frames after switching address space. The small shell
supports `help`, `echo TEXT`, and `exit`; it does not implement fork/exec, pipes
or general filesystem commands. The self-test's expected fault process proves
one supervisor-write rejection while peer workers continue. Other user faults
terminate their process and let peers continue; the self-test then fails if
the fault was unexpected. A kernel-mode exception stops the diagnostic kernel.

Virtio uses PCI modern capabilities, VERSION_1 and FLUSH, a split queue of eight
descriptors and one outstanding request. Polling has a finite iteration budget;
a timeout poisons the device and retains its DMA buffers. Poll counts are not
wall-time guarantees, and this synchronous, nonpreemptible driver can delay
other processes. No SMP, MSI-X, packed queues, IOMMU, hotplug, filesystem,
network rewrite, physical-device timing or general OS conformance is claimed.

## Three-family integration

Pass the following absolute paths explicitly while building (ordinary copied
learner workspaces do not search outside themselves):

```sh
export CMASTERY_RUNTIME_ROOT=/absolute/path/to/completed-runtime-workspace/src
export CMASTERY_RUNTIME_INCLUDE=/absolute/path/to/completed-runtime-workspace/include
export CMASTERY_RUNTIME_ADAPTERS=/absolute/path/to/completed-runtime-workspace/adapters
export CMASTERY_STORAGE_ROOT=/absolute/path/to/completed-storage-workspace/src
export CMASTERY_STORAGE_INCLUDE=/absolute/path/to/completed-storage-workspace/include
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build -R 'kernel_(guest|runtime|integration)' --output-on-failure
```

Use your completed T46 and T47 learner sources in these directories. With the
curriculum runner, `cmastery check . --dependency runtime=/absolute/path/to/completed-runtime-workspace
--dependency storage=/absolute/path/to/completed-storage-workspace` supplies the
same paths (run it from the kernel workspace). For author validation only, the
two `ROOT` paths may point to the separately distributed `assessor/runtime` and
`assessor/storage` directories; headers and adapters still come from the
corresponding public workspace directories. Assessor sources are not included
in the learner bundle.

The first two variables add an actual ring-3 VM program. All five also link the
VM/store into init. The store adapter owns 67 image pages starting at raw page
16; raw page 3 remains the block-driver test. Init alone owns the writable disk
handle. VM service 1 commits integer 42, then the test callback deliberately
loses the success reply and returns an error. A new QEMU boot opens/replays the
store and VM service 2 reads 42. The injected lost reply is a callback experiment;
it is not a claim that the emulated disk lost an acknowledged flush. Exhaustive
bounded crash/fault campaigns remain the store family's separate evidence.

The CMK ABI is in `include/cmk_abi.h`: `int 0x80`, RAX number/result,
RDI/RSI/RDX arguments, other general registers preserved. It provides bounded
console, allocation, boot-archive and block services. Block completion is
all-or-error; only a successful explicit flush closes the declared persistence
boundary. Rebuild the same portable VM/store cores; do not duplicate them inside
the kernel. Their public contracts and original objective credit remain owned
by T46 and T47.

## Evidence and transfer

Keep predictions, source/image identities, complete commands, raw serial output,
first violated invariant, independent cases, minimized defect, causal repair,
dependency audit and changed-condition results. Host parser tests are useful
before booting; they do not establish a working CPU privilege transition.
Revisit after seven days: change a malformed ELF extent, close/reuse a handle,
add a new user fault or tighten the process page budget. Classify what now needs
fresh evidence instead of treating a former green receipt as current mastery.
