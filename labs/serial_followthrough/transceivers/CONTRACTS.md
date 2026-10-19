# Selected transceiver boundaries and evidence adapters, revision 1

This required successor implements the six original PR-T38-06/07 criteria and
their exact seven-day V1 tasks. The parent briefs, their original AX identifiers
and the legacy assessment shared contract remain authoritative. The additions
provide selected device contracts, code, test inputs and a reproducible capture
analysis procedure; they do not replace the required physical observations.

## Environment and prerequisites

Use the repository's ISO C17 profile E: fixed caller storage, no heap, operating
system, UART/transceiver library or floating point in the studied component.
The core uses exact-width integers, live nonoverlapping objects and terminating
single-owner calls. C neither checks the electrical envelope nor proves the
provenance of a capture. Host stdio, Python integer oracles and sanitizers are
verification dependencies. The MPS2 Cortex-M4 image executes the same code on
synthetic samples; it does not model analog transceiver physics.

Reuse actual `UaTx`, `ua_format`, `PuDriver` and `sd_uart` from the PR-T38-01
and T28 UART producers. Their original lifetime, clock-generation and completion
contracts apply. The author campaign links those actual implementations;
learner exports contain only prerequisite headers/stubs. Acquire the prepared
dedicated circuit before the physical phase. Prerequisite knowledge includes
8N1 framing, direction roles, threshold versus hysteresis, differential/common
mode, integer overflow, sample alignment and transmitter-complete ownership.

Inspected sources are R-TR-MAX, R-TR-DRIVER and R-TR-RECEIVER in the successor
resource register. The exact selected revisions matter: older AM26LV32E text
claimed more fail-safe cases than the inspected current revision. Full TIA
standards and arbitrary third-party adapters are not claimed inspected.

## Problem and supplied artifacts

Implement `transceivers.h` in `learner.c`, with public `tests.c`, native and
explicit target build harnesses. Inputs include every integer millivolt in the
declared threshold sweeps, every 8-bit UART value, all 27 three-contact wiring
assignments, line-column omission, short/open/disabled differential receivers,
late transmitter completion, finite token exhaustion and raw two-line transition
samples. Outputs distinguish invalid, incomplete, mismatched and matching
evidence. A GPIO-level decode alone must not satisfy a line-side claim.

Produce a source-backed connection ledger for our original three-contact
teaching adapter, not an inferred DB9 pinout. On both labeled connectors contact
1 is TD, 2 RD and 3 reference. DTE drives TD; DCE drives RD. The straight cable
joins like contact numbers. The independently labeled DTE-to-DTE null-modem
variant crosses contacts 1 and 2 and joins 3. Handshake conductors do not exist
in this adapter. Check the actual built connector against this drawing before
using it; a type name does not inspect a cable.

## Necessity and mechanisms

The synthetic ±3 V classification is a teaching policy, not a MAX3232 threshold
measurement. Open-input idle can look like a healthy idle link. A second
receiver adds a load, not a second transmitter permission or address field.
Stable inverted polarity, multiple threshold crossings and excessive common
mode require different investigations. The supplied samples distinguish these
observables without pretending that one waveform proves its physical cause.

## Operations, invariants and failure semantics

`tr_sample` validates canonical booleans/device tags before publication. The
RS-232 fixture uses ±3000 mV, center unknown. For the selected powered 3.3 V
MAX3232 receiver use the guaranteed threshold bounds 600/2400 mV, inverted
logic, within ±25000 mV; intermediate values are unknown without a measured
history. Its open receiver returns high. Do not substitute the typical
threshold or claim a guaranteed hysteresis width. This part has no modeled
receiver-enable pin; disabled RS-232 requests are invalid.

Differential decisions use the actual signed A−B and twice the common mode
A+B in int64_t, avoiding overflow before checking the selected ±7000 mV
envelopes. Enabled connected inputs use ±200 mV, central region unknown.
The optional sign adapter reverses only the differential interpretation.
The generic disconnected fixture is unknown; powered AM26LV32E open input is
high; disabled output is high impedance. A connected short is unknown. These
states are distinct from valid zero and from an invalid electrical envelope.
The operating-supply and temperature premises are established outside the
sample API; a numerical sample is not a circuit admission certificate.

`tr_connections` accepts exactly three unique contacts per side, one common
reference pair and two output-to-input pairs. Repeated/absent contacts, two
outputs, two inputs and data-to-reference fail. `tr_topology` enforces the
original one-driver, 1..10-receiver, far-end-termination and reference policy.
The scoped count is not a universal device loading rule.

