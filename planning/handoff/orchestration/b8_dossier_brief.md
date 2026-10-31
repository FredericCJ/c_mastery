# B8: three diagnostic dossiers on native hardware (POLISH-DOSSIERS)

Repository /home/fred/dev/c_mastery, branch polish/revision. Read AGENTS.md,
CLAUDE.md, research/v5/expansion/polish_revision/obligations.json
(POLISH-DOSSIERS, and its prerequisites POLISH-LOSS and POLISH-CALIB, which
are done), labs/performance_followthrough/PMU_PROTOCOL.md and CONTRACTS.md,
the source of the obligation
reviews/v5/expansion/native_revision/pmu_hardware_v3/interpretation.md, and
section 8 of /home/fred/dev/c_mastery_claude_opus_polishing_prompt.md.

## Goal

Three representative dossiers that use EXISTING course components:
1. representation/locality (for example AoS against SoA, or a linked
   container against an array layout, from labs/containers_depth,
   labs/dod, or the performance_followthrough workloads);
2. control flow or dispatch, with predictable against unpredictable
   targets (for example the dispatch family, switch against function
   pointer against branch-free code);
3. concurrent-state behaviour: true sharing against false sharing against
   per-thread state, with explicit placement via pthread_setaffinity_np
   within the allowed CPU set (sched_getaffinity), in-thread steady-state
   timing, and per-thread counter groups.

Choose the actual mechanisms and the events this CPU supports. Do not
impose a fixed catalog. The machine is an AMD Ryzen 3 7330U (Zen 3).
perf_event_paranoid is -1 (set by the user: record it in every receipt and
NEVER change it). Discover the supported events from sysfs
(/sys/bus/event_source/devices/cpu/events, perf list) and from the
kernel's AMD event mapping. Justify each event you use from those
definitions.

Each dossier must contain:
- competing hypotheses, and conditions able to challenge the first
  explanation;
- equivalent correctness contracts for the variants compared, checked by a
  test before any measurement;
- controlled workload distributions;
- generated-code inspection (objdump of the measured loops, with the
  variants' relevant instructions quoted);
- event justification;
- placement and measured-thread scope;
- separation of setup and thread-start costs from steady-state work;
- raw evidence (per-repetition rows), analysis, alternative explanations
  and explicit uncertainty;
- binding to the exact source sha256, binary sha256, toolchain versions,
  machine identity (lscpu, kernel, microcode if readable, governor and
  frequency driver, read only), event definitions and measurement
  conditions, including host load before and after each run.

Also apply the measurement gates (tools/measurement_gates.py: acquisition,
scheduling, timing adequacy, comparability, interpretation). An honestly
inconclusive dossier is acceptable. No universal winners, golden cycle
counts or causal claims from correlated counters alone. Keep finite
observations distinct from timing guarantees.

Use the course's own native PMU path as far as it serves:
- labs/performance_followthrough/native.py (doctor, protocol, build, run,
  sample, report) and pmu_*.c;
- tools/run_native_pmu.py and tools/analyze_native_pmu.py.

Where a dossier needs a small driver that does not exist, add it under
labs/performance_followthrough/dossiers/<name>/ (learner-visible, house C
style, pinned clang-format, cm_parse for argv numbers) or under
assessor/performance_followthrough/dossiers/ for anything private. Add a
learner worked example (one dossier written up as a model) under
curriculum/ or labs/performance_followthrough/ as you judge best; do not
edit generated catalogs.

Receipts go under reviews/v5/expansion/polish_revision/dossiers/<name>/:
raw rows, analysis JSON and a DOSSIER.md each. Write fresh paths, and never
edit existing reviews/ receipts.

## Quiet-host protocol (important)

Other work shares this host. The 1-minute load must be below 1.5 for
measurement. Before each measurement batch:
1. Create the marker file /home/fred/.cache/cmastery-tmp/orch/QUIET. The
   orchestrator's drivers pause new steps while it exists.
2. Wait until `cut -d' ' -f1 /proc/loadavg` is below 1.5. Poll every 30 s,
   for up to 60 minutes, and record the waiting.
3. Measure, recording load before and after.
4. Remove the marker as soon as the batch ends, and also on any error
   (use a trap).

Never hold the marker across design or compile work. Never change system
policy: no governor, frequency, paranoid, isolcpus or IRQ affinity
changes. Affinity is per thread, within the process's allowed set only.

## Other rules

- Never run tools/run_*.py with --help; read its source for its arguments.
- Use TMPDIR=/home/fred/.cache/cmastery-tmp/b8; never /tmp.
- Build serially (6.6 GiB RAM).
- Do not commit; the orchestrator reviews and commits.
- Learner artifacts exclude assessor implementations and solution
  analysis.

## Report

For each dossier:
- the question, hypotheses, workloads, events and their justification;
- placement;
- results with uncertainty;
- what was refuted or left inconclusive;
- exact paths.

Also report:
- the files added or changed;
- the tests that check the variants' equivalence, and their exit codes;
- the total quiet-window time used.
