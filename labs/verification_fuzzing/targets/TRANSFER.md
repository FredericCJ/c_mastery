# Required verification transfers VT1-VT7

These continuations implement the original PR-T61-01 through PR-T61-07 changed
conditions. They incorporate `curriculum/v5/contracts/b06.md`; the separate
assessor code is withheld. Families A/B/C/D/F/G/I/J apply. Use hosted C17,
GCC/Clang, CBMC6.11 and Z3; POSIX threads are required only for the publication
experiment. Bring the parent's arithmetic proof, ordered-search invariant,
queue lifecycle table and byte-frame decoder. The exact public interface is
`transfer.h`. No library may supply the taught parser, search or shrinker.

## Problems, invariants and deliverables

VT1 maps T61.01.a/b/c. Implement unsigned32 multiplication with unchanged output
on overflow and checked int64-to-uint16 conversion. Derive `b=0` separately and
`a<=floor(UINT32_MAX/b)` for nonzero b. D1 includes the Euclidean quotient/remainder
argument, zero/extreme/signed-boundary tests, and the CBMC harness. The independent
SMT integer lemma proves the threshold equivalence; the C harness checks the
actual branch, low32 product and output preservation. Their composition establishes
the unrestricted uint32 result contract, subject to each tool's semantics and
the explicit bridge to C integer division. Retain the original monolithic
bit-vector timeout as a limit; timeout is not a counterexample or a proof.

VT2 maps T61.02.a/b/c. Search opaque `{key,identity}` records sorted by descending
int32 key, returning the first index whose key<=query. Equal keys retain their
identity; search neither sorts nor modifies records. D2 supplies the partition
invariant and all-empty/equal/extreme cases. Solver scope is arrays0..6, arbitrary
ordered keys and query. Inconsistent comparators lie outside the sorted-key
precondition; changing comparison direction requires a new proof.

VT3 maps T61.03.a/b/c. Build a three-entry FIFO of borrowed nonnull payloads.
`prepare(context,payload)` runs before publication and owns rollback of its own
partial work on failure. Successful entries cause exactly one notification on
removal; notification does not free the caller's payload. The payload and callback
context remain live until removal returns. Both callbacks may attempt queue
operations, which must reject while busy; they must not directly mutate queue
storage or escape the callback. Detach before notification. D3 includes failure
after the first published entry, two removals, recursive remove/offer refusal,
exact callback counts and a proof that notification is not payload ownership.

VT4 maps T61.04.a/b/c. Incrementally accept one frame: one-byte length0..16,
that many arbitrary bytes, then their xor checksum. Every byte of the exact frame
must arrive, including the zero-length frame checksum0. Trailing bytes and invalid
length make rejection sticky until reset. Finish may repeat without consuming
state; incomplete/bad frames preserve its output. The accepted digest is
`h=33*h+byte` modulo2^32 over payload bytes. D4 compares every three-segment split
against an independently derived single-shot result, includes empty segments,
truncation, a corrupt checksum and reset/reuse. Feed failure may retain the prefix
already consumed; it never publishes a successful digest from rejected state.

VT5 maps T61.05.a/b/c. Shrink schedules of at most64 immutable-ID events: submit,
cancel, complete, deliberately faulty release and irrelevant. The teaching defect
admits release after cancellation before completion. A signature names the first
premature release's stable event ID. Delete only when that same first signature
persists, not when any later defect persists. D5 retains the original schedule,
each accepted deletion, final three-event witness and deletion-minimality check.
This finite event model diagnoses premature release; it never dereferences freed
memory. The result is deletion-minimal under this strategy, not globally shortest.

VT6 maps T61.06.a/b/c. Separate submission, payload write and publication across
two actors. Compare the allowed-observation model for relaxed atomic payload/flag
with release publication and acquire observation of that release. D6 draws the
reads-from and happens-before edges, executes256 two-thread rounds, and names
which observations are modeled versus measured. Atomics prevent C data races in
the weakened example; relaxed alone does not provide inter-object ordering.
Derive a finite reads-from graph checker: initialize payload/flag before both
actors, add sequenced-before edges, add synchronizes-with only when the acquire
reads the release (or its RMW release sequence), take transitive happens-before,
and enforce write-read coherence for each object. Enumerate all four payload/flag
reads-from choices. Relaxed allows all four; release/acquire forbids flag1/payload0
and permits three. Observing only the earlier relaxed submission flag still
permits stale payload. The omitted-synchronizes-with mutant must admit the
forbidden outcome. Compare these graph-derived outcomes with the actual256
release/acquire rounds; do not treat an absent relaxed outcome on one machine
as a proof. The supplied assessor model is specialized to these finite graphs,
not a full C11 axiomatic checker; fences, consume, arbitrary RMWs, SC total order,
non-atomic payload and out-of-thin-air cycles need new models. Physical weak-order
observations remain separate. The RMW delayed variant has a separate assessor
reference; inspecting it makes the learner attempt assisted practice.

VT7 maps T61.07.a/b/c. Fuzz an entire session per input, retaining state between
commands: opcode modulo3 selects reset, feed or finish; feed's next byte modulo20
selects a chunk size clipped to remaining bytes. Missing size ends the session.
Only reset clears stream state. The harness independently accumulates raw bytes
and checks exact validity/digest; it aborts on semantic disagreement. D7 retains
seeds,20,000-session bounded run, minimized failure and deterministic regressions.
Resetting before every command destroys the history being assessed.

## Verification, rubric, hints and variants

Run learner code against `transfer_test.c`; run the opt-in author campaign with
`python3 tools/run_b06_verification_transfer.py`. Require semantic rejection of
five compiled mutants and unsolved stubs, actual solver outcomes and a fuzz log.
The five mutants remove multiplication admission, choose the wrong duplicate
partition, notify twice, reject a one-byte segment and preserve any later failure.
For every VT criterion, `.a` requires its derivation, `.b` implementation and
independent boundary evidence, `.c` changed-condition reasoning and limits.
Level0 lacks the property; level1 reproduces fixtures without its invariant;
level2 satisfies the stated contract and falsification controls; level3 solves
the delayed changed condition unaided after seven days.

Hints progress from a mathematical/postcondition table, to a temporary state
before publication, to a separate oracle that describes the property rather
than copying control flow. For shrinking, record identity before deleting.
VT1-DELAYED changes result width; VT2-DELAYED changes duplicate policy; VT3-DELAYED
adds cancellation with borrowed payloads; VT4-DELAYED permits concatenated frames;
VT5-DELAYED requires the same state transition as well as ID; VT6-DELAYED moves
publication to an RMW release sequence; VT7-DELAYED adds a second logical channel.
Each requires new assumptions and evidence, not larger constants.

## Security, references and limits

Only original synthetic programs and bounded local data are targets. The premature
release trace is a property violation, not evidence of code execution. Keep faulty
execution inside the value model; do not add a real dangling access merely to
produce a crash. Repair waits for completion/quiescence and rejects repeated
release. References are R-B06-FOLLOWUP-C11-TYPES (5.1.2.4 paragraphs5-25),
R-B06-FOLLOWUP-CBMC-UNWIND and R-B06-FOLLOWUP-CBMC-PROPS, with exact bounded
inspection records. The author report is `verification_transfer_validation.json`;
no independent learner, external proof audit or hardware validation is implied.
