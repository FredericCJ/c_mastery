# FW-BOOT: authenticated paired selection and Cortex-M handoff

This required continuation retains PR-T59-02/03/04 and their exact .a/.b/.c
objectives and delayed-transfer identities. Use the original B06 common
contract, SIGNATURE.md and FLASH_MAINTENANCE.md. The hosted OpenSSL experiment
is preserved; the additional target profile below does not change its history.

## Profile, prerequisites and supplied artifacts

Prerequisites are the learner's extent/key/version admission, FWPeer lifecycle,
FHAppend/FHRecord, two-sector recovery and startup range-copy implementations.
Supply those artifacts with their evidence identities. The assessed mechanism
is image parsing, policy, ownership, paired commit and architectural handoff;
Ed25519 field/group arithmetic is not claimed as implemented from first
principles here. A separate cryptographic-internals assessment cannot claim
coverage from this adapter.

The target-provider profile explicitly permits pinned Monocypher4.0.3 Ed25519
and BLAKE2b as external cryptographic primitives. Use its optional Ed25519
module: the default EdDSA/BLAKE2b signature API is a different algorithm. The
provider source lives in a private tool prefix and is not a learner solution.
The runner checks exact source hashes and license; the learner implements
all boot_pair parsing, policy, failure and boot_handoff code from the supplied
stubs. C17 core code has no allocator, libc or operating-system dependency.
boot_runtime.c is an explicit original byte-store memset primitive needed by
the provider's generated code; link no other runtime silently. Host signing
and diagnostics do not become target dependencies by accident.

The actual CPU laboratory is QEMU MPS2 AN386 Cortex-M4 / AN500 Cortex-M7 with
an original synthetic image. Selected STM32 cross-builds use the F439 and
H745 M7 flash map with a128KiB admitted stack region; no board is flashed.
QEMU is not an STM32 flash/cache/clock/dual-core model. Every assembly/MMIO,
linker-address and function-entry claim is an explicit target convention,
not ISO C behavior. Exact Cortex-M4 MPU/VTOR rules use inspected PM0214rev10;
M7 maintenance and the distinct H745 memory system retain separate source
qualification and physical obligations.

## Concrete input format and operations

Implement boot_pair.h. An immutable image envelope contains a32-byte header
followed by514..2048 payload bytes. LE32 magic at0 is0x31423646; byte4 key ID1/2;
byte5 core0/1; bytes6..7 and24..31 must be0; LE32 fields at8/12/16/20 are
version, joint generation, payload load address and payload length. The first
two payload words are the initial stack and Thumb reset-entry addresses; the
first512bytes are reserved for the vector table. The signature is exactly64
live disjoint bytes, established by the outer transport before this API call.
An array parameter is not a runtime length check. Supply a complete bounded
readable envelope; never point this parser into faulting or unowned MMIO.

FBPolicy is trusted configuration: two32-byte keys, presence/revocation masks,
per-core version floors and nonzero generation floor. FBLayout is independently
trusted: base/limit and stack low/high. Reject unknown/missing/revoked keys,
wrong core, reserved bits, truncated/oversized extent, version/generation below
floor, base mismatch, unaligned VTOR, stack not8-aligned or outside(low,high],
non-Thumb entry, and entry outside[base+512,base+extent). Only exact provider
success0 admits; this differs from the hosted OpenSSL bridge's success1.
Authenticate the entire header and payload. Output is unchanged on rejection.

FBImage stores admitted scalar fields and BLAKE2b-256 digest of that exact
envelope. It is an ordinary copyable C record, not an unforgeable capability.
The trusted boot coordinator must retain immutable image ownership and pass
only validated records; C cannot enforce that discipline. Firmware payloads
must already reside in their separate protected target slots and match the
verified snapshot. Copying authentic bytes into a temporary buffer does not
authorize different bytes at the eventual execution address.

fb_joint_record requires both admitted cores with equal generation matching
the live FWPeer, both health acknowledgments, confirmation, no failed peer and
no outstanding/reserved operation. It composes the actual FH record builder;
fb_record_matches checks every stored digest, vector, version and generation.
Use fm_recycle for actual register-model erase/program/reclamation and keep
the old pair in distinct slots. One healthy core cannot commit a failed peer.
The record CRC detects modeled corruption; trusted keys and external rollback
policy provide separate authority. Journal replay is not prevented by CRC.

