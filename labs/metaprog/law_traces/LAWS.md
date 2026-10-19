# Required T68.07 owners, fault schedules and failure-preserving shrinking

Apply X-T68-007 R1-R4 and its original delayed ID alongside this contract.
Complete X-T68-006 and retain PR-T32-02/U-T32-02-02 `testing.faults`: deterministic
input/fault schedule, independent oracle and minimized first divergence. The
retained producer declares a contract, not a C callback ABI. `fault_adapter.h`
is an explicit original adapter to that contract; it does not pretend to import
nonexistent baseline source. Connect your own accepted artifact to this adapter
and record its source/contract identity. Reference use is assessor validation.

Implement `src/law_traces.c` and `src/fault_adapter.c`; public headers, tests and
hosted failure-interception experiment are supplied. Retained `mp_equal`,
`mp_hash` and `mp_find_counterexample` remain actual separately executed imports.
Implement canonicalization, owner transitions, replay and reduction yourself.
No property-testing, string-normalization or ownership library may replace them.
Python is an independent host oracle. The owner core uses supplied allocator
callbacks; the hosted adapter uses malloc/free to make real owned allocations.
This is a declared profile-C allocation experiment extending the retained
allocation-free mechanism; it is not a strict freestanding runtime claim.

## Representation and owner operations

Use hosted ISO C17 with exact uint64, eight-bit octets and an ASCII-compatible
execution character set, checked by a translation assertion. Calls are externally
serialized; controls, outputs, input views and callback-accessible storage are
live, aligned and disjoint. Extents require actual backing objects. Owner
controls begin fully zero-initialized, are never shallow-copied as owners, and
are either empty or own exactly one allocation plus its matching allocator.
Views may copy metadata but borrow only until replacement/drop. The owner cache
is initialized, nonsemantic metadata. The caller cannot forge live pointers or
allocator identities. Output storage and input arrays may not alias an owner's
allocation being replaced. These are C caller premises, not runtime proof types.

`mpt_assign` copies0..8 bytes into new owned storage before dropping the old
owner. Empty content allocates one byte and is a live empty string, distinct
from an empty owner control. Null input bytes are allowed only at length0.
Null output/required allocator callbacks are INVALID; length>8 is RANGE;
allocation failure is NOMEM. All refusals preserve the complete old owner and
its data. Success copies bytes/cache, releases the old block exactly once using
its original allocator, then publishes the replacement with the new allocator.
Allocators return a fresh disjoint writable block of requested size or null;
release is nonfailing. All callbacks return normally, do not reenter, mutate,
destroy or relocate participating controls/views, and do not retain borrows.

`mpt_drop(NULL)` and dropping an empty initialized owner do nothing. Otherwise
cache the allocation/allocator, publish an empty control, then release once.
`mpt_view(NULL)` is an empty byte view; a real owner's view borrows its bytes.
No operation extends that borrow through assignment/drop. A copied view is not
an owner, and a separately allocated equal string has a different address.

## Equality, canonical form and hashes

`mpt_equal` and `mpt_hash` validate their inputs before publishing outputs.
BYTES compares length and all bytes, including embedded zero. ASCII_TEXT admits
only0x20..0x7e, lowercases ASCII A..Z, removes leading/trailing spaces and collapses
interior runs of ASCII space to one. It does not consult locale or normalize
Unicode. Tabs, non-ASCII encodings and embedded zero are RANGE in that mode.
Mode>1 and null nonempty views are INVALID; length>8 is RANGE. A required null
output is INVALID. Refusal preserves output. Cache, addresses, spare capacity
and native padding never affect either equality or hashing.

This explicitly bounded normalized-text relation is not NFC/NFKC or Unicode
case folding: for example UTF-8 U+00E9 versus U+0065,U+0301 are outside its
admitted domain. Compare their byte distinction without claiming this algorithm
solves Unicode normalization. Bytewise object-representation comparison of owner
structs is also different from byte-content equality and is inappropriate here.