`tr_232_circuit` is deliberately narrow: selected MAX3232I, 3.0..3.6 V,
−40..85 °C, four nominal 100 nF pump capacitors and 100 nF bypass, baud 1..115200,
declared 3 kΩ/1000 pF test load. It admits that configuration only; other loads
require a revised source-backed contract and fresh measurements. Component
tolerances, dielectric/rating, probe load and effective capacitance belong in
the equipment ledger, not invented from the nominal integer values. The
datasheet's 250 kbit/s typical rate is not this laboratory's guaranteed bound.

`tr_capture` examines 1..160 strictly time-ordered settled bit-center samples.
Required columns are transmitted logic, line voltage and returned logic; the
differential case also requires both line potentials. Malformed/incomplete
input preserves the result. Valid input publishes counts and the first bad
index (`SIZE_MAX` when none), returning MISMATCH for unknown or inconsistent
samples. It does not discover bit centers, decode a complete waveform, establish
settling time or infer which instrument produced the data.

`tr_edge` examines 2..32 strictly ordered two-line samples in one declared
transition window. It reports known-state crossings, opposite-polarity samples,
central samples and outside-envelope samples. Unknown center samples do not
create transitions; an invalid envelope breaks continuity. Repeated crossings
are compatible with reflection/noise, not unique proof of either. A changed
sign adapter can explain stable complement but does not repair a cable.

The separate `TrBus` variant is an original three-peer half-duplex scheduling
policy, not a physical RS-485 device. A stable zeroed control grants one peer a
strictly increasing, nonwrapping token. No grant while active or in turnaround;
only matching peer/token plus true transmitter completion starts the checked
disable-plus-guard interval. Equality at `ready_at` permits release, not before.
Overflow, backwards time, stale notices and forged boolean values fail without
state mutation. Missing completion retains ownership indefinitely. The
coordinator is trusted and serialized; a malicious peer can ignore it. A real
RS-485 extension still needs compliant parts, loading, bias/termination and
disabled/open/idle guarantees. This policy alone establishes none of those.

## Reasoning and deliverables

- D1: Derive each threshold interval, current-source fail-safe distinction and
  connector direction relation before running tests. Explain the plausible
  errors “typical equals guaranteed”, “unknown equals zero” and “two receivers
  authorize two active drivers”.
- D2: Supply the C implementation and actual prerequisite composition, invariant
  and ownership argument. Show all rejection paths and publication points.
- D3: Retain raw compiler/output/target/map identities, independent arithmetic
  comparisons, first failing mutant and repaired result. Add a fresh falsifier
  which is not a parameter substitution.
- D4: Complete BOARD_CAPTURE.md for the selected circuit, including missing
  evidence honestly. Preserve original raw traces and analysis transformations.
- D5: Perform the exact original V1 after at least seven days without the
  assessor analysis. Change receiver assumptions, wiring roles, cable/load,
  sign convention, multipoint exclusion or waveform interpretation as assigned.

Pure classifiers take constant space/time; wiring is bounded by three records,
capture processing is O(n) with constant auxiliary storage and a hard 160-sample
limit, edge analysis a hard 32-sample limit. `TrBus` has no queue, fairness,
timeout recovery or crash recovery. Measure object/code/stack costs separately
from physical link latency; emulator elapsed time is not transceiver timing.

## Verification and assessor rubric

Run six public selectors and source-bound positive/negative campaigns. Threshold
sweeps must produce 50001 selected RS-232 decisions and 28002 differential
decisions under both sign conventions. All 256 byte frames must compose with
the actual UART formatter; the 16-byte service must complete two producer
chunks before releasing its borrow. Reject missing line columns even when
returned bytes match. Diagnose the supplied stable-swapped, recrossing and
common-mode-excursion samples without claiming a measured physical cause.

Each original objective has its own criterion and delayed task; passing another
selector cannot substitute for it. Level 0 is a wrong relation/unsafe envelope
or inflated evidence; level 1 repeats the visible result; level 2 derives the
premises, implements the relation and rejects the actual counterexample;
level 3 independently transfers under the original changed condition with
bounded evidence. A passing finite campaign is author evidence, not mastery.
Separate private technical analysis is `assessor/serial_followthrough/transceivers/ANALYSIS.md`.

## Graduated hints

First mark where each voltage is measured and who drives each conductor.
After recording that attempt, split unknown, disabled and disconnected cases;
do not collapse them into false. Finally compare the measured interval with
the guaranteed extrema table and inspect the first ownership or evidence-column
transition which differs. Consult the separate assessor analysis only after
sealing the prediction and changed-condition attempt.

## Security and validation limits

All adversarial inputs target original local arrays, connector descriptions and
teaching schedules with synthetic data. No external bus or deployed equipment
is accessed. Physical work uses dedicated prepared hardware and appropriate
measurement range; this authoring task does not authorize wiring, flashing or
host changes. Contention examples remain synthetic. The source-bound report
records actually executed native/Thumb checks; physical, independent learner
and external-review evidence remain separate and not-run until supplied.
