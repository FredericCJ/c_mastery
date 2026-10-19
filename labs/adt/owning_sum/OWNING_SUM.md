# Required owning-sum successor: X-T67-001

This implements T67-01-INVESTIGATION and supplies the changed-condition support
for T67-01-TRANSFER/X-T67-001-DELAYED. The original ADValue API and identifiers
remain unchanged. First complete its EMPTY/NUMBER/PAIR eliminators. For this
stage retain the constructor/failure/move reasoning from X-T66-001, X-T66-003
and X-T66-004; the callback adapter is implemented here, not silently imported
from a ready-made sum/owner library. The actual ad_measure/ad_number provider
is linked and its two original compiled defects are tested again.

Use hosted ISO C17, exact uint32/uint64 types and the declared GCC/Clang
debug/release/ASan+UBSan profiles. malloc/free are permitted only as the supplied
clone/drop backend; implement tag dispatch, staged ownership transfer and
rollback yourself. Python is the independent trace oracle. Operations are
externally serialized; there is no atomic or concurrent-reclamation claim.

## Representation and operations

Implement owning_sum.h and its compiling unsolved source. AdoValue adds a TEXT
alternative to EMPTY, NUMBER and PAIR. TEXT owns 0..65535 exact binary bytes,
including embedded zero; it promises neither UTF-8 validation nor termination.
Its live operations record contains clone, drop and a borrowed context that
outlives every clone and final drop using it. Empty text is distinct from EMPTY.
Do not use memcpy of a live owning representation as a semantic clone.

Init requires unowned storage and sets EMPTY. All subsequent controls are live,
initialized, exclusively owned objects. Constructors overwrite a previous
admitted value only after successful staging. Non-TEXT values use the actual
retained ADValue eliminators. measure returns zero, number, exact pair sum or
TEXT byte length. number accepts NUMBER only. Invalid ADValue tag99, null
required pointer, missing callback, nonzero length with null bytes, or length
above65535 returns ADO_INVALID without publication. Init/drop require nonnull
live storage; arbitrary fabricated native representations are caller errors.

The clone callback runs even for empty text. It returns1 on success, setting
its output to a distinct owned allocation of at least n bytes containing the
same n bytes; for n=0 it sets null. It returns0 on failure, leaves output
unchanged and releases all its partial resources. The backend may distinguish
failure before allocation from failure after allocation/initialization. The
core cannot repair a callback that violates this contract. A successful clone
creates one drop obligation even for empty text. Drop receives the exact
pointer/length/context once, is nonfailing, and accepts null/zero for empty
text. All callbacks must return normally, must not reenter or mutate any
involved control, and must not retain borrowed input/output-pointer storage.

copy stages a fresh clone for TEXT using the source policy, and only then drops
the old destination through its own policy. ADO_NOMEM leaves both logical
owners and their payloads unchanged; diagnostic callback counters may advance.
move drops the old destination and transfers ownership without cloning; the
source becomes EMPTY. Self-copy and self-move are no-ops with no callbacks.
Drop makes the control EMPTY and discharges any previous TEXT obligation;
repeated drop of EMPTY has no effect. Different controls cannot already own
the same allocation. Except exact self-copy/move, controls and scalar outputs
are disjoint. Input byte spans may borrow from the destination's old payload:
staging must finish before that payload is dropped. No callback may invalidate
that borrow during staging.

Concrete examples: PAIR(UINT32_MAX,UINT32_MAX) measures8589934590;
TEXT([0,17,255]) measures3 but number refuses unchanged. Replacing TEXT([3,4,5])
from its own suffix yields independently owned[4,5]. Failed copying from another
TEXT leaves the prior destination allocation and policy live. Copying from
allocator context A onto a value from B drops B once and creates another A
obligation; moving later does not invoke clone.

## Why the exercise is necessary

