# Assessor transfer analysis

This file is private. The public contract states the problems and expected
observable properties; these notes explain the reference construction and
where additional learner reasoning is necessary.

## P1 storage and teardown

The allocated record array and node array receive their effective types through
typed stores. The public control never moves; relocation preserves slot metadata
and node indices, then replaces only the record base. The reference allocates the
replacement before freeing the original, and refuses pins, so failure can leave
the original state intact. Reading every initialized record on relocation is
valid even when its logical slot is unused. A retired slot stays retired.

Node publication follows pin acquisition. If node allocation fails after record
acquisition, release the unpublished record and preserve prior accepted nodes.
This consumes an internal generation: a learner claiming bitwise rollback must
either stage the state or change that claim. Two queue references to one record
need two pins. The caller's store identity is part of handle validity although
it is not encoded in the numeric handle; cross-owner validation is not provided.

Close has two gates: no outstanding pins, then trustworthy unregister completion.
The failed/closing object must stay alive. Cookies require an external lifetime
uniqueness rule, so reinitializing with a reused cookie while an old callback can
arrive violates the adapter contract. This is a programmer/registry obligation,
not ISO C enforcement. Mutant1 frees on a pending unregister; mutant2 leaks the
record whose node was refused. The tests discriminate these faults without UB.

The OwOwner comparison retains its original nonfailing, nonreentrant release
contract. Delayed unregister is handled **before** invoking destruction. A
wrapper that passes an asynchronous operation as the original release callback
cannot satisfy the inherited contract merely by returning void.

## P2 progress and dependencies

`er_transfer` supports a product of accepted count and status; it does not carry
the backend's stable cause itself. The adapter keeps that cause separately and
does not treat ER_OK with a short prefix as total completion. The reference
compares every step with CPWTransfer under the narrower CPW backend contract.
It does not claim equivalence for er_sink implementations that report positive
progress together with ER_IO; those need a richer CPW result.

The real pipe proves the libc adapter uses immediate errno and respects pending
progress. Its successful nine-byte write is not evidence that every pipe/socket
write completes atomically. The dependency images intentionally expose a resolved
malloc and undefined poll callback. Symbol closure alone cannot characterize
an arbitrary user callback's effects. LTO may change helper visibility, so inspect
the linked native image and the callback implementation, not an IR container.

## P3 parser, authentication and durable publication

The v2 optional timestamp changes both header length and CRC extent. Mutant5
retains the old extent and is rejected by a correct decoder; mutant6 clears the
pending message when a queue is full. Shifting begins after a rejected sync,
allowing an embedded valid frame to survive. The independent whole-buffer oracle
stops at a plausible incomplete candidate rather than scanning its payload as
new input; that distinction matters for matching incremental semantics.

In-place encoding shifts high addresses first. Its proof depends on payload
starting at offset0; arbitrary overlapping control or a message structure overlay
is not supported. Compile-time16/64 changes storage size; runtime limits merely
narrow admission. Consumer schedules are bounded fuzz input with deterministic
draining after the schedule, so failure is replayable and cannot be hidden as
an unbounded consumer stall.

Authentication is an explicitly imported primitive with a separately implemented
protocol. Mutant7 authenticates only payload, allowing a sequence change without
tag recomputation. Mutant8 advances the replay ledger before verification,
letting an invalid packet suppress future valid traffic. Both are harmless local
security-property violations. Header/domain/key-ID/epoch coverage and publication
ordering repair them. Passing one RFC vector plus these protocol tests does not
prove HMAC security or whole-program constant-time behavior. Volatile state reset
is deliberately shown to reopen replay; persistence and key management remain
different mechanisms.

The durable adapter transfers queue ownership to its held slot before invoking
storage. A failed or indeterminate commit retains that slot. Reopening and retrying
the same ID uses the actual retained receipt table, not a test-only Boolean.
Mutant9 marks durable after an error; mutant10 drops ownership before resolution.
The write/flush fault model admits both lost pending writes and persistence despite
an error; the only safe retry result is7, not14. The imported transaction has
bounded retention and fixed epochs, not exactly-once delivery in arbitrary networks.

## P4 derivation and research limits

The fixture is original course analysis under its explicit one-interferer model.
The monotone workload recurrence starts at C+B and counts the maximum interfering
releases in that candidate interval. A fixed point is a sufficient completion
bound under those assumptions; a deadline exceedance rejects the proposed budget.
The test's high-utilization case still converges190us, while jitter makes another
case exceed200us. This prevents replacing analysis with a vague utilization slogan.
The UART example fails long-run service-rate feasibility even when a finite burst
can be buffered. A full pipeline proof must account for both CPU and link service.

The Audsley1993 paper was located, but its public mirror returned403 and the
author page offered no download. No inspected-paper claim is made for the general
blocking/jitter theorem. Do not generalize this original bounded derivation to
arbitrary deadlines, self-suspending tasks or an unmeasured board. Board source
contracts and physical timing evidence remain separate; a successful emulator
cannot supply a physical interrupt-latency distribution.
