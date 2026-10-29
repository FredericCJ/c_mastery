# T53.04: publication, interrupt delivery and completion

## 1. Identity and assessment families

This required successor supplies concrete author support for
`AX-PR-T53-04-T53.04.a`, `.b`, `.c` and their existing `-DELAYED`
variants. Families A/B/C/D/F/G/H/I/J apply through the distinct tasks below.
It supplements PR-T53-04; historical project identity, hours and receipts remain
unchanged. The original learner stubs remain unsolved.

## 2. Objectives and inspected sources

Criterion a separates sender ICR progress, arrival, EOI and a CPU/generation
application acknowledgement. Criterion b implements bounded handlers and
release/acquire publication, including an actual x2APIC register-port variant.
Criterion c distinguishes timeout retention from termination and measures the
handler and requested-work intervals separately.

Exact inspected passages and document hashes are registered in
`research/v5/expansion/legacy_smp_interrupts/resources.json` and
`source_access.json`. The selected Intel SDM is the historical September 2016
253668-060US revision: §§10.5.4, 10.6.1, 10.8.1 and 10.12.1–3/10.12.9.
It is not a claim to have read current errata or every APIC mode. C atomics and
ISA device ordering are separate contracts. No unavailable C17 text is presented
as inspected; the C11 draft atomic passages are explicitly identified.

## 3. Prerequisites and learner artifacts

Supply completed PR-T53-02 startup and PR-T53-03 private table/stack analyses,
including a safe permanent-stack handoff and normalized interrupt frame. Supply
the completed retained `kernel_smp` prerequisite explicitly. First explain why
all interrupt-visible objects must remain live through delayed completion and
why EOI does not release a software request's ownership.

The supplied guest composer imports only the explicitly named completed startup
and kernel folders. It does not find or copy assessor implementations. Its
known-hook checks fail if a prerequisite's source structure changes. Port that
adapter deliberately when using a different correct learner implementation.

## 4. Environment, edition and dependencies

The portable core is ISO C17 with `<stdint.h>`, `<stddef.h>` and `<stdatomic.h>`.
It uses fixed storage, no allocation or runtime I/O. Require lock-free 64-bit
atomics for this selected interrupt target; generated code and the final link
must not import compiler atomic helpers. Hosted tests may use libc and pthreads;
those are verification tools, not target dependencies.

The target is the original CMK guest on isolated QEMU TCG, 1/2/4 logical CPUs,
including sparse IDs 0/1/2/4. xAPIC uses `qemu64`; x2APIC uses `max` after
actual CPUID checking. Each changed guest compiler builds the adapted C and
assembly; the retained GCC-specific prerequisite still uses GCC and GNU as.
Target C is general-register-only, freestanding and has no red zone. This is
not a userspace signal handler, a physical board or an asynchronous task scheduler.

## 5. Problem, inputs and outputs

Implement `interrupts.h` in the supplied core stub. A `struct si_box` has one serialized
producer and exactly one destination consumer. It contains one outstanding batch
of one to four copied 32-bit work values and a nonzero 64-bit generation. There
are no borrowed payloads. Different boxes identify different destination CPUs.
Input arrays and ticket outputs must be live and disjoint from box storage.

`si_init` requires a live, exclusively owned box not currently shared. It creates
generation zero and an empty, acknowledged state. `si_publish` validates arguments,
refuses `SI_BUSY` while any prior generation is unacknowledged, refuses `SI_FULL`
after generation UINT64_MAX, copies the batch and release-publishes the next
generation. It changes the output ticket only on success. Errors leave the box
and output unchanged. No arbitrary reinitialization may retire a live request.

`si_drain` is called only by the designated, non-reentrant consumer. A zero ticket
is invalid; a ticket different from the published generation is stale. An already
acknowledged current generation is duplicate. On success, invoke the nonblocking,
non-reentrant callback once per value, in order, then release-publish the
acknowledgement. The callback must return normally, perform no box operations and
not release box or context storage. Consumer and callback lifetime is explicit.
The implementation accesses no batch/context data after acknowledgement.

