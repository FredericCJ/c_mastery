# From a byte stream to a trustworthy hardware observation

This active companion extends T43 performance work and the streaming route.
Use ISO C17 for the finite state machine and record parser; the supplied Linux
adapter deliberately depends on the perf ABI, Linux system calls and GCC/Clang
atomic builtins. `pmu.c` is unfinished learner work. The separate assessor
implementation is selected explicitly for author validation. The collector
`pmu_native.c` (command line only), `pmu_workloads.c` (the paired workloads and
their oracles), `pmu_sampling.c` (ring and drain lifecycle), `pmu_loss.c` (loss
accounting), `pmu_report.c` (JSON lines), `pmu_linux.c`, `pmu_control.c`,
`pmu_integrations.c`, `publication.c`, the course support in `labs/support/`
(`cm_parse`, `cm_clock`, `measurement_gates.py`) and the runner `native.py` are
supplied adapters, not evidence that a learner has implemented these mechanisms.

## Why this mechanism now

A parser can consume all input bytes yet publish an incomplete document. A
counter read has the same kind of distinction: a successful syscall does not
establish that the event ran, that all requested events were measured together,
or that a ratio explains a hardware cause. Begin with the bounded ring,
segmentation, exact accepted-prefix, backpressure and ownership contracts. Bring
completed arithmetic, resource-cleanup and concurrent-publication assessments
before attempting the mapped kernel ring.

The progression is system requirement → abstract contract → argument → C
representation → failure behavior → observation → revised design → transfer.
Deliver a separate argument at each transition. The next consumers are the
deadline collection comparison and the representation/performance dossier.

## Strict grouped acquisition

Implement `pmu.h` using only the supplied `struct pf_pmu_os` operations for descriptor
access. A request contains one to eight events from one PMU type, for the calling
thread on any allowed CPU, excluding kernel and hypervisor execution. The Linux
adapter does not inherit child-thread work. An event type/config identifies an
event request, not portable microarchitectural semantics.

The group is strict: open the requested first event as leader; every remaining
event joins that exact descriptor. A failed leader must stop acquisition. Any
failed member closes the complete acquired prefix and fails the requested group.
There is no reduced-group retry, software-event substitution or cross-PMU merge.
Obtain and preserve each kernel ID. Record the failed requested index and error.

Use fresh, noncopied control storage. The caller must close an existing session
before reusing it. READY permits reset followed by enable. ACTIVE permits disable
followed by a single complete group read. Stop/failed operations close acquired
descriptors in reverse order. Explicit close is idempotent. Close errors remain
separate from the acquisition error; Linux close is not retried after EINTR.
Invalid state transitions make no new OS call. Read EINTR retries have a budget
of eight; short records are malformed rather than pieces to concatenate.

Decode `nr`, enabled time, running time, then `(value,id)` pairs. The byte count
and member count must match exactly. IDs must form precisely the requested set;
kernel order need not equal request order. Reject duplicates, unknown IDs and
running time greater than enabled time. A zero-byte group read or zero running
time is never-scheduled. Raw zero with positive running time is a valid measured
zero. Preserve raw observations and timing even when interpretation is refused.

Use existing `pf_scale`/`pf_cpi` for separately derived checked rational estimates.
Report running/enabled alongside any scaled count. A common multiplexing fraction
does not establish representativeness for a workload whose phases differ.
Instructions zero makes CPI unavailable. Counts from separately executed
workloads cannot form a same-observation CPI.

Required reasoning: prove prefix ownership, exactly-once cleanup, absence of
false successful publication, and the ID-set decoding invariant. Explain why
valid syscall results are weaker than a successfully scheduled observation.

## Bounded sampled-record consumption

Implement `pf_ring_drain` on a supplied native-endian byte ring. This teaching
contract fixes `sample_type` to IP, TID, TIME and PERIOD and `sample_id_all=0`.
It is not a parser for every possible perf record layout. Parse SAMPLE (40 bytes),
LOST (24), and LOST_SAMPLES (16); count and skip structurally valid unknown kinds.
Headers have eight bytes, and lengths must be aligned, nonzero, no smaller than
the header, and no larger than the ring. Partial headers/records remain pending.
Unknown records still require structural validation.

