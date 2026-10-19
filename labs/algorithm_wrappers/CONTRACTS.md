# Typed wrappers, live borrows and transactional backing: PR-T36-08

This required successor supplies the six original `T36.02.a/b/c` and
`T36.10.a/b/c` criteria and their unchanged `AX-PR-T36-08-...-V1` tasks. Keep the
original objective-specific rubric, prerequisite identities and seven-day delay.
Historical hours do not cap this work. Families A/B/C/D/F/G/H/I/J are applicable:
the mechanism is a typed interface with real storage and ownership, not a
renaming of an existing container. Original fixture data remains canonical.

## Prerequisites and environment

Import the completed T14 `CDTree`, `CDRing` and `CDQueue` artifacts, and explain
the T18 ownership/view contract in `labs/api_depth/contracts/OWNERSHIP.md`.
Do not substitute a newly written queue/tree or an external container library.
The ID membership index uses the actual AVL tree with multiplicity disabled.
The rank-then-ID view is a distinct layer: an ID-ordered tree is not itself
rank-ordered. Its index-based internal edges allow a staged tree copy; no
interior pointer in that producer needs rebasing. Identify the exact imported
implementations and their failure/identity contracts before composing them.

Use the active ISO C17 Profile C baseline, actual typed caller storage, and the
selected Linux System V x86-64 and Cortex-M4 AAPCS32 targets. These targets have
eight-bit bytes, 32-bit `int`/`int32_t`, and an eight-byte `AWRecord` with fields
at offsets0/4. Assert those assumptions; they are not universal ISO layouts.
The Cortex image uses only original reset/semihost code and explicit byte
helpers for compiler-generated copies. Audit final unresolved symbols, ELF
segments, map and disassembly. Hosted printing, Python oracles and sanitizers
are verification tools, not target dependencies. N1570 is an inspected C11
draft, not final C17; exact published C17/C23 comparison remains a source gap.

## Problem and operations

Implement `wrappers.h` and the assembly `aw_lower` kernel. `AWRecord` contains
signed32 ID/rank values. No byte arena is cast to a Record array. `AWBank` has
real `AWRecord values[3]` and real `const AWRecord *pointers[3]`; only the
selected representation is active. Value insertion copies both fields. Borrow
insertion retains the caller's live Record pointer without owning the object.
Both interfaces produce the same observable sorted values for valid inputs.
Rank is ascending by default; ties always use ascending ID. The changed policy
reverses rank only. All signed extrema are valid: compare relations rather than
subtracting arbitrary signed operands.

`aw_init` accepts kind VALUE/BORROW, capacity0..3, generation limit1..UINT_MAX,
live context, dispatch FINITE/CALLBACK and complete backing operations. It
initializes fresh caller control, empty ID index and generation1; it does not
acquire a bank. Invalid configuration leaves output unchanged. Initialization
requires no old live owner or outstanding views at the same control address.
`aw_put` checks owner/context/borrow integrity, then nonnull input, duplicate ID,
capacity, generation headroom, comparator results, staged index update and
backing acquisition in that order. Thus duplicate7 at capacity3 is DUPLICATE;
new10 is FULL. An error does not grant permission to destroy a borrowed object.
`aw_remove` returns MISSING for an absent ID without acquiring storage.
`aw_reserve` admits capacity0..3; a request no greater than current capacity is
a successful no-op, otherwise it moves the live representation to a new bank.
Successful insert/remove/growth advances the generation; failures preserve the
owner, old bank and existing views. Destroy is infallible under valid callbacks,
releases its bank, clears retained object/context/callback pointers and marks
the still-live control inactive. A repeated destroy is INVALID.

Replay put(7,2), put(4,1), put(9,2), rejected put(10,0), rejected duplicate7,
remove4, destroy. Also reverse the order of the two rejected operations.
Publish [4:1,7:2,9:2], then [7:2,9:2]; neither rejected update changes it.
No caller Record is freed. Count bank leases/releases separately from object
ownership: returning preallocated storage is not freeing borrowed payloads.

## Ownership, validity and failure semantics

