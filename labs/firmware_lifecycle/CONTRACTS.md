# B06 common exercise and evidence contract

This contract applies to every PR-T57–PR-T64 project and its LAB unit. Each
project supplies the mechanism-specific inputs, API, invariant, boundary and
changed-condition assessment; these obligations supplement its public tests.

## Prerequisites and permitted dependencies

Use the accepted C17 implementation baseline. C11 draft clauses supply precisely
identified semantic reasoning; a draft is not a substitute claim that published
C17 was inspected. C23 comparison work must identify the actual changed clause
and compiler support; unchanged ABI or OS behavior is not an ISO C guarantee.
Before entry demonstrate checked extent arithmetic, object lifetime, byte
decoding, resource ownership and reproducible build capture with the declared
prerequisite artifacts. Reading a module does not accept an artifact.

The supplied portable cores use only C17 types and explicitly declared host
math operations. Implement the mechanism from these primitives: no allocator,
compiler, container, verification engine or protocol library may replace the
mechanism being taught. Host-side Python/CMake/CTest are verification tools.
Kernel headers, system calls and ISA assembly belong to separately named
adapters. A hosted state model does not demonstrate kernel scheduling, device
ordering, cache coherency or flash persistence. For each freestanding adapter
compile with -ffreestanding, inspect relocation and undefined-symbol tables,
linker maps and disassembly for compiler helpers; report every dependency and
provide or remove it. Merely disabling builtins does not prove runtime closure.

## Execution and containment

Run host tests in a disposable working directory using synthetic fixtures.
Kernel modules run only in a disposable VM with a snapshot, no host shared
directories, no bridged networking and no production data; do not insert them
into the workstation kernel. Firmware work uses the supplied model/emulator
first. Physical transfer is restricted to dedicated NUCLEO-F439ZI or
NUCLEO-H745ZI-Q development boards and requires a separate authorized session;
this authoring task does not flash devices or change security settings.
Security cases target only the original teaching arena/dispatcher. The attacker
controls the explicitly supplied bytes, handles or event schedule, not real
credentials. Harmless success is the named sentinel/property transition.
Crash, undefined behavior and sanitizer detection are different observations;
none alone demonstrates code execution or privilege escalation.

## Deliverables and verification

Retain (D1) a prediction before execution, pre/postconditions, representation,
invariants, ownership transitions, failure atomicity and cost derivation;
(D2) learner source and build identity; (D3) complete commands, raw outputs,
tool versions and independently derived expected results; (D4) a minimized
counterexample, repair and regression; (D5) a changed-condition implementation
and comparison; (D6) an evidence-limit statement. Use supplied fixture data,
then independently construct at least one input that distinguishes your repair
from the named plausible wrong implementation. Public finite checks cannot
establish absence of all defects. Sanitizers observe executions; bounded proof
depends on assumptions/unwinding; measurements depend on workload and machine.

## Assessment rubric and hints

Every objective has its own criterion in the project and authored unit. Score
0 for no assessable artifact or a contradiction of its contract; 1 for a
passing demonstration without a causal explanation; 2 for an original
implementation/analysis with a correct invariant, boundary argument and
reproducible evidence; 3 for an unaided delayed changed-condition solution
that identifies which previous proof or measurements no longer apply.
Require all objectives at level2 before technical completion and level3 on the
specified transfer task after at least seven days before independent-transfer
completion. Record whether the attempt was self-reported or externally
reviewed. No fabricated reviewer independence, mastery certificate or absolute
mastery claim is permitted. Hints are graduated in each brief; consulting the
assessor solution changes the attempt to assisted practice.

## Validation states

Canonical writing means authored; a compiling scaffold means executable; a
source-bound observed run means executed. These do not imply learner acceptance,
external assessment, VM execution or physical validation. Each family records
remaining target evidence and each campaign reports exact commands. A model
pass cannot satisfy a missing mandatory adapter investigation. New descriptive
hours are separate from accepted baseline hours and impose no learner-time cap.


## 1: Flash programming as a constrained state change

Implement lc_program(dst,src,n) on live disjoint octet spans: reject if any new byte requests a0-to-1 transition, leaving all bytes unchanged; otherwise program with bitwise AND. n0 succeeds. This is an atomic admission model, not a claim that physical multi-byte programming is atomic.

Prove admission catches every forbidden bit transition and prevents a partially modified prefix. Derive physical program-unit alignment and erase-sector dependencies separately.

## 2: Integrity without authenticity

Implement lc_crc using reflected CRC-32 polynomial0xedb88320, initial state0xffffffff and final complement, processing bytes low-bit first. No CRC library. Verify empty input and123456789, then construct an altered payload with a recomputed checksum and explain why it still lacks authorization.

Derive one byte update by polynomial division and state the initialization/reflection/finalization conventions. Explain collision possibility and absence of a secret authentication key.

## 3: Authenticated image admission

Implement lc_admit(length,slot_capacity,version,floor,signature_ok): accept only length1..capacity, version>=floor and signature_ok exactly1. The verifier verdict is injected by the harness; it is not derived from a learner-controlled flag in a real image. Return0 for all invalid requests before any slot selection.

Specify exactly which bytes and metadata the trusted verifier authenticates; derive bounded extent arithmetic and the immutable authority for the rollback floor.

## 4: Trial boot, confirmation and rollback

Implement lc_select on LCBoot with active and trial slots0/1, confirmed and attempted booleans. If confirmed select active. If unconfirmed and not attempted, select trial and persist attempted1. On later unconfirmed reset select active. Reject invalid fields before output. The model treats attempted persistence as one atomic record; later projects remove that assumption.

Prove repeated unconfirmed resets eventually return to a known confirmed slot. Identify the exact persistence point before transferring control and the risk if it occurs afterward.

## 5: Redundant journal and torn commit records

Implement lc_journal_select over two LCJournal records. A valid record has commit0xa5 and check==seq XOR value XOR0x5a5a5a5a. Select the valid record with greatest nonwrapping seq; equal valid seq with unequal values is ambiguous and rejects. Return selected index through output only on success. Sequence exhaustion requires a migration policy, not wraparound.

Enumerate crashes after each field write and before/after marker programming. State why the XOR check is a synthetic torn-write control, not an authentication mechanism or robust error code.

## 6: Monotonic security floor

Implement lc_raise_floor(current,candidate,confirmed,out): require confirmed exactly1 and candidate>=current; publish candidate on success. Reject rollback or unconfirmed advancement transactionally. Compose it with boot confirmation and identify the trusted persistence needed in real firmware.

Derive the invariant floor never decreases and analyze the confirmation/floor ordering under reset. State the threat model for storage tampering and what a RAM model cannot enforce.

## 7: Dual-core update quiescence

Implement lc_peer_ready(local_epoch,ack_epoch,outstanding) accepting only equal epochs and zero outstanding transfers. Use it in a two-core shutdown model: close admission, drain DMA/IPC owners, obtain an epoch-specific acknowledgement, then permit update. A stale acknowledgement must never unlock a new generation.

Prove no old-generation peer can retain writable storage at update commit. Separate cache visibility, notification delivery and logical acknowledgement.

## 8: Crash capture and recovery evidence

Implement lc_capture_valid(words,n) for exactly4 uint32_t words [reason,pc,check,commit], check=reason XOR pc XOR0x9e3779b9 and commit0xc0de. Reject wrong count, invalid check or missing marker. Build a minimal firmware crash record producer that writes commit last and never allocates, then cut the model after every store.

Define record validity, persistence assumptions, reset-reason interpretation and ownership of storage during recovery; corrupted PC values must never become executable addresses.