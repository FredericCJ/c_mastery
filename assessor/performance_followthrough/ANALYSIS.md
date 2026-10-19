# T43 exact assessor derivations

Author evidence only. Independent review, delayed learner performance, target PMU availability and physical observations have separate authority.

## AX-PR-T43-01-T43.01.a

Original task: Add a variable-length byte-key comparator to the retained stable insertion sort. Count comparator calls and inspected key bytes separately for [aa,ab,a] with explicit lengths; specify prefix ordering, failure admission and output permutation.

The byte-key adapter preserves identity and all-admission-before-write. aa,ab,a requires3 comparator calls but4 inspected byte pairs and sorts a,aa,ab. Undefined cost overflow disables only the ledger.

Changed condition: Transfer to graph edges with variable-length labels and output-sensitive matching.

Reachable variable-label graph emits9 matches across2vertices/3parallel edges; byte inspection is a separate9-work term, not unit label cost.

Evidence selectors: core:sort, graph.

## AX-PR-T43-01-T43.01.b

Original task: Instrument the accepted insertion kernel at comparison, shift and placement events, then enumerate all arrays of length0..6 over keys0..2 with identity tags. Compare shifts with independently enumerated inverted index pairs and detect counter overflow before increment.

All1093 ternary identity arrays through6 compare to retained f_sort: shifts equal independently counted inversions, placements max(n-1,0), stability/permutation retained. SeedUINT64_MAX saturates and reports unavailable.

Changed condition: Replace insertion with binary insertion and derive which count identity survives.

Upper-bound binary insertion shifts the same inversion pairs and retains stable identity; only comparator search work changes. Both variants execute every exhaustive case.

Evidence selectors: core:sort.

## AX-PR-T43-01-T43.01.c

Original task: For one accepted compiled kernel, make three ledgers: semantic events, an actually executed instruction path, and the static connected CFG. Find two inputs sharing a CFG but differing in loop iterations and explain each observation boundary.

Single retained assembly binary has empty and duplicate-boundary traces. Executed memory compares match semantic algo_lower comparison counts; static code is unchanged while loop instances differ. Weighted cycles remain model-only.

Changed condition: Enable inlining and explain how function boundaries and counts change without changing the source contract.

Inline-permitted compilation and its separate disassembly expose changed C call boundaries. Accepted immutable extent/result/checksum are unchanged; instruction counts require the new binary.

Evidence selectors: trace, inline.

## AX-PR-T43-01-T43.02.a

Original task: Prove bounds for f(n,m)=3n*m+5n+7 for integers n,m>=1, giving positive constants and thresholds; explain why holding m fixed changes the one-parameter statement. Check boundary arithmetic independently.

For n,m>=1,3nm<=f and5n<=5nm,7<=7nm give f<=15nm. Fixed positive m yieldsTheta(n);m=n givesTheta(n^2). Finite enumeration checks arithmetic without replacing the universal inequalities.

Changed condition: Allow m=0 and derive a valid two-parameter expression including n+1.

For n,m>=0 use g=nm+n+1. g<=3nm+5n+7<=7g, including m0. Theta(nm) alone would vanish on a valid linear case.

Evidence selectors: certificates:bounds.

## AX-PR-T43-01-T43.02.b

Original task: Extend the retained traversal certificate to a graph with parallel edges and repeated emitted matches. Define V,E,Z and count visit, edge inspection and output operations; exhibit a tiny graph whose Z exceeds V.

Retained CSR admits repeated parallel edges; retained match finds each destination-label occurrence. V2,E3,Z9 demonstrates Z>V. Traversal/edge/emission/byte counts stay separate.

Changed condition: Change output from materialized records to an interruptible streaming sink with partial progress.

The interruptible sink stops at attempted emission5 with4accepted, both identically matching the full output prefix. No failed emission is counted as accepted.

Evidence selectors: graph.

## AX-PR-T43-01-T43.02.c

Original task: Construct two nonnegative cost functions agreeing on every sampled n<=1024 but with different eventual growth, and two algorithms whose asymptotic order differs from their measured ordering within that interval.

f(n)=n,g(n)=n for n<=1024 and n^2 thereafter agree throughout the observed range. g/n grows unbounded. A100n versus Bn^2 crosses at100 independently of their eventual classes.

Changed condition: Require monotone functions and preserve indistinguishability on the measured interval.

