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


## 1: Spatial bounds and a harmless overwrite witness

Implement md_write(arena,logical_capacity,offset,input,n) for an original32-byte arena. Reject capacity>31, offset>capacity or n>capacity-offset before any write; write only the admitted span. Byte31 is a sentinel outside the logical object. Compare with the supplied faulty teaching path that validates only the32-byte physical extent; attacker controls offset/n/data and harmless success is sentinel change.

Derive subtraction-form bounds without overflow and distinguish logical isolation from ISO C allocated-object bounds. State why a sentinel violation proves integrity failure only within the teaching model.

## 2: Allocation arithmetic and truncation

Implement md_extent(count,stride,limit,out) using uint32_t inputs and uint64_t arithmetic; accept only product<=limit and product<=UINT32_MAX, publish the product on success, otherwise preserve out. Compare against a teaching allocator that truncates product to16 bits before checking. Do not actually allocate attacker-sized memory.

Derive the allocation and subsequent access extents in the same value domain. Identify every narrowing conversion and the branch that incorrectly authorizes it.

## 3: Temporal identity and stale access

Implement md_read(arena,claimed_generation,index,out): require live==1, matching generation and index<16 before publishing a byte. Failed reads leave out unchanged. The original arena remains allocated, so the stale-handle witness is defined C behavior; attacker retains an old token across reuse and attempts to read a new synthetic occupant.

Prove a stale token cannot authorize current-incarnation access, state the trusted token origin and require retirement before generation wrap. Compare with an actual heap use-after-free sanitizer target without treating allocator reuse as guaranteed.

## 4: Owned arena views and authorization

Implement md_authorize(arena,principal,generation,begin,length): require live1, exact owner and generation, begin<=16 and length<=16-begin; return admission only, without exposing a pointer on failure. principal is supplied by a trusted harness channel, not copied from request bytes. Define the trusted boundary and compare with a forged-request owner field.

Specify representation, operations, ownership and failure semantics of a checked view. Identify exactly what runtime checks enforce and what trusted-caller discipline or external verification must guarantee.

## 5: Bounded control-flow corruption analysis

Implement md_dispatch(op,grant,sentinel) with op0 read-only no-op and op1 set sentinel77, allowed only when trusted grant==1. Reject unknown op and nonboolean grant before mutation. Analyze the supplied intentionally flawed dispatcher that checks only op range; attacker-controlled op1 without grant must change only the synthetic sentinel in the faulty build.

State attacker control, target-local effect and limits. Distinguish data-only unauthorized dispatch from demonstrated arbitrary code execution; analyze control-flow integrity separately from operation authorization.

## 6: Format-string authority and byte-preserving logging

Implement md_log_copy(input,n,output,capacity) copying exact bytes and appending NUL, requiring n<capacity and no overlap; failure leaves output unchanged. Percent bytes have no interpretation. Compare an original isolated printf(user_format) target using the fixed synthetic string%n and a supplied local integer pointer with printf("%s",user_text); the harmless witness is only that integer changing.

Identify which argument is interpreted as a format and prove the repaired path treats attacker bytes as data. Discuss embedded NUL, truncation policy and logging integrity separately.

## 7: Exactly-once release and error cleanup

Implement md_release(MDOwner) requiring owned exactly1, setting owned0 and incrementing releases once. Repeat release returns0 unchanged. Reject a release counter atUINT_MAX before state change. Build an allocation/failure schedule with caller, queue and callback owners and record every transfer.

Prove ownership conservation and exactly-one destructor call under every failure prefix. Explain why nulling one alias cannot revoke another copied raw pointer.

## 8: Observable work, timing and mitigation limits

Implement md_equal(a,b,n,steps) scanning all n bytes, accumulating XOR difference, reporting steps=n and returning equality. Empty spans compare equal without dereference. Compare with an intentionally early-exit mutant using synthetic secrets only. Inspect optimized code and measure distributions without treating equal loop counts as a universal constant-time guarantee.

Separate functional equality, source-level work count, emitted branches and measured timing. State memory-validity, compiler and CPU assumptions and residual cache/power/speculation channels.