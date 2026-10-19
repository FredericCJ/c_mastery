# T28.11 assessor analysis — withhold from learners

## 11.a

The framing proof relies on disjoint delimiter and data alphabets, exact six
nibbles and final CRC. A new F0 resets the partial count, so every proper prefix
can be discarded. Full frames commit on F1. The cancellation race therefore has
two different outcomes even with identical transmitter-side CANCELLED results.
The 0..8 cut campaign distinguishes them with the actual PuDriver provider.
The polynomial oracle divides the whole message polynomial, independently of
the C byte recurrence. Encoder/decoder roundtrips alone could share a CRC defect.

Admission must reject active state before changing the borrowed array or invoking
submit. Saving/restoring bytes after a BUSY callback does not repair an intervening
DMA read. The terminal callback promise is part of the adapter contract; treating
an arbitrary timeout as terminal can end lifetime too early. PuDriver's local
poll budget is therefore not used. Numeric F439 SRAM admission protects the
selected DMA route but cannot establish object provenance or lifetime.

Reject claims that CRC authenticates, sequence octet alone deduplicates forever,
or cancellation rolls back the receiver. The parser accepts a recomputed CRC for
an attacker-selected synthetic command and accepts replayed valid frames twice.
A proper strengthened protocol must bind identities, sequence lifetime, reset,
durability, acknowledgement loss and authority. Reusing a prior durable protocol
is valid only with an explicit adapter; do not invent exactly-once guarantees.

Rubric: 0 merely compiles; 1 ordinary frame works but ownership/recovery argument
missing; 2 full construction, independent oracle and all boundary cuts with a
correct ownership proof; 3 unseen cancellation/replay variant solved and each
assumption/remaining ambiguity explained. Level3 requires actual independent
learner evidence, not this reference implementation. Original minimum_days applies.

## 11.b

The omitted-flush mutant works with immediate publication but fails the posted
contract. STATUS reads do not flush in this model. Two DATA reads consume two
different bytes; caching the second result cannot restore the lost first byte.
STATUS|ERROR writes ones to unrelated W1C flags. Both are semantic defects even
when a single happy-path value appears plausible.

A flush is not a transaction: CTRL and the first DATA can be delivered before the
second DATA is rejected. Keeping the failed head prevents lost work, but retrying
the service must not append another copy. Phase4 quarantines the operation; explicit
queue resolution can still commit the retained write. No software state assignment
can undo that publication. Timeout after DATA has the same uncertainty. Alternative
representations are valid if they preserve the FIFO prefix and make reuse conditions
explicit. Reject arbitrary-delay repairs and empty mocks that always report ready.

Rubric: 0 fixtures-only stub; 1 immediate-only happy path; 2 effect-faithful model,
single consumption, selective W1C and failure-prefix proof; 3 independently adapts
to posted publication and explains the actual device's visibility mechanism.
The delayed package changes the interconnect assumption, not just queue size.

## 11.c

F439 HAL 1.8.3 HAL_UART_Transmit initializes a tick, waits TXE, writes DR, then waits
TC. It restores gState READY after both success and timeout. Thus READY is a library
admission state, not a proof of line quiescence. The one-byte comparison uses the
same timeout rule, including timeout0 and unsigned wrap. On a failed flag check,
HAL_GetTick may change a modeled flag and the timeout can nevertheless win that
iteration; the independent schedule calculation includes this ordering.

The GET_FLAG and ENABLE_IT guest probes evaluate their supplied flag/interrupt
function expressions twice. Their function bodies use a counter; do not replace
this defined call probe with unsequenced modifications in a scalar macro argument.
Use stable expressions in peripheral macros. Generated volatile layout does not
make arbitrary expressions safe.

The blocking path has no completion callback. UART_EndTransmit_IT publishes READY
before invoking TxCpltCallback. The callback guest immediately starts another
operation and verifies the new BUSY state and next byte survive return from the
old interrupt. No general callback/thread race freedom follows from this serialized
test. A wrapper that cleans up after callback must use generation checks or stage
all old-operation cleanup before dispatch.

A frozen HAL_GetTick with TXE false leaves `(now-start)>timeout` false forever for
a positive finite timeout. The guest's independent 4096-call observer demonstrates
the bounded prefix; the no-return argument follows from the unchanged loop predicate.
HAL_MAX_DELAY removes the time check entirely and is excluded. A loop-count or
watchdog bound is a different contract and must be labeled as such.

Generated headers relocate fields, not their semantics. G474's old F4 offsets name
CR1/CR2, so a stale-map mutant must be rejected even if it happens not to crash.
Require schema provenance, generator determinism, exact offsets, unchanged core
and sentinels. Accept alternative generation syntax. Do not accept a transplanted
F4 HAL with renamed types as a G4 port. The G4 full startup/clock/pin/FIFO/errata
argument remains under the explicit full-reference dependency.

Rubric: 0 object links; 1 unequal one-write benchmark; 2 equal completion/failure
contracts, actual vendor execution, object closure, selected cost explanation,
timeout/callback counterexamples and map transfer; 3 independent changed-device
analysis identifies which arguments survive and which need new evidence. No
hardware latency conclusion is supported by guest size or finite model traces.

Graduated assessor hints: ask for the last possible buffer read; then separate
memory, wire and receiver events; finally reveal only the failed boundary (F1,
posted head, TC or callback entry), never the solution. Keep ordinary and delayed
submissions separate and preserve the original task text and minimum delay.

The changed-device guest executes actual G4 HAL at commit a6001282dfacfff57e9710250f15e4333b578865
with CMSIS device commit626ee412334a5ed2e5b320af5a8d77d69f03a558. The selected
transmit code uses TDR and additionally checks BUSY_TX before decrementing the
remaining count, returning HAL_ERROR when aborted. That branch is absent from the
selected F4 implementation; equivalent serialized successful/timeout schedules do
not prove arbitrary concurrent-abort equivalence. The source audit must include
this difference, not just rename DR. Full G4 initialization remains separate.