Owner control, context, callbacks, banks and borrowed objects remain live and
serialized throughout their required use. Distinct output/control/input
objects do not overlap; a leased bank is distinct from the retained bank and
all control/scratch objects. Value input need only remain live during insertion;
borrow input must remain live and unchanged until removal/destroy. There is no
concurrent mutation or asynchronous callback. These are caller premises, not
properties enforced by a C pointer declaration. Runtime checks cannot validate
arbitrary dangling pointers, forged extents or hidden callback effects.

The backing callback leases one of two preallocated, correctly typed banks.
It does not allocate, block, change the owner/context/payloads, retain extra
pointers, or reenter the owner. A null result means no new lease remains held;
any partial callback work must already have been rolled back. Release is
infallible and returns exactly the previously leased bank. On successful update,
fill the new representation, commit index/storage/count/revision, clear old
borrow pointers, then release old storage. The failure guarantee covers owner
and prior contents, not callback attempt counters or external logging. Returning
the already retained bank is rejected, but arbitrary dishonest callback storage
is not generally diagnosable. Destroy may clean up a still-live object whose
borrowed values/context have been changed; it does not dereference payloads.

Callback comparison receives explicit context and writes a signed result; a
nonzero callback status rejects the update before backing acquisition. The
specified comparator must implement the selected rank/ID policy. Check all
ordered pairs in the bounded candidate set, including self comparisons,
against that policy. This exposes a supplied inconsistent comparator; it does
not establish arbitrary callback purity or laws on all future objects. The
busy check diagnoses same-owner reentry in the isolated test. Valid callbacks
must still obey the contract. Changing comparison state is permitted through
`aw_set_context` only while empty; it records the context epoch/direction and
advances generation. A direct epoch/direction change while populated is
COMPARE. Restore the old context or destroy; do not silently reorder an owner.

`AWView` is a read-only index/generation/owner token, not a raw payload pointer.
`aw_read` checks still-live owner control, generation and index before reading
storage. A successful growth invalidates the token even when values and count
are unchanged; a failed growth and a no-op reserve retain it. Destroy makes it
stale while control is still alive. No use is permitted after actual control
lifetime ends. At the generation limit, mutation rejects EXHAUSTED rather than
wrapping and reviving an old token. Cached borrowed fields detect a mutation of
a still-live Record; restore them before further ordered operations, or destroy.
This diagnostic must never be tested by dereferencing freed storage.

## Representation, reasoning and costs

Draw three Record objects at byte offsets0,8,16: payload24 bytes, not total
owner size. Separately draw the actual integer ring and typed node-slot queue.
Their elements are `int`, not eight-byte Records. Replay put11,put22,take,
put33,put44,failed put55,take through both actual producers. Include every
physical backing slot, head/count, node live/next fields, and removed value.
Both end with33,44; the failure preserves22,33,44. Ring wrap differs from
slot reuse. The actual queue's first-free scan inspects1,2,1,3 slots on its
four admitted puts, total7; full rejection scans none. This is a source-derived
operation count checked against the emitted physical states, not a hardware
load count or measured latency. Traversal costs and retained six-node capacity
must not disappear from the comparison.

Derive lower-bound invariant `[0,lo)<query` and `[hi,n)>=query`, with signed
rank then signed ID order and `lo+(hi-lo)/2`. The native and Thumb kernels use
the selected stride8; empty count performs no element load. For borrowed mode,
project live values into a temporary real Record array before calling the same
kernel. The projection is not a retained copy that changes borrow ownership.
Explain call-preserved registers, argument registers, stack use, and why the
Thumb leaf's internally20-byte frame makes no call at a misaligned public
interface. ABI probes check selected callee-saved integer registers and SP.

Account separately for O(log n) index lookup/lower bound, O(n) movement,
O(n²) bounded callback validation, the full32-node index clone, two leased-bank
slots, stored validation snapshots and stack temporaries. Capacity is three;
this implementation is not an efficient unbounded generic container. Compare
actual optimized `aw_finite_compare` and `aw_callback_compare` symbol sizes and
disassembly under the same flags. One implements comparison; the other dispatches
to an additional implementation, so include that callee and indirect-call cost.
Do not equate the smaller dispatch stub with lower total cost. Finite policies
admit only two directions; adding an operation requires changing/rebuilding the
dispatch implementation, whereas callbacks permit a new implementation of the
same checked semantics. Neither C interface enforces borrow lifetime or purity.

