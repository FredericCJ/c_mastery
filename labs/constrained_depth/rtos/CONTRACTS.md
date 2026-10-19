# PR-T30-02: owned static tasks, pointer queues and actual CM4F context

## Identity and assessed objectives

This additive contract assesses `AX-PR-T30-02-T30.03.a/b/c` and
`AX-PR-T30-02-T30.07.a/b/c`, including their original `-DELAYED` tasks.
Families are construction, diagnosis, boundary/failure, verification,
resource analysis and changed-condition composition. The original project,
fixture, criterion bodies, hours and historical status remain authoritative
historical records. This successor supplies executable work at their exact
integration boundary; it does not implement the first-principles T50 kernel.

## Sources and semantic layers

The passage register is
`research/v5/expansion/legacy_constrained_rtos/resources.json`. Use the inspected
FreeRTOS Kernel V11.2.0 commit
`0adc196d4bd52a2d91102b525b0aafc1e14a2386`: `queue.c` static creation, copied item
publication and ISR send; `tasks.c` static creation and high-water fill scanning;
`portable/GCC/ARM_CM4F/port.c` PendSV and priority validation. Arm DDI0403E.e
EXC_RETURN/frame passages describe the ISA, not an ISO C threading guarantee.
The local original T30 module/project/fixture/assessor passages were inspected.
The precise published C17 text remains an access gap. C17 is the active
implementation baseline; C11 already supplies the atomics used here. The
freestanding support requirements of later editions do not turn C23 into the
selected implementation, nor make interrupt handlers ISO C threads.

## Prerequisites and artifacts

Bring the original PR-T30-01 continuation/scheduler, T20 explicit state
transitions, T21 failure contracts and T29 Cortex-M exception/priority reasoning.
The cooperative backend actually links the separately exported PR-T30-01
`superloop.h` and learner implementation. Its reference campaign uses that
producer unchanged. For the selected kernel integration supply a clean private
checkout at the exact commit above, the explicit freestanding include directory,
memory helpers and linker script named by `guest.py`. None is silently copied
from an assessor directory. Implement `ownership.c`, `cooperative.c` and
`ownership_guest.c`; their supplied versions are intentionally unsolved.

## Environment and permitted dependencies

Host core: C17, GCC/Clang debug, optimized and ASan+UBSan profiles, hosted test
I/O only. Target: original QEMU `mps2-an386` Cortex-M4F teaching image, no network,
static FreeRTOS tasks/queues, no heap, no libc/libgcc/atomic runtime, selected
explicit byte helpers only. The driver records dependencies, assembly, maps,
undefined symbols and compiler stack reports. GCC and Clang use the base ARM
procedure-call convention; integer application code uses soft-float, FP code
and the port use softfp. Neither an emulator result nor instruction-count pacing
is a hardware latency, power or worst-case execution-time measurement.

## Problem, inputs and expected outputs

Implement a pool of three typed payloads, each carrying 0–16 copied bytes.
Slots 0/1 serve ordinary work; slot 2 is exclusively reserved for a fault report.
The queue holds three copied records containing a payload pointer and a
world/generation/slot token. Queue storage does not own or extend the pointed-to
payload lifetime. Emit the 100 payloads `n=0..99`, length `n%16+1`, each byte `n`.
The consumer must receive 100 items and sum **41780**. Reject a third live
ordinary payload before queue publication even when a queue slot is available.

First fill all three queue entries with control records. An offered producer
blocks on a bounded ten-tick send, while a controller observes the blocked state
after two ticks, requests stop, then collects cleanup and suspension. No payload
may leak or be released twice. This finite observation window is specific to
the selected workload, not a promise that arbitrary scheduling meets a deadline.

Then run the ordinary workload and a genuine IRQ producer. IRQ0 priority 160
may call the selected FromISR API with threshold 128; nested IRQ1 priority 64
may not call it. The waiting controller must run before the interrupted lower
priority producer's next instruction. Repeat with floating-point use and with
an additional bounded monitor task that makes seven ownership observations.

