# Register transactions and GPIO ownership

This required PR-T28-01 successor addresses T28.01, .02 and .04, retaining all
nine original criteria and nine delayed tasks. Read their exact task/rubric and
the source register before construction. The prerequisite artifacts are the
T27 identity/image dossier and the FW-03 GPIO interface; use `struct fw_io` rather than
inventing an incompatible port. C17 is the baseline. Hosted tests use libc;
restricted target objects/images must have no external runtime imports. GNU
volatile MMIO conventions, Arm ABI, and F439 device rules are separate contracts.
Host oracles and a Linux PPC64 verifier are not target dependencies.

## Guided derivation

For W1C status15, acknowledging5 should leave10. A read-modify-write that reads15,
computes15&~5=10 and writes10 instead acknowledges the other flags. If bit4
arrives between read/write, the wrong result is21; a direct acknowledgment5
preserves26. Derive both traces before coding. Reading DATA consumes an item:
debugger-like repeated inspection is itself a state transition. A read-to-clear
register needs one captured snapshot passed to diagnostics, not a second read.

The synthetic peripheral is explicitly distinct from STM32 GPIO:

|Offset|Register|Width|Reset|Effects and constraints|
|---|---|---|---|---|
|0|CONTROL|32|0|Read/write, revision1 writable mask3; revision2 mask7. Other bits reject.|
|4|STATUS|32|0|Read preserves; write acknowledges exactly selected low5 bits. Bit0 is TX-ready.|
|8|DATA|8 or32|empty|Read consumes one of at most4 queued bytes; write requires CONTROL.enable and TX-ready, then clears ready.|
|12|RTC|32|0|One read returns then clears all bits; writes reject.|
|16|ODR|32|0|Read/write low16 bits, ordinary assignment.|
|20|BSRR|32|0|Write low16 set/high16 clear; set wins overlap. Reads reject in this stricter model.|

`pm_access` accepts a caller-owned `struct pm_model` initialized to zero except revision1/2
and explicitly injected status/RX/RTC events. Invalid width/offset/mask/direction,
empty read, unavailable transmit, exhausted transmit counter, or full64-entry
access log rejects without changing the model or supplied value. Accepted reads
publish the value; accepted operations append one log entry. Log exhaustion
stops observation and effects together. No allocation, bounded storage, O(1)
except at most3 byte moves on read. This log contract is not real hardware trace.

`pm_begin` requires a non-running request and positive wait bound. `pm_step`
reads status once and publishes one byte only when ready, including the last
allowed poll. Each unsuccessful poll consumes one budget unit; terminal success
or timeout rejects further steps. The interrupt adapter below preserves the readiness/publication state graph
while giving timeout a separately declared clock contract. A real ISR still needs its own priority,
acknowledgment and synchronization contract; this model does not supply them.

## GPIO and electrical distinctions

`pm_gpio(struct fw_io,set,clear)` rejects bits outside16 and overlapping masks. Success
performs exactly one FW_GPIO write with set|(clear<<16), including an empty mask,
and no read. The stronger service overlap policy differs from the selected
hardware's set-priority rule. With ODR3, interrupt-set bit3, then foreground-set
bit2/clear bit0, an ODR read/modify/write loses the interrupt and yields6; the
BSRR transaction preserves14. A later whole-ODR writer can still overwrite that
result. Two ports require two transactions; BSRR provides no cross-port atomicity.

The target adapter uses `struct pm_gpio` offset assertions and explicit32-bit volatile
accesses. RM0090 Rev21 §8.4 has a broad access-width statement, but §8.4.5 restricts
IDR reads to word mode and §8.4.8 restricts the lock sequence. Do not infer every
width from a field type or the overview. The supplied target stub must implement
IDR read and one BSRR write; clock enabling, pin configuration and access authority
are caller preconditions. Never cast host RAM to claim peripheral semantics.

Complete a pin ledger with mode, AF, push-pull/open-drain, pull, speed, package
pin, board routing and external circuit owner. UM1974 Rev11 p25 maps LD1 to PB0
or PA5 according to SB120/SB119, LD2 to PB7 and LD3 to PB14. Actual assembly must
be read before use. The inspected DS9484 Rev14 Table10 maps PB0 to LQFP144 pin46
and LQFP100 pin35; changing package invalidates physical routing evidence, even
when the software port-bit name stays constant. Do not repurpose debug pins or
connect an unknown external circuit. A physical measurement packet must identify
board/assembly, selected bridge state, supply, loading, image and raw capture.

