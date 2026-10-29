# T46.02 assessor analysis and withheld transfer package

Keep this directory outside learner exports. Original criterion IDs, tasks,
minimum seven days and baseline records are retained. Score each objective
independently; a passing shared executable is not a substitute for its argument.

<a id="02a"></a>
## T46.02.a — boundaries, propagation and changed instruction set

The implementation first validates contiguous descriptors, complete widths and
bounded operands. Only after all starts are known does it validate every explicit
branch destination. A reachable worklist then propagates relative heights.
First discovery assigns a height once. Another edge must agree; it does not
create a second first discovery. A finite code array bounds V, E <= 2V and queue
storage, independent of the number of dynamic loop iterations. The 1024-byte
limit is part of the representation, not a claim that recursion needs no bound.

Require a table mapping all 24 byte values to width/need/delta/flow, including
CALL's arity override and RET's exact height. A shared table with an omitted
second SPLIT target is still wrong: semantic mutant 1 accepts the deliberately
dead instruction with an interior false target. The selected target's reachability
does not exempt structural validation.

Withhold `lowering()` until transfer administration. TRUE and FALSE select 42
and 17. Original offsets 6/12 relocate to 7/13. A second function at 10 moves to
11; its internal target 13 moves to 14. Validate then run the lowered output on
the actual unchanged predecessor. Mutant 6 leaves a true target unrelocated and
fails the lowering relation. An alternative validated IR emitter is acceptable
if it derives the same semantics and resource contract. Its table must still
drive both passes. Never accept an implementation that only handles public
constant branch programs.

Rubric: 0 wrong boundary/effect; 1 reproduces examples without finite-work
argument; 2 accounts for all decoded/dead/reachable instructions and proves
base propagation; 3 independently derives both successor relocation and output
expansion bound, then supplies a new multi-function case. Delayed administration
must use a fresh source mapping and record the actual date and learner provenance.
Today's author execution does not satisfy the seven-day observation.

<a id="02b"></a>
## T46.02.b — rejection with valid neighbors

Interior branch `NIL; RET; JMP 3` fails at pc2 because pc3 is an operand. Changing
the target to 0 makes that dead branch structurally valid. A branch from function0
to function1 is invalid even when the destination is an instruction start; its
nearest valid version targets its own entry. The CALL fixture changes only the
arity operand; the local fixture changes only the local number. These positives
reject the superficially defensive solution that refuses every branch/call.

The loop fixture first establishes height1 at pc1. The second arm at pc8 adds
one operand, then jumps back to pc1 at height2. Mutant 2 ignores that conflict.
Replacing only pc8 NIL with GC balances the edge. `vv_load` stages raw decoding
and verification before publication. All 17 prefixes and the complete conflicting
loop preserve the entire initialized output sentinel. Mutant 5 publishes too
early and is rejected on the complete malformed program, not a compiler failure.

Score 0 incorrect phase/reason; 1 rejection-only examples; 2 all five malformed
classes plus valid neighbors and atomic load; 3 independently constructs a second
path/backedge that exposes the first-visit fallacy and explains the incoming
heights. Require actual trace evidence, not a name such as `test_backedge`.

<a id="02c"></a>
## T46.02.c — theorem limits and definite initialization

The uninitialized local reaches the actual VM's initialized-bit guard and returns
TYPE. Wrong-tag arithmetic also returns TYPE. `JMP 0` passes structural analysis
and exhausts seven fuel steps. The two-function example verifies each function's
relative maximum, but 63 caller temporaries plus two callee pushes exceed the
shared 64-slot stack. The second push returns STACK_LIMIT with sp64. All four
leave the result sentinel91 unchanged. Do not remove these dynamic checks after
adding initialization analysis: three of the counterexamples remain accepted.

Define each reachable node's abstract state as a set of definitely initialized
locals. The ordering is reverse inclusion: fewer guarantees is less precise.
The join is intersection. STORE adds one fact; entry contains parameter indices.
A first incoming edge initializes the node's state; subsequent edges can remove
facts. A node can lose at most eight bits, so at most nine processings per node
including its first; outgoing edges are at most twice processings. A node with
a pending queue entry needs no second queue entry, because processing reads its
latest state. Eager rejection at a missing LOAD is sound for the syntactic-path
property: a finite witnessed incoming path lacks that fact. This is not complete
for semantic feasibility, because constant conditions are not propagated.

`initialized()` deliberately lets the initialized short arm reach LOAD29 before
the long arm. When the long arm arrives, intersection loses bit0 and must requeue
LOAD29. Mutant 3 uses union; mutant 4 omits requeue. Both compile, both fail the
same required late-arrival relation for different causes. Assignment of NIL on
both arms is a positive initialized case, while its value tag remains separate.

The withheld oracle enumerates 8^4 pairs of store masks for two sequential
diamonds, for three queried locals: 12288 cases. It computes four concrete path
sets and asks whether each includes the queried local. It does not implement
the verifier's intersection or worklist. The bytecode generator produces valid
height-balanced instructions with a long second arm. This exhaustive finite
shape does not exhaust arbitrary graphs, eight-local programs, recursion or C
implementations. Hand-derived loops, parameters and diagnostics complement it.

Score 0 conflates structural/type/resource safety; 1 lists limitations without
executable witnesses; 2 derives all four runtime outcomes and enforcement
boundaries; 3 derives the initialization lattice, late propagation, bounds and
precision counterexample, then designs a fresh graph before seeing the oracle.
All three objectives require independent explanation for a mastery-level result.

## Resources, source and execution limits

`struct vv_work` is caller-owned scratch: three byte arrays, one int16 height array,
two uint16 arrays, one of length1025. It has no allocation or C recursion.
`vv_load` and `vv_lower` each stage one bounded `struct cvm_program`; the report is
diagnostic output and is explicitly not failure-atomic. New code does not invoke
64-bit division, but the linked actual VM's checked arithmetic may emit it. M4
uses the already validated original signed-division ABI bridge and T11 limb
implementation. Each object and final image is inspected; no runtime archive
is permitted. The stack audit sums all common linked live C frames plus the maximum of the
four sequential test-entry frames plus1024. These entries never call each other;
all their callees remain in the common sum, overcounting siblings. It does not apply to
arbitrary host callbacks, concurrent interrupts or a physical-board watermark.

External review, actual learner work, delayed performance and physical target
observation are pending independent dimensions. The Clang source supports the
dataflow concepts, not these original VM constants or an ISO C safety guarantee.
