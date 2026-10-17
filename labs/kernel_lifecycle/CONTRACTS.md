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


## 1: Failure-safe module initialization

Implement kl_unwind for an ownership mask whose bits0,1,2 mean buffer, workqueue and registered endpoint. Return the decimal release trace with resource numbers3,2,1 in reverse order; unknown bits return0. Derive the acquire/fail matrix, then apply it to the supplied teaching module init labels.

Show each owned bit has exactly one release and no release precedes its dependents. Relate O(3) model work to the number of real acquisitions, including a failure before endpoint publication.

## 2: User-copy progress and ABI admission

Implement kl_copy into a live disjoint destination with capacity cap. Reject n>cap with -1 before writes; otherwise copy min(n,fault_after) bytes and return the completed count. The model input is ordinary accessible memory; a separate guest adapter must use copy_from_user and preserve its actual return convention.

State ownership of unread input, modified prefix and untouched suffix. Explain why a partial-copy return cannot be reused as a negative errno and why cap admission precedes copying.

## 3: Reference acquisition before handoff

Implement kl_ref with op0 acquire and op1 release for a live KLRef initialized to {1,0}. Reject unknown operations, zero/dead acquisition and UINT_MAX overflow; reject repeated release. Last release sets dead exactly once. Failed transitions preserve the struct. Map each transition to kref_init/get/put and protected lookup in the VM adapter.

Associate every reference with a named owner; count conservation is initial plus successful acquires minus releases. Explain why a nonzero count cannot validate a pointer that may already be freed.

## 4: Context-sensitive locking

Implement kl_lock_allowed(context,kind,rt) for conservative teaching policy: context0 task,1 hardirq,2 softirq; kind0 mutex,1 spinlock_t,2 raw_spinlock_t. Mutex is task-only; raw lock is allowed in all three; ordinary spin lock is allowed in all non-RT contexts and only task context in this restricted RT exercise. Reject out-of-range fields. Defend why the policy is deliberately narrower than all Linux APIs.

Derive preemption, migration, interrupt and inter-CPU exclusion separately. A lockdep trace is evidence for exercised lock-order paths, not a proof of all schedules.

## 5: Work cancellation and terminal ownership

Implement kl_work for serialized state {queued,running,closed,released}. Events0 submit,1 begin,2 complete,3 close,4 release. Admission fails when closed or occupied; begin transfers queued to running; complete requires running; close is idempotent; release succeeds only once when closed and neither queued nor running. Rejected events preserve state.

Prove release implies quiescence and closed admission. Compare cancellation return values with work ownership; do not hold a lock required by the worker while synchronously draining it.

## 6: RCU read-side lifetime model

Implement kl_grace with events0 enter reader bit id0..7,1 leave,2 retire,3 reclaim. Enter rejects duplicate IDs and admission after retirement; leave requires membership. Retirement is idempotent. Reclaim succeeds only after retirement with no readers and clears retired. This fixed-reader model is intentionally stronger than general RCU and must be mapped critically to the real primitive.

Show removal prevents new lookup while preexisting readers retain storage lifetime. Identify how refcount acquisition extends ownership beyond a read-side section and why the section alone does not do so.

## 7: Versioned kernel control records

Implement kl_decode on octets [version=1,length=2,lo,hi]. Require exactly4 bytes, decode little-endian16 into unsigned output, and leave it unchanged on all rejection. Then design a fixed-width ioctl or read/write adapter without embedding host pointers, documenting compatibility behavior.

Demonstrate exact-length admission and byte-order independence; distinguish ABI versioning from ISO C object representation and kernel/user pointer validation.

## 8: Unload defense and integrated lifetime audit

Implement kl_can_unload(open_handles,pending_callbacks,closed) returning true only for closed==1 and both counts zero. Reject closed>1. Integrate the previous model with the teaching module: enumerate every close, completion and last-put ordering and provide a guest unload dossier showing why no callback can reach released state or unloaded text.

Construct a global ownership graph and identify the linearization point of shutdown. Give separate evidence for module pinning, object release, callback quiescence and bounded wait policy.