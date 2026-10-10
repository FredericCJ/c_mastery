# Performance, debugging and failure experiments

Use {ref}`T43` to explain and measure cost, {ref}`T44` to move from symptoms to causal evidence, and {ref}`T45` to investigate memory and lifetime failures. Each track adapts earlier small C components. An algorithm, allocator, parser, driver or event loop already built in its owning module becomes the experimental subject; the new project pays for the additional measurement, diagnosis or repair.

## Start the habits early, deepen them in stages

From the first component, state its input sizes, capacities, ownership and failure policy. Predict a boundary result before running it. Keep the exact build command, input, result and unanswered question. In T03/T05, practice object bounds and warnings; in T21/T32, preserve first-cause context, minimize a failure and retain a regression. These are early habits within the existing budgets, rather than a requirement to finish three advanced domains before writing C.

After the named foundations, follow the full project contracts. T43 deepens the T13/T15/T30/T34/T37 cost and timing work. T44 begins with warnings, logs and a small reproducer, then adds stepping, watches, optimized images, I/O, concurrency and firmware diagnosis. T45 deepens the T03/T16/T21/T27/T29/T31/T32/T35 ownership, storage and failure work. Later system and firmware projects name their selected preludes. A cross-reference supplies a route, not automatic credit for an objective.

## Give every complexity claim a cost contract

Record the operation, successful and rejected outcomes, input/output parameters, representation, case and primitive cost. For a graph, both vertices and edges may matter. For a parser, input bytes and produced records may matter. Distinguish worst case, a specified expected-input distribution, and an amortized sequence guarantee. State any bound on key length, comparator cost, recursion or allocation separately.

An exact abstract count such as

$$
C(n)=3n^2+5n+7
$$

has a tight quadratic bound: for integer $n\ge1$, $3n^2\le C(n)\le15n^2$. This supplies constants and a threshold; a finite timing plot does not supply the same proof. A loose upper bound also does not establish a tight bound or a universal speed ordering. Amortized analysis accounts for expensive operations across a sequence; it does not require random inputs. Use the bounded formal and amortized passages in {ref}`R-clrs-4` and the original certificates in T43.

“Cycles” needs an explicit meaning. Executed processor cycles depend on instructions, memory behavior, clock and execution conditions. They are not the cycles of a control-flow graph. Cyclomatic complexity is a static graph metric used in design and testing; it is not a time complexity class. The quantitative architecture reading {ref}`R-ENH-HENNESSY-PATTERSON-6` and the control-flow reading {ref}`R-PERF-NIST-SP500-235` support different claims.

| Measure | State the boundary and unit | Useful evidence and remaining question |
|---|---|---|
| Abstract work | Comparisons, moves, probes, visited edges, or another declared primitive | Exact counters and a proof for the input family; primitive costs may differ |
| Time complexity | Function of named sizes and case, with model assumptions | Upper/lower/tight or sequence argument; finite samples do not prove asymptotics |
| Instructions and cycles | Executed instructions/events or identified processor-counter cycles | Matching image, event definition and observation; stalls and frequency affect interpretation |
| Elapsed and CPU time | Clock, process/thread scope and start/end events, in time units | Raw timestamps and scope; parallel process CPU can exceed elapsed time |
| Space | Logical live bytes, capacity, retained storage, simultaneous scratch, stack or static sections | Lifetime ledger, map, conditional frame bound and qualified observations; categories are not interchangeable |
| Code and control flow | Linked/object code bytes or a declared connected-function graph | Matching build and graph; neither determines runtime alone |
| Traffic and I/O | Useful, read, written and transferred bytes at a named layer; calls, blocks or I/O operations | Model and actual counts kept separate; amplification is undefined when useful bytes are zero |
| Latency and throughput | Particular completion interval versus completed items in a stated window | Per-request events and window counts; queueing and overlapping stages matter |
| Deadline and jitter | Release/start/finish definitions, deadline and variation statistic | Observed misses and distribution; sampled tails are not a WCET proof |
| Energy | Instrument/counter, units, measurement boundary and integration method | Actual qualified energy observation; elapsed time alone cannot establish energy |

A space ledger counts shared storage once and takes the peak over simultaneous lifetimes. Sum of all allocations over a run is not peak live space. A freed allocation may remain reserved by an allocator. Stack-frame estimates need call paths, indirect calls, recursion, callbacks and permitted exception nesting. Keep sampled high water and an analytical bound as separate entries.

