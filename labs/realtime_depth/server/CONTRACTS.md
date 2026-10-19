# T52.05: exact replenishment policy, finite capacity and overload

## 1. Identity and families

This required successor serves `PR-T52-05` and
`AX-PR-T52-05-T52.05.a`, `.b`, `.c`, including each exact `-DELAYED`
variant. Families A/B/C/D/F/G/H/I/J apply. No historical project, unit,
budget or accepted result is replaced.

## 2. Objectives and inspected references

Use `research/v5/expansion/legacy_realtime_server/resources.json`:
`R-LRS-BOOK` contains actually inspected physical137–140 and160–166 of
Buttazzo third edition, with physical161 also viewed. `R-LRS-COURSE` binds
the actual retained server implementation, test, header and original tasks.
The per-dispatch policy below is precisely specified course behavior;
it is not silently identified with the book's active-interval sporadic server.
Follow-up inspection `R-LRS-SPRUNT` reads the original1989 formulation;
`R-LRS-DEFECTS` reads the author-hosted RTAS2010 paper's unit/cohort model,
amplification and premature-replenishment counterexamples. That later paper
disputes the original formulation's claimed periodic-task equivalence. Compare
these arguments explicitly; a textbook theorem label is not sufficient evidence.
The revised2011 technical-report pseudocode is a recorded access gap, and the
historical paper is not evidence about current POSIX or a current Linux policy.

## 3. Prerequisites and imported artifacts

Complete `PR-T52-04` and the retained `PR-T52-05` CMX server first. Supply
your completed `advanced.c` through `--execution-source` and its matching
`cmx.h`/`cmx_advanced.h` include directory. `run.py` never selects an assessor
implementation automatically. The producer-stub control must fail semantically
after successful compilation. Know unsigned modular arithmetic, finite queue
ownership, fixed priorities and conditional timing arguments.

## 4. Environment and dependency profile

C17 is the active implementation baseline. Hosted verification uses GCC/Clang
debug, optimized and ASan+UBSan builds. The target uses strict freestanding
Cortex-M4/QEMU, no libc, allocator, atomics library or hidden arithmetic helper;
the supplied original byte helpers are explicit imports. Audit map, undefined
symbols, assembly, code size and stack-usage records. The Python oracle is a
host tool, never a target dependency. Ordinary unsigned arithmetic is ISO C;
Arm calling conventions, semihosting and compiler helper lowering are separate
ABI/toolchain contracts. This task does not depend on a C23-only facility or
claim inspection of an unavailable exact C17 standard text.

## 5. Problem, inputs and outputs

Implement `server.h` using the unsolved public `server.c` and the completed
retained producer. `SSBudget` owns a `cmx_server`, capacity1..16 and immutable
overflow policy. `ss_init(Q,P,start,capacity,policy)` requires `0<Q<=P<2^31`.
Calls are externally serialized; caller storage stays live and fields are not
mutated outside the API. `ss_update(now)` applies due records; `ss_charge(now,n)`
also consumes positive available budget and schedules its return at `now+P`.
Use the actual retained update/charge for ordinary paths. At a full smaller
ledger, reject a distinct record or conservatively merge the latest credit
with the new charge at the new, later deadline. Equal-time charges coalesce.
All non-OK calls preserve the complete adapter, including last time and due
credits. A malformed existing object returns BROKEN; invalid new configuration
returns INVALID; insufficient budget or rejected storage returns FULL.

`ss_compare` takes one active interval `[begin,end)`, shorter than P, and
strictly ordered timestamps of one-unit server execution within it. It returns
the per-dispatch ledger and a separate grouped ledger replenishing the total
at `begin+P`. The interval includes intervening higher-priority execution. This
operation is a bounded comparison, **not a complete classical sporadic-server
implementation**; intervals crossing a replenishment boundary are rejected.

`ss_workload` takes at most32 request records, capacity1..8, horizon1..128,
Q/P and one of background/polling/per-dispatch policies. All requests arrive
within the horizon and have cost0..1024. Input/output objects are live and
disjoint. Lower input index breaks equal-release ties. New positive-cost work
is rejected when capacity is occupied; zero-cost work completes immediately
without a slot. A request's separate admission, remaining, executed and finish
fields survive completion or rejection. No replacement of an accepted request
is permitted.

The fixed supervisor executes one tick at0,5,10,... at highest priority.
The ordinary lower-priority task releases C2/D10/T10 jobs; at its next release,
unfinished old work is explicitly counted in `periodic_dropped` and a miss.
At a horizon that is a deadline, an unfinished job is counted missed but remains
pending. This is declared drop-old behavior for that diagnostic task, distinct
from the server request queue. Track all five quantities: periodic execution,
dropped work, remaining work, misses and release demand.

Polling gets fresh Q every P. At its first dispatch opportunity without a
pending request it discards that period's budget; higher-priority preemption
before that first opportunity does not count as an empty dispatch. Per-dispatch
service preserves unused capacity and applies delayed credits. Background runs
only when both periodic tasks have no service that tick. Each trace records
service identity (`32` supervisor, `33` ordinary, `UINT32_MAX` idle), budget,
backlog, due credit amount and record count. The polling counter is also
reported for background but has no eligibility meaning there.

## 6. Necessary distinctions and counterexamples

The retained `cmx_server_charge` processes due credits before refusing a new
amount. FULL therefore does not mean its entire call is unchanged. Compare
that real behavior to the transactional adapter; do not repair by hiding a
progressed clock or changing the producer's historical code.

