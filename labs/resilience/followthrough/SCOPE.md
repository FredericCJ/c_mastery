# Owned cancellation cells and duration budgets

This is required implementation support for X-T75-001 R1-R4 and
X-T75-001-DELAYED. The original exercise supplies the source passages, rubric,
learner deliverables and delayed assessment policy. Complete `src/scope_follow.c`
against `followthrough/scope.h`; the original `RZScope` is an explicit dependency.
The assessor implementation is separately packaged. This is C17 hosted C with
8-bit bytes, exact `uint64_t`, C atomics and POSIX threads in the test harness.
No OS clock, timer, RPC framework or garbage collector implements the mechanism.

## Representation, authority and lifetime

An allocated `RzfScope` embeds a real `RZScope`, an immutable parent owner,
an allocator value and an atomic reference count. Each successfully created cell
owns one caller reference. A child owns a separate parent reference. All tree
edges share one clock domain; maximum depth is the retained core's eight cells.
Cancellation is an idempotent atomic request propagated through immutable parent
links. It does not reclaim a cell, interrupt a thread, undo an effect or acknowledge
completion. A raw pointer is not an owner and cannot safely be retained after its
last owner disappears.

`rzf_scope_create(parent, now, budget, allocator, out)` validates the retained
deadline rules, retains the parent, allocates and initializes a child, then
publishes `out`. Allocation failure is `RZ_FULL`, releasing the provisional parent
owner. Depth/admission errors are `RZ_INVALID`; an unrepresentable uncapped
deadline is `RZ_RANGE`. An already expired or cancelled parent may have a child;
its subsequent check reports the inherited condition. Zero standalone budget
creates an immediately expired cell. Retain refuses `SIZE_MAX` without wrapping.

Every output owner location must initially own no reference. Its bits are
unchanged on refusal. An output may not alias a live cell's fields or another
owner being consumed. Release clears the caller's location before decrementing
and releases a zero-count cell and its parent edge, at most eight cells. Separate
owners may be released concurrently; their pointer locations may not be shared
without synchronization. No ordinary cell field may be mutated after publication.
The allocator returns suitably aligned `sizeof(RzfScope)` storage or null;
deallocation is infallible and releases exactly that allocation. Its context
outlives all cells and supports the threads that may release their last owner.
Callbacks must not reenter these operations or change/retain borrowed cell
storage. ISO C cannot enforce these ownership or callback conditions.

A zero-initialized `RzfBorrow` is an acknowledgement ticket. Begin retains one
scope reference; a live ticket cannot be begun again. The borrower may access the
cell until its backend is quiescent. Only then may the ticket owner acknowledge
and release it. Duplicate acknowledgement is `RZ_STALE`. Quiescence is a caller
proof obligation, not a boolean accepted on trust by the API. In the supplied
native test, `pthread_join` establishes it. Acknowledgement of one borrower says
nothing about another outstanding ticket. Neither request cancellation nor
deadline expiry authorizes early acknowledgement.

## A budget across unrelated clock origins

`RzfBudget` contains remaining duration R and a conservative transport bound B.
Export at sender time t calls actual `rz_remaining`, deducting all sender work
already elapsed. It rejects cancelled/expired scopes and `B >= R`. The optional
16-octet encoding is unsigned big endian R followed by B, with no padding or raw
clock timestamp. Decode stages both words and validates `R > B` before publishing;
invalid input preserves the output. Encoding/decoding permit overlapping source
and output storage because all input fields are captured before publication.

Import is called at receiver time u **after** local processing P, with equal-rate
tick units on the two hosts. It rejects `P >= R-B` as expired, otherwise constructs
a scope with remaining duration `R-B-P`, capped by an optional local parent.
The envelope parent must belong to the receiver clock domain. Checked addition
can reject a receiver clock near `UINT64_MAX`. No absolute sender origin is copied.
For actual transport delay d with `0 <= d <= B`, import permits at most
`d + P + (R-B-P) <= R` sender-equivalent ticks after export. A chain repeats this
deduction at each hop. This bounds work admitted before the propagated deadline;
it does not guarantee remote termination, return-message delivery or completion
acknowledgement by that deadline. Unequal clock rates require an additional drift
bound. Unbounded transport delay invalidates this proof.

Concrete input: parent deadline20, export at14 with B2 yields `(6,2)`. A receiver
with unrelated clock1003 after P1 gets deadline1006. Copying sender deadline20
would spuriously expire it; restarting a fresh six ticks at every hop extends the
end-to-end budget. A forged budget can enlarge admission, so this encoding is not
authentication or a trusted authority boundary. The local teaching tests use
synthetic envelopes and threads only; no external service or host-policy change.

## Required experiments and assessment

Supply D1 a tree-owner diagram and the nonextension proof, D2 the implementation,
D3 the traces and failing-control explanations, and D4 the original delayed
attempt and 13-dimension cost dossier. Keep backend handles, tickets and their
contexts alive until join/acknowledgement. Show cleanup after each allocation
failure while constructing a parent and three children. Use a child after its
creator releases the parent owner; do not execute a dangling stack-parent example.

The public `resilience.scope_follow` test contains a real two-worker pause,
cancellation, creator release, join and separate acknowledgements. It asserts
both retains **before** any free, so the omitted-retain mutant fails an invariant
rather than relying on an invalid access. The other mutant forgets elapsed and
transport deductions. The independent Python runner compares actual C outputs
for 16,524 budget cases, 44,220 parent/child arithmetic cases and 1,364 cancellation,
expiry and acknowledgement words, including every ordering of two completions.
It also checks maximum timestamp, depth and reference-count refusal boundaries.
Run `python3 tools/verify_resilience_followthrough.py --driver <built-resilience-foundation>`
inside the repository for these additional traces.

Level1 merely passes the nominal deadline example. Level2 requires exact output
preservation, no leak in all injected construction cuts, admitted arithmetic
agreement, meaningful rejection of both controls and a correct explanation of
why cancellation is not quiescence. Level3 requires the delayed creator-lifetime
variant and a new clock/transport assumption with a revised proof, without copying
the assessor. Go Context's documented earlier deadline and request semantics are
comparators; this C design does not inherit Go's managed storage or type checking.
Hints: first inventory references; next separate creator release from borrower
acknowledgement; finally derive durations before adding a receiver origin.

Native runs and sanitizer results are separately source-bound in campaign reports.
Finite traces do not prove all interleavings, physical clock behavior or learner
mastery. Independent assessment and a delayed learner attempt remain external
evidence; they are not fabricated by author-side tests.