## Deliverables and verification

- D1: exact object/offset/lifetime diagrams, seven queue states, cost model and
  original artifact bindings for all six objective IDs.
- D2: working wrappers, original native/Thumb assembly, staged backing protocol,
  callback and finite dispatch versions, ABI and final-link dependency audits.
- D3: failure snapshots, stale-view histories, copied-value/borrowed-mutation
  counterexample, nine comparator-failure positions and ten backing-failure
  paths covering both representations and five distinct updates.
- D4: independent mathematical lower-bound oracle and physical queue replay,
  raw source-bound commands/results, intended mutation diagnoses, and changed
  tasks with fresh delayed reasoning rather than replaying a reference answer.

Run the public test cases layout, queues, fixture, ownership, backing, contexts,
kernel. The kernel campaign emits7,560 distinct small query packets per native
profile plus signed-extrema checks. Python reconstructs order independently.
The fixture tests all four copy/borrow × callback/finite combinations. The
author campaign runs GCC/Clang debug, optimized and ASan/UBSan profiles and two
Cortex-M4 emulator builds; assembly loads themselves are not sanitizer-instrumented.
Require intended assertion failures for duplicate/full precedence, missing
generation invalidation, accidental retained-pointer value semantics and early
index publication before a failed lease. A crash, timeout or unrelated build
error is not a passing mutation control. Exported learner scaffolds must compile
without private files and reject until their actual mechanisms are implemented.

## Original changed-condition tasks

Each ID below retains its original minimum seven-day independent assessment.

- `AX-PR-T36-08-T36.02.a-V1`: replace a retained index with the generation view;
  enumerate insert/remove/reuse at limit3 and derive retirement/exhaustion cost.
- `AX-PR-T36-08-T36.02.b-V1`: replace stationary backing with two typed banks;
  exercise failed and successful growth while holding a view. Trace both banks.
- `AX-PR-T36-08-T36.02.c-V1`: use the read-only view above; explain and execute
  invalidation after owner mutation despite the absence of a write operation.
- `AX-PR-T36-08-T36.10.a-V1`: use finite dispatch for the same accepted policy;
  execute both forms and compare errors, extension work and actual code costs.
- `AX-PR-T36-08-T36.10.b-V1`: use a stateful context, reject populated epoch
  changes, then change to descending rank while empty and derive invalidation.
- `AX-PR-T36-08-T36.10.c-V1`: fail the bank acquisition at each of insert7,
  insert4, grow2→3, insert9 and remove4; prove rollback and retry exactly once.

## Rubric and graduated hints

For every original objective, level0 permits stale access, loses ownership or
uses an unimplemented prerequisite; level1 reproduces nominal outputs without
the relevant state argument; level2 derives and demonstrates its ordinary
mechanism and boundaries; level3 independently completes its named changed
task with explicit premises, counterexample and resource argument. Specifically
assess .02a representation versus logical identity; .02b both physical queues;
.02c live control versus borrowed payload lifetime; .10a real typed storage and
ABI integration; .10b copied/borrowed ownership and context; .10c admission
precedence and every failed-update snapshot. Shared output does not earn a
second objective unless that argument is explicitly assessed.

Hint1: write down the owner of each pointer and the moment a generation changes.
Hint2: stage the membership index and obtain a distinct bank before publication.
Hint3: make the old bank and a retained view observable at the failed-acquire
boundary; then compare the successful move. For ordering, separate ID membership
from rank order before deriving the assembly search loop. Assessor code and
technical analysis are withheld from learner exports.

## Security, validation and limitations

Only original local synthetic objects and dedicated emulated firmware are used.
The harmless failure observations are a stale-token rejection, unchanged state,
or a deliberately wrong value in an isolated mutant. No external targets, real
credentials, host policy changes or hardware flashing are involved. Borrow
diagnostics operate only on live objects. Do not describe a rejected token or
sanitizer result as arbitrary code execution or universal memory safety.

Author execution is recorded in the source-bound successor receipt; this sheet
does not assert an independent learner passed it. External review, elapsed
delayed assessment and physical observations remain separate. Finite tests,
instrumentation and ABI probes cover the listed conditions, not all C programs,
toolchains, callback behaviors, architectures or future changes.
