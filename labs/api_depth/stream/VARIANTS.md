# PR-T19-04: changed stream assumptions

These required J/B/F/G variants implement exactly
AX-PR-T19-04-T19.08.a-DELAYED (status plus count),
AX-PR-T19-04-T19.08.b-DELAYED (scatter/gather) and
AX-PR-T19-04-T19.08.c-DELAYED (two frames). Use the original criterion tasks,
rubrics, seven-day learner delay and the shared legacy assessment contract.
The original assessor campaign does not become a learner attempt. This is
new additive support, not revised historical acceptance.

## Prerequisites and environment

Import an actually completed `PR-T19-04` `io.stream` producer implementing
`stream.h`, plus its tests and ownership ledger. Implement `variants.h` in
the supplied `variants.c` stub. The author reference imports the actual
`sf_start`, `sf_pump` and `sf_cancel`; a synthetic reimplementation of their
behavior is not composition evidence. C17 profile C; caller-owned stable
storage, externally serialized calls, no heap or target library copying.
Host tests use stdio and string functions. This campaign is hosted Linux
GCC/Clang at O0/O3 and ASan/UBSan; no new freestanding link or physical claim.
The inherited producer build suppresses only its known compact formatting
warning (`-Wno-misleading-indentation`), not semantic diagnostics.

All control, payload, output and callback contexts have distinct storage unless
the callback borrows the offered spans. Handles require the owner to remain
live at the same address. All callbacks are nonreentrant and may not retain a
span, mutate control state, free the owner or throw control across the caller.
Initialize fresh storage only with no outstanding request or old handle use.

## A: count and diagnostic are independent

`SVWriteResult` carries `accepted` and disposition. The count is a prefix of
the offered suffix, at most its length. `SV_PROGRESS` requires a positive
count; `SV_RETRY` requires zero; `SV_WAIT` and `SV_ERROR` may carry any valid
count. Commit the accepted prefix exactly once before interpreting the
diagnostic. Error wins even when the count completes the frame. Terminal
results are stable and make no further external calls; an error is not
permission to replay a prefix. Wait stops the current activation immediately;
retry consumes one callback budget. Zero budget makes no callback.

The adapter must preserve original producer framing and generation behavior.
Test [1,2,3] → [3,1,2,3,6] with accepted counts 2,0,1,0 and terminal error on
the fourth call. Expect three committed bytes and four callbacks. Separately
return error with every prefix length 0..5. Reject a result beyond the offered
extent, unknown disposition, retry with progress and success without progress.
Such rejection cannot undo side effects of a contract-violating callback.

Deliver the result algebra, a state transition table, code, exact callback
traces and the counterexample to a signed count carrying two independent
facts. An error on the final byte must remain observable. Explain the difference
between earlier accepted progress and a diagnostic accompanying this call's
progress. No particular OS syscall is claimed to return this invented struct.

## B: one progress cursor across several spans

`sv_pumpv` offers up to three nonempty borrowed spans: remaining length byte,
remaining payload and checksum. Empty payload is legal and contributes no
empty span. A returned count refers to their concatenation. After a partial
write, trim all fully consumed segments and the prefix of the first remaining
segment. Never restart from that segment's original beginning. The callback
must not retain the stack-allocated span array or its borrowed byte pointers.

Enumerate payload lengths 0..64 and quotas 1..7 (455 pairs); use a decoder
written independently of the producer and vector adapter. Include transitions
inside payload and exactly on each segment boundary, positive progress plus
error, completion stability and invalid status handling from A. Supply a
derivation connecting scalar offset to segment index and within-segment offset.
Merely flattening into another owned frame hides the representation being
assessed and is forbidden for this task.

## C: admission, identity and stream boundaries

`SVQueue` owns exactly two copied frames, each with its producer generation.
Admission picks a free nonretired slot and a strictly increasing 64-bit ticket.
Full queue rejects without mutating either frame or the output handle. A slot
at maximum generation is retired; ticket maximum rejects before increment.
Caller payload may change after successful admission. An old handle cannot
cancel a later occupant. Do not implement cancellation as a raw slot number.

