# Required SPSC implementation and changed assumptions

This required successor supplies implementation/evidence for the original
T26.03.a/b/c criteria in PR-T26-01 and PR-T26-04 and their exact `-DELAYED`
variants. Historical elective metadata, hours, IDs and acceptance evidence are
preserved. Use the original fourteen-field shared assessment contract and the
objective-specific rubric; this file specifies the missing executable mechanism.

## Prerequisites, sources and environment

Retain the actual CMX scalar `cmx_spsc` artifact from `labs/execution/include/cmx.h`
and the execution README contract, T25 publication/reuse arguments and integer
wrap analysis. Complete the earlier T26.03 awareness predictions before running
the new code. Inspected source records and exact locators are in
`research/v5/expansion/legacy_advanced_spsc/resources.json`; do not infer final
C17 text from the inspected N1570 C11 draft. Linux circular-buffer examples are
comparisons of kernel contracts, not ISO C code to copy.

Core profile C: C17, fixed typed storage, no allocator, I/O, OS calls or runtime
library implementation of the queue. Target atomics and implicit helpers must
be audited. Host POSIX threads/conditions/sanitizers/Python are verification
tools, never target dependencies. Strict target closure uses selected Cortex-M4
and Cortex-M0 compiler profiles; successful compilation is not device execution
or ISR authorization. Record unsigned width, atomic lock-free query, layout and
actual object imports. The explicit64-byte member alignment is a selected layout
assumption, not a universal cache-line size.

## Interface and representation

`ASQueue` has separate atomic unsigned head/tail, capacity1–16 power-of-two,
sixteen ordinary four-word `ASValue` slots and producer/consumer active flags.
There is exactly one externally owned producer execution domain and one consumer
domain. Retain queue and input/output/callback contexts until all calls finish.
Pointers denote live aligned objects; initialization/reinitialization requires
external quiescence. Output must not overlap queue/control storage. No copying
of live atomic queue objects, arbitrary pointer validation or concurrent destroy.

`as_init` returns `CMX_INVALID` for zero, non-power-of-two or >16 capacity,
preserving the prior object on rejection; successful init publishes no payload.
The retained `cmx_spsc_init` **remains boolean**: invalid returns0; valid
initialization returns whether both atomic indices are lock-free on that target.
Its false return is not an enum `CMX_INVALID` and must not be compared to that
enum. The new adapter separates shape validation from the measured lock-free
capability; target dependency admission remains a separate gate.

Push copies a value into a free slot then release-publishes head; full returns
`CMX_FULL` without modifying a slot. Pop acquire-observes head, copies all four
words, then release-publishes tail; empty returns `CMX_RETRY` preserving output.
The producer acquire-loads tail before reuse. Each party relaxed-loads the index
it exclusively writes. Head/tail use unsigned wrap; capacity is below half the
counter modulus and slot index uses `counter & (capacity-1)`. No allocation or
dynamic reclamation is needed, but ordinary payload accesses still need both
synchronization directions. Stale coherent observations can cause conservative
FULL/RETRY results; do not promise a third-party instantaneous count.

Optional instrumentation hooks occur after the payload copy and before publishing
the corresponding index. They must return normally, retain no slot/control
reference, not mutate the queue directly, not cancel/longjmp, and be bounded
under any progress claim. They may attempt same-domain reentry only for the
specified changed test: the corresponding active flag rejects with
`CMX_INVALID`, preserving output/queue. These flags are ordinary owner-local
state, not a multi-producer/consumer lock. A second thread using the same domain
violates the precondition and is modeled symbolically, not executed with a race.

`ASCircular` is the changed N−1 representation: normalized head/tail indices,
storage N in1,2,4,8,16, usable capacity N−1. Empty is head==tail; full is
next(head)==tail. N1 deliberately stores zero items. Its operations use the same
forward/reverse publication argument; they do not reuse the full-capacity
distance test. No reentry is admitted for this simpler adapter.

## Concrete work and evidence

