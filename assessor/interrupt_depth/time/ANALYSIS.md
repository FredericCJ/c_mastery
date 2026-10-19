# T29.09–.10 assessor analysis (withheld)

Use the original six ordinary and delayed criteria independently. Do not award
each a pass merely because the core executable succeeds. Native/reference,
generated image, emulator, physical observation and independent learner evidence
are separate columns. The author's selected cases do not prove absolute mastery.

## 09.a

Hardware event -> interrupt assertion -> entry -> begin/high/low/high/end ->
queue -> service is a partial order under the declared device contract. DMA
terminal selection adds a separate generation and quiescence chain; a complete
timestamp does not certify ownership release. Xi accepts IDs0,1,2;3,4,5 are full;
after draining it accepts6. The sequence ledger records exactly three missing
IDs, but cannot infer events before its first observation or more than its
external forward-distance bound. Dp's model provenance stays DP_MODEL in32 actual
zero-drift comparisons. A correct alternative uses rational calibration with
explicit outward rounding and checks the wider arithmetic.

Delayed two-clock challenge: at local1000 the reference sync message could have
arrived in `[50000,50040]ns`; q=20ns/tick, p=500ppm. At local2000, nominal elapsed
20000ns, drift10ns, so `[69990,70050]ns`. A second independently bracketed sync at
local2000 giving `[70000,70020]` intersects consistently; it narrows that instant
but does not prove future rate stability. If incoming-only delay is `[3,11]ns`,
do not use its midpoint as an exact offset. Derive the anchor with the full
one-way bound. Reject a second bracket disjoint from the prediction and diagnose
clock reset, wrong rate, epoch mismatch or a false latency bound before merging.

Score3 requires identifying the measured event and defending uncertainty after a
new clock relation. “Same numerical timestamp means same event” is unacceptable.

## 09.b

The first pair7/8 rejects. The second8/8 with low4 yields
`8*2^32+4=34359738372`, begin15/end19, two attempts,4high/2low/4guard reads.
If total advancement is below one low period, there can be at most one carry.
Equal highs then exclude that carry even with one high bit. This proof relies on
coupled counter state and serialized device reads, not C's arithmetic alone.

The one-bit-high example samples states3(high0),5(low1),8(high0). Concatenation
would produce1, which is not the middle state5. Advancement5 exceeds allowed3:
the implementation refuses before reading. A caller that lies about the bound
defeats the proof; a passing guard check cannot discover every lie.

Independent software stores can expose7:0 between7:FFFFFFFF and8:0 while both
high reads equal7. Actual CPFClock's all-SC operations admit one total order:
equal even before/after sequence with no retirement/wrap violation excludes an
intervening writer, hence its high/low are one committed state. All fields atomic
avoid data races; simply putting a sequence around non-atomic concurrent payload
would not. One writer is essential. Its sequence retires before wrap; an ISR
reader has bounded attempts and can refuse an interrupted writer. Threads test
5000 complementary high/low publications; TSan is a diagnostic for that execution.
The target stages the partial writer explicitly, not an instrumented interruption
inside the CPF write function. Correct alternatives include short masking within
a justified latency budget or a hardware latch documented for the exact device.

## 09.c

At tick105, nominal50 and ceil(50*10000/1000000)=1: sample `[1049,1061]`,
event `[1040,1059]`. A second interval beginning1059 touches; simultaneity is
possible. Beginning1060 proves strict order under the shared epoch/bounds.
Quantum20, ppm0 gives `[1100,1110]`. Keeping the old conversion after changing a
prescaler is wrong even if ordering happens to survive.

For `n=10^6*k+r`, drift is `k*p + floor(r*p/10^6) + (r*p mod10^6 !=0)`.
Since p<=10^6, result<=n; multiplication of r and p fits64 bits and the combined
result fits. Check scaled delta before multiplying. Reference target executes
MAX64 at q1, failing anchor addition, `(MAX32)^2=18446744065119617025`, then
one-ppm upper overflow. Actual generated division calls link original T11 code;
removing those already-compiled objects must fail with unresolved helper import.
No external libgcc is accepted. Python big integers and enumerated interval sets
provide independent expected endpoints and ordering; finite cases remain bounded.

Score3 requires an independent derivation and new overflow/uncertainty case,
not only reciting implementation branches. An alternative rational quantum must
specify rounding and wider-product strategy. Saturation without a changed API
contract is not valid repair.

## 10.a

Name each schedule and check terminal/state as well as return code. Xd cancel
retains storage in STOPPING; full completion may win, subsequent stop is duplicate.
Stop-first after partial transfer wins STOPPED; later full completion is stale.
An old ticket cannot affect a new generation. Clearing software while a device
continues requires an external retained supervisor, a new epoch, device stop and
pending-write drain before recovery. Numeric flags are model evidence, not device
authentication. Actual provider mutants reject early cancellation ownership,
premature queue-slot release and fabricated physical provenance. Changed reset domain:
if the supervisor also resets, its monotonic epoch guarantee is lost; use another
retained authority or quarantine/reinitialize all old work under a proved fence.
Do not infer an unobserved bus transaction from the software model.

## 10.b

The board protocol is a complete acquisition/analysis assignment. Its empty
template must be rejected as unobserved; synthetic checker controls are never
board measurements. MPS2 emits7 real emulator IRQ records, queue4/drop3, bounded
CMSDK acquisition brackets. Each sample lies in its recorded interval; conversion
is40ns/tick under QEMU's inspected25MHz SYSCLK connection. Software timer-arm time
is not an external edge and not exact interrupt assertion. No64-bit peripheral
or GPIO/analyzer measurement occurred. Actual original runtime closure is proven
for these images only.

With entry marker moved to service, measured external-edge-to-marker includes
queueing/service scheduling. Keep entry capture if the intended quantity is IRQ
response. Measure marker overhead using paired enabled/disabled workloads with
equivalent useful work and document analyzer threshold/rate uncertainty. Outlier
maxima describe the observed campaign, not a universal latency bound. A correct
variant may use trace hardware only with an explicit loss/overflow/timebase
contract. Score3 requires recomputing what the instrument now measures.

## 10.c

`claim_cases.json` is assessed by explanation, not keyword classification.
1 SC proof requires single writer, atomic fields and no wrap; unchanged atomic
types alone do not support relaxed-order equivalence.
2 Empty ELF imports proves selected reachable closure, not every source branch.
3 QEMU intervals establish that virtual run, not worst-case silicon latency.
4 A hash-matched board/analyzer capture supports those observed intervals, with
instrument and route assumptions, not all revisions.
5 A changed revision needs errata/clock/routing qualification and rerun.
6 Moving a DMA buffer into inaccessible CCM invalidates the route premise; compile
success cannot fix it.
7 Inspecting includes does not exclude implicit helpers; inspect generated objects
and link closure, deliberately remove the helper provider.
8 Coherent samples do not prove losslessness; compare independent event IDs and
source counts, include drop counters/wrap bounds.

For each criterion use levels0–3 from the public contract. Award level2 only when
the exact evidence and its limits are explained. Delayed tasks are separately
withheld until the original minimum-day interval after an actual independent
attempt. No author run substitutes for that learner evidence or external review.
