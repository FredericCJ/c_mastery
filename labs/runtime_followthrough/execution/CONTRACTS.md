# T46.03–04: checked execution, continuations and call frames

This is an additive implementation of the six original T46.03/T46.04 criteria
and their unchanged delayed tasks. Prerequisites are the original CVM1 raw codec,
verified images, T46.02 worklist verifier, checked integer derivation, stack
ownership and bounded state machines. Implement `execution.h` in `learner.c`.
Bring your actual predecessor VM primitives and verifier; the author campaign
explicitly imports separate assessor predecessors. Neither C nor a library
supplies the guest interpreter, continuation or tail-call mechanism for you.

## Representations and admission

`Vs` owns a copied `CvmProgram`, a `Cvm` with bounded operands/frames/heap, the
execution state, numerical mode, host capability mask, epoch/serial, and invocation
identities. Its representation is intentionally inspectable. The caller owns a
zero-initialized or terminal session and disjoint, live inputs, scratch and result
objects. `vs_start` borrows scratch only while verifying, then owns its image.
Changing the caller's program afterward cannot change execution. Starting a
running or pending session is refused. Failed admission preserves the session
byte-for-byte; scratch is diagnostic and may change. Direct mutation of live
session fields violates this API's discipline; C does not prevent it.

One `vs_step` executes at most one instruction. Fuel is checked and consumed
before fetch; no-fuel refusal leaves IP at the unexecuted instruction. This is
an explicit successor diagnostic policy: the retained `cvm_run` increments its
diagnostic IP before discovering no fuel. Both preserve the public result on
failure. Success publishes a result only at the final RET; intermediate progress,
pending work and failure preserve it. Failure is sticky until a new start.
The operand stack and diagnostic frame/IP/budget fields may already have changed
when an instruction fails. This API promises no general internal rollback.

CheckedV1 uses int64 intermediates for int32 add/subtract/multiply, with range
admission before converting; division first rejects zero and INT32_MIN/-1.
Derive negative division as truncation toward zero, not floor. Execute extrema
and their nearest valid neighbors, recording every push/pop and output sentinel.
Separate TYPE, RANGE, STACK_LIMIT and VERIFY with minimal images. A VM status is
not a C exception or proof of memory safety against a hostile native caller.

The changed numerical contract is explicitly selected WrapV2. Add/subtract/
multiply use modulo2^32 arithmetic. Map a high unsigned result back with
`-1 - (INT32_MAX-distance)` or an equivalent representable expression; do not
depend on an out-of-range signed cast. INT32_MIN/-1 returns INT32_MIN in WrapV2;
zero divisor still fails. The implementation profile requires 32-bit unsigned
int, asserted at translation. Neither changing optimization nor choosing C23
silently changes the selected guest semantics. Published C17/C23 text access
remains separately tracked; no signed-overflow equivalence is claimed.

## Continuation and host ownership

HOST takes an integer argument, checks the invocation budget, charges one unit,
then checks the selected service bit (IDs0–31). A denied service produces
HOST_ERROR without issuing a request. An allowed service creates one pending
`VsRequest` containing epoch, serial, service and copied integer argument. The
session retains its frames, IP and operands; it invokes no blocking C callback.
`vs_take` transfers that request once. Completion requires a pending taken request
and exact epoch/serial; stale, early and duplicate completions preserve state.
Success pushes one integer, failure becomes sticky HOST_ERROR. Completion does
not charge another guest instruction or host invocation. Restart increments the
epoch, preventing an old completion with a reused serial from matching. All
identity counters refuse wrap.

The caller must keep the session alive until the actual provider is quiescent.
This package is a serialized pending/resume mechanism. It does not implement OS
I/O cancellation or prove that an external provider has stopped accessing memory.
The request contains values rather than borrowed stack addresses. A never-finishing
provider leaves the session pending: finite fuel is not a wall-time deadline.
Derive an embedding protocol before composing this with an asynchronous store.

## Function versus invocation

Each descriptor identifies a function. Each live invocation has its own frame,
initialized-local bits, saved IP, operand base and monotonically assigned
invocation identity. CALL moves arguments into fresh callee locals and leaves
caller temporaries below the new base. RET restores that base and pushes one
return value into the caller. A seventeenth simultaneous frame is refused before
constructing it. Direct and mutual recursion both use this explicit frame array;
the interpreter's C call chain does not recurse for guest calls.

Hand-trace sum(5) while the entry function retains 100. The nested sum contributes
15 and the final result is115. Record function and invocation IDs, IP, initialized
local0, operand base and stack height at every call/return. Derive 66 dispatched
instructions from the bytecode, independently of a measured fuel difference.
Change to mutual recursion whose two functions use the same local numbers; prove
the frame identity still separates bindings.

The delayed extension introduces TAIL24, width three, with CALL's target/arity
operands and an immediately following RET in the same function. Its reachable
relative height must be exactly argc. Validate a normalized CALL/RET image with
the predecessor verifier before admission, then execute TAIL by replacing the
current frame while retaining its caller's base. Copy all arguments before
clearing locals. TAIL assigns a new invocation identity without increasing depth.
This explicitly selected API dialect is not an ordinary CVM1 wire image.
Compare a30-step accumulator using ordinary calls (FRAME_LIMIT at16) and tail
calls (result465 at depth2), including mutual tail calls. Tail elimination changes
frame consumption and dispatch counts; it does not make arbitrary recursion free.

## Deliverables and evidence

Submit the implementation, original/changed semantic tables, complete state
transition diagram, ownership argument, call/return trace, independent arithmetic
oracle, failure/fuel boundary dossier and generated-resource account. The public
invocation requires both real predecessor artifacts:

```
python3 labs/runtime_followthrough/execution/run.py --source YOUR/execution.c --verifier YOUR/verifier.c --cvm YOUR/cvm.c --build build/my-execution
```

Run GCC/Clang debug, optimized and sanitizer host profiles. Compare automatic and
caller-owned static sessions by interleaving two disjoint machines; static storage
changes lifetime and C stack pressure, not synchronization or ownership rules.
In a separate host-only probe, instrument actual interpreter function entry/exit
and compare sixteen guest frames with a bounded17-level native C recursion.
Report sizeof frame, operand array, VM/session, compiler stack records and the
observed native span separately. C stack address observations are ABI-specific.

For strict targets use the explicit builder providers and audit each generated
object, final imports, map, sections and stack usage. Original helper code closes
compiler-generated dependencies. Host compiler instrumentation, arithmetic oracle
and packaging are not target dependencies. Native/Arm/PowerPC execution supports
only those named toolchains and bounds; emulation is not board observation.

Hints: first preserve the output sentinel through one step; then separate a
terminal error from suspension; then copy call arguments before changing their
frame; finally derive why both epoch and serial are needed across restart.
The separate assessor package contains withheld variants and semantic mutants.
Original seven-day requirements remain evidence to collect from real learners.

Nystrom's inspected [Call Frames](https://craftinginterpreters.com/calls-and-functions.html#call-frames)
explains invocation-specific local storage and return positions. Here locals have
their own fixed arrays, rather than clox's shared value-stack window. The exact
numerical, pending and TAIL contracts above are original curriculum definitions.
All defective specimens and malformed bytes stay in the original local teaching
VM. Harmless failure/status observations do not establish arbitrary native code
execution or privilege escalation.
