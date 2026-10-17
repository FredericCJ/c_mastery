# B05 bounded mechanism contracts, revision 1

This original family isolates failure mechanisms in the accepted CVM, CST2,
CMK2 and CMX progression. C17, exact uint32_t, 8-bit octets, Linux x86-64 hosted
GCC/Clang. Mechanism code uses profile C primitives and standard type/limit
headers without mechanism libraries. Profile A host verification uses libc and
Python. These serialized models make no native atomic, kernel, MMU, cache,
device, WCET or power-failure guarantee. Strict freestanding ports require new
generated-code/link audits, including implicit division helpers; none is claimed.

All caller controls are live, initialized and correctly typed. Required pointers
are nonnull; arrays have stated extents; input, output and control storage are
disjoint. No concurrent mutation or reentry occurs. No dynamic allocation.
Nonzero status preserves every mutable object and output. Named B5 constants
are authoritative; UNIMPLEMENTED is never success. Enforcement is explicit
runtime checks inside valid native objects, with caller discipline for lifetime
and serialization. C23 adds no automatic page-permission, reclamation, or
asynchronous-device lifetime guarantee. The mathematical models are unchanged
by changing the C edition; separately justify any used language feature.

## Stack-height verifier

Accept 1..16 B5Instruction records and an unsigned maximum output. Instructions
are PUSH (+1), POP (-1), ADD (requires2, leaves1), JUMP (target only), BRANCH
(consumes1, target and fallthrough), HALT (requires exactly1). Entry is PC0 with
height0; maximum stack8. Validate opcode and jump/branch target in EVERY record,
including unreachable records, before reachable analysis. Every reachable join
must have a single consistent input height. Reject reachable fallthrough past
end, underflow and incompatible joins with CONFLICT; overflow with RANGE;
malformed structural input with INVALID. Publish maximum only after acceptance.
Unreachable structurally valid stack errors are not executed or height-checked.
A JUMP-to-self program is valid and nonterminating: structural verification does
not imply runtime type safety, initialized values, resource bounds or progress.
Worklist O(n+edges), fixed O(n) storage; compare with the accepted richer CVM.

## Original committed-prefix recovery

Input checkpoint_generation/value describes a trusted durable checkpoint;
0..8 records contain generation, value, XOR checksum and commit flag. Checksum
is generation XOR value XOR 0x93c72a5d; its weakness is intentional and must be
demonstrated. Scan a committed prefix, stopping at the first commit0 (ignore all
later tail bytes). Commit values other than0/1 before that boundary are INVALID.
Every committed record, including an already-checkpointed one, must have the
correct checksum; corruption is CONFLICT. Generations <= checkpoint are ignored
as old records; newer generations must be contiguous current+1 without wrap.
Return the final value only after all prefix records validate. O(n), O(1) space.

The fixture models commit visibility AFTER durable payload, then acknowledgement;
it does not implement physical flushing. A storage adapter that reorders or lies
about durability invalidates that premise. No checksum implies authentication;
XOR collisions permit undetected paired changes. Keep CST2's full-page redo and
indeterminate-outcome contracts when integrating; this single-value model does
not replace them. A failed recovery preserves the input failure image and output.

## Synthetic user copy

Four B5Page records each contain16 bytes and present/user/writable flags. The
synthetic address range is [0,64]; no host address or page table is accessed.
For count>0, input is valid readable storage, disjoint from all page records.
Check overflow/extent before arithmetic and all touched page permissions before
any destination write. Missing permission returns CONFLICT and leaves ALL pages
unchanged. Range error returns RANGE. Zero count at64 is legal; null input is
legal only for zero count. O(count), fixed storage. This preflight guarantee
requires stable mappings throughout: real kernels need pinning/locking/fault
recovery and must state partial-copy semantics. Arithmetic user-range acceptance
alone does not grant permission or prevent TOCTOU.

## Modular polling timer

arm sets last=now and remaining=delay, armed1 and expired=(delay==0). Delay must
be below 2^31 ticks. poll requires armed state, a nonnull output and an actual
elapsed interval strictly below 2^31. Unsigned modulo subtraction measures that
interval even across wrap. At/after remaining expiry latch expired, set remaining0;
otherwise subtract elapsed. Update last and return expired. An observed modular
delta >=2^31 returns RANGE unchanged; a full extra wrap is intrinsically
undetectable and outside the caller interval assumption. Expiry remains latched
until rearm. This is not a deadline queue or clock source. O(1), fixed storage.

## Fixed-priority response bound

own_cost>0, deadline in1..1000000, blocking uint32_t; up to4 higher-priority
tasks each cost/jitter<=1000000 and period>0. All values are declared integer time
units. Start R0=own+blocking; iterate Rnext=own+blocking+sum(ceil((R+jitter)/period)*cost).
Accept the least fixed point <=deadline; otherwise RANGE and unchanged output.
Use checked/wide intermediate arithmetic. No iteration may silently round down.
Model: independent sporadic higher-priority releases on a preemptive uniprocessor,
fixed priorities, declared release jitter and bounded lower-priority blocking;
all relevant overhead is already included exactly once. A failed sufficient
analysis is not an observed deadline miss. Observed maximum is not proven WCET.
Explain bound on iterations from monotonic integer R, and expose model changes.

## Reader-ticket grace period

Zero-initialize B5Grace. Four reader identities allow one active ticket each.
enter allocates a unique increasing nonzero uint32_t ticket, failing with RANGE
when identity is active or the counter exhausted. exit requires exact active
ticket. begin, with no period pending, snapshots active tickets. done waits only
while any captured ticket remains active; a new reader reusing the same slot
does not extend that period. done clears pending on completion; polling without
pending is INVALID. Storage for retired data must remain until the corresponding
period completes. One writer is serialized; no nested readers or counter wrap.
O(4) operations, fixed storage. Native C threads need atomics and publication
ordering; these ordinary fields are NOT thread-safe RCU. The model teaches the
pre-existing-reader rule without claiming an implementation of Linux RCU.

## Asynchronous request cancellation

Initialize B5Request {1,IDLE,0,0}. Every operation carries exact generation and a
result0(success),1(error) or2(cancelled); only COMPLETE publishes result.
SUBMIT from IDLE resets count/result and enters INFLIGHT. CANCEL from INFLIGHT
or CANCEL_REQUESTED is idempotent, enters CANCEL_REQUESTED, and does not release
resources. Only trusted COMPLETE, supplied after the adapter established device
quiescence, enters QUIESCENT and increments terminal_count exactly once.
Cancellation may lose to successful completion. REAP requires QUIESCENT, then
increments generation and returns IDLE, or retires permanently at UINT32_MAX.
Stale/duplicate/out-of-state operations return CONFLICT unchanged. Descriptor,
buffer and wait-record ownership persists until complete+reap. O(1), fixed storage.
An event named COMPLETE is not evidence a real device stopped; adapter correctness
is a separate obligation. This model neither issues I/O nor resets hardware.

## Evidence and reuse

Change only src/depth.c in a learner workspace. Keep tests, headers and contracts
trusted; separate assessor implementations/mutants from publication. Importing
accepted CVM/CST2/CMK2/CMX code requires an explicit adapter with compatible
contracts, source revision and ownership mapping; these isolated functions do
not automatically link older artifacts. Submit proofs, counterexamples, raw
commands, source hashes, failure traces and delayed transfer. Finite tests and
same-author mathematical oracles are neither independent assessment nor mastery.
