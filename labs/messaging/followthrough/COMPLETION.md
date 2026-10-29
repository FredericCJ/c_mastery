# Retained concurrent completion winner

This required successor supplies the concurrent-adapter clause of
X-T73-003-R3 and supports the lifetime argument of X-T73-011. Complete
`src/completion_follow.c` against `completion.h`, after the serialized
`struct msg_operation` and owned-value exercises. Import those actual public interfaces;
do not replace the mechanism with a future, task or reference-counting library.
The environment is hosted C17, Linux/POSIX threads and the existing separate
GCC/Clang debug, optimized, address/undefined-behavior sanitizer and thread
sanitizer profiles. The shared exercise specification and original inspected
atomic/proposal references apply; this is an original C protocol, not a C++
proposal conformance implementation.

## State, ownership and concrete operations

`mfx_create` allocates one independent control, connects and starts the actual
serialized operation, and returns one retained token. Its output must own no
previous token. Invalid arguments, injected failure and allocator failure leave
the output unchanged. The control borrows the receiver context; the receiver
may destroy that context on completion. The optional final-release observer
borrows a **separate** context that remains live until all tokens are released.
An allocation-free target implementation is a separate dependency profile.

`mfx_retain(existing,&out)` derives a second owning token from a token the caller
already owns and keeps live for the call. It refuses reference-count exhaustion
without publishing output. It cannot turn an unprotected raw pointer into an
owner; reading an atomic count in freed storage would already be invalid.
The output owns no previous token and is distinct from every live token being
used concurrently. Each thread receives its token before starting, by an external
synchronization edge. Moving a token clears its former variable; copying the C
pointer without retaining does not create ownership.

`mfx_cancel` records stop intent with one atomic transition. It returns OK if
no completion has yet claimed the control, or CLOSED after a claim. A repeated
pre-claim request remains OK. It does not call the receiver, stop a backend,
join any thread or release any token. A late request can set the intent bit but
cannot revise the already selected outcome.

`mfx_complete(token,status,&owner)` accepts OK, FAILED or CANCELLED and a unique
owned-value descriptor (or the empty descriptor). One atomic compare/exchange
selects the winner. If cancellation precedes that successful claim in the
atomic modification order, the winner delivers CANCELLED; otherwise it delivers
the supplied status. The winner clears the caller's descriptor before invoking
the receiver and transfers ownership even for failure/cancellation. The receiver
must release or move that owner once. Losers return BUSY and preserve their own
descriptor. Only the winner touches the ordinary operation/pending value/context.
Every contender retains its own control token through the whole call, so a loser
cannot inspect reclaimed control storage. The receiver may free its application
object; it must not free the independently allocated control or consume another
thread's token. It may release a distinct token it owns.

`mfx_drop(&token)` clears that token before decrementing the count. The final
release frees the control exactly once, then invokes the optional observer.
Dropping an empty token is harmless. Releasing a control before any completion
does not synthesize a receiver call; the caller must establish backend quiescence
and recover its still-borrowed context separately. This is deliberate separation
of lifetime from completion. No operation may reinitialize a live control.
Callbacks return normally; nonlocal exits, cancellation unwinding and access to
context after its callback are outside the contract. Public C fields and tokens
are an instructional representation, not enforced privacy or linear types.

## Required derivation and evidence

Draw the control and context ownership graph for a creator, two competing
completers and a cancellation requester. Derive the flags' modification order,
one-winner invariant and the separate reference-count lifetime proof. Explain
why an atomic winner without retained losers is insufficient, and why retained
storage without a winner would permit concurrent ordinary-field accesses.
Justify relaxed increments from an already protected owner and acquire/release
final decrements; identify the external publication edge. State progress limits:
compare/exchange retries and allocator/callback behavior have no wait-free bound.

Submit the implementation, transition table, exact token/destructor ledger,
independent oracle results, sanitizer logs and a generated-code inspection.
The supplied tests exercise allocation refusal, count exhaustion, cancellation
without completion, duplicate completion, creator release before contenders,
receiver-freed heap context and128 three-thread competitions. The independent
Python model enumerates3,279 nonempty words of length at most7 over two completion
statuses and cancellation. It checks status, exactly-once receiver invocation,
payload disposal and final control release. This sequential model does not
exhaust native schedules or substitute for a C memory-model argument.

Pass requires every listed invariant and evidence item. Superficial completion
passes one ordinary callback while conflating stop and destruction. Understanding
explains all losing paths and the two separate lifetimes. Independent transfer
withholds a trace where the creator drops first and a completion receiver destroys
its application state while another contender still holds a control token.
Require the learner to predict each release before executing the changed trace.
Graduated hints: count tokens rather than pointers; then isolate the claim from
the receiver; finally inspect every access following a possible callback.

Two compiled controls remove the winner exclusion or omit the retain increment.
The latter is rejected by a count assertion **before** an unsafe access, so the
test need not manufacture a use-after-free. No crash or sanitizer report is
called arbitrary code execution. All resources are original local synthetic
objects and threads; no third-party targets or system security changes occur.
Author campaigns are bounded implementation evidence, never independent learner
assessment or proof for arbitrary schedules, language implementations or hosts.