**D03a (A,B,C,D,J):** Implement both retained scalar and new four-word contracts,
explain the status mapping, and test requested capacities1,2,4,16 plus invalid
0,3,17. Fill exactly, reject one extra, drain FIFO, preserve empty output.
The delayed original reentrant producer task must show the lost-update trace
when two nested invocations snapshot the same head, then demonstrate the new
same-domain rejection without calling it an MPMC algorithm. Include the symmetric
consumer collision: two claims of the same slot/reuse right.

**D03b (B,C,D,F,G,J):** Derive both ordinary-payload paths:
write→release head→reads-from acquire head→read, and
read→release tail→reads-from acquire tail→next write. Pause an actual producer
after copying but before head publication: consumer must return empty unchanged.
Pause an actual consumer after copying but before tail release: producer must
return full. No sleeps establish enrollment. Exchange4000 multiword values at
each requested capacity with two threads, beginning near unsigned wrap; verify
sequence, inverse, salted word and triple. Repeat under both compilers and
separate memory/race instrumentation. Model weakening either required edge:
identify the now-unordered conflicting ordinary accesses, not an invented
deterministic value result for undefined C. The early-publication/reuse hook
mutants remain defined under the forced test's synchronization and are rejected
at the boundary assertion. The delayed multiword task is assessed by this real
variant, not scalar natural atomicity.

**D03c (A,C,F,H,I):** Exhaust every reachable six-bit fresh-index state for
capacities1,2,4,16 and compare the actual unsigned queue at translated near-wrap
positions. Independently count states/edges. Include coherent stale-observation
states across two reduced-width wraps, with explicit read monotonicity and
occupancy premises. Report actual head/tail offsets, alignment, reported host
line size, lock-free query and generated-code helper closure for each available
target. No target may silently import libatomic or memcpy into the strict
profile. The delayed N−1 variant must implement/retest its different admission
rules, including N1 zero capacity, rather than merely change a constant.

Inspect `helper_probe.c`: changing the payload to a64-word aggregate copied
across a callback can introduce memcpy on the selected GCC M0 profile. A
standalone four-word diagnostic did not emit that helper in the recorded run;
compiler lowering depends on the complete source and selected options. Compare
the actual core's explicit four-field copying
without changing ownership semantics. Audit AS alone and the composed CMX
image separately: the retained CMX initialization's runtime lock-free query
can itself require a helper. Unsupported strict closure is an explicit rejection,
not permission to remove the capability check or silently link a runtime.

Complexity: each no-hook operation has a fixed number of operations and a
four-word copy; memory is fixed16 slots plus indices/flags. A loop retrying FULL
or RETRY has no per-request bound merely because one call is bounded. Lock-free
atomic implementation, hook behavior and scheduling are additional premises;
neither native elapsed time nor padding establishes wait-free machine progress.

## Verification, hints, rubric and limits

Deliver D03a/b/c source, state/ownership proof, invalid/reentrant counterexample,
independent model comparison, commands/raw logs, object/link maps and changed
variant defense. Public learner scaffolds must compile before expected named
`AS_ASSERT` rejection; reference implementations and detailed analysis remain
under assessor. Use `tools/run_advanced_spsc.py` for source-bound author evidence.
It does not grade a learner or satisfy the original minimum delayed interval.

Hint1: separate which party writes each index from which party owns each slot.
Hint2: an old tail observation can reduce apparent free space; it must not grant
an unwarranted reuse right. Hint3: compare logical unbounded positions with their
modular representations, then mark the exact synchronization edge allowing an
ordinary read/write. A head/tail equality rule means different capacity in the
normalized-index representation.

Keep each original four-level rubric:0 missing/wrong behavior;1 ordinary examples
without justified assumptions;2 only the corresponding D packet with independent
oracle, ownership argument and intended negative controls;3 requires the original
independent delayed variant after its specified interval. Assess init/status and
reentry for .a, both multiword ownership edges for .b, and wrap/changed-capacity/
target closure for .c. Shared evidence does not silently earn all three.

All failures are original local synthetic teaching targets. No unsafe native
two-producer or weakened-order payload race is used as a numeric oracle. The
observable violations are lost/duplicate modeled values or a failed boundary
assertion, not arbitrary code execution. Tests and finite models leave general
proof, unsupported target, external review, learner and physical dimensions
distinct. LRUN-18's MPMC/reclamation work remains required separately.
