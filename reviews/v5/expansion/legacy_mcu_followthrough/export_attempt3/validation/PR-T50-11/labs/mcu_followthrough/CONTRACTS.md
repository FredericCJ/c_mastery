# Original MCU runtime followthrough

This additive laboratory supplies the exact original PR-T50-01 through12
objectives and their original delayed variants. Read the corresponding unit
and `criteria.json` before changing code. The frozen `execution` family,
its incomplete learner sources, original fixtures and historical receipts
remain distinct. This package supplies a true direct call-boundary switch,
generation handles, a dedicated task-local fault route and an actual one-shot
sleep adapter; the earlier PendSV-mediated cooperative route and periodic
SysTick provider retain their own contracts.

Prerequisites are C17 integer/lifetime reasoning, caller-owned storage,
function-call ABI, linker load/run addresses, exception frames, finite queues
and the original T50 sequence. Use GCC/Clang C17 on the host. Target profiles
use explicit ARM GCC or Clang cross-compilation, ARM binutils and QEMU10.2.1
AN386 Cortex-M4 or the specified AN500 Cortex-M7 transfer. There is no libc,
libgcc, compiler-rt, libatomic, allocator or hosted CRT in the target image.
Semihosting is only the emulator observation/exit channel. It is not a board
transport or a permitted production runtime service.

## Ownership and failure boundaries

All `Mf` and inherited `cmx` operations require one external serialization
domain. The target uses saved PRIMASK for scheduler data. BASEPRI exercises
are separately encoded and cannot silently replace that domain. Inputs and
outputs are valid, live, appropriately aligned and disjoint where required;
caller storage outlives all task/result/device users. No callback may reenter
or mutate borrowed state. C pointers do not become valid merely because a
numeric bounds check succeeds.

`Mf` reserves slot0 as idle and admits fifteen applications. Its public handle
is `(generation<<5)|(slot+1)`, with a32-bit generation. It rejects stale or
out-of-range identities. Caller stack bases are8-aligned, at least128 words;
odd word counts round the top downward to the next8-byte boundary. Refusal
must preserve existing state/output. A selected partial-admission budget
exercises rollback before stack publication. Generation exhaustion retires
rather than wrapping. `executing` records physical stack ownership separately
from logical current. Join is single-consumer; detached tasks have a supervisor
reaper. No task can reap its own active stack.

Fault records retain the first cause. A safe task-local fault removes waits,
exits and lets safe peers continue; protected-resource inconsistency poisons
the system and retains ownership until an explicit supervisor policy repairs
or closes it. Handler-mode, stacking and unstacking faults are outside the
recoverable-frame premise. Canary damage, saved-SP range checks and an MPU
permission fault are different observations. The selected MPU case prevents
one explicit32-byte region access; it does not prove complete stack isolation.

Timers require forward admission and observation distances below half-range.
Equal deadlines retain insertion identity. Polling emits generation tickets;
old tickets cannot affect a reused slot. A caller that misses a full unseen
clock wrap has violated the premise even if unsigned subtraction returns a
small number. `MfOrder` is a separate fixed-epoch list/heap comparison transfer,
not an alternative generation-owned timer API. Its cost counters count actual
comparisons/moves; they are not physical time.

## Two switch ledgers

| Mechanism | Software state | Hardware state | Alignment |
|---|---|---|---|
| Direct integer ABI call | r4-r11,LR plus pad:40 bytes | none | saved/call SP mod8=0 |
| Direct FP ABI call | integer frame plus s16-s31;FPSCR in pad:104 bytes | none | saved/call SP mod8=0 |
| Basic exception switch | r4-r11,EXC_RETURN:36 bytes | r0-r3,r12,LR,PC,xPSR:32 bytes | saved PSP mod8=4 from aligned top; chooser MSP mod8=0 |
| Extended exception switch | previous software plus64 high-FP bytes | previous hardware plus72 low-FP/FPSCR/reserved bytes |204 payload bytes plus possible architecture padding |

Fixture36/40,68/72 and204/208 are explicitly selected payload/alignment
arithmetic. Do not insert fixture padding into an assembly frame whose reader
does not skip it. The hardware basic words occupy the low end of the selected
Armv7-M frame, followed by the low-FP extension. Inspect actual disassembly,
EXC_RETURN and recorded raw-frame observations before changing a decoder.
`MF_FP` tests all high registers and asynchronous low registers; integer-only
execution cannot validate the FP path. FPSCR rounding is selected per task.

## Actual sleep and integrated targets

The periodic preemption image uses SysTick priority0x80 and PendSV0xff. NMI
transfer uses a separate single-outstanding mailbox and calls no scheduler
API. Repeated requests coalesce in the PendSV pending bit while the explicit
request counter keeps their multiplicity. A blocking wrapper rejects handler,
PRIMASK,BASEPRI and FAULTMASK contexts before waiter enrollment.

