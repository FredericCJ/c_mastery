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


## 1: Vectored partial progress

Implement li_advance(lengths,count,position,bytes) for a valid vector cursor. Compute remaining extent with checked size_t arithmetic, reject excess progress transactionally, then advance across zero-length elements and normalize the terminal position to{count,0}. This is caller-side accounting, not a writev replacement.

Prove accepted progress plus remaining extent equals the original stream length, including zero spans and overflow. Distinguish kernel acceptance from remote delivery or persistence.

## 2: Edge readiness and fair draining

Implement li_ready_after(result,budget_exhausted) for a synthetic read result: positive progress retains readiness, -2 means EAGAIN and clears readiness,0 EOF clears readiness,-1 terminal error clears readiness. Return1 while progress remains, even when a dispatch budget was exhausted. Exercise the actual anonymous-pipe EPOLLET adapter and compare drain-to-EAGAIN with a local ready queue.

Distinguish kernel notification, locally remembered readiness and actual EAGAIN. Derive a fairness budget that does not lose readiness or spin forever after EOF.

## 3: Shared submission ring arithmetic

Implement li_slot(head,tail,capacity,out) for uint32_t monotonic counters: capacity is a power of two in1..2^30, unsigned tail-head must be<capacity, and output is tail&(capacity-1). Reject invalid/full states unchanged. Then map the arithmetic to raw io_uring offsets and acquire/release operations after feature probing.

Derive the bounded-distance interpretation under counter wrap. Separate single-producer ownership, ring-slot lifetime and payload-before-tail publication.

## 4: Generation-tagged completion ownership

Implement li_event on LIRequest with state0 idle,1 submitted,2 cancel-requested,3 completed,4 released. Events0 submit,1 request cancel,2 target completion(result),3 release. Require matching generation for every event. Completion is allowed from1/2 once; release only from3 once. Failed transitions preserve the object.

Prove each target completion is consumed once and cannot authorize release of another generation. State finite-generation exhaustion policy and metadata versus payload lifetime.

## 5: Cancellation acknowledgement versus target completion

Implement li_releasable(cancel_ack,target_terminal) accepting only boolean inputs and returning target_terminal. Build a two-CQE schedule harness that permutes cancellation completion, target completion and close; inspect actual io_uring cancellation result codes on the original temporary-file/pipe operations.

Name separate ownership of cancel request, target request and payload. Explain how success, not-found and already-completing cancellation results change scheduling but do not substitute for a target terminal event.

## 6: Deadlines, timer events and wraparound

Implement li_expired(start,now,duration,out) for a wrapping32-bit tick model with duration1..2^31-1 and elapsed distance known below2^31. Set out when unsigned now-start>=duration. Reject invalid duration without changing out. Use timerfd in the Linux adapter and keep real monotonic timestamps separate from modeled ticks.

State the half-range observation assumption and prove equality is expiration. Distinguish wall-clock corrections, monotonic elapsed time and deadline policy.

## 7: Durable replacement and crash cuts

Implement li_durable(write_done,file_synced,renamed,directory_synced) requiring all flags exactly1. Then write a local temporary-file replacement adapter: write fully, fsync the new file, rename within its temporary directory, fsync that directory, and report exactly which stage failed. Use a synthetic crash-cut ledger; do not claim real power-loss durability from a normal process exit.

Specify observable atomicity, persistence, filesystem assumptions and failure recovery. Account for open descriptors to the replaced old inode and permissions on the new object.

## 8: Zero-copy claims and retained buffers

Implement li_buffer_release(submitted,terminal,borrowers) returning true only when submitted/terminal are booleans, every submitted operation is terminal, and borrowers0. Use it to compare read/write, sendfile/splice and a mapped-buffer path over synthetic local files; measure bytes copied, syscalls, CPU time and retained memory under the same output contract.

Distinguish copy avoidance, kernel retention, DMA mapping and application borrowing. Derive memory-pressure/backpressure costs and what the selected syscall actually promises.