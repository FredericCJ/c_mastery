# PR-T32-02: model sequences, reduction and correlated oracles

This is the required successor for the six original T32.04/.05 criteria and
their six `-DELAYED` variants. Preserve the original project, hours, fixtures,
unit and criterion identities. Use C17 profileC with a separately declared
hosted Python/POSIX verification harness. Implement the mechanism yourself.
Supply an actual completed PR-T32-01 `architecture.c` and header to `build.py`;
the author implementation calls that provider unchanged. Learner exports
contain unsolved sources and no private assessor implementation.

## Representation, input and ownership

`TmQueue` embeds the actual two-usable-slot/three-physical-slot queue and caller
storage, with a nonwrapping revision. Initialize fresh or fully quiescent
storage only: no old control pointer or borrowed view may survive reuse.
Controls stay at fixed addresses, are not copied as owners and are externally
serialized. Valid controls, inputs and outputs are live and disjoint.
Initialization starts revision1. Successful push/pop and nonempty clear change
the revision once; rejected operations and empty clear do not. Clear drains
the actual queue. At UINT64_MAX, mutation refuses unchanged; reads/borrows
remain legal. After all borrows are discarded the entire fixed-storage world
may be retired without mutation or allocation cleanup. There is no hidden
reference count or claim that C can check quiescence.

A borrow returns a const pointer into the actual first slot plus owner and
revision. It is valid only until the next successful mutation, owner retirement
or storage lifetime end. `tm_read` validates live owner/revision/current slot
before dereference and preserves output on refusal. It cannot validate a freed
owner, prohibit direct use of the raw pointer after invalidation or make a
concurrent read safe. Never execute expired-storage dereferences as tests.
The changed condition demonstrates logical stale views while the underlying
typed object remains live. Returning to the same ring slot does not revive an
old revision.

`TmPacket` is a copied record of at most4 uint16 values. Canonical wire is
opcode1, count0..4, then exactly that many big-endian two-byte values. The
external original contract therefore defines length=2+2*count. Error precedence
is invalid C API arguments, header shorter than2, wrong opcode, count>4,
truncated payload, trailing bytes. Every failure preserves output. Both
decoders stage output. One walks pair offsets; the other folds individual
bytes and arbitrates errors afterward. They share a specification and thus
are not fully independent merely because their code differs.

The deliberately permissive reference masks count bit7, admitting a
noncanonical alias. Compare only the reconciled common domain. For
`01 82 01 02 fe dc`, canonical COUNT and permissive successful [258,65244]
are both explained by their declared contracts; disagreement alone is not a
bug. The independent normative vector `01 02 01 02 fe dc` has exactly6 bytes
and values258,65244. `correlated.c` deliberately gives a cursor and byte fold
the same wrong little-endian helper, and two checks the same wrong2*count
length authority. Keep these small negative specimens flawed. Agreement513,
56574 and rejection of the valid six-byte packet are falsified by the explicit
external wire contract, not by majority vote between implementations.

`TmFir` stores one prior int32 input. Each input and prior state lies in
[-1000000,+1000000]; admitted block length is0..64. Inputs/output/control are
disjoint. The output is an int64 numerator in units of one half, so no floating
rounding or integer division hides the original exact rational relation.
Validate the complete block before publishing any state/output. For a sample
x, emit x+previous and then retain x. Optional saturation clamps the numerator
to[-4,+4], corresponding to outputs[-2,+2]. Empty chunks preserve state.
Scale only when the transformed inputs remain in range. Chunking preserves
results only with unchanged coefficients/arithmetic and carried history.

## Required experiments and independent expected results

Enumerate every length0..6 word on PUSH0/PUSH1/POP at capacity2. Before adding
clear, derive the new list transition: clear discards all pending values;
conservation now includes a discarded count. Enumerate the four-operation
alphabet too. After each prefix compare status, output, count and refusal
preservation against append/front-delete/list-clear semantics. Add32 explicit
64-step seeded histories. The exact generator is `lcg32-high16-v1`: update
state=(1664525*state+1013904223) modulo2^32, choose (state>>16) modulo4.
Store the generator identity, seed and complete event list, not seed alone.