Hash the one-octet canonical length followed by the exact canonical bytes with
FNV-1a offset14695981039346656037 and prime1099511628211, unsigned64 multiplication
modulo2^64. The length is a framing prefix, not native sizeof. Hashing{0,1}
in BYTES yields16906520802787011418. Hashes are not authentication or cryptographic
integrity. Equal canonical values imply equal hashes; collisions need not imply
equal keys. A constant hash meets that implication while providing poor table
distribution. The set of all byte strings of length<=8 is larger than2^64, so
no64-bit function can be injective on the entire admitted byte domain.

Prove reflexivity, symmetry and transitivity from equality of deterministic
canonical byte sequences, separately from the hash law. Compare actual Rust
derived equality/hash, which includes cache unless the type's implementation
excludes it, with a custom canonical equality/hash implementation. Rust's
standard Hash does not promise portable fixed output bytes; the C recurrence
here has an additional explicitly specified byte/width contract.

## Deterministic fault adapter and trace operations

`MptTrace` is a fully initialized array of at most12 steps and fail_at0..12.
fail_at0 disables scheduled refusal; otherwise the numbered allocation attempt
returns null. A positive ordinal beyond the trace's attempts causes no refusal.
SET(slot,value) chooses slot0/1 and one of five immutable synthetic strings:
0=empty,1=A,2=a,3=space-A-space,4=B. It creates/replaces an owner and sets its
nonsemantic cache to slot+11. DROP requires a live owner; CHECK requires both
owners live. DROP's value and CHECK's slot/value must be zero. The full trace is
validated against the correct ownership model and schedule before allocation,
including its suffix after a potential first divergence. Invalid histories are
INVALID with no callback invocation or output publication.

The supplied equality callback returns0/1 or a negative/other value signaling
callback failure; the hash callback returns uint64. Both are live, deterministic,
nonreentrant, borrow-only and effect-controlled for every replay. CHECK first
calls equality once, then hashes each owner once only if equal. A false equality
does not establish the other equality laws: assess those separately. Deliberate
wrong callbacks are selected only for original local synthetic targets.

`mpt_replay` returns OK for an admitted completed or falsified history, publishing
the first failure class/step and allocation/refusal/release/live counters.
Classes are NONE, HASH_LAW, ROLLBACK, CALLBACK and CLEANUP. On NONE first_step is
the step count. A scheduled allocation refusal is expected and must preserve
the previous owner; it is not itself a defect. Replay copies pre-call content
before a refused replacement and checks preservation. Stop at the first defect,
then dispose every remaining owner. Ordinary successful runs have live0 and
releases equal successful allocations, including owners dropped during the run.

An unexpected host malloc failure differs from the declared schedule. Clean up
and return NOMEM without publishing the result. A separately compiled hosted
GNU-compatible linker `--wrap=malloc/free` experiment forces this path both in
initial replay and during a shrink candidate. It observes unchanged outputs and
zero live blocks. This harness extension is not an ISO C facility.

## Shrinking contract and independent transfer

`mpt_shrink` first replays the input. INVALID/NOMEM propagate unchanged; a valid
NONE history returns NO_FAILURE unchanged. Retain its exact failure class.
Repeatedly try deletion of each single step from left to right; if none keeps
an admitted history with that same class, try decrementing each nonzero SET
value code from left to right; then decrement fail_at if greater than1. On the
first accepted candidate restart the entire order. Publish only after a full
pass accepts none. attempts counts candidate replays, excluding initial replay.
Unexpected NOMEM aborts unchanged; it must not be treated as a failed candidate
and used to claim local minimality. Output controls, attempts and input are
disjoint. No out-of-contract callbacks or native pointers are executed.

The decreasing measure is count + sum(SET value codes) + fail_at, at most72.
There are at most25 candidate attempts per pass and73 passes, so attempts<=1825
without counter wrap. This proves termination and a local minimum for the
declared moves, not a global minimum or uniquely simplest explanation.

The supplied wrong normalized-equality/raw-hash callback reduces the six-step
seed to SET0=a, SET1=A, CHECK, preserving HASH_LAW. Decrementing a field to empty
under the error-on-empty callback gives CALLBACK and must be rejected. A second
nonmonotone callback always compares equal and hashes only single A/B to1;
SET0=B,SET1=a,CHECK is a local minimum, although A versus empty is a smaller
HASH_LAW witness. Retain both as a concrete counterexample to global-minimum claims.