The cooperative alternative uses the actual PR-T30-01 scheduler, two explicit
continuations and the same ownership core. Test stop after every accepted count
0..101: accepted count is `min(cut,100)`, sum is the prefix of the formula above,
steps are `2*accepted+3`, with one blocked transition and two wake transitions.
Requested preemption or ISR semantics return `CR_UNKNOWN` with output unchanged.

## Mechanism and misconceptions

Publishing a pointer is not transferring ownership by language magic. A
higher-priority consumer can complete before `xQueueSend` returns to the sender.
Therefore the sender first changes OWNED to OFFERED. Successful queue return
forbids any further sender access. Failed send means this item was not published,
so the sender may abort OFFERED to OWNED and release it. A completion notification
can precede the worker's final instruction; the controller must separately
establish suspension before deletion or storage reuse. A free pool alone is not
thread quiescence. A stack fill observation is not a proven stack bound.

## Representation, invariants and failure semantics

Initialize only a fresh world with no active users and a never-reused positive
world identity. Public structs are inspectable teaching representations, not
unforgeable capabilities; do not fabricate tokens or mutate live state.
Serialize each pool metadata transition. Input/output/control objects are live
and disjoint. A consumer exclusively owns its immutable payload while reading.
Valid states are FREE, OWNED, OFFERED, CONSUMER and permanently RETIRED.
Each successful allocation increments its slot generation; MAX is usable once,
then release retires that slot. Close denies new allocation but leaves all
cleanup legal. Drain requires close and every slot FREE or RETIRED.

Every failed admission preserves output and existing ownership. `CR_STALE`
rejects old worlds/generations and freed tokens; `CR_BUSY` rejects state-incompatible
operations. A read capacity failure preserves bytes and length. Allocation is
O(3+16), metadata transitions O(1), copied read O(16); all storage is explicit.
This is externally serialized ownership, not a lock-free allocator. Kernel
critical sections and ISR masking provide the selected port's synchronization;
ISO C by itself does not impose that execution model.

## Resource and context derivation

Reproduce the original synthetic ledger: `256+256+8*12+64+128 = 800` bytes.
Do not relabel it as this image's allocation. Record actual `sizeof`, linker
alignment/padding, application and idle stacks, task/queue controls, globals,
pointer queue storage and all pointees. On the selected target the three work/
control stacks reserve 12288 bytes, or 16384 with the monitor; task controls are
252/336, queue data 96, queue control 88, pool 136 and idle stack 2048 bytes.
Other kernel globals, idle control and linker gaps remain in the actual map.
The identical 96-byte queue number has a different representation from the
historical eight-by-twelve-byte fixture.

Annotate PSP, EXC_RETURN and the actual PendSV `r4-r11,lr` save (36 bytes).
Basic hardware frame: 32 bytes. FP extended hardware frame: 104 bytes; the port
additionally preserves `s16-s31` (64), giving context subtotals 68/204 before
possible four-byte alignment and application/kernel call frames. The supplied
FP probe separately saves 72 bytes for its own calling-convention obligations.
Inspect all sixteen high FP values across blocking task switches. In nested
handlers, IRQ1 overwrites S0, and the outer S0 must restore; EXC_RETURN and
captured MSP/PSP distinguish frames. Nested MSP delta includes compiler C frames,
so it must not be equated to the hardware frame alone.

Build the actual call/nesting tree: task → probe → delay → scheduler/PendSV;
task → IRQ0 → IRQ1, with IRQ0 → FromISR queue → deferred scheduling. Compile
reports give local function frames, not automatic transitive bounds. Enumerate
generated direct calls, dynamic calls and helpers. Unknown indirect depth,
recursion, asynchronous nesting, library internals or tool uncertainty remain
unknown; `cr_budget` refuses an unknown bound instead of substituting zero.
No `.su` maximum or successful high-water scan certifies an arbitrary whole image.

