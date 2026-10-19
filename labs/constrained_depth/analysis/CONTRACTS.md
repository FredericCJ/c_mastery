# T30.06/T30.10: a conditional response bound and its evidence boundary

This additive contract supports all six original criteria in PR-T30-04 and their
six named delayed variants. The original project, fixture, rubric and historical
status remain authoritative records. This package adds reference implementation
and executable assessment support; it records no learner completion.

## Identity, objectives and families

Assess AX-PR-T30-04-T30.06.a/b/c and AX-PR-T30-04-T30.10.a/b/c, including each
original `-DELAYED` child. Families A/B/C/D/F/H/I/J are relevant. The mechanisms
are recurrence admission, blocking versus deadlock, arrival assumptions, typed
evidence, generated dependencies and observer effects. A finite successful trace
is evidence about that run; it is not a proof of schedulability or WCET.

## Prerequisites and actual imports

Bring the original PR-T30-01 release model, PR-T30-03 chronology/latency work and
PR-T30-04 fixture. Complete the actual `rta_response` producer declared in
`labs/realtime_depth/response/response.h`, the CMX mutex/scheduler interface in
`labs/execution/include/cmx.h`, and the ceiling adapter in
`labs/realtime_depth/ceiling/ceiling.h`. The author campaign explicitly links
these reference providers. Learner builds must name their own completed sources;
the export supplies headers/contracts/harnesses, never assessor implementations.

The Cortex-M4 extension explicitly imports the T11 original division runtime:
`runtime64_arm.c`, `aeabi_uldivmod.S`, `divide.c`, `free.h`, and the selected
original byte-helper provider. The runtime's direct-ABI divide-by-zero hook does
not make a C division by zero defined. Every C divisor here is positive.

## Environment and dependency profile

Host analysis uses ISO C17 syntax, GCC/Clang debug/release and ASan+UBSan. Exact
published C17 text remains an access gap; no C11 draft is relabeled C17.
Python unbounded integers and process supervision are host verification tools.
CMX operations are externally serialized; this workload is a deterministic
logical-time driver, not an ISR or native-thread measurement.

The separate original guest uses Cortex-M4 base AAPCS, a strict no-library
link, and QEMU's `mps2-an386` model with its declared 25 MHz timer clock. GCC and
Clang target translations have separately recorded flags. Compiler-generated
memory calls and 64-bit division are explicit dependencies, audited in the final
map, symbols and assembly. No target allocator, host libc or libgcc is permitted.
GDB uses an exclusive temporary UNIX socket, no network endpoint. Only the
owned disposable QEMU instance is started, interrupted and terminated.

## Problem, inputs and outputs

Implement `analysis.c` and `target.c` from the public declarations. `CAConfig`
identifies an image, clock, interrupt/cache/probe policy and clock resolution.
The all-zero image field is not a certificate; callers supply the real digest.
The type compares identities; it does not authenticate their provenance.

For tasks `(C,T,D)=(1,4,4),(2,5,5)` with zero blocking, output response estimates
1 and 3 and utilization 13/20. With target blocking 1, output 4. Preserve the
actual analyzer trace and status: converged, exceeded, and iteration limit are
different outcomes. Zero periods, bad indices, arithmetic overflow and unsupported
premises cannot publish a success certificate. Refusals leave both outputs intact.

`replay.c` supplies 1,152 cost/period/blocking combinations and a 20-unit release
and completion trace. `oracle.py` reconstructs them independently. The retained
ceiling workload supplies low/high/medium arrivals, four units of low-owner work,
one high unit and three medium units. Run no-protocol, inheritance and immediate
ceiling modes under identical work. `cycle.c` supplies a two-mutex opposite-order
case. `driver.c` is an intentionally endless local pipe wait, supervised and
reaped by the host; its bounded observation is an unresolved model branch.

The target computes `UINT32_MAX/3`, copies 32 input bytes 0..31 and records their
sum 496. The wide variant also checks `UINT64_MAX/3`. Compare the common 32-bit
input operation's generated path before and after selecting 64-bit arithmetic;
the extra wide boundary check is a separate correctness workload, not an equal-
cost benchmark. A 25,000- or 12,500-count CMSDK timer generates actual guest
interrupts. The host records RAM outputs, timer registers, interrupt counts,
image/configuration, host timestamps, read overhead and the debugger transcript.

## Why these exercises are necessary

Utilization alone is not a response certificate. Priority inheritance cannot
remove a resource cycle. A driver wait, self-suspension, uncharged interrupt or
unknown cost invalidates the selected recurrence's premises. An observed maximum
does not become a model bound by being copied into another column. A wider C
integer operation can introduce a compiler helper, stack frame and new path.
Stopping a simulated processor also changes its virtual peripheral progress;
that observation does not specify a physical board's debug-freeze policy.

## Required reasoning and invariants

Admit only one CPU, fixed preemptive priority, justified execution/blocking
bounds, no self-suspension, bounded drivers, charged interrupts and the supplied
priority order. This wrapper deliberately excludes release jitter and deadlines
greater than periods; the imported analyzer's broader arithmetic does not prove
those models. The admission flags are claims to justify, not facts discovered by
the program. Costs and blocking use one consistent abstract time unit.

For independent periodic tasks or sporadic releases separated by at least T,
derive the half-open-interval arrival count `ceil(x/T)` for positive x, under the
stated critical-instant/no-extra-carry-in premise. Iterate
`R_next=C_i+O_i+B_i+sum ceil(R/T_h)*(C_h+O_h)` from the target base cost. Explain
monotonicity and why a fixed point within the constrained deadline is conditional
on the arrival and interference argument. Reject arbitrary burst arrivals; a
finite trace cannot establish minimum separation for all future releases.