One activation services only the oldest active ticket, with the stated callback
budget; it returns one `SVEvent` containing identity, total accepted prefix and
status. Terminal completion frees that slot after publishing the value event.
Whole-frame FIFO preserves framing on one byte stream. Fairness is conditional:
if callbacks finish and eventually accept bytes, a frame has at most66 bytes
before the next admitted frame. A backpressured head can block its successor;
round-robin byte interleaving would corrupt this wire format. There is no
unconditional latency guarantee or thread safety claim.

Cancel before any byte is accepted permits later work. Cancel after positive
progress taints the transport. Any terminal error or invalid callback result
also taints it, even if all bytes were reportedly accepted. Pumping a tainted
queue must make no callback and must preserve the output event. Queued frames
remain owned. `svq_reset_transport(q,1)` asserts the external owner has stopped
old I/O, replaced/reset the channel and reset the peer decoder; it does not
perform or verify those actions. Passing0 refuses the reset. The public test
uses a new synthetic sink; it does not claim actual socket cancellation or
reconnection. Pending frames may then resume on that new byte stream.

Test full admission, copied ownership, FIFO order, cancellation of queued and
partly transmitted requests, stale cancellation after reuse, duplicates,
retirement and a diagnostic after a complete frame. Compare the complete
decoded output with the admitted noncancelled frame sequence. Supply an explicit
transport-reset ownership ledger and a counterexample to resuming a partly
transmitted frame on a fresh peer without resetting its cursor.

## Verification, assessment and costs

Use the public runner with explicit sources; it never copies an assessor:

```sh
python run_variants.py --stream-source stream.c --variant-source variants.c --case result
python run_variants.py --stream-source stream.c --variant-source variants.c --case vector
python run_variants.py --stream-source stream.c --variant-source variants.c --case queue
```

The unsolved sources must compile and fail an `SV_CHECK` assertion. Compile
errors, sanitizer reports and arbitrary crashes are not semantic-control
success. The author campaign separately supplies completed sources and checks
seven discriminating mutations: discard diagnostic-associated progress, hide
final-byte error, replay a vector prefix, omit checksum, accept stale cancel,
bypass transport taint and choose newest ticket. An unsolved producer must also
fail, even when the variant implementation is complete.

Assess each original criterion independently. Level0 violates a required
observable invariant. Level1 reproduces a nominal frame but cannot explain
progress/ownership. Level2 derives all applicable bounds, reproduces independent
boundary traces, rejects the relevant mutation and explains residual limits.
Level3 completes the named changed condition unaided after the recorded delay,
including the new failure semantics and a counterexample to an overstated
equivalence with a language-owned stream. C types do not enforce nonreentrancy,
live-owner handles, external exclusion or the reset assertion. Finite passing
tests do not establish absolute mastery.

Start copies O(n) into one66-byte frame; two-slot admission and selection are
O(1). A pump performs at most budget external calls; vector preparation examines
three segments per call. Callback work/latency is external. Storage is two full
frames plus state/tickets; record `sizeof` and padding for the actual data model.
No allocation or extra payload-flattening is permitted. Compare one-frame and
two-frame backpressure, ownership and storage costs with equivalent wire/error
contracts, not merely throughput under different guarantees.

Graduated hints: first write count and status as separate columns; next derive
the remaining concatenation from a scalar offset; finally distinguish request
cancellation from repairing the shared transport. Assessor analysis and
implementations are separate. Changed-condition author execution is recorded
in its source-bound successor report, while independent review, delayed learner
attempts and physical/foreign-system observations remain separate.

Security scope: original local C components and synthetic bytes only. An
adversary may choose input bytes, callback results, stale live-owner handles and
cancellation order. The properties are prefix integrity, request identity and
bounded admission. Harmless witnesses are duplicated bytes, wrong-frame
cancellation or a callback on a tainted synthetic stream. A malicious callback
can already violate its borrowed extent; this API cannot sandbox it. Repair
the state/ownership rule and rerun regressions; no arbitrary code execution or
real-service exploit claim follows from these demonstrations.
