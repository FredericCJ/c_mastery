# Audited contract gaps: certificates, matching and clocked state

These additive C17 exercises preserve all earlier project identities, sources,
contracts and acceptance records. They make three previously different or absent
executable contracts explicit. All backing C objects must be live and outputs
must be disjoint from inputs/control storage. Single-owner serialized execution
is required. The core may use scalar arithmetic, arrays and structs, but no tree,
substring-search, VM, memcpy or other library implementation of the mechanism.
Host Python/CTest/ASan/UBSan and the separately linked baseline integration are
verification dependencies, not freestanding target dependencies.

SC_OK0, INVALID1, FULL2, DUPLICATE3, DONE4, BUDGET5. Rejection leaves scalar
outputs and control state unchanged except sc_run's explicitly reported partial
progress. A forged pointer is outside the live-object contract; malicious integer
indices inside an otherwise live certificate are in scope and must be checked.

## Tree certificate and duplicate-policy adapter

SCTree contains64 actual typed SCNode slots, root(-1 empty or0..63), count0..64
and balanced0/1. Each live node holds an int32 key, child indices(-1 absent),
parent(-1 root) and cached height. Dead node fields are ignored. Valid certificates
have exactly count live nodes; every live node is reachable once from root;
strict global BST ordering holds, parents agree, null height0/leaf1 and cached
heights equal recomputed heights. If balanced1 every child-height difference is
at most1. balanced0 allows explicitly intermediate rotation snapshots but still
requires all other invariants. sc_tree_check writes the root height only after
the entire certificate passes. It rejects cycles/shared children, wrong parents,
duplicates, unreachable live nodes, invalid indices and stale heights without
dereferencing an unchecked index. Recursive depth is at most64 after visited
checks; stack usage must be measured separately from this logical bound.

sc_unique_insert(context, contains, insert, key) adapts a separate component.
contains returns0/1 without mutation; other results reject. A hit returns
DUPLICATE without calling insert. A miss calls insert once:0 maps OK,2 maps
FULL, other results map INVALID. Those callback values are the observed algo.h
contract, not a universal allocator convention. The callback retains its own
failure-atomic state guarantee; the adapter cannot repair a callback that mutates
then reports failure. No callback may reenter or retain the adapter's temporary
state. Single-owner serialization is essential between contains and insert.

The existing algorithms family explicitly defines successful duplicate no-ops;
PR-T36-05 asks for reject-duplicate certificates. Both remain unchanged. The
new adapter documents the conversion. tests/algorithms_integration.c can be
explicitly compiled against a completed algorithms workspace, deriving parent
links from existing child indices instead of pretending the old node stored
parents. Author campaigns name the actual baseline assessor source/header and
hash them; learner exports must name their own accepted source revisions.

## All overlapping byte-span matches

sc_matches accepts text length0..64, pattern length0..16 and output capacity.
NULL is legal exactly for zero-length spans and zero-capacity output. Results
are all matching starting offsets in ascending order, allowing overlaps. Empty
pattern matches all n+1 boundaries, including offset n. A longer pattern yields
zero matches. Count and output bytes publish only after a preflight proves all
matches fit. FULL preserves every output byte and count. The two-pass direct
comparison has O((n-m+1)*m) work for nonempty patterns and O(n+1) for empty
pattern, with O(1) extra storage. The input remains immutable throughout both
passes. Output overlap is outside this contract. A changed-condition streaming
adapter must retain boundary-spanning matches and choose atomic versus incremental
publication explicitly; it cannot inherit the two-pass guarantee automatically.

The earlier algo_match returns only the first match and empty-pattern offset0.
PR-T36-06 requires all n+1 boundaries. This new API resolves the mismatch without
silently changing the earlier API or delegating matching to a library.

## Clocked original machine

SCMachine has four byte registers, sixteen byte memory cells, instruction-index
pc, halted0/1 and uint32 commit count. Program length1..16 is immutable and
SCInstruction fields are operation,a,b,c. LI(0): ra=b, c must0. SUB(1):
ra=rb-rc modulo256, all register indices0..3. BNZ(2): if ra!=0 set pc=b;
b must be a valid absolute instruction index even if branch not taken, c0.
ST(3): memory[b]=ra; LD(4): ra=memory[b]; b0..15,c0. HALT(5): a=b=c=0.
Unknown operations reject. HALT preserves the current instruction index, matching
the earlier fixture oracle. Other nonbranch instructions advance pc by1. Falling off the program is
INVALID and commits nothing. Commit-count exhaustion rejects before increment.

sc_clock snapshots all old operands, validates before publishing, then commits
state once and writes SCControl: pc_before/after, reg_write bitmask, mem_write
boolean, branch_taken boolean, alu byte (LI/SUB/ST/LD result, otherwise0).
Calling on halted returns DONE unchanged. No input or interrupt arrives inside
one transition. sc_run executes up to budget clocks and writes executed count;
it returns DONE on halt, BUDGET on nonhalt exhaustion, or the current decode
error. Earlier successful clocks remain committed on error. Budget0 performs
no clock and is not HALT. The seven-instruction fixture commits11 clocks, then
stable halt; it is not an emulator for a real ISA or an elapsed-time model.

## Assessment and containment

Expose only synthetic certificates, binary strings and teaching instruction
arrays. The attacker can choose these values, cannot supply arbitrary host
pointers, and has no external target. Harmless success is a rejected bad
certificate, a missing/extra match, wrong synthetic register or a forbidden
partial commit. A crash is not a demonstrated security privilege violation.
For each mutant document input, violated invariant, observable failure, repair,
regression and residual live-storage/callback/serialization assumptions.

Named semantic controls must compile and exit1 with SC_CHECK:, never be credited
for build failure or an unclassified signal. Independent Python graph, slicing
and mathematical transition models use different representations. Six compiler
profiles and a separate Cortex-M4 link audit are evidence only for their actual
assertions. No physical observations, independent assessor or learner mastery
are claimed. Changed-condition and delayed variants are required beyond anchors.
