# Assessor analysis for PR-T36-06

Do not include this file, `lookup.c`, kernels or the independent oracle in a
learner export. Assess the unchanged T36.08 objective IDs and their V1 variants,
including fresh delayed work. Author reference success cannot earn that credit.

## T36.08.a: typed probing and incomplete quadratic coverage

The actual CDHash uses CDBucket size12, unlike a made-up packed key/state pair.
The x86 address is base+4*(3*index); Thumb constructs the same bounded stride.
The key and state loads are from live typed slots admitted by the wrapper.
System V caller-save registers suffice for the leaf kernel. Thumb saves r4–r8
and LR (24 bytes), restoring the caller's state; the probe verifies r4–r11 and
SP. Native probe verifies RBX/RBP/R12–R15 and SP. Neither calling convention
licenses arbitrary C lifetime/alias behavior. Probe execution is selected-ABI
evidence, not proof about all assembler/linker implementations.

The fixture key22 starts6: live6/live14/live16/deleted/live2/empty produce
6,7,0,1,2,3, missing, six probes. Keys16/2 hit0/2 in one. The bounded iteration
guard also stops an all-deleted or full absence after exactly capacity. The
checked wrapper validates all metadata and per-key reachability in quadratic
time before the linear kernel; claiming the entire checked API is O(capacity)
would be wrong. With clustered start0, modulo-derived expected paths cannot be
reused. Actual provider tests cover both policies for capacities1..16.

For the delayed quadratic policy, capacity8,start0 gives0,1,4,1,0,1,4,1: only
three positions. Store the searched key at6 under the valid linear policy and
observe that a naive change of probe policy cannot find it. The executable trace
does not implement quadratic insertion or claim an arbitrary loaded quadratic
table is valid. Require the learner to derive a coherent insertion/search/load
contract before choosing another policy. The independent oracle checks the actual
trace, not only a hand-written constant in the C test.

## T36.08.b: transaction and callback boundary

The byte fixture gives offsets1,3 with six comparisons for `[97,0]`;0,2 with
seven for `[0,97,0]`; and boundaries0..5 with zero comparisons for empty pattern.
The source stages65 outputs before capacity publication. `AL_SPACE` changes count
and work while preserving data; the actual `AD_SPACE` changes neither count nor
state/data. Return status is insufficient to infer either contract. Tests inspect
all short capacities for the finite enumeration, both null-span boundaries,
byte255, maximum empty-pattern output and output canaries.

The callback variant first computes all results, so its comparison count remains
the full search work even when the first callback rejects. A cancellation on the
second offset of the overlapping pattern retains the first accepted offset0,
attempts two callbacks, and must never invoke the third. Empty-pattern cases
inject failure and cancellation at each ordinal0..6. A callback that writes then
returns failure violates its own contract; the framework has no ability to undo
that side effect. An asynchronous callback would require owned tickets, storage
lifetimes and quiescence, not merely another return code.

Reject the misconception that staged array output implies transactional external
callbacks. The CANCEL mutant ignores rejection, continues invoking the callback
and returns success; it must fail the status/attempted/publication assertion.
COUNT leaves the short-output count unchanged and is rejected as a contract
mismatch, even though it resembles the valid prerequisite behavior. OVERLAP
advances by the pattern size after a match; all-overlap enumeration exposes it.

## T36.08.c: actual prior prefix state

The composed implementation calls the exact frozen T15 functions. Initialization
copies a nonempty pattern and derives its prefix table. Feed uses a temporary
ALStream and a65-offset staging array; actual provider capacity65 cannot overflow
its maximum64 matches per chunk. Only after sufficient destination space does
the adapter commit. Failure reports required count while retaining the original
provider state, permitting an exact retry. Empty matching is an explicitly
separate adapter with one initial boundary; resetting that initial-boundary flag
at every feed would duplicate a boundary at each split.

There are sum(2^n,n=0..5)*sum(2^m,m=0..3)=945 whole-span pairs. Each n has n+1
splits, totaling4815 streaming packets per compiler profile. Python independently
constructs the byte vectors and enumerates candidate windows, checking complete
actual offset lists and whole-span comparison counts. Output cardinality is not
enough. The PREFIX mutation changes the actual prerequisite failure-link update
after a hit; streaming overlap cases distinguish the faulty provider composition.

Counter saturation, prefix-table integrity and borrowed-input immutability belong
to the original provider contract; the adapter does not validate arbitrary forged
prefix tables. The laboratory has no concurrency or callback reentry. The emitted
packets and bounded emulator results are author observations only. A valid learner
must explain a fresh split, failure-output translation and missing premise without
copying this analysis, after the original minimum seven-day delay.
