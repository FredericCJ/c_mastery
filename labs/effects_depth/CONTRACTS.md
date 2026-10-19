# Effects observations and bounded replacement components

This additive laboratory implements the ordinary and changed-condition tasks of
PR-T13-01 through PR-T13-04. The original project profiles, leaf criteria and
seven-day delayed variants remain required. Host measurements here are author
observations, not a learner's delayed assessment. The assessor implementations
are excluded from learner workspaces.

## Environment and dependency boundary

Use C17 with GCC and Clang on Linux. `effects.c` is a first-principles component:
only the public header's exact integer and size types are permitted; no libc
allocator, parser or sorter may implement its mechanism. Its allocator and
attempt callbacks are explicit dependencies. The logger additionally permits
ISO C atomics; target-generated atomics and retry behavior require a separate
audit. Host tests may use libc, POSIX threads, tracing and a reference `qsort`.
Those host facilities do not become target dependencies.

`process_test.c`, `locks_test.c`, `state_test.c` and `highwater_test.c` are
explicit Linux/POSIX adapters. GNU linker wrapping, ELF shared objects,
`__libc_malloc`, and GCC stack-usage files are implementation interfaces, not ISO
C guarantees. ARM object studies do not execute an interrupt handler or prove a
device latency. Runtime prefixes are private; do not replace the system loader,
libc, locale or security configuration.

## Common object and failure rules

All objects must be live, suitably aligned and initialized. A count admits null
data only when it is zero. Output/control objects and input ranges are distinct
unless an operation explicitly says otherwise. Storage callbacks return at
least the requested bytes or null; returned storage remains live until release.
Callbacks must not reenter the same component, retain a caller's temporary
buffer, mutate its control object, or access storage after release. External
serialization is required for cache, pool and parser instances. Independent
instances may run independently only when their dependencies do too.

Errors preserve outputs and logical state except the declared diagnostic
counters/failure latch. Caller-supplied output pointers cannot be validated for
arbitrary lifetime or write permission by ISO C. Do not execute invalid-pointer
tests. Cloneable C structs do not supply unique ownership; copying a live owner
or a borrowed view is outside this contract. Generation checks diagnose valid
live handles, not dereferences through dead owner pointers.

## Cache and pool

Initialize `EFCache` with generation 1, no storage/borrows/failure and a valid
allocator. `ef_checksum` sums at most 32 unsigned bytes; its maximum is 8160.
`ef_cached` admits that same input and result but acquires 32 bytes lazily and
retains them. Every call copies only the admitted bytes; the unused suffix is
not initialized or exposed as input. Output equality therefore does not imply
equal allocation, retention or first-use behavior.

An allocation failure records `failed=1`, returns `EF_NOMEM`, and publishes
nothing. With `permanent=0`, the next call retries; with `permanent=1`, later
calls return `EF_FAILED` until reset. Success clears the latch. A borrow grants
read-only access only to the most recent admitted prefix, whose length the
caller retains. While any borrow exists, writes and reset return `EF_BUSY`.
Returns decrement the count; no return is valid without a matching borrow.
Reset releases storage exactly once, clears failure, and increments generation.
At maximum generation reset returns `EF_FULL` unchanged. The owner is quiescent
before destruction. Allocation counters in the test backend measure requests,
acquisitions, releases and live bytes separately.

`EFPool` starts zeroed with 16 one-byte slots. Each admitted take increments
`requests`, then obtains the lowest free index or returns `EF_FULL`. A full
request still counts; a saturated request counter refuses before mutation.
Returning requires an allocated index below 16. This allocator can produce a
request and a new live object without acquiring memory from the OS or calling
`malloc`. It does not automatically grow; the refill variant must disclose any
backend acquisition separately.

## Parser state and bounded callbacks

Initialize `EFParser` with cursor 0, selected seed, generation 1 and no borrow.
`ef_parse` accepts at most 32 bytes, uppercases ASCII lowercase bytes, advances
the cursor by the admitted length, and returns one borrowed view. Any outstanding
view makes same-instance parse/reset busy. Independent instances preserve each
other's output. `ef_view_release` checks the live owner's generation and clears
the view. Reset requires quiescence and a nonmaximal generation, clears cursor
and scratch, sets the seed, and increments generation.

`ef_random` requires a live parser and advances the specified 32-bit linear
congruential recurrence. It is reproducible per instance and explicitly not a
cryptographic generator. Resetting a seed does not reset clocks, locales,
external callback state or input streams; record such inputs for replay.

`ef_attempt` uses 16 automatic bytes and accepts 0–16 attempts. Each callback
receives that buffer and returns positive success, zero retry or negative
failure. `used` counts completed callbacks. Exhaustion returns `EF_INCOMPLETE`;
zero attempts perform no callback. The declared `nonblocking` backend capability
is mandatory. A count limit cannot interrupt a callback that blocks forever.
No claim of total bounded elapsed time follows from bounded iteration alone.

## Sorting and logger

`ef_sort` sorts 0–16 integers in nondecreasing order by stable insertion, without
subtraction in comparisons. `ef_sort_records` moves key, identity and all 32
payload bytes together and preserves original order among equal keys. Invalid
length preserves the array. Both use constant auxiliary storage and at most
120 key comparisons/shifts for length 16. The record form uses one temporary
record; code size, compiler frames and copy traffic are separate costs.

Initialize `EFLog.owned` with `ATOMIC_FLAG_INIT`. A valid call attempts ownership
once; busy returns 0 without changing the log, success copies at most 32 bytes,
publishes length, releases and returns 1; invalid input returns -1. Readers must
obtain the same flag. No software spinning or libc is allowed in this component.
ISO `atomic_flag` guarantees do not make every machine sequence wait-free or
bound interrupt response. ARMv6-M may require an external atomic helper and
ARMv7-M may emit a load-exclusive/store-exclusive retry. Either result must be
reported before selecting an ISR backend. The host threaded run is not a nested
interrupt experiment.

## Verification and deliverables

Run `python3 tools/run_effects_followthrough.py` in the repository under Linux,
with the explicitly provisioned source/runtime prefix. It compiles and runs
positive controls, three semantic mutants and the learner stubs in six profiles;
preserves command output, exact pins and raw timing samples; traces original
child programs; compares tagged runtime/configurations; and audits generated
objects. A nonzero control must contain its named assertion, not merely fail to
compile or crash. Never weaken expected failures to accept an environment error.

Submit the leaf's effect/call graph, its original and changed-condition analysis,
source/object evidence, exact commands and measurements. The existing leaf
rubric remains the independent assessor's criterion. In particular, explain
every unresolved callback, runtime-internal lock, dynamic frame and unavailable
physical observation. A successful observed path does not close an unknown
callee or prove every possible execution.

## Security and containment

All data, shared objects and binaries are original teaching artifacts. The
static-scratch flaw demonstrates a harmless overwritten byte; it makes no code
execution claim. The lock cycle is a finite wait-for graph, not a deliberate
host deadlock. Thread tests join all workers. The stack experiment allocates its
own guarded mapping and reads it only after joining. The source inspection and
object audits require no hardware flashing or privileged settings.
