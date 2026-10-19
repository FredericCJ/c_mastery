# Identity reuse, finite tags and target atomic width

This required successor covers original PR-T26-02 and PR-T26-05 T26.05.a/b/c
and their exact delayed variants. Keep their fourteen-field shared contract,
original rubrics and seven-day interval; construction is required despite the
historical elective label. Author execution is not independent acceptance.

## Prerequisites, sources and profile

Bring the actual `cmx_identity_pool` interface from `cmx_advanced.h`, the
SPSC/progress distinction and unsigned arithmetic/lifetime analysis. Use the
inspected source records in
`research/v5/expansion/legacy_advanced_identity/resources.json`: Michael's2004
hazard-pointer paper sections2.3 and3.3 on incarnation/reinsertion limits,
Michael–Scott's ABA discussion, and N1570 lock-free query clauses. Historical
architecture claims in a2004 paper are not current hardware specifications.
N1570 is a C11 draft, not the final C17/C23 text; current compiler modes and
observed object queries are separate evidence.

The pool core is C17 profile C, fixed typed arrays and no allocator/library
implementation of identity management. Every pool operation requires external
serialization, a live aligned initialized pool and live disjoint output storage.
The separate CAS functions assess atomic representation/lowering; they do not
make the ordinary pool fields safe for concurrent access. Hosted tests, Python,
sanitizers and native libatomic are verification tools. Strict target images
must exclude all runtime helpers and visibly reject unsupported operations.

## Representation, operations and failure

`AIPool` contains16 slots, a generation per slot, used/retired flags and an
immutable configured generation maximum. `ai_init` admits exactly8 or24
generation bits, initializes every generation to1, and rejects invalid input
unchanged. A32-bit handle stores a1-based slot index in the low8 bits and the
generation above it. Zero and indices17–255 are invalid. `ai_take` selects the
first unused nonretired slot, marks it live and returns its handle; FULL leaves
the output unchanged. `ai_valid` checks both liveness and the exact generation.
`ai_release` rejects stale/invalid/double release, otherwise ends that incarnation.
It increments generation if below the maximum; at the maximum it permanently
retires the slot instead of wrapping. No payload dereference is needed to test
an identity. Null control/output admission is explicit, not an attempt to
validate arbitrary pointer bits.

The handle is meaningful only with its originating pool and that pool's lifetime.
Another pool can issue equal numeric bits. Do not reset/reinitialize while any
old handle, borrow or pending operation can survive: restarting generation1
would defeat the proof. Tag validation alone grants no borrow lifetime; release
must already be excluded while an actual borrower accesses payload. Specify
that ownership protocol separately rather than calling a generation a lock.
Each operation scans at most16 slots or checks one slot; storage is fixed.

The retained CMX pool remains unchanged:16 slots,16 generation bits stored in
its existing32-bit handle and saturation after65535. Its Boolean validity and
enum release status are preserved. Compare interfaces and representations;
do not silently relabel that producer as the new24-bit implementation.

## Concrete work

**D05a (A,B,C,D,F,J):** Implement the pool and execute the original
A1→B1→A2 identity trace with symbolic addresses, then its exact delayed index
version. The teaching CAS uses atomic integer indices: root0→1→0 makes a paused
comparison of0 succeed although incarnation1 became2. No freed native pointer
is dereferenced. Compare a full `(generation,index)` word, where257→258→513
rejects the stale expected257. Distinguish a successful numeric CAS from
authorization to act on the old logical object. Test invalid indices, stale and
double release, full output preservation and generation advancement. Run the
actual retained CMX producer through every65535 incarnation of its first slot
and demonstrate retirement/fallback to slot1.

**D05b (A,B,E,F,H,J):** Use the same stale-reference trace to compare allocation
generation tags, deferred reuse and continuous validated hazard protection.
Record per-slot/tag width, pending references, retained storage and scan cost.
Generation checking detects a different allocation incarnation but does not
prevent a release between check and use. Deferred reuse retains storage until
the relevant readers quiesce. A hazard protocol must publish and validate while
the reference is safe and retain protection across its last hazardous use.
Neither deferred reuse nor a hazard pointer universally prevents logical ABA:
remove and reinsert the same still-live node without changing its allocation
generation and show the stale link expectation can recur. A version updated
on each relevant structural transition or a stronger algorithm invariant is
needed. The delayed compact32-bit requirement is implemented as8 index bits
and24 generation bits with permanent retirement; inspect exact maximum-word
encoding and reject a wrap-based availability shortcut.

**D05c (A,C,D,F,H,I,J):** Exhaust every8-bit start tag and positive distance1–256
in the independent model, keeping the stale tag. Exactly256 start/distance
pairs regain equal bits after a complete cycle. Independently execute all4080
admitted incarnations of the16-slot saturating8-bit pool and reject every
stale handle; the next allocation fails. Project the24-bit saturation boundary
without claiming to have executed268435440 allocations. With fixed16 slots and
permanent retirement, at most16×(2^g−1) new incarnations can be admitted across
the pool lifetime. The original unbounded-uptime variant must choose explicit
availability loss, bounded maximum reuse, more representation/memory, or a
proven global quiescence reset. It cannot promise unlimited new identities,
fixed finite tags and unrestricted survival of all stale references together.

Inspect actual pointer+tag and compact-word atomic representations. The hosted
comparison admits only its asserted no-padding selected ABI, uses live pointers,
checks success and expected-value update on failure, prints size/alignment and
actual lock-free results, and records linked libatomic identity/imports. A
non-lock-free result is not evidence of a particular hidden locking algorithm
unless its implementation is inspected. Target Cortex-M4/M0 object/link audits
separately select the serialized pool plus32-bit CAS and the double-width pair.
Keep compiler alignment/large-atomic warnings in the evidence. Do not silently
add helpers to strict profile C or infer128-bit support from an ISA feature name.

## Deliverables and assessment

Supply D05a/b/c source and state invariants, each stale-reference trace with
explicit object identity, the reduced exhaustive comparison, cost/availability
ledger, actual CMX composition results, native and target object evidence, and
the corresponding original delayed defense. Public stubs compile before
AI_ASSERT rejection. `tools/run_advanced_identity.py` records author execution;
reference code, oracle and technical analysis remain under assessor.

Hint1: draw separate columns for address/index bits, allocation incarnation and
membership in a data structure. Hint2: protection of lifetime does not imply
the structure never changed. Hint3: count how many distinct generations exist
before designing the reset path; the reset's quiescence proof is part of its API.

Use the original levels separately:0 invalid result/ownership or unsupported
claim;1 ordinary reuse example without causal boundary;2 the corresponding
packet with exact invariant, falsification control and independent expected
result;3 its original changed task derived independently after the delay.
Assess model/implementation correspondence for .a, protection/cost and compact
representation for .b, and exhaustive wrap/availability/target limits for .c.

Security scope is original local synthetic identity state only. The harmless
success condition is accepting a stale numeric identity in a deliberately
flawed model or rejecting an assertion. This is not a native use-after-free
exploit. No physical observations, full concurrent pool proof, universal
hazard/ABA safety or independent learner/reviewer evidence is claimed.
