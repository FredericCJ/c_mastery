# T30.05 progress-qualified watchdog and sleep handoff

## Identity, families and objectives

This additive contract implements the six original criteria
`AX-PR-T30-05-T30.08.a/b/c`, `AX-PR-T30-05-T30.09.a/b/c` and their exact
`-DELAYED` variants. Families: semantic reasoning, construction, boundary,
diagnosis, falsification, recovery, resource analysis and changed conditions.
Historical project identity, eight hours and acceptance evidence remain intact.
The assessment tasks remain in the canonical supplement; this supplies concrete
source, target bindings and acceptance procedures. No learner has been assessed.

## Inspected sources and semantic boundaries

The source register is
`research/v5/expansion/legacy_constrained_supervision/resources.json`.
Actual inspection includes the complete parent project/fixture/six criteria,
Arm DDI0403E.e B1.5.17–19, RM0090 Rev21 F42/F43 power/RCC and IWDG passages,
and ES0206 **Rev24** §§2.2.1–3,2.2.6–7,2.8.1–4. The errata requires PVU/RVU
clear before Stop even though the reference manual's IWDG note permits continuing
without waiting. It also requires APB frequency greater than twice IWDG frequency
for reliable update flags. Nominal LSI32kHz is not a tolerance bound.

ISO C17 atomics and object lifetimes govern the hosted progress core. Inline
assembly, interrupt masks, device registers, wake events, compiler helper ABI and
QEMU execution are separate contracts. Exact published C17 was not newly read;
no source inspection is inferred from a title. QEMU10.2.1 implements WFE as a
yield/NOP, ignores SEVONPEND and makes SLEEPDEEP read-as-zero/write-ignored.
Its WFE instruction runs therefore do not validate a physical event latch.

## Prerequisite knowledge and artifacts

Bring the completed T30 cooperative/RTOS/timing work, T29 interrupt ownership and
mask restoration, T27 clock/MMIO reasoning, T04 overflow and T03 lifetimes. Read
the retained PR-T30-05 fixture. Required learner artifacts are a definition of
useful transaction commit, the owned interrupt/vector map, a whole-world reset
policy, and a list of all clock-dependent peripherals. This package provides
standalone public headers, compilable unsolved stubs, tests, independent models,
and the original M4 harness. The target links the original explicit byte helpers
from `assessor/execution/support.c` only in assessor execution; learner equivalents
are supplied explicitly by the learner. No assessor implementation is exported.

## Environment and dependencies

Hosted C17, 32-bit unsigned, GCC/Clang debug/release and ASan+UBSan; native
two-publisher pthread execution additionally runs both TSan profiles. The target
is the original QEMU mps2-an386 Cortex-M4, CMSDK timer0/1 at25MHz, IRQ8/9;
GCC O0/O3 and Clang O3, no libc/libatomic/libgcc/CRT, explicit helper source,
strict final symbol/map/assembly/stack audit. Atomics must lower without hidden
helpers. The F439 MMIO backend is compiled and linked, never run on MPS2.
No hardware is flashed. Physical F439 peripheral behavior remains unobserved.

## Concrete problem and supplied inputs

Implement `supervision.h`, `power.h`, `sleep.h` from permitted C and explicit
target primitives. Replay epochs `[1,1],[2,1],[2,2],[3,3]` from zero. Feed only
when every task advanced since the last **successful** observation: `1,0,1,1`.
Add a third stalled task, forced publication during capture, terminal counters,
and a defined false-health specimen that announces progress before commit.

Implement the bitmap alternative using atomic exchange, retaining progress seen
across failed windows. Force a publisher between supervisor exchange and result
processing; its new bit must survive for the next window. Contrast load/clear.
Inject hung-task, overload and failed-boot reasons: permit exactly three recovery
attempts, then terminal safe state. Corrupt each of256 bits of the32-byte record.

Run original wake inputs before check, between check/wait and after wait; retain
already pending work and the caller's PRIMASK. Exercise spurious WFE return and
recheck. `oracle.py` supplies32 architectural cases including4 blanket-clear
counterexamples, separately from actual emulator instruction execution.