The capacity is a power of two. Unsigned `head-tail` must not exceed capacity;
the caller supplies a coherent snapshot with at most one ring of pending data.
Wrap may split either header or payload. Copy into actual typed scalar objects,
not an alignment-dependent cast. Consume at most the requested record budget.
Each completed record advances the tail; failure preserves the current record
while retaining the prior accepted prefix. Loss is separately accumulated with
overflow checks. The callback borrows a copied sample only until it returns.

`pmu_sampling.c` obtains a writable mmap with one metadata page and a
power-of-two number of data pages (eight by default, `--pages=P` for 1 to 256).
It validates kernel offsets/sizes before accessing the ring, snapshots
`data_head` with acquire ordering, consumes published bytes, and orders all reads
before publishing `data_tail` with release ordering. Exactly one consumer owns
`data_tail` at a time: the concurrent drainer thread until it is joined, then the
calling thread for the final drain. These are Linux/GCC/Clang implementation
contracts, not an assertion that a learner's C ring is layout-compatible with the
kernel. Every run disables the event, joins the drainer, drains a final time
unless the drainer's parser already failed (that status is then reported and
no further record is consumed), reads the event's counts, then unmaps and
closes. If the drainer thread cannot even be started, nothing is enabled. It
does not promise lossless collection at arbitrary rates. Loss and insufficient samples remain evidence, not silently
discarded trials.

The sample output includes actual process mappings, instruction addresses,
timestamps, periods and thread identity; the producer retains ELF build ID,
binary hash, symbols and disassembly. Correlate IP using the recorded mapping
and ELF load offsets. A sampled instruction pointer may skid from the event's
cause. Default `precise_ip=0` makes no exact-instruction claim; the optional
PRECISION argument requests 1, 2 or 3 explicitly. A rejected precision request
stays rejected: the event is reported unavailable, never reopened with less. This revision makes no unverified top-down or IBS raw-event claim.

## Accounting for lost samples

A sampled observation is only as trustworthy as its account of what it did
not see. Linux reports sample loss in three different ways, and they answer
different questions (perf_event_open(2), man-pages 6.17):

| Source | What it counts | When you learn it |
| --- | --- | --- |
| `PERF_RECORD_LOST` in the ring | samples the ring had no room for | only when a *later* record fits |
| the event's lost count (`PERF_FORMAT_LOST`, Linux 6.0+) | every record of any type (mostly samples) the kernel failed to write into this event's ring for lack of room; not samples skipped while throttled | whenever you `read()` the event |
| `PERF_RECORD_LOST_SAMPLES` in the ring | samples a hardware sampling unit dropped | in the ring, separately |

**Worked trace.** Take a ring that holds 6 sample records, and 64 sample
attempts with nothing draining until the event is disabled. Attempts 1–6 fit.
Attempt 7 does not: the kernel counts one loss and remembers that it owes a
`PERF_RECORD_LOST` record, which it will write in front of the next sample that
fits. Nothing ever fits again, so after disable the drain finds 6 samples and
*no* loss record. An in-band total of 0 here means "no record arrived", not
"nothing was lost". The event's lost count, read after disable, says 58.

So the collector treats the three sources as follows:

- the kernel count, when available, is the total: in-band records report a
  subset of the same losses, so the two are never added;
- without the kernel count, the total is **unknown** and the in-band sum is a
  lower bound;
- hardware drops are reported separately;
- throttling is a gap no count covers: while the event is throttled the
  kernel takes no samples at all and records only `PERF_RECORD_THROTTLE` and
  `PERF_RECORD_UNTHROTTLE`, which the supplied consumer counts;
- an observation is called lossless only when it was `measured`, the ring was
  parsed to its end, the kernel count is available and zero, and no hardware
  drop, throttle record or uninterpreted record was seen. A run that failed,
  never ran or took no samples claims nothing about loss.

Draining *while* the workload runs (`--drain=concurrent`) gives the kernel
room to write its deferred loss records and keeps later samples, at the cost
of a second thread competing for the machine. Compare both modes before
deciding which distortion your experiment can tolerate. A final-only drain
also biases the retained samples towards the *start* of the run, because the
ring fills first and later samples are the ones lost.

Your `pf_ring_drain` keeps `PERF_RECORD_LOST` and `PERF_RECORD_LOST_SAMPLES`
in separate counters (`lost` and `hardware_lost`), because they describe
different events. `pmu_loss.c` (supplied) reconciles them with the kernel
count; read its header before interpreting a sampled result.