Both functions are monotone nonnegative, so monotonicity does not rescue finite extrapolation. Actual31pairs at three working sets report measured intervals only.

Evidence selectors: certificates:finite-growth, pairs.

## AX-PR-T43-02-T43.03.a

Original task: Trace alternating growth and failed allocation in the accepted table. Keep successful append costs, failed-attempt work and maximum single-operation cost in separate columns; include a nonempty initial table.

Retained CDVector starts live5/cap8. Three failed growth attempts preserve pointer/extent/capacity/content/copies;35successful appends cost91, maximum33. Failed attempt count and allocator latency are not hidden in append cost.

Changed condition: Introduce deletion with shrink at half capacity and find a grow/shrink thrashing trace.

Grow8->16 then shrink at half-full on each deletion makes every alternating operation cost9. Eight such operations do not have the constant credit of insertion-only doubling.

Evidence selectors: core:table, certificates:potential.

## AX-PR-T43-02-T43.03.b

Original task: For the nonempty table live=5,capacity=8, retain Phi=2*live-capacity and derive every prefix through the next growth. Test a proposed certificate that drops Phi0; separately evaluate live=1,capacity=8.

Phi0=2 at5/8. Every measured prefix checks actual=amortized-PhiFinal+Phi0;105amortized corresponds91actual at40/64. DroppingPhi0 is an accounting error. For1/8,Phi0=-6.

Changed condition: Construct a piecewise potential supporting expansion and quarter-full shrinking.

Offset6 covers insertion-only reachable negative potential. For quarter-full shrink use2n-c above half andc/2-n below; all12-step reachable transitions preserve nonnegativity and amortized cost<=3.

Evidence selectors: core:table, certificates:potential.

## AX-PR-T43-02-T43.03.c

Original task: Replace fixed-cost element copies with records owning variable-size payloads and a fallible deep-copy operation. Identify each assumption of the retained doubling proof that changes and derive a byte-weighted sequence bound.

The retained clone batch records completed byte prefixes0,1,8 and37 across failures and success. For payload lengths1,7,29,2,100,3,5,11, doubling copies48 bytes plus158 append bytes, total206. Each payload is recopied at most ceil(log2 N) times; failed attempts add their actual completed clone prefixes.

Changed condition: Use move-only logical ownership with a constant-size handle and compare rollback requirements.

Move-only handles have constant descriptor size. Acquire replacement storage before transferring ownership, invalidate the old descriptors after publication, and permit no fallible operation after transfer. Allocation failure preserves old owners.

Evidence selectors: core:payload, certificates:payload.

## AX-PR-T43-03-T43.04.a

Original task: Choose a branching lower-bound search path with a duplicate-key boundary. Record taken instructions from the actual trace and classify loads, branches and arithmetic under a declared weighted model; reject the alternative of counting every disassembled instruction once.

GDB executes the actual retained branching lower bound on duplicate boundary2. Each PC visit is counted, including repeated loop instructions, and load comparisons equal the semantic ledger. The illustrative load2/branch3/other1 weights are separate from hardware timing.

Changed condition: Change branch direction without changing final index and compare mix and prediction.

n8/key3 and n7/key3 both return index4 but take different first directions and relativePC paths; native evidence is attached to each actual trace.

Evidence selectors: trace.

## AX-PR-T43-03-T43.04.b

Original task: Derive a speedup bound when40 percent of baseline time is improved twofold, then include a new serialization overhead of10 percent of original time. Use the parent260-cycle model only as a model and compare an actual changed working set separately.

40%improved2 gives1/(.6+.4/2)=5/4. Adding.1original-time overhead gives10/9. The parent260cycle model remains synthetic. Actual size16/1024/65536 raw trials separately expose working-set dependence.

Changed condition: Overlap the improved phase with an unchanged phase and rederive the critical path.

Overlapping stages10and8 finish after10;halving10makesmax(5,8)=8, speedup5/4. Adding stage times would predict18/13 and answers a different system.

Evidence selectors: certificates:amdahl, pairs.

## AX-PR-T43-03-T43.04.c

Original task: Construct two connected CFGs with equal cyclomatic complexity but different nested-loop work; emit graph validation that rejects disconnected nodes and duplicate edges. Record code bytes independently from either graph metric.