`si_status` and `si_timeout` are producer operations. A stale ticket cannot inspect
a new request. Timeout marks outstanding work but does not cancel it, release its
storage or permit reuse. Late completion returns `SI_OK` while the historical
`timed_out` flag stays set until a successful next publication. At the terminal
generation, completion and status remain legal; only new publication is refused.

`struct si_round` is an externally serialized, trusted acknowledgement collector for up
to four CPU indices. Initialize a nonzero generation and nonempty four-bit
required mask. Reject wrong generation and CPUs outside the required mask;
duplicate observations cannot advance the mask. Completion is exact equality of
completed and required masks. A caller may forward only an acknowledgement
obtained from the matching destination's acquire observation. This is not an
authentication protocol for attacker-supplied acknowledgements.

The concrete a trace is required mask 0b0011, generation 7, observations
(CPU0,g6), (CPU0,g7), (CPU0,g7), (CPU1,g7). Expected completed masks are 0,1,1,3;
results are STALE, OK, DUPLICATE, OK. Explain which event permits each transition.

## 6. Why this mechanism is necessary

An idle ICR means the sender has finished sending, not that the remote code has
finished. An interrupt count does not validate its payload. EOI releases the
controller's in-service state, not an application buffer. A single interrupt may
cover four queued values; repeated interrupts may see an already completed batch.
The publication protocol, rather than one-notification-per-work assumption,
determines exactly which work is eligible and when it may be overwritten.

## 7. Required reasoning and invariants

Derive `acknowledged <= published == sequence` at quiescent producer boundaries,
and the release/acquire chains for payload reads and reuse. Explain that sequence
and timeout are producer-owned, while the callback's output is consumer-owned
until acknowledgement. Atomicity alone is insufficient without those lifetime
and ownership premises. Prove at most four callbacks per drain and constant
storage/operation cost under this capacity. Identify the callback cost separately.

For the ISA adapter, draw the xAPIC ICR high/low write and delivery-status path,
and the x2APIC single 64-bit MSR 0x830 path. x2APIC has no delivery-status polling
bit. CPUID.1:ECX.21 precedes mode enable in IA32_APIC_BASE; MMIO is no longer the
register interface in that mode. Reserved bits remain zero. Put the explicit
device-ordering fence before notification; ISO C release/acquire does not define
an APIC MMIO/MSR transaction. EOI writes zero; spurious interrupts get no EOI.

## 8. Implementations and experiments

Implement the public C and guest/assembly stubs. Preserve all general registers
at the interrupt boundary and obey the ABI stack alignment. Program each CPU's
one-shot timer, then periodic timer, and stop with initial count zero. Record
CPUID feature bits and ARAT (2 means leaf unavailable); count 1000000 divided by
16 is an experimental input, not portable microseconds. ARAT says neither the
frequency nor a physical interrupt-latency bound. Stop does not erase an already
pending interrupt: allow the observed final queued periodic event and account
for it, then inspect current count zero.

Send generations 1–6 with one value each. Generation 7 carries values 7,8,9,10
under one publication; notify twice and show only four work callbacks. Generation
8 carries 100+CPU index. Each CPU must finish 11 values with sum 155+index and
acknowledgement 8. Record every CPU's timer/IPI/EOI counters and own identity.
On multi-CPU runs, hold the last CPU with interrupts disabled, let all peers
complete generation 8, declare a bounded timeout, and prove reuse is refused
without changing the retained batch. Then release it and observe late completion.

Inject early application acknowledgement and missing-EOI controls separately.
For the latter, confirm the target's own ISR and IRR bits for vector 241, prior
acknowledgement 1 and outstanding generation 2 before reporting the intended
diagnosis. A killed or hung VM is not that evidence.

Record fenced TSC intervals around handler C work and around the callback drain.
Report requested-work ticks and handler-minus-work ticks separately for each CPU.
These values include instrumentation/emulation and exclude assembly entry/exit;
they are not calibrated durations, worst-case bounds or universal overhead.