## Measure a component through its complete operation

Draw the events before choosing timers: accepted request, submit, function return, completion, buffer release, and any requested persistence or peer-observation boundary. An asynchronous submit can return while a borrowed buffer is still in use. Overlapping spans cannot be added blindly to obtain end-to-end latency. Inclusive and exclusive function attribution require a stated nesting model. Subtracting process CPU time from elapsed time does not universally measure waiting.

For read/write work, record requested bytes, each returned positive count, cursor advancement, EOF and retry/error history. Measure setup, read, processing, write, requested synchronization and cleanup separately when useful. Label the filesystem/device, buffering, file lifetime and warm/cold assumption. A successful write, complete transfer, requested synchronization and experimentally established crash durability are different claims. Use the exact Linux interface and synchronization readings assigned in T43/T44; reuse the existing partial-I/O adapters.

The abstract block model in {ref}`R-PERF-VITTER-2001` names item count, fast-memory capacity and block size. A block transfer in that model is not automatically one system call, cache miss, DMA transaction or physical media operation. Compare these layers to explain a discrepancy, preserving their units.

## Profiling routes and experimental controls

| Route | New investigation on a reused component | Evidence limits and practical alternative |
|---|---|---|
| Portable/hosted harness | Paired, repeated consumed-result trials; exact work and lifetime counters; clock/read-overhead controls | Preserve raw samples, workload, build identity and ordering. Inspect generated code when work may have disappeared. Reuse T34's harness |
| Linux PMU | Identified per-process events, grouped counts, enabled/running times and permitted sampling evidence | Availability and event meaning vary. Multiplex scaling is an estimate and requires positive running time. Denied/unsupported events remain explicit; clock/work measurements close narrower claims |
| Linux cgroup v2 | Read permitted CPU, memory, per-device I/O and pressure accounting; compare a prepared delegated quota when available | Configuration is distinct from accounting. Ancestor limits, membership, controller and writeback scope matter. Read-only observations and supplied finite snapshots support a no-root route |
| Compiler instrumentation | Matching baseline versus bounded entry/exit hooks, coverage/arc counts and a supported single-threaded gprof comparison | Instrumentation changes code, stack and timing. Counted calls, sampled self time, estimated child time and off-CPU waits are separate. Record dropped or incomplete traces |
| Bare metal | Identified counter or timer around a reused kernel; empty-bracket and interrupt variants | Verify presence/access/enable/clock/wrap/debug conditions and shared trace ownership. An ambiguous multi-wrap interval cannot yield a unique delta. Timer/GPIO alternatives need their own actual setup |

The route contracts are bounded by {ref}`R-PERF-LINUX-CLOCK-6-19`, {ref}`R-PERF-LINUX-PERF-EVENT-6-19`, {ref}`R-PERF-LINUX-CGROUP-V2-6-18`, {ref}`R-PERF-GCC-INSTRUMENT-16-2`, {ref}`R-PERF-GPROF-2-47` and {ref}`R-ENH-ARMV7M-DDI0403EE`. Record the installed versions rather than assuming the inspected versions are installed. Use already permitted per-process capabilities or a prepared delegated environment. Each project distinguishes an actual run, a supplied-data analysis and unavailable evidence.

Before comparing speed, keep correctness, ownership and failure gates fixed. Separate setup from the measured operation, declare warmup and workload families, vary execution order, preserve outliers and state the sample/quantile convention. Compare instrumented and uninstrumented artifacts; measure observer cost rather than subtracting an assumed constant indiscriminately. A debugger stop, allocation interceptor, log call or sanitizer can change the behavior being measured. Repeat only when the changed condition or unresolved uncertainty justifies it.

## Debugging progression: from a symptom to a repair

Begin with a defined-state wrong-result defect. Predict the first invariant that should fail, then use warnings, an assertion or bounded log to locate it. Assertions support internal diagnostics; externally required validation must survive a build that disables them. Preserve the first useful cause before later cleanup or logging changes its context.

Next, make a bounded same-failure reproducer. The reducer distinguishes PASS, the same FAIL, and UNRESOLVED, such as malformed input or a different failure. A 1-minimal result can still be larger than another trigger. Reduction localizes an input; the causal explanation still needs the invalid state transition. See {ref}`R-PD-DDMIN-ISSTA2000`.