The explicit sequential and nested graphs each have6 nodes and7 edges, metric3, while n8 work is16 versus64. Validation rejects duplicate edges and disconnected nodes. Separately compiled C functions report the two work counts and actual symbol byte sizes.

Changed condition: Add an early exit that changes feasible paths while keeping the graph metric unchanged.

Reroute the existing inner-scan termination continuation from the outer predicate to the exit. E and N stay unchanged, while sentinel-dependent termination now exits the whole scan. Feasible paths and work change despite metric3; adding a new independent branch would instead require recomputing the metric.

Evidence selectors: certificates:cfg, trace, symbols.

## AX-PR-T43-03-T43.14.a

Original task: Write acceptance limits for a reused pipeline covering latency, throughput, retained bytes and traffic; select a minimal justified measurement vector and state why each omitted observable does not decide this requirement.

Latency, throughput, retained bytes and traffic form the minimum vector for the four requirements, each with units and thresholds. Energy is unavailable and not inferred from elapsed time.

Changed condition: Add a hard stack bound and determine which prior evidence no longer suffices.

A new hard stack bound invalidates a timing-only decision until call paths, indirect targets, recursion, interrupts and dynamic frames are bounded. Actual .su qualifiers expose the unresolved work.

Evidence selectors: certificates:decision, stack, pairs.

## AX-PR-T43-03-T43.14.b

Original task: Resolve the parent faster-but-larger candidate comparison, then construct a Pareto tradeoff where neither variant dominates in latency and retained storage. Require equivalent correctness and failure contracts before ranking.

Synthetic B fails the code and storage limits despite lower modeled cycles. Actual binary search and retained lookup return equal query results with262144 versus786436 retained bytes; timing order is disclosed.

Changed condition: Require a stricter tail deadline while allowing more retained memory.

A stricter tail deadline may favor a larger candidate only after fresh tail evidence and every hard limit pass. A median advantage cannot certify a deadline or override a storage budget.

Evidence selectors: certificates:decision, pairs.

## AX-PR-T43-03-T43.14.c

Original task: Freeze a reproducibility dossier with model proof, finite oracle, actual runtime observations, unavailable capabilities and pending physical evidence in separate records. Introduce a deliberately stale binary hash and reject the mixed result.

The dossier binds commands, raw outputs, current hashes and separate model, finite, native and unavailable categories. The guard rejects stale hashes and missing mechanisms while preserving historical receipts.

Changed condition: Ask a second reader to reproduce one changed-condition decision without author explanations.

Second-reader reproduction and the minimum-seven-day unaided learner attempt remain unobserved. Author reruns cannot fill either dimension.

Evidence selectors: guard, certificates:decision.

## AX-PR-T43-04-T43.05.a

Original task: Extend the lifetime fixture with two borrowed views of work and a scratch interval ending exactly when result begins. Implement a sweep over event times and compute live, capacity, retained and slack peaks without counting views as owners.

Half-open ownership sweep gives peaks live144@3,capacity224@7,retained128@10,slack136@7. Two views contribute0owned bytes. Scratch ending at result start does not overlap.

Changed condition: Transfer an owner between components at the same timestamp and preserve global accounting.

Transfer at7 changes component attribution while total224is unchanged. Events at equal timestamps are evaluated after ends and starts as one half-open state, not an order-sensitive transient.

Evidence selectors: certificates:lifetime.

## AX-PR-T43-04-T43.05.b

Original task: Parse actual static, dynamic,bounded and unbounded .su records from the selected build. Add one unknown indirect callee and a mutually exclusive branch; compute only feasible known call-chain sums and return unknown for the unresolved path.

Actual GCC stack records include static64/128 frames, bounded outgoing-call40, dynamic VLA32plusunknownextent. Only feasible chains combine; unknown indirect edges and unbounded extents leave whole-program bound null.

Changed condition: Enable LTO and rebuild the call graph before combining frame data.

LTO binaries get their own direct/indirect callgraph and symbol sizes; removed/inlined symbols cannot import old simultaneous frames. We retain null whole-program certification.

Evidence selectors: stack, lto.

## AX-PR-T43-04-T43.05.c

Original task: For a bounded parsing pipeline, report symbolic O(n) payload, actual reserved bytes, allocator overhead, resident pages and sampled stack separately. Explain a run where freed payload falls but RSS stays high.