## Startup and handoff obligations

Construct original vector/startup/linker artifacts. Copy initialized data and
zero BSS within linker-bounded ranges; retain vectors and assert stack/image
budgets. Execute signature/policy checks before selecting the candidate.
At handoff, the caller is privileged thread mode on MSP, with peripherals/DMA
stopped, no active exception, pending NMI, outstanding lazy FP frame or foreign
stack/interrupt owner. These are preconditions requiring platform-specific
shutdown; PRIMASK alone does not establish them. This is a controlled handoff,
not hardware reset or restoration of every peripheral register.

Disable maskable interrupts and SysTick, disable all previously configured MPU
regions, then install a2MiB read-only executable code region and a selected
128KiB read/write execute-never stack/data region. Preserve explicit privileged
background-map policy. DSB/ISB delimit effective changes. Write admitted VTOR,
set CONTROL0, load admitted MSP and branch to the admitted Thumb entry without
returning through a C stack frame after MSP changes. This does not sandbox a
privileged application, protect DMA, or authorize other core startup.

## Deliverables and acceptance

D1: derive each header bound, authority provenance, invariant and failure
matrix; explain why signed bytes, current policy and live peer health differ.
D2: implement both stubs; bind source, compiler, provider version/license and
actual helper closure. Explain the explicit memset implementation and inspect
its call sites; freestanding flags alone do not prove closure.
D3: run RFC8032 section7.1 TEST1, a corrupted vector, ordinary admissions,
608 individual header/payload mutations, signed invalid entry, revoked key,
floor changes, wrong core/extent, unchanged-output failures and failed-peer
publication. Expected result is an exact refusal, not merely a crash.
D4: compose two authenticated old/new pairs in separate slots with the real
FWPeer/FHRecord/FMRecycle implementations. For each of40 modeled cuts recover
the exact old or new pair and rebind its full digest/metadata. Preserve the old
pair when core1 fails after core0 reports healthy.
D5: build and run original Cortex-M emulator images, verify actual startup and
handoff registers, then run separate original images that attempt SRAM
execution and code writes. A contained MPU fault is the expected target-local
violation witness; an unrelated emulator crash is not a pass. Audit M4/M7
disassembly, linker map, section sizes, stack records and undefined symbols.
D6: retain raw commands and failure receipts; compare the hosted success1 and
target-provider success0 conventions and all target-versus-host limitations.

The assessor must reject fail-open signature, ignored Thumb-entry, and forged
peer-health mutants at named assertions, and reject the public stubs
semantically. Rubric level0: compile/ordinary signature only. Level1: successful
admission without authority/failure argument. Level2: D1–D6 with source-bound
positive/negative evidence and separate model/emulator/target claims. Level3:
independently design the original delayed key-revocation/joint-generation
variant after seven days and expose a changed assumption that invalidates a
previous proof. Hints: authenticate complete bytes before publishing metadata;
separate the three success conventions; then draw ownership at the MSP switch.
Withheld technical analysis lives under assessor/firmware_lifecycle/targets.

## Security, cost and validation limits

Only synthetic public teaching seeds, original images and disposable emulator
instances are used. The attacker controls envelope/signature bytes or one
modeled peer failure; harmless success is a mutant publishing unauthorized
synthetic metadata or an original invalid instruction/data access faulting
inside the emulator. No production signing key, deployed bootloader, external
service, host setting, hardware flashing or option-byte command is involved.

The signing key is deliberately present in emulator test code solely to create
original fixtures. A production verifier must not contain it. Provider4.0.3
fixes an older signing timing leak; its current documented comparison-result
timing caveat is recorded, and no constant-time, power-analysis or fault-
injection certification is claimed. The verification result is public here.
RFC vectors and common-provider signing tests are finite integration evidence,
not an independent cryptographic proof. Report stack/code costs and linear
input hashing separately from fixed-size group arithmetic and flash latency.

Actual commands/results are in boot_pair_validation.json. The current work
does not yet implement fault-contained reads of torn H745 ECC records, actual
STM32 reset/clock/peer startup or physical power-cut/retention qualification.
Those obligations stay required; Cortex-M emulator evidence cannot close them.