Use source and instruction stepping, selected frames, pure conditional breakpoints and valid-object watches to discriminate competing explanations. Keep the exact image and debug information. Watchpoints have target resources, lifetime, thread and stopping-granularity limits. In optimized code, a source variable or statement may have been removed or moved. A core or fault snapshot is a stopped state, with missing-history and missing-memory limits. The precise command readings are in {ref}`R-PD-GDB-20261009`.

Advance to partial I/O, descriptor reuse, race/deadlock/predicate histories and reset/HardFault/ISR/DMA diagnosis. Reconstruct cursor, ownership and happens-before transitions rather than patching the last visible crash. Avoid treating stale errno after success, a longer sleep, a larger buffer, or a cleared single pointer as causal proof of a repair. Compare debug, optimized and suitable instrumented builds after fixing the invariant.

## Bug laboratory: mechanism, observation and mitigation

The same visible crash or corrupt output can have different causes. Each T45 brief asks for a pre-run prediction, a controlled negative case, an observation, a discriminating experiment, the first supported invalid transition, a causal repair and delayed transfer. Tool diagnostics and native undefined-behavior symptoms are not fixed portable output strings.

| Failure family | Mechanism and useful experiment | Repair and mitigation limits |
|---|---|---|
| Buffer over/underflow, off-by-one, missing NUL | Separate capacity, initialized length and terminator space; probe lower/upper/exact boundaries and preserve sentinels | Validate before invalid arithmetic/access, maintain length/capacity invariants and rejection policy. A legal-address wrong-cell write still needs a logical invariant |
| Wrapped or truncated size | Trace checked count×stride plus headers/terminator, signed conversion and narrowing | Check before each operation and preserve ownership on rejection. Unsigned wrap is defined arithmetic but may violate the size contract |
| Stack exhaustion | Bound depth, frame size, callbacks, reentrancy and permitted nesting; compare compiler records, samples and isolated child-process exhaustion | Reduce/bound depth or frames and reserve justified space. Per-function reports, fill scans and canaries do not prove every execution fits |
| Stack-local buffer corruption | Observe a bad extent/initialization transition within an existing frame | Repair that object's contract. More total stack capacity does not repair an out-of-bounds access |
| Fixed `.bss`/RAM placement or startup error | Inspect link-region capacity, alignment, load/run sections and copy/zero/retained ranges | Repair placement or startup ranges. Link success does not bound dynamic use or establish correct initialization |
| Heap/stack collision | Use an explicitly declared shared interval and dynamic-region guard; distinguish it from stray writes | Enforce allocator/stack boundaries for that map. Hosted heaps do not universally follow this model |
| Leak or forgotten release | Trace every acquisition and partial-failure prefix, owning/borrowed edges, cleanup order and live/retained bytes | Restore exactly-once release and bounded retention. Reachable retained storage differs from a lost-owner leak; RSS alone is not the oracle |
| Failed resize, UAF, double/invalid free, escaped alias | Trace generations and extents; preserve old owner on failed nonzero resize; invalidate prior aliases on success, including reused address tokens | Fix ownership and publication. `free(NULL)` is valid; clearing one owner does not clear other aliases. Diagnostic quarantine cannot establish correct lifetime |
| Uninitialized state | Distinguish addressability from field initialization and track propagation to a consuming branch/output | Initialize complete semantic state before publication. Zero bytes are not a universal pointer/value initialization rule; padding is not semantic field state |
| DMA/callback/ISR lifetime and reentrancy | Trace submit, timeout, cancel request, confirmed completion/quiescence and reuse; introduce late/stale completion and shared-scratch cases | Keep borrowing until the declared completion/reclaim condition. A timeout or cancellation request alone does not establish device quiescence |

`.bss` is a fixed linked section: it does not grow during execution. “BSS overflow” can mean a linker region-capacity failure, incorrect startup ranges, corruption of a global object, or dynamic regions colliding under a custom layout. Write the exact mechanism in the dossier; each needs different evidence. T45 supplies separate models and actual map/startup observation tasks.

The held explanation {ref}`R-effective-c-2`, historical detector mechanism {ref}`R-MEM-ASAN-PAPER-2012` and current bounded tool/linker readings complement one another. Use ASan/UBSan for their selected checks, separate unsanitized Memcheck for supported comparison, and LSan for the declared leak phase. MSan is a conditional experiment with its own platform and external-code instrumentation requirements. A detector's silence is not a complete ownership or correctness proof. Actual board observations remain separate from hosted tools and finite histories.

## Experiment dossier and delayed transfer