The actual owned1MiB allocation reports live0->1048576->0 and three RSS page readings. Allocator metadata stays unknown. A bounded parser has separate symbolic inputn and reservedcapacity; neither equals sampledRSS or .su.

Changed condition: Move scratch from heap to a bounded stack frame and reassess the requirement.

Replacing80heap scratch by bounded80stack shifts the requirement vector: reduce heap capacity and include the frame only on its feasible chain. A prior heap-only limit cannot certify new stack headroom.

Evidence selectors: host:memory, certificates:storage-spaces, certificates:lifetime.

## AX-PR-T43-05-T43.06.a

Original task: Implement the parent's two-block LRU write-back adapter and compare every reference sequence of length<=5 over blocks0..2 and read/write actions with a simple independent list model. Keep CPU cache fixtures out of this policy.

Independent ordered-list oracle compares every9331reference sequence through5 against two-slot C timestamp LRU, for reads/writes over3blocks. Dirty evictions/finalflush happen once;invalidindex leaves state unchanged.

Changed condition: Replace LRU with FIFO and construct a sequence that distinguishes them.

FIFO runs the same exhaustive corpus against FIFO oracle. Blocks0,1,0,2,0 distinguish LRU24readbytes fromFIFO32; changing a label without changing recency logic fails.

Evidence selectors: core:cache.

## AX-PR-T43-05-T43.06.b

Original task: Add write-only, repeated-read and empty streams to the traffic ledger. Preserve read, written and useful byte totals as integers and represent an undefined amplification ratio explicitly.

Write-allocation counts an8byte read on cold write and an8byte dirty flush. Eight reads of one warmblock yield8/32traffic/useful;emptydenominator is undefined. Integer counters precede rational interpretation.

Changed condition: Use compressed blocks and distinguish logical useful bytes from encoded transfer bytes.

Compressed logical64bytes with encoded12bytes yields3/16at that boundary. Report encoded transfer/logical use separately;do not substitute uncompressed blocksize in numerator.

Evidence selectors: core:cache, certificates:traffic-ratios.

## AX-PR-T43-05-T43.06.c

Original task: Run the same disposable-file read twice through the accepted adapter, retaining accepted-byte/syscall observations and permitted device accounting. Predict cache/readahead/shared-work hypotheses before collecting data.

Three actual reads return identical8192byte contents and retain syscall counts/times. Devicebytes remain null because no exclusive device attribution exists;no global cache manipulation occurred.

Changed condition: Repeat after an unrelated local process accesses the same file and assess attribution limits.

Thirdread follows a separate owned process reading the samefile. It tests this sequence only;shared pagecache/readahead/otherprocess activity preclude assigning all device traffic to this call.

Evidence selectors: host:read-pass.

## AX-PR-T43-06-T43.07.a

Original task: Instrument submit, caller return, completion, callback return and release for the retained async adapter. Include completion-before-submit-return and cancellation-request-without-quiescence schedules.

Retained FAsync cancel returns pending without release;complete releases device state once. Ownerwrapper remains borrowed through caller andcallback, including completion-before-return.

Changed condition: Permit a callback to enqueue a successor using the same owner and derive the new lifetime interval.

A callback-enqueued successor adds an accessor interval. The release boundary becomesmax(callerReturn,callbackReturn,successorCompletion)=9in the checked model, not prior3.

Evidence selectors: core:async, certificates:async.

## AX-PR-T43-06-T43.07.b

Original task: Compute latency, throughput and occupied-time union for three overlapping requests, including a failed completion excluded from successful-item throughput. Publish the exact observation window and censoring rule.

Three overlapping0..8,2..10,7..14requests have latencies8,8,7. Window0..12contains1success,1failure,1censored;successfulthroughput1/12and occupiedunion12. Failure/censor never become fastsuccess.

Changed condition: Use a sliding observation window containing requests submitted earlier.

Window5..12 includes prewindow requests in occupancy;throughput counts its successfulcompletions while cohortlatencies retain original starts. Different populations cannot share a silent denominator.

Evidence selectors: certificates:async.

## AX-PR-T43-06-T43.07.c

Original task: Measure process and per-thread CPU time around two worker threads plus a blocking wait. Compare their scopes with elapsed time and explain a negative elapsed-minus-process-CPU value without calling it negative waiting.

Actual two-worker/main-blocking run captureselapsed,processCPU,mainthreadCPU,andbothworkerCPUs. The observed signedwall-minus-process is retained without calling it waittime.

