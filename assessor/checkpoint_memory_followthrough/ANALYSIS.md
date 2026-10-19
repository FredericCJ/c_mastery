# P1 criterion-specific assessor analysis

The retained nine criteria and their exact changed-condition text are authoritative.
The following is a reference solution argument, not learner evidence. Use the
public contract for premises and numeric input; never infer mastery from this run.

## t1401c

`CPMPool.objects` is a declared typed array; the arena is a declared byte array.
`tests/memory.c` copies int123 into and out of arena bytes, and reads the real int.
The allocated variant `cpx_init` initializes actual records/nodes through typed
lvalues; teardown releases both. An alignment-only cast is not a valid repair for
declared character storage. Accept a separate typed pool or serialization through
memcpy, not an arbitrary pointer reinterpretation. Require the learner to identify
which lifetime/effective-type rule changes with allocated storage.

## t1405c

The literal FIFO trace yields popped11,22 then33,44,66,77. Prefix admission with
two free slots admits exactly two; staged atomic failure commits none. The staged
pool's injection counter also rolls back; repeating the same failure is intended.
The independent eight-action model stores values in a shifting array, not handles
or ring indices. It covers6561 traces; retirement reduces capacity on longer traces
and is tested separately. The linked adapter acquires and pins before publishing,
and releases its unqueued record on each of four node failures. Mutant2 omits that
release and fails live-state accounting, not merely compilation.

## t1602a

The align/header derivation is in the learner contract. Required results include
8/16/refused32 and unchanged25, plus exact fit24 at32 and refusal at31. The latter
rejects mutant5's premature used-offset update. Base alignment8 with required16
must refuse without modifying a previously valid arena. No pointer dereference or
bit-mask assertion substitutes for the caller's actual alignment promise.

## t1804b

Queue and callback each own a pin. Pop/unpin leaves one; cancellation does not end
it. `cpx_lease_complete` accepts only the matching cookie and quiescent event,
then reset succeeds. Nonquiescent/stale/duplicate completions cannot decrement.
Mutant7 resets despite pins and must fail before any expired pointer is used.
This is a serialized ownership protocol, not proof of thread-safe synchronization.
Accept a copying alternative only if its destination lifetime and copy cost are
accounted and cancellation cannot publish into a dead destination.

## t1804c

Lookup validates before exposing a pointer. Generations1–3 are consumed and the
slot retired; mutant8 wraps and admits an old identity. Relocation allocates and
copies while old storage is live, then commits; allocation failure leaves old
storage/handles intact. Pins forbid moving while a raw borrow is authorized.
The tests deliberately never compare an expired pointer after success. Handles
remain scoped to the same owner object; using them against another store is a
missing precondition, not a generation guarantee.

## t1301c

Actual M4 images compile the retained core plus public checks and original runtime.
The driver has no recursion or unknown callbacks and masks IRQs. Summing every live
C frame once plus1024 bytes for assembly/exception allowance is conservative for
this call graph and is compared with the32KiB reservation; no board watermark is
inferred. All five object imports and final image providers are inspected. Compiler
memory helpers resolve only to the original `assessor/execution/support.c` routines.
The hosted effect probe records four comparator/logger calls, with four malloc/free
pairs or four poll waits in the deliberately false no-effect controls. Clean code
has neither. Allocator/registration effects in the transfer ledger are separate.
Native LTO matrices preserve `cpm_prefix` and the caller probe; stack protection
introduces a dependency and the resolved teaching malloc control has none unresolved
but still violates the strict effect contract. A call graph with unknown callbacks
is not closed by `nm -u`. Accept a different sound audit with inspected providers;
reject a whitelist based only on unresolved names or a blanket no-libc claim.

## t1614b

Init failures release0/1/2 acquired backings; successful registration makes later
close responsible for quiescence. Growth has two allocation cuts: the first leaves
all unchanged; the second releases its temporary records. The independent ledger
counts live extents and bytes, alignment and nonoverlap, preserving old contents
and owner bytes before commit. Four node cuts accept0/1/2/3 and preserve FIFO values.
Halving capacity to2 retains the same four input records and admits two. Exact-fit
and one-short are independent arena boundary cases. Mutant1 tears down early and
mutant2 leaks an unqueued live record; both are defined logical rejections.

## t1614c

With alignment1 padding is zero. Arena payload/header totals per iteration are
4/12,48/9,29/12; retained used is16,57,41. Pool payloads are4,64,29; payload-slot
slack is60,0,35, separately from typed metadata and nodes. Both retain allocated
backing after logical release, while the pool frees slots for reuse. The arena
retains used bytes until reset. Independent literal checksum totals are10,456/640,
251. Compare31 samples/configuration/compiler, not one minimum. Instrumentation
scans live state and adds checks, so its cost belongs in the measured interval.
Only distributions0 and2 have equivalent admitted payload work. Even there the
contracts and different backing/control footprints must accompany any timing claim.
Alternative measurement intervals are valid if explicitly partitioned and replayed
for both backends. Reject timing claims based on unequal silently dropped work.

## t2104b

Three init stages publish the owner only after registration. The contract requires
failed registration to retain nothing; a register-then-fail API needs an expanded
state and cannot be substituted. Close waits for pins, requests unregister once,
then stays pending. A matching quiescent acknowledgement disposes nodes before
records and a repeated close does not repeat releases. The actual OwOwner adapter
imports its nonfailing/nonreentrant release contract, retains three owned blocks,
and records extra control bytes/callbacks. C provides neither move-only types nor
automatic destruction. A concurrently executing callback requires synchronization
and a trustworthy unregister acknowledgement supplied by the external backend;
the serialized reference does not manufacture such a backend guarantee.

## Evidence limits and plausible wrong solutions

All reference and defined semantic mutants must compile before execution. A crash
or sanitizer report is not the intended rejection for these ownership controls.
Leaving a learner stub unimplemented is correct author behavior; accepting it is a
validator defect. Raw target disassembly/map are evidence of those builds only.
Reject conclusions that cancellation means completion, pointer equality survives
free, padding proves type, empty imports imply no effects, or maximum latency proves
a bound. A valid solution may use copying or reservation instead of pins/staging,
provided every changed ownership, failure and resource contract is assessed.
