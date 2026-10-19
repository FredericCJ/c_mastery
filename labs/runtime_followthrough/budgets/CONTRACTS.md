# T46.09: resource refusal, accounting and native waiting

This required packet implements the unchanged three `AX-PR-T46-09-T46.09.*`
criteria and their seven-day `-DELAYED` tasks. It consumes the actual CVM1 core,
the T46.02 verifier, T46.03 resumable execution and T74 moving collector. Build
those learner artifacts first; this exercise does not substitute an accounting
wrapper for implementing the underlying mechanisms. `learner.c` deliberately
leaves the two new observation adapters unfinished. Pass predecessor sources
explicitly to `run.py`; missing imports must refuse, with no assessor fallback.

Use C17 on the stated hosted/native or strict target profile. POSIX process and
pipe experiments are host-side only; they are not target dependencies. M4 and
PPC64 builds use original runtime helpers, explicit source objects and no libc,
libgcc, compiler-rt, libatomic or HAL. Inspect generated objects, final closure,
map, disassembly and stack records. The selected M4 ORVM enum correction is the
explicit R-VG-ORVM-ENUM-2026-01 source port from the collection packet; it records
original/generated identities rather than modifying the retained provider.
C23 provides no automatic instruction, allocation or deadline enforcement for
these C interfaces. No final published standard wording is claimed inspected.

## Accounting contract

`vl_run` borrows an initialized, stopped Cvm and a verified-or-verifiable image,
executes the actual core and reports deltas in its three budgets and two event
counters. `vl_step` observes one actual Vs step. Objects and output storage must
be live and disjoint. Caller code may not mutate the VM budgets through a
callback. Counters must not wrap during the measured interval. Invalid null
arguments return RANGE without publishing either output. On an executed result,
publish the report even on refusal, but publish the value only on CVM_OK or
VS_DONE respectively. A refused call can have paid work and prior side effects.
Do not infer rollback from an unchanged result output. Reinitialize before each
independent replay, and retain the refused diagnostic state before doing so.

| Ledger | Unit and exact charge boundary |
|---|---|
| fuel | One dispatched guest instruction. Verification is before dispatch. An instruction reached with zero fuel refuses without decrement; an instruction whose operation subsequently refuses has already paid one unit. Original CVM advances the fetched IP before checking fuel, while Vs checks fuel first. Do not equate those diagnostic IPs. |
| GC | One root examination, each of two fields of a reached pair, then a whole16-cell sweep charge. No object is freed unless the complete mark pass and full sweep budget are available. A refused mark can consume units and alter marks. |
| host | One attempted invocation after INT-tag and nonzero host-budget checks. The unit is consumed before discovering a missing callback or receiving callback failure. A wrong tag or zero budget reaches no callback. |
| allocations / collections | Successful pair allocations / completed original collections during the interval. These are events, not bytes, elapsed time or instruction counts. |

`fixtures.h` supplies a concrete9-instruction image:
`INT7 INT8 PAIR STORE0 GC LOAD0 FIRST HOST1 RET`. Its host service returns
argument+11. At GC there is one initialized local root and one reachable pair.
Derive all three budgets, output and intermediate ownership before running it.
If HOST succeeds but RET has no fuel, the host effect remains although no guest
result is published. That boundary is necessary for later durable-service work.

The changed collector is actual `orv_collect`, whose model is
`slots + 2*used_before + 16 + reachable`, including validation of unreachable
used fields and one scan per reached object. It stages relocation and publishes
VM/root/statistics only on success; insufficient budget returns OR_FULL without
consuming the original VM's GC budget. Therefore report its work independently.
For three used pairs, two reachable and one root slot, compare the original and
moving collectors on cloned semantic state. Explain relocation and generation
updates as well as the different counters. Neither formula counts CPU cycles.
ORGraph's array search and the staged Cvm copy also have real execution/storage
costs even where they are not charged individually by that abstract ledger.

## Required experiments

1. **T46.09.a, families A/F/H/J:** implement both accounting adapters and run
   fuel-only, explicit-GC-only, host-only and composed images. Submit a ledger
   per instruction with result publication, budget-before/after, allocation and
   callback effects. Derive refusal charge policy, then compare all fuel0..10,
   GC0..20 and host0..2 combinations for the supplied image with an independent
   piecewise expectation. Import the moving collector and recalibrate its units
   using the three-used/two-reachable graph. Do not rename its counter “cycles.”