## 9. Verification and acceptance

Run `tests.c` under GCC/Clang debug, optimized and address/undefined sanitizers.
Use `trace.c` with `oracle.inputs()` and `oracle.check_model()` for all 4096
six-action histories (24576 transitions/profile). The independent Python model
does not import the assessor C implementation. Run `native.c` with pthreads:
four exclusive consumers process 80000 batches/320000 values. Include GCC and
Clang ThreadSanitizer profiles; clean stress is evidence under those executions,
not a C memory-model proof.

Build the learner guest explicitly, for example from the repository root:

```sh
python3 labs/smp_depth/interrupts/guest.py \
  --lab labs/kernel_smp --kernel COMPLETED_KERNEL \
  --startup-public labs/smp_depth/startup --startup-completed COMPLETED_STARTUP \
  --implementation COMPLETED_INTERRUPTS --work NEW_PRIVATE_BUILD --cpus 4 --mode 1
```

Require successful translation/link and zero undefined target symbols before
accepting execution evidence. Correct guest exit is 33 plus `SI_GUEST_PASS:`;
intended controls require exit35 and their exact `SI_GUEST_CHECK:` diagnosis.
Feature refusal is its own negative result. A crash, sanitizer finding, generic
marker or harness timeout cannot substitute. Check all CPU/generation values,
not merely the final line. The source-bound campaign and its negative guards
retain commands, raw logs and exact source identities.

## 10. Assessor rubric

Score each original criterion separately. Level0: incorrect ordering, target
containment violation or invented evidence. Level1: ordinary timer/interrupt
success without the criterion's ownership/counterexample reasoning. Level2:
a supplies the exact CPU/generation trace and coalescing invariant; b supplies
actual bounded handler/register/link evidence and feature/frequency limits;
c supplies distinct early-ack/missing-EOI outcomes, late completion and separate
measurement intervals. A defect in one cannot be compensated by another's score.
Level3 requires the corresponding named changed condition after seven days,
with fresh unaided reasoning and evidence under changed assumptions. Author
reference execution does not mark a learner assessed or independent.

## 11. Graduated hints and separate analysis

1. Draw send, arrival, EOI and application completion as four events.
2. Write down which agent can change each field and when a batch may be reused.
3. A duplicate interrupt may safely observe an already acknowledged generation;
   an old CPU acknowledgement cannot satisfy a different CPU's required bit.

The assessor solution and technical argument are withheld in
`assessor/legacy_smp/interrupts.md`; learner exports do not include it or completed
implementation files.

## 12. Named changed-condition variants

a-DELAYED: coalesce four requests into one batch notification; inject a duplicate
notification and retain each request's order and completion boundary.
b-DELAYED: change xAPIC to x2APIC only after exact feature/register inspection;
execute MSR timer/IPI/EOI operations and reject an unsupported CPU before access.
c-DELAYED: delay one CPU while peers finish, timeout without reuse, then release
and complete the retained generation. These are implemented author opportunities;
the actual delayed learner attempts remain unassessed.

## 13. Containment

Only original curriculum guest programs, synthetic copied integers and private
throwaway disks are used. No network or host-disk passthrough, deployed equipment,
credentials, host security changes or flashing. The early-ack and missing-EOI
mutants are deliberate isolated negative specimens. They do not demonstrate code
execution, privilege escalation or any claim about third-party targets.

## 14. Evidence status and limits

The maintained author campaign reports actual executed profiles independently
of learner progress. Current support binds its final receipt; earlier failed
attempts remain separate. No result is a finite proof of absolute mastery.
Physical timer calibration, current CPU errata, arbitrary APIC configurations,
general interrupt nesting and external human review remain outside this observed
scope. TSC-deadline was inspected for comparison but is not the selected executed
variant; x2APIC is. Hardware-capable feature bits alone are not execution evidence.