The sleep image selects the QEMU CMSDK25MHz timer0 as a continuously running
elapsed source and timer1 as one-shot wake. Timer1 RELOAD0/VALUE=count is a
different encoding from SysTick LOAD=counts-1. WFI occurs with the saved
mask held; enabled pending interrupts wake it even before their handler runs.
Recheck ready/pending state, compensate the observed interval once, then
restore the incoming mask. A source stopped during sleep fails this contract.
The pure `MfSleep` model intentionally rejects elapsed>planned; actual
`MfNap` records overshoot from programming/wake overhead. Keep their scopes
separate.

The integrated target fixes input/pool capacity4, output capacity2 and
diagnostic capacity2. ISR producer work is bounded; no printing occurs in
normal ISR paths. Reject-full/drop-full policies account for every sample.
Recovery joins/reaps a failed worker from another stack and dynamically reuses
its storage with a new generation. The CPU hog continues to be preempted.
The borrowed-buffer transfer holds a stable software pool lease until consumer
completion; it is not evidence of physical DMA. The UART transfer receives
64 bytes through the actual emulated CMSDK peripheral with at most4 reads per
interrupt. A pending device/caller abandonment requires retaining storage
until completion or acknowledged reset, not merely a timeout request.

## Run and retain evidence

The defaults compile incomplete public sources and must not pass. Completed
providers are explicit inputs. For a completed learner implementation:

For a warning-clean incomplete baseline scaffold, explicitly select
`--cmx labs/mcu_followthrough/baseline/cmx.c` in the native runner and
`--runtime-source labs/mcu_followthrough/baseline` in the target runner.
Those additive copies insert unused-parameter casts only; the original
`labs/execution/src` learner sources remain untouched. A semantic rejection
from the compiled scaffold is expected until its mechanisms are implemented.

```
python labs/mcu_followthrough/run.py --source YOUR/core/mcu.c --cmx YOUR/cmx.c --build NEW_DIRECTORY
python labs/mcu_followthrough/target/build.py --source YOUR/target --runtime-source YOUR/execution --core-source YOUR/core/mcu.c --mode direct --run --build NEW_DIRECTORY
```

Other target modes are `preemptive`, `sleep` and `integrated`; compiler,
optimization,frame-pointer,FP,moved-data andM7 variants are explicit switches.
Use fresh output directories. Keep commands, compiler versions, source/object/
image hashes, undefined symbols, map, disassembly, `.su` files, complete trace
and named negative controls. Public `report.c` is observation scaffolding;
all other target/provider TODOs remain learner work. Never find a private
reference through an implicit fallback.

`core/supplement.c` checks a separate twelve-unit service trace: RUNNING time
is CPU service, READY time is waiting for CPU, and WAITING time is blocked.
It also supplies the stale-expiry consumer boundary and a bursty-work case.
Compile it with the completed core and cmx source, the same include paths and
strict C17 flags used by `run.py`. It must fail against incomplete providers.

For the deepest direct task, the selected call-path budget is the compiled
entry frame plus four simultaneously live `nested` frames plus the larger of
the probe/yield/switch chain and assertion-diagnostic chain. Read `.su` and
assembly for each compiler configuration. Four recursive calls at depth3
include depth0; counting only three loses a live local array. In FP mode the
probe consumes112 bytes and the direct switch104. This direct image has no
enabled interrupt source; that premise does not extend to preemption images.
For nested exceptions, budget the suspended PSP frame separately from the
simultaneous SysTick and NMI frames and C chains on MSP, including alignment.

The preemption trace records a25MHz elapsed source independently of SysTick.
It gives maximum observed pend-to-chooser latency, ISR duration, empty-read
overhead and first-handler-FP instruction interval. Zero counts mean below
the selected counter resolution. The lazy extension paint changes18 words
to1 painted word:17 stored low-register/FPSCR words plus a reserved word.
All18 extension words were already reserved. These are model observations,
not a worst-case board bound. A hypothetical clock running1% fast makes
nominal32768 counts expire before one second; derive a conservative count
using the maximum frequency if the no-early-expiry contract includes that
uncertainty. This worked premise is not an oscillator measurement.

Predict before execution. D1 is the ownership/state/frame or dependency
derivation. D2 is the exact mechanism implementation. D3 is independent
expectation plus named adversarial/negative evidence. D4 is the original
changed condition with its new premises. A matching marker with an invalid
lifetime,language,frame or timing explanation fails. Rubric0 is incorrect or
missing;1 is a nominal example without cause;2 is the full original relation
with a discriminator;3 is the changed condition independently justified.
Hints: locate the first ownership transfer; compare the saved layout with its
reader; isolate the shortest failing state before recombining components.

The exact original delayed variants require at least seven days before an
unseen independent learner attempt. Author/reference runs do not administer
that assessment. External review, physical board timing, energy, real DMA,
clock uncertainty and full current silicon errata remain separate evidence.
Published C17 wording was not newly inspected; the ABI/manual/model passages
and their edition limits are registered in the research resource ledger.
