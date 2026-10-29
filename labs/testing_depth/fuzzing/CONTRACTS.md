# PR-T32-03: bounded parser fuzzing and evidence-based triage

This required successor covers all three original T32.06 criteria and their
three named delayed variants. Retain the historical brief, fixture, hours,
unit IDs and original assessment text. The active baseline is ISO C17/profileB.
Host sanitizers, libFuzzer, Python and process controls are verification tools,
not target dependencies. The exact published C17 text remains inaccessible.
No existing parser library may replace the studied mechanism. Explicitly
import the completed actual PR-T32-02 decoder and PR-T32-01 queue sources using
`build.py`; the author reference imports those providers unchanged.

## Core inputs, output and state

`tf_parse(0,...)` retains opcode1,count0..4, exact2+2count bytes, big-endian u16
values and maximum input10 bytes. A copied `struct tf_packet` holds count, four values
and an absent optional tag. No pointer into input is retained. Successful
output is fully initialized; rejection preserves the entire caller output.
No static parser state or allocation exists. Parsing the same bytes after any
other case returns the same result. Caller input/output/control objects must
be live, correctly aligned and disjoint. A null output or invalid profile
returns INVALID. Oversized input returns LENGTH before input dereference,
including a null pointer with an oversized count. At an admitted size, a
nonzero length requires a live complete input span. Header/field/error order
is documented below; do not turn that C extent premise into a security claim.

The changed grammar is a separate profile1, not a silent relaxation of the
original: opcode2,count0..4, flag0 or1, count big-endian u16 values, and exactly
one tag byte when flag1. Its length is3+2count+flag and cap12. Validate cap,
arguments, complete header, opcode, count, flag and exact length in that order.
After validation, build the canonical original frame and call the actual
decoder; publish tag and record only after its success. Missing/extra tag and
unknown flags reject. Count0 is valid in both profiles. Work is O(admitted
count), at most4 decoded values; large inputs are rejected in constant work.

The stateful changed condition uses `struct tf_session`, initialized explicitly with
epoch0/inactive. Begin requires a strictly greater nonzero epoch and resets
next sequence and total. Apply requires that active epoch, the exact next
sequence0..65535, and a valid original packet. Stage parsing and checked sum
before publishing total, next and output. Malformed input, duplicate/out-of-order
sequence, wrong epoch and uint32 total overflow preserve all protected state.
After sequence65535 succeeds the session refuses further messages until a new
epoch begins. Epoch cannot wrap; at UINT32_MAX retire the entire quiescent
fixed-storage session. Calls are externally serialized; this is a synthetic
sequence protocol, not authentication or an unforgeable replay defense.
Reset occurs at a declared new session, not between messages merely to make
repeated input seem stateless. The original stateless parser remains separate.

## Corpus, oracle and actual campaign

Classify every row of `seeds.json` before execution. The original four cases
are valid pair, truncated pair, valid empty packet and excessive count, with
expected OK/LENGTH/OK/COUNT. Added seeds cover opcode, trailing bytes, maximal
values and both valid/invalid optional-field transitions. The independent
Python grammar computes results using byte extents and `int.from_bytes`;
the in-process semantic oracle is a separately written byte fold. Neither
coverage counters nor agreement of two implementations is the acceptance
oracle. The source-bound campaign retains every deterministic case and output.

Enumerate both profiles, opcode0..2, count0..5, flag0..2 and lengths0..16 over
a known byte pattern. Add2048 recorded insert/delete/XOR mutations with seed
0xC0FFEE and generator `lcg32-seed-byte-insert-delete-xor-v1`; record the exact
input stream as well as the generator. Check all outputs, unchanged failures,
repeatability, explicit session sequence/reset boundaries and integer extrema.
Use separate GCC/Clang debug, release and ASan+UBSan builds. A learner stub must
compile and reject through a named semantic assertion, not a build failure.

