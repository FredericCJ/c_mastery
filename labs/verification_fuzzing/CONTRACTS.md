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


## 1: Bitvector arithmetic and complete bounds

Implement vf_add(a,b,out) for uint32_t: return1 and sum when representable, else0 with unchanged output. Write a CBMC harness for every input and a sentinel output, with overflow checks enabled; separately exhaust a reduced8-bit domain and explain why that enumeration is not a proof for32-bit implementation code.

Derive the pre-add inequality without executing overflowing signed arithmetic. Prove unchanged output on failure as a separate postcondition and identify pointer validity assumptions.

## 2: Loop invariants and binary-search termination

Implement vf_lower_bound on a sorted nondecreasing uint32_t array, returning the first index with value>=key or n. Use half-open[lo,hi), calculate midpoint without overflow and retain O(log n) comparisons. Empty input permits a null array. Establish sortedness as a precondition, not a fact proved by a search test.

Prove all indices below lo are<key, all at/above hi are>=key, and hi-lo strictly decreases. Derive memory bounds and handle duplicate values without changing the intended first-position contract.

## 3: Stateful property harness and ownership

Implement vf_ring_step for VFRing with positive capacity, head<capacity, size<=capacity. op0 push rejects full and increments size; op1 pop rejects empty, advances head modulo capacity and decrements size. Reject invalid state or operation without mutation. Build a sequence harness using a Python list oracle and generation from valid/invalid transition classes.

Prove size conservation and rejection atomicity at each step; define ownership of queued elements separately from the scalar ring indices. Explain why an oracle sharing the same count representation could reproduce the defect.

## 4: Parser fuzzing with semantic oracles

Implement vf_frame for bytes[length,payload...,xor]. Require n>=2, length<=16, exact n=length+2 and XOR of payload equal final byte; publish length only on success. Empty payload is valid with bytes[0,0]. Fuzz all inputs0..64 bytes, check round-trip canonical encoding on success and output sentinel on failure.

Separate memory-safety, grammar, integrity and canonicalization properties; bound fuzz input processing without excluding empty, malformed or huge harness inputs.

## 5: Counterexample minimization

Implement vf_shrink for an in-place byte vector of length at most65535 whose failure predicate is sum(bytes)>=threshold. Greedily remove each byte when the remaining sum still fails; retry the same position after removal. Return final length. If the original does not fail, leave it unchanged. The result is1-minimal under deletion, not globally shortest.

Prove predicate preservation after each deletion and justify termination. Distinguish1-minimality from globally optimal shrinking and discuss order-dependent outcomes.

## 6: Bounded schedule exploration

Implement vf_request for events0 submit,1 cancel,2 complete,3 release. Submit once; cancellation only after submit and before completion; complete once after submit whether or not cancelled; release only after completion and only once. Enumerate all event words through length6 and compare state/rejection to an independent transition relation.

State the safety invariant released implies completed and derive every reachable state. Explain which liveness claims require a scheduler/environment assumption beyond finite enumeration.

## 7: Deterministic fuzz harness composition

Implement vf_digest(frame,n,out): validate with vf_frame, then derive a32-bit rolling digest starting from0 for that input only, digest=digest*33+byte across payload bytes. Preserve output on invalid input. Repeated A,B,A calls must give the same results for A. Keep corpus generation and sanitizer runtime outside the target dependency graph.

Derive the required isolation between fuzz invocations, including allocator cleanup and thread joining. Distinguish deterministic input-derived randomness from wall-clock seeding.

## 8: Proof claim audit and independent transfer

Implement vf_claim(tool_success,unwinding_complete,assumptions_witnessed,unchecked_properties): accept a bounded-proof receipt only when the first three flags are exactly1 and unchecked count0. Use this predicate as a checklist, not a cryptographic verifier, and audit the saved CBMC/fuzz campaigns against actual logs and source hashes.

Bind the claim to source, compiler/solver configuration, assumptions, model bound and checked properties. Explain why local self-reported flags cannot establish reviewer independence or tool honesty.