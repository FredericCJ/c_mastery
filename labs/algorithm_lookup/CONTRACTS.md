# Typed probing and byte-span composition: PR-T36-06

This required additive contract supplies the original `T36.08.a`, `.b`, `.c`
mechanisms and their unchanged `AX-PR-T36-06-T36.08.*-V1` variants. The original
project, units and objective-specific rubric remain binding. Historical hours
are descriptive; they do not cap this extension. The source-bound successor
records author implementation and execution separately from external review,
independent learner assessment and physical observation.

## Prerequisites and environment

Bring the completed T14 `CDHash` and T15 `ADMatcher` learner artifacts, their
headers, failure contracts and source hashes. Their public contracts are
`labs/containers_depth/CONTRACTS.md` and `labs/algorithm_depth/CONTRACTS.md`.
The author experiment links the actual existing implementations without edits.
Do not use a library hash table, search, string matcher or container to supply
the mechanism being assessed. `prerequisite_stubs.c` is only a link scaffold:
replace it with those completed learner artifacts, never with assessor sources.

Use ISO C17, Profile C, eight-bit bytes, exact32-bit unsigned integers and
32-bit `int`/`unsigned`. The selected ABIs are Linux System V x86-64 and AAPCS32
on Cortex-M4. Static assertions bind the actual `CDBucket` layout: twelve bytes,
key at0, value at4, state at8. These are target layout assumptions, not general
ISO C guarantees. Retain the public typed object; character storage with a cast
is not an alternative allocator. The Cortex image is strictly freestanding:
original reset/linker/semihost harness, no hosted runtime, and explicit byte
helpers only for compiler-lowered aggregate copying. Inspect generated code,
map, unresolved symbols, final program headers and both ABI register probes.
GCC/Clang, Python, hosted printing and sanitizers are host verification tools.
Exact published C17/C23 source text remains an access gap; no C11 draft is
relabeled C17 and no C23 relaxation is relied upon.

All pointers denote live objects of the declared type and adequate extent.
Inputs, outputs, control records and callbacks' external records are disjoint.
Null payload is permitted exactly when its length is zero. Output controls
are required even for zero output. Caller owns every object and serializes
access: no concurrent mutation, signal interference or callback reentry.
No function can portably prove that an arbitrary pointer denotes valid storage.
These lifetime and alias obligations are caller discipline, not a C type-system
or sanitizer guarantee. Keep initialized stream state/prefix tables intact.

## Problem and representations

Implement `lookup.h` and the two original assembly kernels. Use the supplied
`fixtures/algorithms/PR-T36-06.json` unchanged. `ALRequest` contains a key and
validated start index. `ALProbe` has found index (`-1` if missing), used count
and sixteen visited indices. The raw kernel receives actual `CDBucket` slots,
capacity1..16, request and trace. It visits the start, wraps one position at a
time, skips tombstones, stops at empty/found, and otherwise stops after capacity
visits. It reads the table and request only. It initializes index/count and
overwrites only the used trace prefix. Its inputs are trusted; call the checked
wrapper for untrusted metadata. No function allocates memory.

`al_lookup` additionally validates capacity, states, live/occupied totals,
duplicate keys and reachability under the table's actual hash policy. Every live
slot must be reachable before an empty slot from its key's start. This explicit
validation costs O(capacity²); distinguish it from the O(capacity) trusted kernel.
It stages the trace. Invalid metadata leaves value/trace unchanged; missing
publishes a bounded trace and preserves value; success publishes value/trace.
Unused checked trace entries are zero. The ordinary hash is key modulo capacity;
the completed provider's deliberately clustered mode starts at zero. Both must
match the actual provider's successful/missing result and probe count.

Explain why the key22 fixture visits6,7,0,1,2,3, why deleting a slot cannot make
it empty, and why a full/all-deleted table still terminates. These are worst-case
bounds; expected constant time needs distribution and load assumptions. The
inspected Open Data Structures example maintains substantial empty capacity;
its unbounded-until-empty loop is not the full-table contract of this laboratory.

`al_match` admits text0..64 and pattern0..16 as bytes, including zero and255.
Return all overlapping starting offsets. The empty pattern matches each of the
n+1 boundaries. A longer pattern has no matches. Count one comparison for each
tested byte pair, including the first mismatch in a candidate window. The
implementation may stage at most65 offsets; no terminators, `strlen`, `strstr`
or delegated comparison are allowed. On success write exactly the matches and
publish count plus `ALMatchWork {comparisons, required}`. Insufficient capacity,
or null destination with nonzero required output, returns `AL_SPACE`, publishes
required count/work and preserves every destination element. Invalid controls
or spans preserve all outputs. Thus this API deliberately differs from the
actual T15 provider, whose `AD_SPACE` preserves the count as well as data/state.

## Required reasoning and deliverables

- D1: derive the probe invariant (visited prefix, key exclusion, bound and wrap),
  actual layout/ABI permission, and the empty-versus-deleted counterexample.
- D2: original x86-64 and Thumb assembly, checked typed wrapper, all-overlap C
  matcher, exact imported provider identities, link audit and representation/
  ownership/failure tables. Explain every branch and effective address.
- D3: independently enumerate fixture paths and byte comparisons; retain actual
  emitted packets, canaries, rejected states, complete failure-output checks,
  counters and resource costs. Assess both normal and clustered provider modes.
- D4: implement the three original changed-condition tasks below, retain an
  unaided later attempt under the exact unchanged variant ID, and explain what
  changed in the correctness argument and resource/publication contract.