The delayed assessment changes the stateful operation or failure dependency,
requiring a new reference model and precondition-preserving reduction moves.
Do not count rerunning the assisted sequence as an unaided delayed attempt.

## Deliverables, tests and rubric

- D1/R1: derive canonical forms and the equality/hash law, implement the actual
  owner and pure operations, and distinguish cache/content/representation.
- D2/R2: prove the three equality laws, unsigned recurrence and shrinking bounds;
  provide the local/global counterexample and the exact callback/lifetime model.
- D3/R3: run retained laws plus law_owner/canonical/shrink/base under six profiles;
  independently recompute hashes and model histories, kill8compiled ordinary
  semantic controls and reject4compiling unsolved stubs. Retain the real callback
  fault adapter's source and the first minimized divergence.
- D4/R4: implement the allocation-failing owner/trace generator, check every
  schedule boundary, preserve preconditions and class under shrinking, execute
  the unexpected-host-failure experiment and changed callback relation, and
  complete all13cost rows. Assess an unaided changed stateful case separately.

Tools: run_metaprog_law_traces.py, verify_metaprog_law_traces.py,
run_metaprog_law_investigation.py and check_metaprog_law_traces.py --self-test.
The Python oracle uses immutable byte/list states and independent integer hashes;
it does not call the C model or import C expected outputs. Record finite scope,
seed, exact input/tool/source identity and every observed failure. Mutants must
compile and fail MPO_CHECK without sanitizer termination. Public tests alone
earn at most level1. Level2 requires all criterion obligations, and level3 an
unaided delayed changed-assumption result with assistance recorded.

Graduated hints: (1) classify precondition errors before calling the property;
(2) reset the allocation schedule for every candidate;
(3) preserve failure class and stop minimization on unavailable environment
evidence. Separate private technical analysis lives under assessor/.

## Costs, enforcement and boundaries

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual sizeof owner/view/trace and initialized empty/live encodings. |
| Reachable storage | Two live owners plus one staged replacement; at most3blocks of<=8bytes in replay, matching allocator contexts. |
| Lifetime | Borrow invalidation on successful replacement/drop; failed replacement preserves it. |
| Mutation/aliasing | Disjoint controls/outputs and external serialization; no shallow owner copy. |
| Allocation | One request per SET including empty; all scheduled boundaries and unexpected initial/candidate refusals. |
| Movement/copying | Payload copied before old release; trace candidates copy bounded records; pointer equality is not value equality. |
| Destruction | Iterative nonfailing releases; no recursive drop; callbacks must not block without a declared bound. |
| Dispatch | Typed allocator/equality/hash callbacks; actual object disassembly and retained producer calls. |
| Concurrency | No atomic or concurrent ownership guarantee; replay instances require disjoint contexts or explicit serialization. |
| Failure | Old owner/outputs preserved on refusal; callbacks deterministic; scheduled refusal is distinct from unavailable host memory. |
| Compilation | Clean/unchanged build observations, declared GNU linker test extension and actual Rust compiler comparison. |
| Binary/stack | Six-profile size, object, stack and implicit helper logs; bounded trace copies, no freestanding inference. |
| Boundary | Exact byte/FNV contract; bounded ASCII relation; no portable Rust hash, Unicode normalization, authentication or physical timing claim. |

Pure operations are O(L), L<=8; replay is O(T*L), T<=12; shrinking is bounded by
1826replays including the initial one, with at most12allocations each. Fixed
stack/control storage is independent of heap lifetime. Seven1000-shrink CPU-time
samples use zero separate warmup, identical wrong-callback contracts and full
cleanup; preserve resolution/noise and operation counts. Finite tests, quiet
sanitizers and type signatures do not prove arbitrary callbacks correct.

All security-relevant observations are original local synthetic property
violations, never attacks on external services or real data. Repair the actual
invariant and rerun its regression; no crash or incorrect hash demonstrates
code execution. Exact published C17/C23, independent human review, physical
observation and unaided learner assessment remain separate evidence limits.