The Stop register model begins HSI ready, SYSCLK=HSI, all bus prescalers1,
PWR enabled, normal regulator, no PLL/HSE/CSS, no debug sleep/stop and SysTick
disabled. Reject each missing quiescence premise and unsupported profile before
writes. Clear only owned PWR mode bits, set SLEEPDEEP, execute WFI, then restore
configuration. Completion must wait for HSI/SWS evidence; it publishes one new
time epoch, with either explicit retained-clock delta or unknown elapsed time.

## Why these mechanisms are necessary

A periodic supervisor running is not evidence that its workers progressed.
Publishing before commit can produce a false positive without a data race.
Updating the baseline after a failed window loses a slow task's contribution.
Atomic fields alone do not make a coherent multi-field snapshot. Finite versions
cannot wrap unnoticed. A bitmap needs an atomic transfer of pending ownership.
An uncleared work notification is not interchangeable with an old unrelated
hardware flag. Ordinary Sleep, Stop and Standby have different retained state.

## Representation, invariants, ownership and failure

`CSWorld` is fixed-address, exclusively initialized; count/boot are immutable,
one supervisor owns `last[]`. Publishers make one SC admission CAS, update one
atomic epoch and release the even sequence. They return BUSY on contention;
an ISR must never spin waiting for a preempted publisher. Supervisor reads one
SC snapshot and refuses changed/odd sequence. Sequence/epoch exhaustion refuses
new progress, never wraps; quiescent reconstruction needs a new externally
distinguished boot/world identity. SC is a sufficient policy, not a weakest-order
proof. A skipped BUSY progress report must be retried by task-level ownership or
treated as loss of health; it may not be silently called committed evidence.

Bitmap publishers own no borrowed object; OR publishes task bits, exchange
transfers pending bits to the sole supervisor. Failed windows accumulate seen
bits. Multiple commits may coalesce; this is health evidence, not a work counter.
Recovery mirrors detect specified accidental bit corruption, not malicious
tampering, arbitrary correlated corruption or physical durable atomicity.
Initialize a fresh record only at an authorized lifetime boundary; reboot must
not silently zero retries. Logical terminal state performs no hazardous action.

`CSBus` callbacks are trusted, live, nonreentrant, do not fail or mutate controller
state, and own the selected MMIO registers. Portable Stop entry requires ordinary
interrupts masked; `cs_f439_stop` preserves PRIMASK and rejects handler/BASEPRI/
FAULTMASK contexts. Quiescence mask31 attests DMA, flash, APB, outputs and a valid
enabled wake source. This is an explicit caller obligation, not a hardware scan.
No pending interrupt is cleared. An already pending interrupt may cause immediate
return. Finish checks HSI/SWS and refuses bad time order without changing state.
No peripheral restart is authorized until phase2; after a clock failure phase1
remains recoverable by the caller and no elapsed time is invented.

F439 Stop uses a deliberately selected HSI16 profile: all PLLs are already off;
there is no PLL frequency to restore. A PLL-based application must supply a
separate oscillator/PLL-ready/switch/prescaler/Flash-latency restoration sequence
before restarting consumers. Standby restarts at reset and loses ordinary SRAM;
it is not implemented by this returning Stop function. These changed profiles
remain explicit engineering work, not implied by passing the HSI profile.

IWDG start writes the irreversible start key, unlock, prescaler and reload. It
waits a bounded number of polls before the initial reload. WAIT cannot roll back
the running watchdog; caller must finish configuration/recovery in its real time
budget. Regular reload occurs only after all-progress authorization. Selected
MMIO writes use32-bit accesses and readback/DSB; ownership excludes concurrent
configuration. LSI tolerance, APB/IWDG ratio and physical reset behavior must be
established for actual equipment. IWDG is not WWDG's early/late window policy.

## Required implementation and experiments