2. **T46.09.b, families B/C/D/F/G/J:** isolate seven capacities: instruction fuel,
   frame16, operand64, heap16, explicit roots8, GC budget and host budget. Supply
   small verified bytecode or an explicit valid preloaded heap/root state for
   each. Distinguish the two STACK_LIMIT causes using state, not just an enum.
   Check unchanged value output, paid units, occupancy, root balance and replay
   after reset. Compose near-zero fuel and host budget at HOST; then change the
   input tag and derive which refusal wins. Include prior side effects.
3. **T46.09.c, families A/D/F/G/H/J:** author a local callback whose read awaits an
   externally released byte. The supplied3-instruction guest has fuel3. Observe
   callback entry, no result during a bounded observation, release it and reap
   the owned process. The callback deliberately lacks its own deadline and
   violates the original embedding policy. Explain analytically why guest fuel
   cannot supply that missing deadline, or bound compiler/verification/GC time.
   Replace it with actual Vs pending/take/complete behavior: retain image, VM,
   request identity and native provider until completion/quiescence. Execute an
   unrelated guest while the first request is pending, reject stale completion
   and restart, and cancel an owned provider before marking the session failed.

For the third experiment the assessor supplies a separate executable protocol;
your implementation must follow the same observable contract. Use three local
pipes (entry, release, result), one owned child and synthetic integer7. Wait for
entry before beginning a100ms no-result observation. Positive completion returns
18; cancellation preserves result sentinel91. Use absolute monotonic harness
deadlines, handle interrupted/short I/O, close all pipe ends, and reap the exact
child. A no-block control must be rejected because its result appears before
release. This is not permission to alter host settings or signal other processes.
Finite100ms observation is a bounded experiment, not a proof of indefinite delay
or a hard scheduling guarantee. The experiment's five-second containment policy
is distinct from the intentionally faulty callback's absent policy.

The live pending state cannot be restarted. Take the request once; complete only
the matching epoch/serial. A step attempted while pending returns VS_STATE with
zero charged work. Joining/reaping the native producer is the proof that it can
no longer access retained resources in this particular process adapter. Merely
setting a cancellation flag, observing timeout, or publishing HOST_ERROR does
not establish quiescence for another adapter. A separate child address space
also differs from a thread borrowing a VM pointer; do not transfer that proof
without revisiting lifetime assumptions.

## Deliverables, assessment and transfer

Submit implementation, actual predecessor identities, each initial prediction,
instruction/state ledgers, independent oracle, complete refusal traces, compiled
counterexamples, local-process cleanup trace and resource-cost discussion.
Execute `python run.py --help`, supply all six predecessor paths, and preserve
the output of both successful and deliberately incomplete implementations.
Host-only oracles/process probes must not enter target images. For freestanding
builds use the explicit builder and inspect all14 objects as well as the linked
image. Passing a linker alone does not audit the runtime closure.

Hints are graduated: first mark the charge point separately from result
publication; next stop at HOST and GC with exactly one remaining unit; finally
draw ownership from request submission through native quiescence and terminal
result. For collector changes, list what is visited even when unreachable.

Rubric0: wrong outcome or ownership;1: reproduces a result but conflates budgets
or cannot localize a refusal;2: derives all ordinary charge/order relations,
isolates all seven capacities and distinguishes the host waiting contract;
3: implements the original changed-condition collector, near-limit composition
and pending protocol with revised costs/ownership and an independent case. Grade
each objective independently. A fake PASS print, a negative program that merely
does not compile, or a finite latency maximum claimed as WCET fails the related
criterion. Appropriate alternative representations are accepted with equivalent
contracts; concealing consumed work or retaining global roots indefinitely is
not equivalent.

Delayed IDs retain their exact original tasks and minimum seven-day gap. The
assessor changes collector work units, refusal ordering or provider behavior and
withholds reference implementations. The package is authored for administration;
actual independent and delayed learner performance remains pending. Source
register, objective mapping and current campaign records carry exact validation
scope, seeds/bounds, compiler identities and limitations.

Primary inspected contracts are the retained CVM/ORVM/Vs implementations and
POSIX.1-2024 read, poll and waitpid passages. POSIX specifies empty blocking-pipe
read behavior, poll timeout/readiness and child-status interpretation; the
curriculum's fuel, callback and lifetime policies remain original contracts.