For Q3/P10, service1@2 and2@5 returns1@12 and2@15. For the inspected active
interval4..7 with service@4,@6 and higher-priority preemption@5, grouping
returns2@14 while per-dispatch returns1@14,1@16. Ending the active interval at
every preemption changes the algorithm. A mean rateQ/P alone does not establish
equal latency, any-window interference, or identical admitted demand.

## 7. Required reasoning and costs

Prove `remaining + sum(outstanding credits) == Q`, every amount positive,
strictly ordered future deadlines within P of last time, and count<=capacity.
Never subtract before validating remaining. Time advances less than2^31 per
call, and P is below that half range; elapsed gaps violating that premise are
not recoverable from the timestamp alone. Prove why latest-record merging moves
every affected eligibility time later, preserves conservation, and cannot
manufacture service. Delaying credits can starve a burst longer than the
unmerged policy; it is not latency equivalent.

With unit service at integer timestamps, exact per-dispatch credits constrain
service to Q in each half-open interval of P ticks, including initial Q and
credits returned exactly at the boundary. Conservative merging preserves that
upper bound. Do not apply this proof to the grouped comparison or polling;
classical periodic-task interference claims require independent scrutiny,
including the later primary paper's counterexamples and preserved cohort ages.
Distinguish the book's larger-number urgency convention from CMX's smaller one.

The ledger operations use O(capacity) bounded time/space; staged publication
adds one ledger-sized automatic object. Workload evaluation is O(H*N) with
fixed-size per-request and per-tick output, no allocation. Include stack/copy
costs and scheduling overhead excluded from logical ticks. Code-size/stack
artifacts from the complete harness are not isolated abstraction overhead or
physical WCET.

## 8. Deliverables

D1: predicted budget/queue tables and the causal arguments above.
D2: adapter, grouped comparison and three-policy complete trace implementation.
D3: normal/refusal/wrap/overflow/overload experiments, exact commands and raw
outputs, independent oracle comparison and seven discriminating controls.
D4: the three changed conditions below, with a new prediction before execution.
Supply explicit import identities, generated-code audit and a cost review.

## 9. Verification and acceptance

Run `run.py --source YOUR/server.c --execution-source YOUR/advanced.c
--execution-include YOUR/include --case ledger` and similarly `transaction`,
`grouped`, `workloads`. Pipe `oracle.py` inputs to `--case stream`; validate the
captured output with `oracle.py --output PATH`. The independent host model checks
1200 twelve-transition modular histories and540 complete workloads, not merely
an output checksum. Run the explicit-import `target_run.py` on the M4 emulator
and retain map, nm, assembly and stack evidence. A compile error, fault, timeout
or sanitizer diagnostic is not an intended semantic-control success.

Required anchors: full16-record rejection and equal-time coalescing; near-wrap
due7; half-range/backward refusal; actual raw partial progress versus unchanged
wrapper; smaller-ledger delay; grouped/preempted difference; requests just after
an empty poll; identical bursts and overload. With one cost2 request@2,
Q3/P10 and horizon30, background finishes5, polling13, per-dispatch4.
With sustained overload, every original request is admitted or rejected, every
admitted unit is executed or pending, backlog never exceeds capacity, and the
supervisor executes exactly its releases. Mutants replenish immediately,
duplicate amounts, publish due progress on refusal, merge earlier, group at
the last dispatch, retain empty-poll budget, or omit supervisor service.

## 10. Objective-specific rubric

For `.05.a`, level2 requires exact three-policy state transitions and the
preemption/grouping distinction; level3 independently repeats the changed
active-interval experiment and identifies the bounded comparison limit.
For `.05.b`, level2 requires the conservation/time/storage argument and real
partial-progress observation; level3 derives and tests the smaller-ledger
delayed-merge policy without an early credit.
For `.05.c`, level2 requires complete equivalent-input traces, policy-specific
interference reasoning and both excess-service controls; level3 handles
sustained overload with explicit admission and supervisor guarantees.
Level0 violates a contract or claims a false guarantee. Level1 merely reproduces
ordinary output and cannot diagnose the relevant counterexample. Passing finite
tests never establishes absolute mastery.

## 11. Graduated hints and private analysis

First label each consumed unit with its permitted return time. Then compare
the number of outstanding units to Q, not only average utilization. Finally
write the publication boundary before calling a producer that can progress.
The separate assessor implementation/analysis is withheld from learner export.

## 12. Changed-condition assessment

After at least seven days and an unaided prediction, perform the exact original
`.a-DELAYED` preemption-inside-active-interval comparison;
`.b-DELAYED` smaller-ledger conservative merge;
`.c-DELAYED` sustained overload with explicit rejected/delayed work and preserved
supervisor service. Change arrival phase while preserving total demand as an
additional composition check. Author execution of these references is not an
independent learner attempt or a demonstrated physical timing bound.

## 13. Scope and containment

Original synthetic programs only, local host processes and owned QEMU image.
No network target, host policy change, deployed equipment or board flashing.
All negative controls remain intentionally flawed and separately selected.

## 14. Status and limitations

Author reference and unsolved learner scaffold are distinct. Current campaign
status comes from the source-bound successor support; do not infer execution
from this document. The original accepted model remains historical. Full
active intervals spanning replenishment boundaries with historical-defect
reproduction and explicitly corrected cohort/debt accounting, a real RTOS
server adapter, physical scheduling cost and independent delayed learner/human
assessment are separate required obligations; none is established by this
bounded comparison, finite oracle or emulator run.
