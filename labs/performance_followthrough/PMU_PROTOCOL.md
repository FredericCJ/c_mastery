# From a byte stream to a trustworthy hardware observation

This active companion extends T43 performance work and the streaming route.
Use ISO C17 for the finite state machine and record parser; the supplied Linux
adapter deliberately depends on the perf ABI, Linux system calls and GCC/Clang
atomic builtins. `pmu.c` is unfinished learner work. The separate assessor
implementation is selected explicitly for author validation. `pmu_linux.c`,
`publication.c`, `pmu_native.c` and their experiment orchestration are supplied
reference adapters, not evidence that a learner has implemented these mechanisms.

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

`pmu_native.c` obtains a writable mmap with one metadata page and eight data
pages. It validates kernel offsets/sizes before accessing the ring, snapshots
`data_head` with acquire ordering, consumes published bytes, and orders all reads
before publishing `data_tail`. These are Linux/GCC/Clang implementation contracts,
not an assertion that a learner's C ring is layout-compatible with the kernel.
The experiment disables before the final drain, then unmaps and closes. It does
not promise lossless collection at arbitrary rates. Loss and insufficient
samples remain evidence, not silently discarded trials.

The sample output includes actual process mappings, instruction addresses,
timestamps, periods and thread identity; the producer retains ELF build ID,
binary hash, symbols and disassembly. Correlate IP using the recorded mapping
and ELF load offsets. A sampled instruction pointer may skid from the event's
cause. Default `precise_ip=0` makes no exact-instruction claim; an optional final
native argument requests 1, 2 or 3 explicitly. A rejected precision request must
stay rejected. This revision makes no unverified top-down or IBS raw-event claim.

## Exercise and verification commands

Build your unfinished protocol implementation and run the tests:

```sh
python3 labs/performance_followthrough/build.py --protocol --output /tmp/pmu-learner
/tmp/pmu-learner
```

Visible cases exercise failed leader/member creation, control/read errors,
short records, zero running time, unexpected/duplicate IDs, interrupted reads,
cleanup, every ring origin, incomplete and malformed records, bounded progress
and loss. Add independent fixtures before revising the implementation. Passing
these does not establish a complete kernel-ABI implementation or all interleavings.

The author-only deterministic campaign explicitly selects the assessor source:

```sh
python3 tools/run_native_pmu.py --protocol-only --report /tmp/pmu-protocol.json
python3 tools/run_native_pmu.py --report /tmp/pmu-native.json
```

Choose a fresh report path on each run. The second command also builds the
optimized unsanitized observation binary and attempts actual hardware collection.
`--sanitizers` adds instrumented protocol tests; those binaries are never used
for performance observations. A denied hardware request is recorded separately
and does not fail unrelated deterministic checks. `passed` means the producer
completed its declared scope; inspect each `gates` field independently.

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
uses the existing `algo_insertion` and `algo_merge` reference adapters. Search
deliberately lifts the old 1024-element bound into a separately specified sorted
immutable extent. The other workloads compare representations or operation
decomposition; they do not replace the original finite proofs.

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
not a socket throughput claim. It uses actual `struct tc_collection`, `struct ev_parser`,
`struct ev_queue` and `struct ev_sum` implementations selected as author reference adapters;
their learner exports remain unfinished. Cancelled items never enter the parser.

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

Write hypotheses before interpreting results. Check whether elapsed work is
large enough relative to the timer and PMU bracket. Do not infer a cache-miss
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