Prove termination without relying on an empty slot or a zero text byte. Before
computing `n-m`, establish `m<=n`; bound `n+1` and every stage/output access.
Derive O((n-m+1)m) comparison work for naive search, O(n+matches) output work for
empty matching, and the prefix algorithm's amortized bound from failure-link
decreases. Staging65 uint64 offsets costs520 bytes, plus control and the actual
copied prefix state; do not call this constant-storage streaming independent of
the declared maximum. Record `sizeof`, compiler stack use and code size for each
selected build if making a resource comparison. Counts alone are not timings.

## Changed-condition variants

`AX-PR-T36-06-T36.08.a-V1` (minimum seven days): implement a bounded quadratic
probe trace with `(start+i*i) mod capacity` for i0..capacity−1. Capacity is1..16,
so the arithmetic is representable. Construct an incomplete coverage case,
calculate its visited set, and explain why a bounded search can safely terminate
while still being incorrect for a table populated under a different probe rule.
Do not claim all quadratic schemes fail or that primeness alone settles every
coefficient/load policy. Invalid parameters leave the output unchanged.

`AX-PR-T36-06-T36.08.b-V1` (minimum seven days): `al_publish` replaces output
storage with a synchronous fallible callback. It first enumerates into private
bounded storage, then calls in increasing offset order. Return0 means one
accepted publication,1 failure,2 cancellation; any other nonzero value is failure.
A rejecting callback must have no publication effect for that invocation.
Previously accepted effects remain committed. Stop at the first nonzero return,
including cancellation; never call later callbacks. Publish attempted/accepted
counts and the full pre-enumeration comparison count even after early rejection.
Invalid arguments call nothing and preserve the report. The callback may use
its own live context but may not mutate the borrowed input, retain internal
addresses, reenter this invocation or trigger concurrent access. There is no
rollback of arbitrary side effects, asynchronous cancellation or OS cancellation
guarantee. Contrast this contract with failure-atomic array publication.

`AX-PR-T36-06-T36.08.c-V1` (minimum seven days): `ALStream` composes the actual
T15 `ad_match_init`/`ad_match_feed`, not a newly substituted prefix algorithm.
For nonempty patterns, stage a copy of that state and request a complete private
match list; admit destination capacity before committing state. For empty
patterns, publish the initial boundary once, then each consumed byte's ending
boundary. Empty later chunks publish nothing. Each chunk has0..64 bytes; the
total offset is uint64. Offset overflow, invalid controls or corrupted declared
state return before output publication. `AL_SPACE` publishes required count but
consumes no input and preserves stream/data. Retry with changed capacity must
produce the same offsets. This adaptation intentionally translates the prior
unchanged-count failure contract. Compare every split with whole-span independent
enumeration, including a match spanning the split and an overlapping next match.

## Verification, rubric and hints

Predict the fixture before running `tests.c`. For each selected ABI compile the
reference or learner implementation, actual prerequisites, test harness and
matching ABI probe. Public unsolved scaffolds must compile and fail a semantic
assertion, not merely fail to link. The author campaign uses GCC/Clang debug,
optimized and ASan/UBSan modes; the two Cortex builds execute in a dedicated
MPS2 emulator and have no unresolved or dynamic runtime dependencies. A Python
oracle independently enumerates945 whole-span pairs and4815 two-chunk splits
and checks the actual emitted results. It also checks the fixture's declared
expected data rather than trusting its title. Diagnose mutations that stop at a
tombstone, lose overlaps, suppress required count, ignore cancellation, or reset
the actual prefix after a hit. A mutation counts only when its intended assertion
fails. Assembly memory accesses are not automatically instrumented by sanitizers.

For `.a`, level2 requires exact fixture/full/deleted traces, termination and
layout/permission arguments; level3 independently constructs and explains the
quadratic coverage failure. For `.b`, level2 requires both ABI kernels, actual
typed/prior imports and all distinct failure-output contracts; level3 implements
and diagnoses partial callback publication/cancellation. For `.c`, level2 checks
actual paths/comparisons and unchanged surrounding data using independent
enumeration; level3 composes and explains streaming state and chunk retry under
changed splits. Merely printing the expected offsets is level1; unexplained
violations are level0. Exact per-leaf assessment and minimum delay are unchanged.

Hints, in order: first draw the admissible slot states and the first state that
can exclude a key; next separate result staging from result publication; finally
write the state/count/data triple on each failure path and compare it with the
actual prerequisite API. Record an unaided attempt before consulting a hint.
Assessor implementations, independent oracle and technical analysis remain
separate under `assessor/algorithm_lookup/`; they are not learner imports.

## Security and evidence limits

Only original local teaching targets, synthetic typed tables and byte spans are
used. Invalid native pointers are reasoning cases, never dereferenced as tests.
The harmless failure conditions are wrong/missing results, bounded extra probes,
unexpected callback effects or violated output contracts. No external service,
credentials, deployed firmware or host-security change is involved. These are
functional/security-property counterexamples, not proof of code execution.

`research/v5/expansion/legacy_algorithm_lookup/validation.json` records exact
author commands, inputs, outputs and observed failures; `exports.json` records
actual learner export/compilation. Passing finite tests is not independent
assessment, proof for all inputs, physical timing or absolute mastery. Preserve
historical receipts and any failed attempt. Remaining PR-T36-07/08 and T37
obligations stay required and separate.