Keep contract and units; predicted symptoms and competing causes; source/image/input/tool/environment identities; finite timeout and capacities; raw observed events; first invalid transition and discriminating evidence; reduced trigger; causal repair; regression/measurement matrix; and remaining claims. Retain the original failing case and a nearby case that rejects a symptom-only patch.

Practice with guidance first, then receive a symptom-only variant whose cause and oracle are withheld until the dossier is complete. A reviewer or peer can prepare an unseen case. For solo work, delay and use an unopened seeded variant, then disclose what the generator or prior work already revealed; self-authored known faults have weaker independence. A public supplied oracle replay is regression practice, rather than an unseen diagnosis gate.

After roughly one to two weeks, change capacity, failure point, build configuration, nesting or completion order and repeat the reasoning. A delayed failure reopens that claim. A repaired defect passes only when the invariant, regression and changed-condition evidence agree. Static fixture checks accept the supplied finite contracts; learner C, actual debugger/profiler/tool records and physical measurements remain separate activities.

<!-- generated drill directory -->

## Project and drill directory

These milestones are already charged in their owning modules. The directory adds **0 hours**. Reuse the named prior implementations; each new brief budgets its additional experiment, diagnosis or repair.

### Complexity and performance measurement

| Project | Experiment or drill | Hours |
|---|---|---|
| {ref}`PR-T43-01` | A cost contract before a stopwatch | 8 |
| {ref}`PR-T43-02` | Prefix credits for a reused growing table | 6 |
| {ref}`PR-T43-03` | Instructions, graph cycles and a constrained decision | 8 |
| {ref}`PR-T43-04` | Simultaneous storage and a conditional stack certificate | 8 |
| {ref}`PR-T43-05` | Block traffic rather than a cache-time guess | 8 |
| {ref}`PR-T43-06` | Return is not completion | 8 |
| {ref}`PR-T43-07` | Accepted bytes, buffered time and requested sync | 10 |
| {ref}`PR-T43-08` | Paired observations with an honest tail | 8 |
| {ref}`PR-T43-09` | PMU estimates with unavailable outcomes | 8 |
| {ref}`PR-T43-10` | Read-only cgroup scope and conditional control | 10 |
| {ref}`PR-T43-11` | Compiler observations are a changed program | 10 |
| {ref}`PR-T43-12` | A conditional Cortex-M cycle certificate | 10 |

Owning-module project work: **102 hours**.

### Debugging from symptoms to causal evidence

| Project | Experiment or drill | Hours |
|---|---|---|
| {ref}`PR-T44-01` | Evidence-ready warning and logging dossier | 6 |
| {ref}`PR-T44-02` | Tri-state minimal reproducer | 8 |
| {ref}`PR-T44-03` | First invalid state with GDB | 10 |
| {ref}`PR-T44-04` | Optimized image and saved-state investigation | 8 |
| {ref}`PR-T44-05` | Memory evidence reconciliation | 8 |
| {ref}`PR-T44-06` | Partial I/O and descriptor autopsy | 10 |
| {ref}`PR-T44-07` | Race, deadlock and lost-wakeup dossier | 8 |
| {ref}`PR-T44-08` | Reset, HardFault and DMA investigation | 10 |
| {ref}`PR-T44-09` | Tool disagreement and differential diagnosis | 8 |
| {ref}`PR-T44-10` | Delayed blind causal repair | 6 |

Owning-module project work: **82 hours**.

### Memory, storage and lifetime failure laboratory

| Project | Experiment or drill | Hours |
|---|---|---|
| {ref}`PR-T45-01` | Prediction and controlled observation contract | 4 |
| {ref}`PR-T45-02` | Extent, underflow and terminator repair | 8 |
| {ref}`PR-T45-03` | Checked sizing and narrowing repair | 8 |
| {ref}`PR-T45-04` | Stack capacity, high water and exhaustion experiment | 12 |
| {ref}`PR-T45-05` | Fixed RAM, startup and collision diagnosis | 8 |
| {ref}`PR-T45-06` | Leak and acquisition-prefix repair | 8 |
| {ref}`PR-T45-07` | Resize, stale alias and release repair | 12 |
| {ref}`PR-T45-08` | Initialization and publication repair | 8 |
| {ref}`PR-T45-09` | Asynchronous lifetime and reentrant scratch repair | 10 |
| {ref}`PR-T45-10` | Unseen memory causal repair dossier | 8 |

Owning-module project work: **86 hours**.

