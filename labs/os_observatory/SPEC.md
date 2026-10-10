# Hosted OS laboratory contracts

The learner implements `include/oslab.h`. `src` contains unsolved functions;
assessor sources live outside this family. Hosted libc/POSIX are permitted here.
This family cannot establish the freestanding or physical target claims.

## Scheduler model

Inputs contain up to 16 jobs with integer arrival, positive service and weight
1..16. A run admits at most 256 ticks. Reject invalid input and runs exceeding
that horizon; partial output on a rejected run is not an accepted schedule.
Idle slots are -1. Job index resolves equal arrival order. Arrival at a quantum
boundary occurs before the old job is requeued. FCFS and SJF are nonpreemptive;
SRTF compares remaining service every tick; RR keeps its turn until the requested
quantum expires. Dispatch count records each new nonidle contiguous segment.

MLFQ has three levels with budgets 1/2/4, demotion on exhausted budget, FIFO within
a level, higher-level preemption and a simultaneous boost every 16 ticks. This
is a named teaching policy, not a claimed Linux implementation. Stride chooses
the smallest pass, increases it by integer floor(1024/weight) each tick, and
admits new jobs at the minimum active pass. All programs here are CPU-only;
blocking, CBS and true preemption are supplied by the execution family.

For each policy predict the complete trace and derive response, waiting and
turnaround from a second method. Compare differing arrival/service distributions.
T49-04 additionally requires checking the Linux scheduler source/doc revision;
this finite model does not claim to reproduce EEVDF.

## Memory model and actual observations

Implement CLOCK using occupancy, reference bits and a hand. Hits set a reference
bit; misses clear visited set bits before replacing the first unreferenced or
empty slot. Page IDs are values, not process pointers. Compare an independently
traced sequence, FIFO's anomaly and an offline optimal-replacement oracle in the
learner dossier. The test anchors only the CLOCK mechanism.

Hosted probes must validate fork copy isolation against MAP_SHARED publication,
wait/reaping, joined atomic thread work, shared dup offsets, unlink/open-file
lifetime, async-signal-safe self-pipe notification, mapped page touches and exact
logical read/write content. They must close/reap/unmap on failure. Report measured
fault counts without asserting a platform-independent count or a latency bound.
Successful fsync is not a physical power-failure experiment.

## Resource and namespace assignments

Use `observe.py` for read-only Linux cgroup, scheduler, clock, resource and I/O
metadata. This observation alone does not satisfy a cgroup-constrained workload:
the T49-09/10 brief requires a delegated/sandbox cgroup trial, recorded limits,
workload commands, raw counters and cleanup. Unsupported permissions are recorded
as unavailable. Do not alter global host scheduler settings to obtain a pass.
Namespace isolation/device-completion explanations remain project-specific gates;
the descriptor and pipe observations are limited runnable anchors.