Changed condition: Include a child process and determine which CPU accounting clocks change.

OwnedchildCPU is recorded with waited-childrusage;parentprocessclock measures onlyparentthreads. ActualchildpositiveCPU and rawparentCPU demonstrate the accounting scopes.

Evidence selectors: host:clocks, host:child-clock-scope.

## AX-PR-T43-07-T43.08.a

Original task: Inject positive partial progress, EINTR, EAGAIN, zero and terminal EIO at each cursor boundary of the retained eight-byte write. Track attempt count and accepted prefix; reject impossible over-reported counts before arithmetic.

The retained writer executes40 cursor/fault cases. Positive results advance once; retries consume budget; zero, EIO and over-reporting stop with the exact accepted prefix. Four attempts of3,retry,2,retry end at prefix5.

Changed condition: Add a vectored write spanning two buffer boundaries.

The vectored cursor crosses extents3,2,3 through six partial writes and yields abcdefgh exactly once. Each borrow is limited to the current live span.

Evidence selectors: core:write.

## AX-PR-T43-07-T43.08.b

Original task: Collect separate buffered-write and requested-file-sync observations in a disposable directory, including an injected sync error. Retain file creation/close lifetime and distinguish requested durability from a crash experiment.

The owned file records accepted bytes, write interval, actual fsync interval and status. A local synchronizer injection returns failure despite readable bytes; it is explicitly not a kernel error observation.

Changed condition: Publish by temporary file plus rename and identify the additional directory synchronization boundary.

The actual sequence is write8, file fsync, rename, directory fsync. These requested boundaries do not establish survival through power loss.

Evidence selectors: host:write-sync, host:injected-fsync-error, host:publish.

## AX-PR-T43-07-T43.08.c

Original task: Reconcile eight accepted bytes, logical file size and a shared4096-byte device observation. Include sparse-file allocation and delayed writeback as competing explanations; state what further measurement distinguishes them.

The captured shared4096/logical8 ratio is512 only for that scope. Actual sparse-file length, allocation blocks and logical payload are separate; physical traffic remains unavailable.

Changed condition: Compare a preallocated file with a newly created sparse file under equal byte correctness.

Preallocation preserves the same8 payload bytes while reporting actual allocation units before and after. st_blocks is storage allocation, not device write traffic.

Evidence selectors: certificates:traffic-ratios, host:sparse-preallocated.

## AX-PR-T43-08-T43.09.a

Original task: Implement checked timespec subtraction and nanosecond conversion with a declared maximum duration. Handle borrow across seconds, equal timestamps, reversed inputs and overflow; record actual clock identity and getres result.

C interval arithmetic accepts a1ns borrow and equal zero, then rejects reversed, unnormalized and overflowing intervals without publishing. Actual clock resolution is recorded separately from bracket cost.

Changed condition: Use a wrapping hardware counter with an independently justified less-than-one-wrap bound.

The wrapping counter requires an independently supplied bound below2^32. Its observed modular delta cannot establish that bound; a full-wrap allowance is ambiguous.

Evidence selectors: core:interval, host:clocks, core:dwt.

## AX-PR-T43-08-T43.09.b

Original task: Extend the paired harness with31 raw pairs, randomized or alternating order, warmup and a consumed checksum. Compare median paired difference with difference of medians; preserve the pairing before sorting summaries.

Actual31 pairs at three sizes retain pair identities, alternating order,8192 queries and checksums. Median paired difference and difference of medians are separate statistics; the parent example gives3 versus4.

Changed condition: Introduce a drift trend and compare blocked versus alternating trial order.

Synthetic linear drift gives31 units of blocked-order bias versus1 with alternation despite zero treatment effect. Actual scheduling noise remains visible and no significance claim is inferred.

Evidence selectors: pairs, certificates:samples.

## AX-PR-T43-08-T43.09.c

Original task: Create a tail report with an explicit nearest-rank rule, strict deadline relation and retained outliers. Add one delayed trial and show how the estimated tail changes; distinguish observed maximum from an asserted WCET.

Nearest-rank p95, strict misses, sample size and maximum retain outliers. Adding a10000-unit trial changes the tail; no finite maximum becomes WCET.

Changed condition: Introduce censored timeout trials and derive a report that cannot hide them as missing data.

