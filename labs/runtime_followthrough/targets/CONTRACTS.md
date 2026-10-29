# One image, several environments: PR-T46-11 successor

This required package extends the unchanged T46.11 criteria and their seven-day
variants. Families B/C/D/F/I/J assess three different boundaries: portable bytes,
environment-specific execution, and generated runtime closure. Retain the original
VM and its C17 API. T46.01/.02 decoding and verification, T46.03/.04 execution,
T46.09 budgets, T08–11 linking/startup and the T48 user ABI are prerequisites.
Submit your original core, a frozen CVM1 image, prior budget predictions and a
revision-bound dossier. The source compiler stays hosted. A target that prints
the expected answer without executing that core fails.

The source register is `research/v5/expansion/legacy_vm_targets/resources.json`.
Read its named passages and the actual `cvm.h`, `SPEC.md`, `probe.c`, startup and
linker script. These are primary implementation contracts, not ISO promises.
The C17 baseline does not change for C23: no C23 feature is needed here. Exact
published ISO wording remains a separate access gap. The CPU vector layout,
semihosting, ELFv2 and the kernel system call ABI are target conventions.

## Representation and cost derivation

The frozen 19-byte program encodes one zero-argument function, entry zero, code
length eight: `INT 41; HOST 1; RET`. CVM1 multibyte fields are little endian,
irrespective of the host. Write the bytes by hand before using the supplied
fixture. Do not serialize a `struct cvm_program` or `struct cvm_value` C struct. C object padding,
pointer width and native integer order can change while the image stays fixed.

The shared public `labs/kernel_followthrough/runtime/probe.c` executes six cases
plus malformed-length and VM-recursion checks. It uses static VM/program storage,
one caller at a time, and an embedding callback. Derive status, output, callback
count, callback loop iterations and remaining fuel/host allowance for each:

| Case | Initial fuel | Host allowance | Callback policy |
|---|---:|---:|---|
| 0 | 32 | 1 | input + 1 |
| 1 | 32 | 0 | input + 1 |
| 2 | 32 | 1 | explicit failure |
| 3 | 1 | 1 | input + 1 |
| 4 | 2 | 1 | input + 1 |
| 5 | 32 | 1 | 256 bounded native loop iterations, then input + 1 |

Each fetched VM instruction consumes fuel before its operation. HOST consumes
its host allowance before calling native code. A failing run preserves the
caller's output sentinel -99; it does not roll back completed host effects.
Fuel limits VM steps, not native instruction count or wall time. Explain why
case 4 can call a service yet fail to return a value. The native bounded loop
adds work without another VM step. VM recursive calls consume explicit frame
slots; they are not recursive C calls. The image loader must reject the missing
last byte before any VM execution.

## A: shared core and changed endian/ABI

Use `run.py --cvm YOUR_CORE.c --build YOUR_BUILD` in the exported workspace.
It imports the supplied public probe, demands an explicit core, and supplies no
assessor fallback. The original learner core still intentionally fails.
Record the complete trace, byte sequence, program and VM object sizes, compiler
and flags. Then build a big-endian PowerPC64 ELFv2 image with Clang and LLD,
explicit original byte helpers, a separate Linux entry/syscall adapter and
QEMU user emulation. The relevant startup is supplied from
`labs/smp_depth/acpi/big_endian.S`; inspect its ELFv2 global-entry and stack
assumptions. This route depends on Linux syscalls, and is not bare-metal Cortex-M.
Check actual ELF endianness and symbols, execute it, and compare the portable
bytes and semantic fields. Explain any changed C struct size rather than forcing
the native layout to match. The assessor provides the exact command recipe in
the source-bound campaign; substitute your owned core explicitly.

## B: actual M4 and independent kernel environment

Build using `target_build.py` with explicit `--cvm`, `--probe`, `--runtime`,
`--driver`, `--headers` and `--build` arguments. The runtime argument must name
permitted original support routines, not a library archive. The driver implements
bounded diagnostic formatting/semihosting only. Run the final ELF using
`qemu-system-arm -M mps2-an386 -cpu cortex-m4 -display none -monitor none
-serial none -semihosting-config enable=on,target=native -kernel IMAGE.elf`.
This chosen profile reserves 256 KiB ROM, 64 KiB RAM and 32 KiB call stack, inside
the emulator's larger memory mapping. It is not an STM32 board claim.

