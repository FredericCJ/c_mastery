# Required T65.02 propagation and incremental parsing investigation

Complete original X-T65-002 R1-R4, C12 and its delayed variant. Retain your actual
er_parse_pair and er_result implementation. Implement propagation.c and
propagation_macro.h from primitive C17 operations. No strto*, parser library,
GNU statement expression or automatic rollback facility is permitted. Hosted
errors.h includes setjmp.h; this packet makes no strict freestanding claim.

## Complete-buffer forms and single evaluation

epr_pair_explicit and epr_pair_macro implement the original grammar and exact
error-offset contract: two nonempty decimal fields separated by one colon,
each field and total at most65535, exact end, leading zeros allowed, no signs or
whitespace. On failure preserve sum. Field overflow identifies the first digit
exceeding65535; a missing field/delimiter identifies length; total overflow
identifies length. For example1:65534 succeeds,65536:0 fails RANGE at4,1:2x
fails INVALID at3 and1:65535 fails RANGE at7. Compare actual er_parse_pair.

Derive a checked digit step v <= (65535-digit)/10 before multiplication. The
explicit version uses ordinary result values and goto. The macro version uses
the same checked primitive through this statement protocol:

`EPR_TRY(fresh_identifier, expression, destination_pointer, failure_label);`

Expression returns er_result; destination points to a live er_result. Evaluate
both expressions exactly once, assign the complete result, and jump to the
caller label only on a non-OK result. The caller supplies a fresh temporary name
that occurs in neither expression. Do not introduce an undisclosed fixed name.
Both expressions and callbacks obey the no-reentry/no-nonlocal-exit premise.
The label is in the current function and must not enter a variably modified
object's scope. Propagating an error does not destroy previously acquired owners.

Require if/else use, side-effect counters, a passing explicit fresh-name case,
and strict compiler rejection of use as an initializer. Deliberately reuse a
caller variable as the temporary: compile the resulting type-capture example,
but do not execute an uninitialized self-reference. Caller-selected freshness is
discipline, not macro hygiene. Compare static inline epr_identity: it evaluates
its argument once but cannot jump or return from its caller. Count destination
evaluation as well as the result expression. No expression-context claim is made.

## Changed representation: segmented signed input

EprParser holds absolute accepted position, phase, two magnitudes, digit-seen
and sign flags, terminal code/error offset and eventual sum. Fresh zeroed storage
may be initialized with signed_first0/1. In mode1 the first field alone may begin
with one minus; its magnitude remains at most65535. A plus, sign-only field,
negative second field or whitespace is invalid. Negative zero is permitted.
The final mathematical sum must be0..65535. Keep sign and magnitude separate;
check second >= first before subtracting a negative first field.

feed accepts a borrowed extent and final0/1. No input is retained. It returns
code, bytes consumed by this call, cumulative absolute position and error offset.
On a valid nonfinal prefix return AGAIN and preserve caller sum, even when both
fields already exist: later bytes may invalidate exact end. Final success alone
publishes sum. Empty segments and NULL+0 are legal without null arithmetic.

A bad byte is not consumed. Prior accepted bytes remain consumed, the absolute
error offset identifies the bad byte, and the parser latches failure. Numeric
overflow similarly refuses the offending digit. Missing final input or total
range failure identifies the consumed end. Later structurally admitted calls
on a failed parser consume0 and retain its exact error. Later calls on a successful
parser consume0 and republish its cached sum. No repair or implicit reset occurs.
The caller must initialize a new operation before parsing another record.

Before any byte access, reject null required arguments, nonzero null extent,
invalid final flag or length exceeding SIZE_MAX-position. Structural refusal
consumes0 and preserves parser/sum; length overflow returns RANGE at the current
position. This admission precedes terminal-state replay. Valid live controls are
unmodified except through the API; forged arbitrary representations are outside
the contract. All controls, outputs and input bytes are disjoint and externally
serialized. Do not infer ownership or pointer validity from numeric checks.

Concrete transition: feed '-12:' nonfinal consumes4 and returns AGAIN with sum91
unchanged; then feed '15x' final consumes2, returns INVALID at absolute6 and
preserves91. Returning an old segment-relative offset2 loses the absolute error
location. '-12:11' fails RANGE at6; split '-' / '65535:65535' succeeds with0.

## Deliverables, checks and assessment

Submit state/cardinality and progress invariants; both complete-buffer forms;
incremental state transitions; exact positive/negative compiler commands; an
independent grammar/integer model; mutation results; and cost observations.
Enumerate all strings length0..6 over0,1,9,colon,x for the original parser.
Enumerate signed cases over0,1,9,colon,minus,x length0..5 at every split. Add
UINT32-sized magnitudes,400 zero prefixes, embedded zero, incomplete signs,
negative totals and finite absolute-position refusal. Compare outputs, not only
status. Error canaries and consumed positions are independent pass/fail gates.

The four anchors separately assess ordinary propagation, segmented progress,
macro evaluation and actual retained parser controls. Nine compiled mutants
target double evaluation, relative offsets, premature publication, negative-wrap
acceptance, omitted failure jump, sum overflow, trailing input and retained early
publication/trailing controls. A compiler failure, sanitizer abort or unrelated
crash does not count as a killed semantic mutant. Learner stubs must compile and
fail named assertions. The independent oracle covers finite input sets only.

R1 must distinguish error ownership from rollback; R2 must defend both forms and
their admission rules; R3 must falsify the stated faults with independent expected
results; R4 must derive the changed progress/grammar model and C12 limits. Merely
passing nominal examples earns at most level1. Use the original four-level rubric.
Hints: separate sign from magnitude; retain a global accepted position; publish
only at exact final end; make the macro's temporary-name contract explicit.
Separate assessor analysis supplies the sealed variant; do not copy it for a
delayed unaided attempt.

## Costs and enforcement

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual parser/result size/alignment and finite phases. |
| Reachable storage | Fixed parser fields, no retained byte buffer. |
| Lifetime | Input borrowed only during feed; control survives all segments. |
| Mutation/aliasing | Prefix progress persists while final sum is transactional. |
| Allocation | No allocator calls by source/object inspection, no malloc claim for host tools. |
| Movement/copying | Result values copied; no payload byte copy or hidden buffering. |
| Destruction | No owned external resource; propagation does not unwind other owners. |
| Dispatch | Ordinary calls and macro expansion; no dynamic dispatch. |
| Concurrency | Serialized control only, no concurrent-feed guarantee. |
| Failure | Lexical/range terminal versus structural unchanged refusal. |
| Compilation | Strict initializer/name-capture refusals and fresh-name positive. |
| Binary/stack | Six-profile emitted objects, undefined imports and stack observations. |
| Boundary | Absolute offsets, exact extents, edition and macro enforcement limits. |

Time is O(bytes inspected), storage O(1). Seven10000-iteration samples compare
the same unsigned complete-buffer workload across retained, explicit and macro
forms; include correctness checks and call overhead, zero separate warmup and
raw CPU resolution/noise. Do not compare incremental and complete-input costs as
equivalent contracts. Compiler observations are neither a universal speed claim
nor an absolute mastery result. Only original local synthetic teaching targets
are used. No code execution exploit, physical observation or human-independent
assessment is claimed; exact published C17/C23 remain inaccessible.
