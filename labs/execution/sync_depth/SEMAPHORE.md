# T51.02: permit conservation across deferred interrupt delivery

This required successor retains the three `PR-T51-02` leaf criteria and their
original seven-day changed-condition variants. Its families are semantic
reasoning, construction, boundaries, failure/recovery, falsification and transfer.
The target is an original teaching image in an owned QEMU `mps2-an386` instance;
no deployed system, physical flashing or host security change is authorized.

## Dependencies, supplied inputs and environment

Supply the completed `PR-T51-01` CMX implementation and generational wait
successor, with their original headers. `run_semaphore.py` requires explicit
CMX, wait and semaphore source/include arguments. The supplied `semaphore.c`
and `semaphore_irq.c` are unsolved; completed implementations and technical
analysis remain separate assessor material. Other inputs are the public tests,
original `PR-T51-02` fixture, target harness and original course linker script.
No library semaphore or scheduler may replace the mechanism. The core is C17
with no allocation or runtime library calls. The target port additionally
uses Armv7-M instructions, NVIC/SCB registers and declared compiler extensions.
Original course memory helper sources are explicit target dependencies;
hosted verification tools are not target dependencies.

## Representation, lifetime and operations

`CDSem` owns one stable `CWWorld` semaphore and borrows a live CMX kernel.
Initialize before use, with maximum>0, initial<=maximum and either FIFO or
priority policy. Every wait on this object must use `cds_take`; every post must
use `cds_post`. Every participant, token and control object remains live under
the wait successor's generation/lifetime contract. All core operations share
external exclusion. Output objects are live, distinct from control/input
storage and are not concurrently accessed. Resetting live state is forbidden.

`cds_take` uses CMX's ordinary deadline contract: an available credit may be
consumed immediately, including at an expired deadline; otherwise an already
due deadline returns TIMEOUT without enrollment or output publication. Use
deadline=now for this declared nonblocking try operation. No permit carries
mutex ownership: any permitted producer may post, and posting does not prove
that the producer owns a protected resource.

`cds_post` either wakes exactly one pending waiter or increases the free count.
It never schedules, blocks, or allocates. It reports eligible participants,
array probes, whether a waiter woke, and whether a scheduler request should be
deferred. A failed post preserves semaphore/task/output state. At maximum,
return FULL; do not wrap or silently coalesce counted events. Binary maximum1
uses the same explicit refusal rule, which differs from a remembered/coalesced
event bit. Shared finite diagnostic counters also refuse before overflow.

For priority admission choose the most urgent effective priority, then the
oldest original wait stamp. The reference policy adapter exchanges two ordering
stamps only while under exclusion, invokes the actual CMX completion transition,
and restores both original stamps. Derive why this changes the declared queue
policy without replacing completion/generation ownership. A callback, interrupt
or observer that bypasses the common exclusion invalidates this argument.
Priority ordering can starve a low-priority waiter; token conservation does not
establish fairness. FIFO also needs scheduling assumptions for eventual execution.

`cds_close` wakes pending waiters with CLOSED and prevents further posts. Free
credits remain represented in the closed object until its storage is retired;
they do not disappear from the ledger. Already terminal tokens retain results.

## Deferred-credit representation and failure policy

`CDDeferred` is a bounded mailbox, independent of the semaphore. Under a saved
interrupt mask, offer increments pending/accepted or returns FULL unchanged.
It does not inspect CMX objects. A lower-priority drain makes at most its stated
budget of posts, retiring a credit only after a successful post. If the target
semaphore is full or closed, pending credits remain owned by the mailbox;
record the partial drained count and arrange another drain after capacity
becomes available. Do not busy-loop or silently discard them. Explicit mailbox
close cancels the remainder and refuses later offers.

Prove two ledgers using mathematical integers: initial+successful posts equals
successful acquisitions+free credits, and accepted deferred offers equals
drained+cancelled+pending. The latter bounds counter increments without assuming
that an arbitrary corrupted mailbox is valid. `volatile` is not thread
synchronization: this mailbox is for the declared single-core interrupt exclusion
model, not unsynchronized ISO C threads or a multiprocessor queue.

## Exact criterion experiments and independent expectations