Maintain exact tagged evidence kinds: requirement, model bound, observed mean,
observed maximum. Never compare quantities with different units or workloads.
The requirement 5 and bound 3 in the stipulated logical workload are separate
from measured host debugger-read nanoseconds. Altering image, clock, interrupt,
cache, probe or resolution identity invalidates reuse of the old measurement
record even if its numbers look plausible. This is a conservative identity gate,
not a proof that all equal-identity runs are equivalent.

Inputs and outputs are disjoint live objects. No callbacks, allocation or
retained ownership occur in the admission/evidence core. Admission scans at most
16 tasks and delegates at most 128 recurrence iterations; its result includes
the provider's fixed-size trace. The debugger harness owns its process/socket
until both are closed; a timeout is not an intended semantic failure.

## Implementation and experimental deliverables

Submit the admission/evidence implementation, the actual linked analyzer and
CMX/ceiling adapters, recurrence derivation, complete release/finish trace,
low/high/medium and opposite-order explanations, unsupported-driver ledger,
four-column evidence table with units/provenance, and all changed-condition work.
Include the target ELF/map/nm/disassembly/.su files and exact helper origins.
List manual assembly stack frames separately from compiler `.su` records; the
division ABI wrapper reserves 40 bytes before its C call. Do not sum local
frames into an asserted whole-program stack bound without a complete call chain.

Retain actual raw debugger observations and calibration overhead. The first
sample is warm-up/setup, not an uninstrumented timing run. Two 60 ms host-run
intervals separated by 80 ms halted intervals demonstrate observed selected
virtual behavior. Host scheduling changes those durations; compare recorded
timestamps, not requested sleeps. The GDB `m` packet does not universally promise
MMIO access width: this harness's aligned four-byte reads are tested against the
selected QEMU model, never exported as a hardware debugger guarantee.

## Verification and independent expected results

Run all public cases under six host profiles. Compare every replay row against
Python integer recurrence, including zero base, exceeded deadline and blocking.
Reject deliberate controls that omit premises, relabel LIMIT as converged,
ignore cache/probe identity, or relabel an observed maximum as a model bound.
The unsolved scaffold must compile and fail an explicit semantic assertion.
ASan/UBSan failure, compile failure or a crash cannot count as a detected mutant.

The actual imported ceiling workload must report high completion 8 without a
protocol and 5 under inheritance/immediate ceiling, with direct waits 6/3/0.
Those are logical units with zero transition overhead. The proposed second
wait-for edge in `cycle.c` would close a cycle; actual CMX returns DEADLOCK
before admitting it. This is a distinct cycle-detection policy, not an effect of
the priority boost. Cleanup must still release both mutexes.

Target builds must have no unresolved symbols or forbidden runtime archives.
The retained 32-bit path contains UDIV and no retained division helper; the wide
path contains the explicitly imported `__aeabi_uldivmod` and original limb
division. A deliberately omitted wide runtime must fail linking on that symbol;
classify this as a dependency control, not a runtime exploit or semantic mutant.
For each executed image, verify ready=1, quotient1431655765 and fault sum496.
Timer interrupts progress while resumed and timer VALUE/count remain unchanged
between stopped samples. The disabled-timer control must reach ready=1 but show
no interrupt progress. An unsolved target reaches ready=2, not success.

## Rubric and acceptance

For each of the six original criteria, level0 lacks its required artifact or
substitutes a claim for evidence; level1 passes ordinary examples but conflates
the relevant model/measurement boundary; level2 implements the exact contract,
explains invariants and rejects the discriminating counterexample; level3 also
completes the named delayed changed condition unaided and explains which proof,
dependency or measurement must be redone. Assess .06a recurrence and blocking;
.06b inversion/cycle distinction; .06c arrival/premise audit; .10a typed evidence;
.10b exact critical-path dependencies; .10c reproducible probe/configuration.
Shared tests do not award a criterion whose explanation or variant is absent.

## Graduated hints

1. Separate arithmetic validity, model admission and the solver's terminal status.
2. Draw wait-for edges independently from the priority values. Inspect the actual
   CMX return before explaining why the second call does not block.
3. Compare configuration fields before reading evidence values. In the map,
   distinguish an unused object reference discarded by section GC from a retained
   call. For debugger work, read the timer clock's model, not a board assumption.

The separate assessor analysis and implementation are withheld from learner
exports. There is no worked solution in this public contract.

## Changed conditions and delayed assessment

Execute all original variants: blocking1→response4; opposite lock acquisition;
minimum-separation sporadic source; changed cache/clock/interrupt identity;
32→64 arithmetic on Cortex-M4; and debugger intervention changing peripheral
progress. The last is an actual QEMU all-stop experiment, not a physical watchdog
freeze test. An independently selected board would require its manual, debugger
configuration and new physical observations. Minimum seven-day unaided learner
assessment remains separate from these author reference runs.

## Security, status and limitations

All executable targets are original teaching programs. Synthetic queue/fault
bytes and private temporary files only; no host clock/security change, board
flashing, production system or external service. The endless-driver specimen is
confined to one owned child and pipe, then killed and reaped. The local debugger
endpoint is unadvertised inside a private temporary directory.

Authored/scaffolded and executed statuses are recorded in the source-bound
successor support and campaign; no independent human review, unaided learner
attempt or physical timing validation is inferred. Finite tests falsify specific
wrong implementations; they do not prove absolute mastery or every C execution.
The original solver's exact model argument and the target ISA/ABI are separate
from ISO C. Physical WCET, cache effects and debugger/watchdog behavior remain
new-environment evidence obligations, not hidden successes of this experiment.