**Final versus concurrent drain.** With `--drain=final` nothing reads the ring
until the event is disabled, so a ring smaller than the run's samples saturates
and stays full: the kernel keeps counting losses but has no room to write a
`PERF_RECORD_LOST`, so typically `unreported_lost` equals `total_lost`. With
`--drain=concurrent` a second thread waits in `poll` (the event wakes it on every
sample; at most 1 ms between passes), drains and publishes `data_tail` while
the workload runs; it is joined before the final drain. A final-only run asks
for no wakeups. The drainer competes with the workload for the machine, which
is itself a distortion to report. `drains` counts drain passes.

**Throttling.** When samples arrive faster than `perf_event_max_sample_rate`
allows (the kernel's budget is that rate divided among its ticks), sampling
stops until the next tick. Every sampling and check line therefore reports
`max_sample_rate`, `cpu_max_khz` and `period_floor_cycles` = ceil(maximum
frequency / rate), and `throttle_bound` when the requested period is below that
floor. These values are read once, when the run starts. They are an estimate,
not a guarantee: the floor assumes the CPU's maximum frequency, and the kernel
lowers `perf_event_max_sample_rate` by itself when sampling interrupts take too
long (it logs "perf: interrupt took too long ... lowering
kernel.perf_event_max_sample_rate"), so the limit can fall during or between
runs. This host's kernel log shows it falling from the boot default 100000 to
78000 and then 62000 within half an hour of boot. The robust evidence is
the observation's own `throttle_records`: any throttle record makes the run
`throttled` and never lossless; when the ring was parsed and the kernel count
read, its loss accounting is `complete-except-throttled` (the ring-loss count is
exact, but no total exists), otherwise it stays `unknown`. At the start of this
revision's runs the rate read 50000 samples/s and the floor 87809 cycles at
4.39 GHz; periods of 1000 and 20000 then produced throttle records on every
run. The default period, 200000 cycles, was chosen with about twice that floor
as margin; the margin of any run is its `sample_period` divided by the
`period_floor_cycles` it recorded.

A sampling line reports, separately: `status` and `stage` (`complete`, or where
the run stopped: `config`, `open`, `map`, `layout`, `thread`, `control`,
`parser` or `read`; only `parser` is your `pf_ring_drain`, everything else is
the host or a supplied adapter), `errno` (open/map), `control_errno`,
`parser_status` (null if your parser never ran), `samples`,
`retained`/`not_retained` (the collector's own bounded memory, `--retain=S`),
`inband_lost` (and `lost`, its older name), `hardware_lost`, `kernel_lost` (null
when unknown), `lost_support` (`supported`, `not-requested`, `header-missing`,
`kernel-rejected` or `undetermined` when the event itself was refused),
`loss_accounting` (`complete`, `complete-except-throttled`, `unknown` or
`inconsistent`), `lost_lower_bound` (the larger of the in-band total and the
kernel count whenever that count was read, in every state), `total_lost` and
`unreported_lost` (null unless `complete`), `throttled`,
`throttle_records`, `unthrottle_records`, `lossless`, `unknown` records,
`drain_mode`, `data_pages`, `sample_period`, `drains` and the throttle-floor
fields above.

## Exercise and verification commands

`native.py` is the learner workflow. It uses only files in `labs/` (this
directory and `labs/support/`), never `tools/` or `assessor/`, and changes no
host policy.

```sh
python3 native.py doctor                   # access, paranoid level, capabilities, clock source
python3 native.py protocol [--sanitizers]  # deterministic tests against ./pmu.c
python3 native.py build --pmu-source pmu.c --output build/native
python3 native.py run --build build/native --family numeric --n 8192 --report runs/numeric-8192.json
python3 native.py sample --build build/native --family numeric --n 1048576 --repetitions 64 \
    --pages 1 --drain final --retain 0 --report runs/final.json
python3 native.py report runs/numeric-8192.json runs/final.json
```

`protocol` builds the protocol, loss-accounting and ring-consumer tests with
your `pmu.c` under GCC and Clang (`--sanitizers` adds ASan/UBSan builds and a
ThreadSanitizer build of the ring tests, which run the real drainer thread
against a model producer) and reports each result as
passed, semantic rejection (a contract assertion: fix `pmu.c`), build error,
crash or timeout. Visible cases exercise failed leader/member creation,
control/read errors, short records, zero running time, unexpected/duplicate IDs,
interrupted reads, cleanup, every ring origin, incomplete and malformed records,
bounded progress, loss and concurrent consumption. Add independent fixtures
before revising the implementation. Passing these does not establish a complete
kernel-ABI implementation or all interleavings. (`build.py --protocol` still
builds the protocol tests alone.)

`build` compiles the collector from your `pmu.c` plus the supplied adapters with
`-O2 -g`, never with a sanitizer: a sanitizer runtime measures itself, so a
collector that links one is refused by `build`, `run` and `sample`. The sort
family needs your algorithms component (`--algorithms-source algo.c`); the
service family needs `--events-source`, `--collection-source` and
`--timer-source` together. Without them those families are reported
unavailable, never substituted. Every component is recorded with its path and
hash; supplied adapters are labelled SUPPLIED. `--test-implementation` labels a
build (or a `protocol` run) made from a supplied test or reference
implementation TEST IMPLEMENTATION, and every report from it carries that
label: such results are never evidence of your own work. Inside the author
checkout, a source that is one of the checkout's assessor implementations (it
lies in the checkout's `assessor/` tree, or it is a byte-identical copy of a file
there) is refused without that label; a directory of yours that merely has the
name `assessor` is not. In a learner workspace, which has no `assessor/` tree,
the label in `build.json` is self-asserted by whoever ran the build: it records
a claim and the source hashes, not proof of authorship.

`run` performs one bounded paired comparison (calibration, then up to 31 pairs
in alternating order) and judges it with the gates below; `sample` performs one
sampling observation with explicit ring size, period, precision and drain
mode; `report` prints both kinds, re-deriving each run's calibration and gates
from its raw rows and each sample's loss accounting and lossless claim from its
raw loss fields (a stored verdict is never trusted; a sample report from an
older collector without those fields is refused), and ranking only supported
comparisons. Every run and sample report records the host load average and
available memory before and after. Numbers on the command line are plain decimal digits. Exit status:
0 completed (a completed run may still be inconclusive or refused), 1 a
contract, workload or ring-parser check rejected your component, 2 invalid
request, build error, refused build or host/adapter failure, 3 the family is
not built in.

The collector itself is a small command line (`pmu_native.c`):

```text
pmu_native count|work FAMILY N VARIANT [REPETITIONS]
pmu_native sample|check FAMILY N VARIANT [REPETITIONS [PRECISION]]
           [--drain=final|concurrent] [--pages=P] [--period=CYCLES] [--retain=S]
pmu_native families
```

N is 1 to 4194304 (allocation 65536, sort and service 1024), VARIANT 0 or 1,
REPETITIONS 1 to 1024 (default 16), PRECISION 0 to 3, P a power of two in 1 to
256, CYCLES 1000 to 4294967295 and S 0 to 1048576. Every number is plain ASCII
decimal, parsed by `labs/support/cm_parse.h` with the range checked before any
narrowing: `""`, `"+1"`, `" 1"`, `"1x"` and `"4294967297"` are rejected with exit
status 2, never turned into 0 or 1. `check` validates a request and the
workload oracle without opening any perf event; `check_native_args.py` uses it
to test every argument's boundaries, including the `PF_PERF_CONTROL_FD` and
`PF_PERF_ACK_FD` descriptors of `work`. Exit status 1 means a workload failed its
equivalence, oracle or checksum check or your ring parser stopped on the
kernel's records, and 3 that the family is not built in. Every requested
repetition count is honoured, including when sampling. Count lines carry
`strict_group: true` and `acquisition: "strict-group"` only in `count` mode; in
`work` mode perf stat counts and the line says `perf-control`.

## Timing and interpretation gates

A paired comparison is judged in five separate dimensions
(`labs/support/measurement_gates.py`, shared by `native.py` and the author
campaigns), so that one failure cannot hide behind another:

| Gate | States |
| --- | --- |
| acquisition | `valid` only when every row was measured, scheduled (`0 < running <= enabled`), read with distinct IDs and cleaned up; otherwise `invalid` |
| scheduling | `full` or `multiplexed` (`unknown` after invalid acquisition) |
| timing | `adequate` only when calibration reached its target and every measured row ran at the calibrated repetitions and reached its own target; otherwise `inadequate`, or `inadequate-exhausted` when 1024 repetitions still fell short |
| comparability | `comparable` (31 complete pairs, one row per pair and variant, the same checksum in every row), `incomplete-pairs`, `duplicate-rows` or `not-comparable` |
| interpretation | `supported` only when all pass; `refused` for invalid acquisition, duplicate rows or unequal work; otherwise `inconclusive` |

Each row's timing target is the largest of 50 µs, 100 times the cost of a
clock read and 1000 times the clock resolution. The cost of a clock read is the
upper quartile (nearest rank) of the 31 back-to-back clock-read brackets that
row recorded: a bracket stretched by preemption measures the scheduler, not the
clock, and up to 7 such outliers leave the estimate unchanged. Because every
accepted row is at least 100 times that cost, the typical clock read is under
1% of every accepted interval. Calibration doubles the repetitions from 1 until
the shorter variant reaches twice its target (headroom, so run-to-run variation
does not drop the measured rows below it) or 1024 repetitions are used. The
clock source matters: a host using `hpet` pays microseconds per read, raising
the target to hundreds of microseconds (`doctor` and every report record it).
Raw rows are always kept. A median paired difference and the number of pairs
where variant 1 was faster are produced only for `supported` comparisons; a
comparison with any short row, a multiplexed or incomplete one is inconclusive
and feeds no ranking or competence claim. The same rule judges timer-only
benchmarks (`tools/run_deadline_pilot.py`), whose rows carry the target of a
separate clock probe.

The author-only deterministic campaign explicitly selects the assessor source:

```sh
python3 tools/run_native_pmu.py --protocol-only --sanitizers --report /tmp/pmu-protocol.json
python3 tools/run_native_pmu.py --report /tmp/pmu-native.json
```

Choose a fresh report path on each run. The first command runs every
deterministic test above under GCC and Clang, mutation checks that reintroduce
each repaired defect (with `--sanitizers`, also two ThreadSanitizer mutants that
weaken the `data_head` acquire or the `data_tail` release), the collector's
argument tests and the Python tests of the gates and `native.py`. The second
also builds the unsanitized observation binary with the same `-O2 -g` as
`native.py build` and attempts actual hardware collection: the gated sweep,
sampled records and a bounded saturation experiment (one data page,
200000-cycle period, final and concurrent drains alternated over three trials;
a throttled trial makes the outcome `throttled`, never `saturated` or
`saturation not reached`). Instrumented binaries are never used for
performance observations. A denied hardware request
is recorded separately and does not fail unrelated deterministic checks.
`passed` means the producer completed its declared scope; inspect each `gates`
field independently. `tools/analyze_native_pmu.py` verifies a campaign's hashes
and re-derives every calibration and gate from the raw rows with the current
rules, including for historical receipts, without rewriting them.

The environment preflight records available PMUs/events, CPU, topology, allowed
affinity, cgroups, process capability/seccomp state, virtualization detection,
kernel and perf versions. Native Ubuntu alone does not guarantee access.
No policy is changed. If permission is denied, request authorization for a
one-off privileged execution of the already reviewed binary via
`--hardware-only --binary PATH`; do not lower `perf_event_paranoid` silently.
Root execution is an experiment condition and must appear in provenance.

## Controlled investigations

The supplied workloads compare equivalent results with allocation/initialization,
oracle comparison and checksums outside the measured interval. Stable sorting
uses the algorithms component's `algo_insertion` and `algo_merge` and is built
only when that component is supplied. Search deliberately lifts the old
1024-element bound into a separately specified sorted immutable extent. The
other workloads compare representations or operation decomposition; they do not
replace the original finite proofs.

| Investigation | Competing explanations and discriminating work |
| --- | --- |
| Linear/binary search | Comparison count versus branch/locality cost; sweep 16, 1024 and 65536 elements and inspect retained branches. |
| Packed/wide records | Extra bytes touched versus arithmetic cost; equal values and sum, 8-byte versus 64-byte records, including a 16-MiB working set. |
| Allocation | Acquisition count versus memory locality; compare complete per-object/block lifecycles with the same live values. |
| Numerical reduction | Dependency chain versus vectorization; identical uint64 modular sum using one or four accumulators; inspect generated loops before interpretation. |
| Dispatch | Branch prediction versus indirect-call cost; equivalent operations, runtime function-pointer dispatch and retained noinline calls. |
| Stable sorting | Quadratic comparisons versus merge overhead/scratch; retain original failure contracts and stability tests, sweep three sizes. |
| Concurrent state | Two threads update distinct relaxed atomic counters in adjacent or 64-byte-aligned slots; inspect whether coherence interference, placement or thread-start overhead explains differences. |
| Integrated service | Dispatch cancelled and live deadlines through the actual timer collection, segmented corpus parser, copied bounded event queue and independent sum consumer; compare scan/heap policy with the same event lifecycle. |

For each condition retain 31 paired blocks, alternate A/B order and warm both
variants before measuring. Performance variants run without sanitizers; process
CPU placement is observed, not controlled by changing global policy. Equal
endpoint CPU IDs do not exclude an intervening migration. Retain all trials and
both event IDs; reject ratios for invalid groups. Each process prepares both
variants, so this is a warmed comparison, not a cold-cache experiment.

The concurrent comparison measures **only the calling thread's user events**;
its peer has no inherited event group. Wall duration includes creation, barrier,
work and join; these are identical lifecycle boundaries for both layouts. The
64-byte separation is an explicit tested layout intervention, not a universal
cache-line-size assumption. Check the detected target before calling it false
sharing. The service is a bounded in-process deadline-driven stream consumer,
not a socket throughput claim. It uses actual `struct tc_collection`,
`struct ev_parser`, `struct ev_queue` and `struct ev_sum` implementations, built
in only when the events, collection and timer components are supplied (author
campaigns select the assessor implementations explicitly). Cancelled items
never enter the parser.

Counted groups contain instructions and cycles. Further branch/cache/target raw
events require new explicit requests and verified event semantics. The standard
perf comparison uses disabled-at-start events and an explicit control-pipe
enable/acknowledgement before the batch and disable/acknowledgement after it.
The initial prctl-only comparison produced `<not counted>` and is retained as
failed comparison evidence; exit zero alone does not validate counters. The
repaired producer requires positive counts and running time for both events,
and compares the checksum with a separate local-acquisition run of that exact
batch. Control-return instructions are included, so separate runs need not match
exactly. `tools/check_perf_control.py` tests actual pipes and malformed ACK
rejection without hardware. A focused privileged rerun can use
`--hardware-only --crosscheck-only --binary PATH --report FRESH_PATH`.

Write hypotheses before interpreting results. The timing gate checks whether
elapsed work is large enough relative to the timer; also consider the PMU
bracket. Do not infer a cache-miss
cause from cycles alone, equate CPU cycles with a fixed-frequency wall clock,
or turn observed maxima into WCET. A rigorous inconclusive result is acceptable.

## Publication failure is a real operation

The supplied `pf_publish` synchronizes a written staging file, renames it, then
synchronizes the directory. Injected synchronizers go through this same code.
File-sync failure retains staging and the old destination. Directory-sync
failure after rename leaves the new destination visible with durability
unconfirmed. The caller still owns descriptors and cleanup; it cannot claim a
rollback after rename. No simulation establishes power-loss survival.

## Assessment and remaining evidence

Submit state diagrams, invariants, source, exact commands, raw records, source
and binary identities, failure counterexamples, and a hypothesis dossier with
limits. Protocol competence, successful hardware measurement and causal
diagnosis are three different gates. Independent transfer is a fourth external
assessment: an author reference run cannot award it.

Level 1 reproduces nominal examples; level 2 derives and verifies acquisition,
ownership, parsing and interpretation contracts; level 3 independently selects
and revises a design under an unfamiliar workload after the established delay.
Any fabricated count, mixed observation or successful-publication claim on
failure rejects the corresponding assessment regardless of measured speed.

Primary sources inspected for this revision: [Linux perf ring documentation](https://docs.kernel.org/userspace-api/perf_ring_buffer.html),
[Linux perf security](https://docs.kernel.org/admin-guide/perf-security.html),
and [perf_event_open manual](https://man7.org/linux/man-pages/man2/perf_event_open.2.html).
The inspected host is AMD Ryzen 3 7330U, family 19h/model 50h; AMD identifies
the product as [Zen 3](https://www.amd.com/en/support/downloads/drivers.html/processors/ryzen/ryzen-7000-series/amd-ryzen-3-7330u.html).
[AMD's PMC guidance](https://docs.amd.com/r/en-US/57368-uProf-user-guide/1.2.1.-Performance-Monitor-Counters-PMC)
requires the matching processor PPR for event definitions. A matching model PPR
and advanced-event interpretation are not claimed obtained here. These references
were consulted 2026-10-29; actual ABI/header and tool identities belong to each run.