Run the same bytes through the actual T48 ring3 boot-file user route: the kernel
loads the archive member, the VM verifies it and the embedding reports all six
rows. Preserve the hosted emitted image, archived member, actual user ELF,
kernel image, emulator command and serial output. Check their identities. The
current author provider is `tools/run_kernel_runtime_followthrough.py`; its
frozen receipts are distinct from a future run of your learner-built kernel.
Compare result/status/fuel/host fields across Linux, M4 and kernel. Kernel
scheduling/timer interrupts need not match a Cortex-M run with interrupts masked.
The one-page user-stack scratch adaptation is an explicit T48 successor, not a
claim that the original automatic verifier fits every stack.

For the changed-adapter task, `port_adapter.py` creates a registered public probe
successor changing only input+1 to input+2 and its normal-result check. Keep the
image and VM core byte-identical. Execute both adapters on Linux, BE and M4.
Predict which values change and which statuses, budgets and callback counts must
not. Reject any claim that bytecode verification enforces the embedding service's
meaning, latency, memory safety or authority.

## C: generated-code and startup audit

Inspect every generated object's imports, final undefined symbols, linker map,
disassembly, section ranges and stack records. Vectors contain 256 words; MSP
is 0x20010000 and all exception entries are Thumb addresses. The reset routine
masks interrupts, copies a nonempty initialized data section and clears a
nonempty BSS before entering C. With `--poison-bss`, the test startup first fills
BSS words with 0xa5, then clears them. With both `--poison-bss --skip-bss`, the
actual image must fail the startup check. Default-zero emulator memory alone is
not proof that a zeroing loop ran.

Reject unexplained imports even if dead-section collection might later remove
the offending function. Compile a separate 64-bit variable division specimen
for Cortex-M4; observe its actual helper requirement. Either implement the
specified helper under a separate reviewed contract or reject this dependency.
Do not silently add libgcc, compiler-rt, libc, libatomic, newlib runtime or a HAL.
Compiler-provided headers here are build inputs, not linked implementations.

Repeat O0/O3 with GCC and Clang, and GCC O3 LTO. The LTO link must use the GCC
driver with startup/default libraries disabled. Audit the post-LTO image and
generated stack records; pre-link LTO symbols are insufficient. The supported
link explicitly roots the original memcpy/memset implementations. An unrooted
negative build demonstrates a late emitted unresolved helper, separately from
the compiled implicit-division dependency control. No failed compilation is
counted as a successful semantic mutation test.

For the immutable bounded probe, derive a conservative C stack bound from live
generated functions, keeping file-qualified duplicate names. Sum live static
frames plus 1024 bytes of adapter/ABI allowance; justify absence of C recursion,
dynamic stack use and nested interrupts. Check it against the reserved 32768
bytes and verify static storage does not overlap that reserve. This conservative
sum is not an arbitrary callback bound, measured high-water mark or WCET.

## Deliverables, rubric and hints

Submit the hand-derived byte/cost table, owned core/provider identities, per-target
raw build/run evidence, object and final closure, startup/stack argument, the
rejected helper and BSS control, and changed-endian/adapter/LTO results. Explain
what each check can miss. The independent expected relation is the three-step
derivation above, not output copied from your implementation.

Each of T46.11.a/.b/.c is graded separately: 0 missing/incorrect; 1 nominal
transcript without a causal argument; 2 complete assigned environment and
counterexample evidence with correct byte/cost/closure reasoning; 3 independently
predicts and implements the corresponding changed-condition variant and diagnoses
a newly introduced failure. Passing a finite campaign does not certify complete
mastery. A build without execution, a hidden library, mixed image identities,
unexplained stack claim or false physical claim fails the affected criterion.

Hint 1: separate portable image identity from native object layout. Hint 2: mark
the exact point at which INT, HOST and RET spend their budgets. Hint 3: compare
pre-link imports with the post-LTO disassembly, and poison BSS before testing its
clear operation. Assessor reasoning and withheld seven-day variants are excluded
from learner exports. Actual independent attempts, external review and board
measurements remain pending.

All executable work targets original teaching images and local emulators, with
synthetic integers and private build directories. The helper control is never
linked as an accepted image. No deployed firmware, host security changes,
credentials or third-party targets are involved.
