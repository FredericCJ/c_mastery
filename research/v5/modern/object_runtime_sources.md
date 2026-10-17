# T74 inspection, reflection and authoring gate

The actual modern plan T74 list, baseline labs/runtime/include/cvm.h and
PR-T46-08 were read before design. The baseline already implements a bounded,
nonmoving, stop-the-world mark/sweep collector with explicit VM roots; it is
retained. Its existence does not supply shapes, copying, cards, weak ephemerons,
cycle trial deletion or compiler-assisted moving roots. The M11 census passage
was inspected as discovery/coverage guidance, not as a substitute for primary
mechanism sources. The eleven planned mechanisms are registered with44 explicit
derivation/construction/falsification/transfer leaf objectives.

Focused primary reads: V8 pointer-compression value-tagging and heap constraints;
Lua5.4.9 internshrstr, luaS_newlstr and resize-failure restoration; pinned V8
hidden-class transition/IC guards and validity-cell discussion; V8 write-barrier
purposes, elimination preconditions and slow-path allocation; Orinoco generation,
scavenging/forwarding, compaction and incremental/concurrent distinctions;
Lua5.4 manual2.5.3/2.5.4 on resurrection, finalizers, weak tables and ephemerons;
CPython candidate residual counts/rescue traversal and destruction phase order;
LLVM safepoint Overview/Core Concepts and Explicit Representation, including
all-copy relocation and derived-base association. N1570 clauses6.3.2.3 and7.20.1
were read as C11 draft evidence; exact published C17 remains unavailable.

Rolling-source bytes are cached only under build/modern-sources and hashed in
the register. A byte hash identifies a snapshot, not a claim to have read every
passage or independently reviewed it. Compiler sources, source languages and
production collectors were not built or executed. The original C kernels are
new teaching programs with narrower explicit contracts. Inspection occurred
before their specification; author host spike evidence is separate from final
source-bound campaigns. No physical target behavior is inferred from a link.

Bacon/Rajan ECOOP2001 concurrent cycle-collection paper was discovered, but the
attempted university PDF mirrors failed (one404 and other fetch errors). It is
an unavailable reference, not an inspected authority. The actual trial-deletion
exercise therefore uses the inspected CPython primary description for a
synchronous candidate collector, and preserves the concurrent research extension
as required work. The source mismatch is visible, not an equivalence claim.

Research decisions: retain integer handles where C cannot safely manufacture
pointers; separate root-map completeness from pointer tagging; distinguish
single-mutator incremental barriers from concurrent ordering; use a whole-state
transaction only as an explicit measured design choice; keep finalization policy
different from prompt deterministic resource destruction. Whole-document review,
independent learner assessment, full actual-import integration and changed-target
measurements have not occurred.