`pm_line` models two drivers only: the local output is push-pull/open-drain, the
peer can pull low, and an optional external pull raises a released line. Return
low/high/floating/contention as0/1/2/3. It has no analog RC/voltage/current model;
an internal pull is not evidence that an external bus's rise-time requirement is
met. Unknown flags reject. Derive all16 combinations and distinguish release
from actively driving high.

## Debounce as a changed-assumption exercise

`pm_filter_init` starts logical level0 and zero events. Threshold must be positive.
With duration0, accept a new level after threshold consecutive samples; saturate
the run counter at threshold. Sequence0,1,0,1,1,1 with threshold3 gives exactly
one accepted transition at the final sample. An interrupt edge is just a sample,
not an already-debounced event.

With duration>0, require duration and max_gap in1..2^31-1. Timestamps are uint32
ticks with successive deltas below2^31. A backward/ambiguous timestamp rejects.
Track the candidate's first sample; a sample gap exceeding max_gap restarts the
candidate interval even if the sampled level matches. Only elapsed duration
qualifies; repeated timestamps cannot manufacture duration. Across wrap, use
bounded modular elapsed values. The environment must sample frequently enough
that candidate age stays below2^31; elapsed arithmetic is not an unbounded clock.
Event counter exhaustion rejects the entire operation without changing state or
the changed flag. An accepted sampled interval does not prove the contact never
bounced between observations. State the sampling/electrical assumption explicitly.

## Evidence and assessment

Run `python run.py --build PATH`; the initial learner stub compiles and fails.
An assessor may select an explicit external `--source` without exporting it.
Submit implementations, source/pin tables, predicted and observed access logs,
independent bit-set/suffix derivations, target offsets/disassembly and runtime
closure. O0 and optimized GCC/Clang, sanitizers and big-endian model execution
answer different questions. Inspect actual word loads/stores and linked images;
sanitizer success does not establish device ordering or physical safety.

For T28.02.b, classify compiler observability, atomic bus transaction, CPU/device
ordering and ownership publication separately. A volatile flag shared by ordinary
threads does not create a happens-before edge for a non-atomic payload; the
data-race example is reasoned, not run with a portable golden value. A cacheable
cross-core variant adds cache-line ownership/maintenance and platform-specific
notification. ISO C atomics alone do not automatically operate on MMIO safely.

Hints: first draw one transaction and its owner; next derive the smallest bad
interleaving or sampled suffix; finally compare the specific register's width
and side effects, not a neighboring register. Independent transfer retains the
original seven-day delay. Level2 requires a causal explanation and a compiled
negative control for the relevant mechanism; author test runs are not learner
level3. Board timing, non-author review and actual learner evidence remain pending.

## Interrupt adapter changed condition

Implement `pm_irq_begin` and `pm_irq_event` with an initially zeroed, single-owner
`struct pm_interrupt`. The model controller must already be enabled. Arming sets its IRQ
bit, then rechecks readiness: data ready before arming must not wait for an edge
that already happened. Tick limit lies in1..2^31-1. A serialized dispatcher calls
`event` for IRQ notifications and for the independent deadline timer; it must
continue to deliver the deadline even if no peripheral IRQ arrives. Spurious
notifications do not spend a polling budget. One status read per event controls
publication; ready at the deadline wins, otherwise deadline exhaustion returns
-2 and disarms. Success publishes one byte and disarms; duplicate/late events
reject. Backward or half-range timestamp ambiguity rejects without releasing
ownership. The log must have room for the complete event (up to three accesses)
before effects. Model-capacity failure leaves the request armed for explicit
recovery, never silently reports cancellation. Begin reserves four access slots
before enabling, so its immediate recheck is observable.

This is a synthetic interrupt/deadline adapter, not an STM32 ISR or a scheduling
latency guarantee. The real IRQ delivery/priority exercise is T29. Preserve the
separate clock contract when transferring from a poll-count limit; they are not
the same physical duration. Test pre-arm readiness, spurious IRQ, readiness on
the final tick, timeout without any IRQ and stale events after disarm.