The compiled unused-storage mutant changes a reserved byte on failed full
push while preserving all nominal pop values. Snapshot protection detects it;
an output-only list assertion does not. Separate output, count, protected
extent and borrow invalidation properties. Mutations may corrupt still-live
unused bytes only inside the original teaching object; no out-of-bounds access
is needed. At least one meaningful distinction must be justified explicitly.

Minimize seed7's actual failing history by deletion while retaining the exact
`storage-preservation` class. Keep original and minimized events, rejected
candidates, tool/source/binary hashes and first divergence. The result is
1-deletion-minimal under this predicate, not globally shortest by fiat. An
invalid input, crash, sanitizer failure or unrelated assertion is not the same
failure. `trace.c` exposes the classified observation; it is not a solution to
the queue extension.

For the timing changed condition, `schedule.c` creates two actual pthreads.
Lowercase a/b reads a shared atomic counter; uppercase A/B commits that
thread's increment. A mutex/condition variable enforces a supplied admitted
event order, rechecking the predicate after every wake. Each read must precede
its commit. The flawed policy stores its old snapshot+1; the repaired policy
uses atomic fetch-add. Both are data-race-free but only the latter composes
the increment atomically. Enumerate all6 valid schedules, compare with a
separate finite state model, and reduce `.a.b..A.B.` to `abAB` while preserving
lost-update and complete thread operations. Reordering to `aAbB` removes the
failure: event ordering is part of the reproducer. This is controlled scheduling,
not a claim to reproduce arbitrary native timing or weak-memory executions.
Unexpected thread creation/wait/join failures or a process timeout fail the
experiment, never count as semantic counterexamples. Use original local
processes and synthetic integers only; no host security changes.

For each decoder compare all count0..5, extent0..12 and opcode0/1 combinations
over an asymmetric deterministic byte pattern, with Python `int.from_bytes`
and separately declared error precedence. Explicitly test the noncanonical
alias outside the common domain, not as an unexplained differential failure.
For FIR enumerate all three-sample words in[-3,+3], all four chunk splits and
both saturation policies using Python Fraction convolution. The original
[1,2,3] yields numerators[1,3,5], doubled input [2,4,6] yields[2,6,10]. Under
saturation, both third outputs are2, whereas twice the first is4: unrestricted
scaling fails. Wrong initial history or reset-at-chunk mutations must be
diagnosed as changed premises, not silently tolerated.

## Assessment, deliverables and limits

| Original objective | Explicit assessed mechanism and changed condition |
|---|---|
| T32.04.a | Complete0..6 list histories plus versioned longer seeds; add clear and derive discarded-state transition before generation. |
| T32.04.b | Independent abstract/refusal/extent properties and unused-byte counterexample; pointer borrow/revision lifetime and same-slot reuse. |
| T32.04.c | Exact class-preserving minimized trace with binary/source identities; preserve actual two-thread event schedule during reduction. |
| T32.05.a | Two actual decoders, reconciled extent/endian/precedence; classify permissive noncanonical acceptance. |
| T32.05.b | Fraction-derived scaling and carried-history chunks; saturation-at2 counterexample with exact values. |
| T32.05.c | Actual shared wrong helper produces agreeing errors; independent six-byte authority defeats the shared length mistake. |

D1 is the transition/arithmetic/authority derivation. D2 is the implementation
and explicit imported producer, plus owned lifetime and failure contracts.
D3 is retained raw observations, independent oracle, source/tool identities,
mutation diagnosis and minimized replay. D4 is each exact changed condition
with the original7-day minimum and an unaided attempt recorded separately.
Level0 lacks the mechanism; level1 reproduces examples while confusing oracle
authority or state; level2 satisfies every named original property and
falsification; level3 transfers under changed assumptions and identifies why
previous evidence cannot be reused. Hints: draw abstract states; name the first
different publication; finally isolate which shared assumption escaped both
implementations. The separate assessor analysis is not exported.

Queue operations are O(1), clear O(capacity), byte decoding and FIR O(input),
all with fixed bounded storage. Python enumeration and minimization costs are
verification overhead; the native thread harness uses the OS runtime. No
unbounded-completeness, physical target, independent-human review or learner
mastery claim follows. Published C17 remains unavailable. This original packet
is not a third-party protocol; mathematical rational results do not establish
floating-point equivalence or physical DSP accuracy.