Two timeout censors remain in the population. With one observed miss the miss lower bound is3; a quantile beyond the censor threshold remains unresolved.

Evidence selectors: certificates:samples, pairs.

## AX-PR-T43-09-T43.10.a

Original task: Collect one permitted or denied per-process PMU attempt for the reused kernel and retain CPU, kernel, event, scope and binary identities. Classify unavailable, unsupported and observed-zero results separately.

Four actual caller-thread hardware event requests retain event identity, status, errno or raw count, CPU model, kernel and binary hash. They currently return ENOENT unavailable; no host policy is changed.

Changed condition: Move the same request to another CPU model and identify event meanings requiring new authority.

A different CPU requires new event authority and actual observations. A generalized event name cannot transfer cache or retirement semantics by itself.

Evidence selectors: pmu, environment.

## AX-PR-T43-09-T43.10.b

Original task: Implement checked grouped-counter decoding for raw1200/3000, enabled20,running10, then test zero-running, running>enabled, mixed group IDs and arithmetic overflow. Preserve raw counts alongside scaled rational estimates.

Checked rational scaling gives2400 and6000 and CPI5/2. Zero-running, running greater than enabled, mixed groups, duplicate IDs and overflow reject. A measured raw zero is valid.

Changed condition: Use two nonstationary phases with identical aggregate running fraction but different sampled phases.

Two equal-duration phases with1000 and100 events yield estimates2000 or200 depending on the sampled phase, despite the same10/20 fraction. Scaling assumes representativeness.

Evidence selectors: core:pmu, certificates:multiplex-phases.

## AX-PR-T43-09-T43.10.c

Original task: Compare the work contract with available instruction/branch/cache events or explicitly unavailable evidence. Give two hypotheses for equal instructions but different cycles and choose a discriminating additional observation.

Semantic work and elapsed times are available while hardware instruction, branch and cache counts remain null. Frequency and memory-stall hypotheses need different discriminating observations.

Changed condition: Change branch predictability while keeping semantic work fixed.

Alternating and LCG condition streams retain one million comparisons and separate checksums/times. Linked disassembly is retained because a source if may become branchless; branch-miss causality remains open.

Evidence selectors: pairs, pmu, certificates:multiplex-phases.

## AX-PR-T43-10-T43.11.a

Original task: Write a read-only inventory/parser for a captured or permitted cgroupv2 hierarchy. Classify cpu.max, cpu.stat, memory.current, memory.peak, io.stat and ancestor policy; preserve absent controllers as unknown.

The observer reads existing membership, mount information, ancestor limits, CPU, memory, IO and pressure fields. Missing data stays unknown; policy and membership writes are zero.

Changed condition: Move the workload into an already prepared delegated subtree in a separate authorized experiment.

No already-prepared owned delegated subtree was selected. An isolated quota experiment remains a separate conditional observation and cannot be replaced by these shared hierarchy snapshots.

Evidence selectors: cgroup.

## AX-PR-T43-10-T43.11.b

Original task: Parse keyed snapshots with reordered fields, new optional keys, a decreasing gauge and a reset cumulative counter. Derive per-device deltas and refuse a fresh-window memory peak without the required same-descriptor reset history.

Keyed parsers handle reordered and optional fields. A decreasing gauge is valid; a decreasing monotonic counter is a reset. Device8:0 matches by identity before an8192-byte delta; read-only peak stays historical.

Changed condition: Add a disappearing device and shared-inode writeback attribution.

Disappearing8:1 and new8:2 produce unknown deltas. Shared-inode writeback remains an attribution limit even when device parsing is correct.

Evidence selectors: certificates:cgroup, cgroup.

## AX-PR-T43-10-T43.11.c

Original task: Compare a bounded workload with unchanged read-only observations and the object-lifetime ledger. Explain why quota ratio, observed CPU fraction, pressure and retained payload need not agree.

The bounded owned workload runs between read-only snapshots. Its0/1MiB/0 payload ledger stays separate from aggregate cgroup memory, CPU and IO. Quota fractions and overlapping PSI do not force equality with object costs.

Changed condition: Use a captured throttling history with burst and ancestor limits.

Captured child quota.8 and ancestor.5 cannot promise.8 CPU share. Burst200 and throttle300 are scoped records, not quantities to force into wall-minus-CPU or summed PSI.