The actual LLVM callback repeatedly executes in one process. Its output
oracle, sentinels and repeat invocation expose wrong values/state retention.
Run an explicit finite budget, seed, maximum input length, per-input timeout,
RSS limit and artifact directory; retain initial and resulting corpus hashes,
compiler/runtime configuration, coverage log and actual executable identity.
The campaign explores inputs; a clean finite run does not prove completeness.
Do not silently discard the documented input cap or let a tool default become
the curriculum contract. Adding an optional field first changes this grammar
and oracle, then the corpus and tool cap.

## Original local diagnostic specimens and containment

Only the original `defect.c` parser, its corrected compile-time control,
`timeout.c`, learner implementations and synthetic byte files are targets.
Use short-lived local processes; no network, real credentials, third-party
software target, host security changes, operational firmware or privilege
changes are part of this lab. Inputs are at most16 bytes for the diagnostic
driver. Parent process timeouts and the fuzzer's explicit limits bound resource
use. No invalid return-address execution or code-injection demonstration is
required or implied.

The bounds specimen checks the two-byte header/count but deliberately omits
the exact payload extent before reading its declared values. The original
truncated fixture `01 02 00 01` exposes an out-of-bounds one-byte read. Run only
the instrumented specimen, save the actual AddressSanitizer report, source and
binary hashes, access kind/size and source frame. Reduce bytes while retaining
that diagnosed heap-buffer-overflow read at `tf_teaching_bounds`; an unrelated
signal, compile failure, different sanitizer class or timeout is not equivalent.
The expected shortest length is2 because the function rejects shorter inputs
before inspecting payload; `01 01` is a candidate whose actual report must be
observed. Verify all single-byte deletions no longer reach the diagnosed read.
The corrected control requires exact2+2count before access and rejects the
same inputs. Preserve the original negative specimen rather than repairing
away the exercise. This demonstrates only the observed read-bounds violation;
it does not demonstrate arbitrary execution, data theft or privilege change.
The author campaign explicitly uses the available GNU addr2line through the
sanitizer's supported symbolizer option and pins its identity. A raw diagnostic
without the required source frame remains incomplete triage evidence; do not
silently relax the predicate when an external symbolizer is missing.

The timeout specimen loops only when its first synthetic byte is255. The
parent must terminate and record that exact owned process, retain the input,
and show the one-byte trigger plus empty/nontrigger controls. Its timeout is
distinct from a memory diagnostic. A normal parser campaign timeout is a
failure/gap to diagnose, never a passed semantic control.

The noncrashing changed condition flips the first decoded value in a dedicated
compiled mutant. Ordinary process completion and sanitizer silence can coexist
with incorrect output. The semantic oracle must expose it and retain its
input/class separately; libFuzzer's assertion report is not mislabeled an
AddressSanitizer memory error. Residual limitations include unvisited inputs,
tool/compiler/allocator dependence, unchecked caller lifetime assumptions and
the absence of a physical target or external independent assessment.

## Deliverables, assessment and transfer

| Original objective | Required assessment and exact changed condition |
|---|---|
| T32.06.a | Cap10 original grammar, deterministic reset, staged output and bounded work; separately implement session epoch/sequence/reset rather than resetting every message. |
| T32.06.b | Classify original and semantic boundary seeds, retain actual mutation/corpus evidence; define profile1 optional tag before expanding corpus/cap. |
| T32.06.c | Actual diagnosed read, source/binary/report identity, same-site reduction and corrected replay; expose a noncrashing wrong value with a semantic oracle. |

D1 derives grammar, failure precedence, ownership, resource bound and the exact
security property. D2 supplies the implementation and explicit actual imports.
D3 supplies deterministic and coverage-guided commands, raw reports, corpus
identities, minimized regressions and repaired-control results. D4 completes
each original delayed variant after its7-day minimum, unaided and separately
recorded. Level0 lacks the property; level1 repeats seeds but confuses coverage,
crash or tool silence with correctness; level2 satisfies every exact mechanism
and diagnosis with counterexamples; level3 changes grammar/state assumptions
and rebuilds the oracle/evidence argument independently. Hints progress from
writing the length equation, to marking publication, to separating semantic,
diagnostic and timeout failure classes. Private technical analysis is excluded
from learner exports. Author reference runs do not certify learner mastery.
