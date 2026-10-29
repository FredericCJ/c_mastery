# Timestamp capture qualification and analysis protocol

Applies to T29.09/.10 and the unchanged PR-T29-05 delayed variants. This is an
author-written physical experiment package, not a report of performed board work.
Use a dedicated documented teaching board, synthetic events and local instruments.
No host security change, deployed equipment, external target or firmware flashing
is authorized by the curriculum-authoring task. A later operator must qualify the
exact board/revision, schematic and clock/interrupt/memory route before running.

Prerequisites: the T27 bring-up record, T28 DMA route/cache protocol, T29 frame,
handoff and lease protocols, a known-good event generator and analyzer. Reuse the
original ownership providers; obtain an externally retained reset supervisor if
testing software-only reset while a device remains active. The supplied MPS2
target is an executable reference for an emulator, not a board image. Its CMSDK
32-bit count must not be relabeled a latched64-bit device counter. A board adapter
must implement `struct xk_io` from actual documented register semantics and state whether
it uses a latch, coupled timer cascade or interrupt-safe software counter. An
undocumented cascade cannot use the hi–lo–hi proof by assumption.

Record MCU/board revision, exact reference manual/errata/schematic passages, power
and oscillator configuration, timer source/prescaler, interrupt route/priority,
memory and DMA placement, compiler/flags/map/disassembly/ELF SHA256 and runtime
closure. Document voltage compatibility and use common reference ground; instrument
input must not drive a board output. Choose analyzer sample rate from the shortest
edge/interval of interest and document threshold, probe loading, channel skew,
sample quantization and synchronization error. A scope's displayed precision is
not its accuracy. Save raw captures before computing summaries.

Instrument separate channels for generated external event E, ISR entry I, and
deferred service S; use an additional correlation channel or bounded sequence
encoding to match IDs. Capture before/after counter reads in software. Do not print
inside the measured critical section. If GPIO writes are buffered, state whether
the marker denotes instruction issue or visible pad transition and include the
documented barrier/visibility requirement. Apply the lease/handoff marker-cost
paired comparison to the same workload and record any changed interrupt masking.

Run empty, singleton, queue-full and sustained bounded bursts; delayed IRQ/service;
counter rollover close to sampling; DMA cancel-first/complete-first/stop-first;
stale generation; and the documented reset-domain transition. Sequence accounting
must distinguish source events, accepted records, explicit drops and outstanding
records. A coherent timestamp is not evidence of zero drops. Advance sequence
epochs before ambiguity/exhaustion and preserve independent source counts.

For two clocks, capture at least two independently bracketed sync observations.
One-way delay must be bounded or offset remains underdetermined. Declare the
anchor interval, integer quantum (or justify a rational extension), rate-error
bound and calibration epoch. Intersect a second observation with the predicted
interval to check consistency. Two successful sync points do not prove the rate
bound for all future time. Recalibrate after a clock/prescaler/reset change.

The JSON capture contains external files with SHA256, measured event intervals and
acquisition tick brackets. The host checker recomputes outward interval bounds,
event-delay bounds and conservation; it verifies raw file identities when asked
to check an observed capture. It does not authenticate the operator, oscilloscope,
hardware or truthfulness of the declared bounds. Synthetic self-tests are labeled
synthetic and cannot supply physical evidence.

For each row retain `[event_lower,event_upper]` from analyzer uncertainty, sampled
tick bracket `[begin,end]`, and event-to-sample delay bounds. Compute calibrated
sample and event intervals independently. Require nonempty intersection with the
analyzer event interval; disjoint intervals fail the calibration/delay hypothesis.
An intersection is consistency, not proof that bounds are globally sound. Compare
event order only when intervals are disjoint or a separate causal proof applies.
Record the measured interval width and missing-event count without replacing them
with a mean. Save all outliers and raw capture IDs.

Acceptance: exact qualified identities; all named schedules represented; raw files
hash-verified; sequence conservation; valid monotonic brackets and checked range;
calibration/observation consistency; explicit marker quantity and uncertainty;
ownership/reset conclusions justified from the earlier protocols. A required
physical run is pending until an operator supplies this evidence. Missing board
information blocks only that qualification, not the portable implementation.

Delayed marker variant moves I to S: change the reported quantity to external
event-to-deferred-service, include backlog and scheduler delay, and compare under
the same workload. Delayed revision/placement variant repeats route, errata,
clock, GPIO and cache qualification and generated closure; do not carry the old
physical claim across a changed DMA-inaccessible memory placement.