Evidence selectors: cgroup, certificates:cgroup.

## AX-PR-T43-11-T43.12.a

Original task: Build matched baseline, bounded entry/exit-hook and supported-pg variants of one existing single-threaded kernel. Verify compile and link flags, recorder exclusion and normal profiling-output lifetime.

The same retained kernel runs as baseline, compiler hooks and-pg at compile and link. The bounded recorder excludes itself; gmon is produced by normal exit in an owned directory.

Changed condition: Link an uninstrumented helper and identify the missing call boundary.

The executed uninstrumented helper has no hook boundary. Its nm address is checked against the trace so absence is disclosed instead of interpreted as zero executions.

Evidence selectors: profile.

## AX-PR-T43-11-T43.12.b

Original task: Implement a LIFO reducer for bounded enter/exit records, including recursion, an unmatched exit, a dropped record and a nonlocal exit. Compute exact inclusive/exclusive time only for complete justified spans.

The C reducer gives work9/4, child5/5 and recursive inclusive7/exclusive5. Dropped, open or unmatched traces invalidate exact totals. Actual hook logs have an independent reduction.

Changed condition: Replace recursion with interleaved task IDs and require a stack per context.

Interleaved task IDs need separate invocation stacks. Properly nested tasks0 and1 overlap and aggregate9 inclusive units; one global LIFO stack is insufficient.

Evidence selectors: core:hooks, profile.

## AX-PR-T43-11-T43.12.c

Original task: Measure matched instrumentation overhead and code/stack changes, then use callers with cheap and expensive inputs to falsify uniform child-cost attribution. Keep CPU samples, call counts and off-CPU elapsed time distinct.

Matched baseline, hook and-pg runtimes, code sizes and frame files expose instrumentation changes. Two expensive and eight cheap calls distinguish actual caller cost from uniform attribution; the captured4/16 estimate differs from16/4.

Changed condition: Batch calls to reduce bracket overhead and determine whether the new boundary changes the question.

Batch bracketing reduces timestamp cost per call but measures batch elapsed time and hides individual tails. CPU samples, call counts, inclusive wall time and off-CPU delay remain distinct.

Evidence selectors: profile, certificates:profiling.

## AX-PR-T43-12-T43.13.a

Original task: Audit the parent's captured DWT prerequisite record in order: exact core/device, trace access, feature presence, ownership, enable, clock and debug state. Reject reading presence fields while the documented trace gate makes them unknown.

The captured DWT certificate checks premises in order. Clear TRCENA makes feature presence unknown; absent, unowned and disabled have separate outcomes. No MMIO or trace ownership configuration occurs.

Changed condition: Use a device timer with a different clock and state its own access and ownership prerequisites.

A peripheral timer requires its own access gate, divider, clock domain, width and ownership. CPU frequency cannot substitute without source authority.

Evidence selectors: core:dwt, certificates:counter-premises.

## AX-PR-T43-12-T43.13.b

Original task: Implement modular delta and checked unit conversion for the supplied32-bit endpoints. Require an independent maximum interval, reject a bound permitting a full wrap and report overhead subtraction only as sensitivity.

The32-bit wrap from4294967280 to48 gives64 cycles. Bound1000 admits4000ns at16MHz; a full wrap or bound32 rejects. Subtracting overhead8 gives a separate3500ns sensitivity.

Changed condition: Change clock frequency during the interval and derive what can still be inferred.

With an unknown split between16MHz and8MHz,64 cycles bounds duration to4000..8000ns. A known32/32-cycle split gives6000ns; a single-frequency conversion is unsupported.

Evidence selectors: core:dwt, certificates:counter-premises.

## AX-PR-T43-12-T43.13.c

Original task: Produce a table separating captured arithmetic, emulated functional execution and dedicated-board cycle observations. Design a GPIO/timer cross-check with stated observer resolution and include a debugger-halt interval.

Captured arithmetic, prior functional guest work and physical observations remain separate. GPIO or timer cross-checks need actual resolution and halt evidence. This campaign supplies no silicon energy or WCET result.

Changed condition: Transfer the kernel to a cached Cortex-M7 and identify new memory-system assumptions.

Cortex-M7 needs fresh cache state, wait-state, DMA coherence, interrupt and clock premises. A functional pass or another core model does not transfer physical timing.

Evidence selectors: certificates:counter-premises.