For `T51.02.a`, start maximum2/initial1. Two waits acquire one permit and enroll
one waiter. Four posts produce one direct handoff, two free permits and one
overflow refusal: initial1+accepted3=acquired2+free2. Add a timeout immediately
before a post; the freed credit belongs to the semaphore, not the timed-out
operation. Contrast binary saturation, then test UINT_MAX as an actual capacity.

For `T51.02.b`, construct the C17 core over the shared wait protocol and the
separate target wrapper. Verify due-deadline try, wrong context, saved-mask
nesting and no scheduler call from posting. At every refusal check outputs as
well as count. Retire the final credits on explicit cancellation/close; a
full deferred mailbox is a visible admission failure, not successful delivery.

For `T51.02.c`, execute all24 orders of two individually identified posts,
timeout and close against a separate test ledger. Check state after every
transition. A duplicate-credit-after-handoff control must fail immediately.
Repeat priority admission with0..16 participants, recording16 array probes each
time (272 in this selected sweep). Explain this fixed-bound scan cost and the
additional scans performed by the imported CMX/wait components. Do not infer
constant cost when changing the configured bound. Reenroll an urgent task100
times while the low-priority task stays waiting; show why a finite trace is a
starvation counterexample prefix, not an infinite-run proof.

## Actual interrupt changed condition

Configure IRQ0 at raw priority0x80 and PendSV at0xff. The kernel-call policy
permits the former nonblocking wrapper and defers scheduling to PendSV. The
wrapper saves/restores PRIMASK exactly. Then reconfigure IRQ0 to priority0,
which is too urgent for this declared kernel-call policy. Its attempted direct
post must be refused without mutation; it may only offer to the independent
mailbox and pend the deferred handler. Keep BASEPRI at0x80 while generating three
distinct urgent interrupts: two offers fit, the third returns FULL, and PendSV
remains pending. Restoring BASEPRI allows one PendSV invocation to drain both
credits and perform the actual logical CMX scheduler selection. Confirm IPSR16
in IRQ0 and IPSR14 in PendSV. NMI/HardFault never access these controls.

The threshold is an explicit course port policy, not an ISO C or universal RTOS
rule. Lower numeric interrupt priorities are more urgent; BASEPRI masks the
selected and less urgent levels, while PRIMASK excludes configurable exceptions.
Selected CMSIS-Core register/NVIC passages and the actual core_cm4.h SHPR/ICSR
layout support the register choices. Pending exceptions do not count all events;
the mailbox retains the event count independently of coalesced PendSV state.

## Verification, artifacts and rubric

Build reference, unsolved semaphore, unsolved wait and unsolved CMX imports
separately. A scaffold must compile and fail the intended semantic assertion.
Controls also duplicate handoff credit, wrap a full counter, ignore priority,
schedule from a post, acknowledge a closed post, lose deferred credit, restore
the wrong mask, or schedule in the forbidden urgent handler. Require matching
first-divergence arguments, not just failed exit codes. Compiler failure, crash,
timeout and sanitizer output do not count as semantic controls.

Deliver implementations, ownership/state diagrams, mathematical ledgers,
pre-execution predictions, exact traces, independent expectations, scan counts,
source/toolchain hashes and target map/undefined-symbol/disassembly evidence.
Require zero undeclared target helpers or archives. The target guest's synthetic
exceptions and logical CMX task states are actual emulator observations; they
do not establish physical IRQ latency, a new context-switch implementation or
peripheral behavior.

Each original criterion requires its own evidence. Level0 lacks a coherent
contract; level1 produces an ordinary happy-path demonstration. For `.a`, level2
reconciles every token destination and level3 transfers the timeout-before-post
argument. For `.b`, level2 proves refusal/ownership/interrupt boundaries and
level3 independently designs the urgent-to-deferred port. For `.c`, level2
checks each finite transition and kills the named defects; level3 changes the
wait policy, derives fairness limits and the changed resource model. Retain
the exact original delayed variant identifiers and seven-day requirement.

## Hints and limitations

1. A completed waiter and a free permit are mutually exclusive destinations.
2. A timeout removes a pending claim; a later independent post still owns a token.
3. Keep mailbox acceptance separate from semaphore delivery and schedule only
   from the declared deferred context.

Current execution status belongs to the scoped receipts, not this authored
contract. External review, unaided delayed learner attempts and physical
observations remain separate. Finite traces, sanitizers and emulator results
cannot prove universal progress, all interleavings or absolute mastery.