## Deliverables and verification

Submit implementation, transition diagram, exact source/ELF annotations, full
resource ledger, call/nesting tree, failure traces, profile results and an
equivalent-workload comparison with the cooperative backend. The public oracle
checks 6561 independent four-operation ownership words and every cooperative
stop cut. Tests cover zero-length payloads, copied input, stale identities,
terminal generation retirement, failed publication rollback, fault reservation,
unknown/overflowing budgets and unchanged outputs. All reference profiles must
pass; each semantic control and each unsolved stub must compile then fail its
specific assertion. A crash, timeout, sanitizer finding or compile failure is
not a successful semantic negative control.

Use `guest.py --help` for explicit target imports; it neither downloads a kernel
nor flashes a board. The author campaign is `tools/run_constrained_rtos.py`.
Target acceptance requires real queue blocking, worker suspension, IRQ handoff,
FP/integer variants, an added task and strict whole-ELF dependency closure.
Retain actual maps/assembly/stack records with the command and source hashes.

## Per-objective rubric

- **T30.03.a:** level 2 implements ownership, bounded blocked-stop cleanup and
  suspension; level 3 explains consumer-before-send-return and demonstrates the
  priority-valid IRQ producer plus rejection at priority 96.
- **T30.03.b:** level 2 binds the exact PendSV/BASEPRI/PSP/EXC_RETURN annotations
  to the selected commit and ELF; level 3 executes integer→FP changes, all high
  register checks and revises both compiler and architectural stack accounting.
- **T30.03.c:** level 2 states create/block/wake/ack/quiesce/delete lifetimes and
  distinguishes the T50 kernel; level 3 imports the real cooperative scheduler,
  executes all stop cuts and refuses unsupported preemption/ISR contracts.
- **T30.07.a:** level 2 separates 800-byte historical arithmetic from the actual
  complete image ledger; level 3 accounts for pointer pointees, padding and the
  added monitor, rather than counting only queue entries.
- **T30.07.b:** level 2 supplies actual frame/call/nesting evidence and explicit
  unknowns; level 3 verifies nested FP restoration and refuses unbounded/unknown
  admission without claiming high-water is a bound.
- **T30.07.c:** level 2 rejects third ordinary ownership before publication and
  closes a blocked producer; level 3 reserves fault capacity under overload and
  shows cleanup remains possible at terminal generation.

Level 0 has missing or contradictory evidence; level 1 is a happy-path run or
unsupported explanation. Level 3 is an assessed demonstration under the stated
conditions, not absolute mastery or proof from a finite test suite.

## Hints and delayed transfer

Hint 1: trace the preemption point immediately after a queue copy, before the
sender resumes. Hint 2: draw separate payload, queue-record and task lifetimes.
Hint 3: compute hardware, port and C-call frames in separate columns; compare
EXC_RETURN bit 4 against actual register preservation. The six original named
`-DELAYED` tasks remain separate unaided attempts after at least seven days.
Author execution establishes available reference support, not learner completion.
The additional task variant must repeat resource and priority arguments.

## Security and isolation

Only original teaching programs and their dedicated local QEMU image are used,
with synthetic byte payloads. No credentials, external services, host security
changes, physical flashing or deployed target. Mutants deliberately violate
fault reservation, offered lifetime, quiescence or IRQ priority and succeed only
as confined assertion detections. No memory corruption or privilege-escalation
claim follows from an assertion, hardware fault or undefined behavior.

## Validation and remaining evidence

Current author results are recorded separately in the source-pinned successor
support and campaign. The learner export remains unsolved and contains no
assessor implementation. Independent human review, unaided delayed learner
assessment and physical stack/timing measurements remain distinct. This static
application integration does not prove all kernel executions, arbitrary call
graphs, general cancellation or physical interrupt latency.