Submit the three cores, target port, build/map/symbol/assembly/.su artifacts,
coherent snapshot argument, bitmap linearization diagram, false-health trace,
reason-record policy and per-mode clock/retention ledger. Run8192 independent
six-step progress histories and20000 native commits. Preserve all failing mutant
logs. Distinguish stalled task, admitted slow task and supervisor starvation:
none can be identified solely from a missing feed. Derive a watchdog deadline
from maximum admitted commit/observation delay plus LSI uncertainty and reset
margin; do not substitute nominal oscillator frequency for a bound.

For energy use synthetic3.3V×10mA×2ms plus3.3V×1mA×8ms =92.4µJ. Add
3.3V×20mA×0.5ms wake overhead:125.4µJ per one completed transaction; two
transactions sharing that wake cost total217.8µJ, or108.9µJ each. Report exact
femtojoules or a rational per-transaction result, and detect every64-bit product/
sum overflow before output publication. Plan an actual integration of V(t)I(t),
trigger/timebase alignment, shunt insertion error, bandwidth, sampling aliasing,
trace/debug state, wake/idle duty and uncertainty. No synthetic number is a
physical measurement.

## Verification and acceptance

Run `tools/run_constrained_supervision.py --report
reviews/v5/expansion/legacy_constrained_supervision/campaign.json` with explicit
assessor sources. The expected campaign includes six hosted profiles for core,
power-model and replay; eight native profiles; three target profiles with exact
semantic controls and unsolved-stub rejection. Positive cases must exit0 with
their exact marker; mutants/stubs must exit1 with their own assertion marker,
not a compiler error, crash, timeout or sanitizer finding. Independently compare
all progress traces and the architectural model; require current source/log/image
hashes, successful strict links and no undeclared target dependencies. The guard
also exports a standalone learner package and rejects altered evidence.

## Objective-specific rubric

Level0: incorrect/missing result. Level1: supplied sample passes but invariants
or boundaries remain unexplained. Level2: implement, derive and falsify the exact
criterion below. Level3: independently solve its named changed condition after
at least seven days, with assumptions and evidence limits explicit.

| Criterion | Level2 evidence | Level3 changed condition |
|---|---|---|
|T30.08.a|All-progress decisions since last feed; useful commit boundary|Before-commit false health and corrected boundary|
|T30.08.b|Coherent capture, terminal finite identities, ISR BUSY ownership|Atomic bitmap handoff and intervening publication|
|T30.08.c|Three permissions then safe, all reason paths|Corrupt counter defaults safe; no unauthorized reinitialization|
|T30.09.a|Before/between/after wake, pending retention and saved mask|WFE consumption/recheck, model versus emulator limit|
|T30.09.b|Separate Sleep/Stop/Standby ledger; selected HSI restoration gate|External wake with unavailable timestamp publishes unknown time epoch|
|T30.09.c|92.4µJ arithmetic and physical measurement design|Wake overhead and useful-transaction denominator|

## Graduated hints and assessor separation

Hint1: compare against the last successful authorization, not the last poll.
Hint2: move pending-bit ownership with one RMW, then reason about a publisher
immediately after it. Hint3: masking prevents the handler consuming a pending
notification; it does not itself clear the hardware's wake condition. Hint4:
read the exact device erratum before assuming its manual's generic note applies.
Solutions and technical analysis are withheld under
`assessor/legacy_constrained/supervision.md`; public stubs remain unsolved.

## Variants, containment and validation limits

Each original delayed ID remains required. The supplied executable changed
conditions prepare an independent assessment opportunity; they do not certify
learner independence. Dedicated synthetic host processes and original private
QEMU images only. No host security changes, production peripherals or flashing.
All low-power current, oscillator tolerance, physical reset/wake, external pin
wiring and physical WFE latch observations remain unrun. F439 Stop/IWDG is
register-model tested and target compiled; QEMU ordinary Sleep runs do not close
that hardware evidence. External review and delayed unaided learner work remain
separate. Finite tests, sanitizers and these models do not prove absolute mastery.