The original three alternatives were trivial values. Adding ownership changes
copy, replacement, failure and destruction, even when the old measurements
still compile. List every eliminator and classify its new TEXT behavior. A
default branch that returns zero is not an implementation of TEXT measurement.
Rust enum constructors/match bindings provide language mechanisms absent from
this C representation, but a wildcard can also silently handle a future Rust
alternative. The supplied Rust probes demonstrate exhaustiveness and wildcard
behavior; they do not claim Rust clone is equivalent to the fallible callback.

The GCC manual distinguishes -Wswitch from -Wswitch-enum: a default suppresses
the former missing-case diagnostic, while the latter still diagnoses an
omitted enumerator. Run old/new C probes with both compilers, with positive
controls and exact diagnostic text. This is observed diagnostic policy, not
an ISO C theorem about every possible tag value. Record sizeof(enum) and
release/debug branches without treating native layout as a wire format.

## Deliverables and assessment

D1 supplies cardinalities, admitted tag/payload states, callback ownership
graph, and the old-eliminator migration table. D2 implements staged text
construction/copy, replacement, move, drop and both checked eliminators.
D3 includes the independent transition corpus, callback failure cuts, all
compiled controls, diagnostic probes, symbol/branch/stack observations and
before/after resource counts. D4 repeats with a different allocator context,
empty text, a borrowed suffix of the old destination and a callback that fails
after obtaining storage. State the new policy before coding.

Use four original rubric rows: R1 assesses sum versus representation and
diagnostic enforcement; R2 assesses cardinality/active member/migration
reasoning; R3 assesses original boundary/control/diagnostic observations; R4
assesses complete owning construction, rollback, move/drop and changed clients.
Nominal outputs alone earn at most level1. Level2 requires each full contract
and resource ledger. Level3 additionally needs an unaided delayed variant and
a counterexample to the old assumptions; author tests cannot award that grade.

Graduated hints: (1) distinguish cloning a payload from copying its pointer;
(2) write down who owns the old destination until clone succeeds; (3) audit
default branches and callback context lifetime separately from the enum switch.
The assessor implementation and technical analysis are separate from export.

## Verification and costs

Four CTest anchors distinguish base/value/owner/rollback. Eight compiled
controls omit pair contribution, accept invalid tags, shallow-copy TEXT, drop
before fallible clone, omit destruction, retain moved-from ownership, omit TEXT
measurement, or accept TEXT as NUMBER. Only a normal named ADO_CHECK assertion
counts as rejection. The harness records real allocation/drop counts and has
an emergency allocation registry solely to clean resources after a deliberately
failed assertion; that cleanup never changes semantic counters or repairs a
production owner. Sanitizer crashes are not successful mutant rejection.

The independent Python value/ownership model enumerates10000 four-operation
words plus512 seeded32-operation traces,56384 transitions. It does not call the
C reference to derive expected outputs. Clone failure is an explicit operation;
the focused cases separately exercise failure after allocation and cleanup.
Changing a byte after source construction cannot change the owned text.

All thirteen cost dimensions apply: tag+union+callback native representation;
reachable payload bytes plus borrowed context; constructor-to-final-drop
lifetime; exclusive mutation and permitted old-payload input borrow; one clone
request per TEXT copy (including empty); n copied bytes versus metadata move;
exact release obligations; direct retained elimination plus indirect callbacks;
external serialization; unchanged-owner failure with changed counters; actual
compiler diagnostics and source closure; compiler object/stack reports; and
language/ABI/callback boundaries. Runtime samples execute the same three-byte
construct/copy/drop workload seven times; they include malloc and observation
cost and make no universal allocator-performance claim.

Security scope is this original local program and synthetic bytes only. The
adversary may select tags, lengths and callback failure points within admitted
objects. Observable failure is wrong output, lost owner, duplicate ownership
or missed cleanup. The correct repair is staged publication and explicit
release authority; no crash is equated with arbitrary execution. A malicious
callback or invalid pointer lies outside the contract and is not a sandbox.

Status: authored and executable; only a current source-bound campaign records
actual executions. Independent learner assessment, non-author review and
physical validation are unobserved. Published C17/C23 text access gaps remain;
existing inspected drafts are not relabeled.
