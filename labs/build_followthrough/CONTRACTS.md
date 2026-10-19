# T33/T34 followthrough laboratory

C17 is the target language. GCC/Clang, Python, CBMC, Frama-C, profiler and sanitizer runtimes belong to the hosted evidence harness. They do not expand the permitted component or firmware dependencies. The original project briefs, profiles and assessment objects remain authoritative. `criteria.json` binds every original task and changed condition to a local mechanism; it is an exercise map, not an execution receipt.

Build the incomplete learner core with `python build.py --kind formal --output /tmp/formal` or `--kind performance --output /tmp/performance`. Supply an explicit `--source` only when reviewing a separate implementation. Expected starter behavior is a named `BF_CHECK`/`BP_CHECK` semantic failure, never a compile failure. Keep prediction, derivation, independent counterexample, execution manifest and delayed transfer notes separately. Assessor files are excluded from normal export. No author reference run records an independent learner success.

## PR-T33-01: a scalar sum with a commit boundary

`bf_sum` is a scalar total, distinct from the original foundations prefix-array provider. The caller supplies live `unsigned` storage for `extent` elements and a writable output. The API rejects n>4, n>extent, absent output or absent nonempty input; invalid values exceed 10. It leaves output unchanged on rejection and returns zero for empty input. Baseline callers keep output disjoint. The changed contract admits output equal to one input element: validate and read all input before the single output store. Lifetime/extent are caller obligations, not recoverable from arbitrary pointers.

Use a mathematical sequence A fixed at entry. Before iteration i, write `0<=i<=n`, `acc=sum(A[0:i])`, `0<=acc<=10*i`, with every remaining element live and unchanged. Establish initialization at i=0, substitute the addition and increment into preservation, then use i=n at exit. The bound alone cannot imply the functional sum. Prove n-i strictly decreases without underflow because i<=n precedes subtraction. There are n input reads/additions and constant stack; validation combined with accumulation still has one pass. The pair-loop variant needs i even and a single residual tail. A maximum requires an explicit BF_EMPTY status. For signed inputs use a guard before every addition; cancellation at the end does not repair an overflowing intermediate.

Separate safe access, correct result and termination. A fixed-cell loop with a volatile condition never changing can be safe but fail progress; a routine returning zero can terminate safely with the wrong result. Do not execute unbounded counterexamples to establish termination. For callbacks, a finite call budget yields bounded attempts, not a bound on a single blocking callback's wall time.

## PR-T33-02: machine models and annotations

Trace expression types before writing a mathematical equality. On the declared host with `uint8_t`, 250 and 10 promote to int, sum to 260, then narrow to 4. `UINT_MAX+1u` wraps during unsigned-int addition. `INT_MAX+1` is never executed as a defined C test; a signed overflow guard or precondition is required, including when discussing C23 representation.

Write requires/ensures/assigns and loop clauses for the trusted kernel and a separate total validation entry point. `sum_wp.c` deliberately starts with a range proof and a false goal; learners must add the functional relation and public-validation/frame proof. A range-only inner contract does not prove the scalar sum or failed public calls. Keep exact tool version/machdep, generated goals, unsupported features and solver outcomes. The frame mutation writes a live neighboring element: correct return value alone cannot reject it. Alias permission changes frame and old-state postconditions. `sum_cbmc.c` can deliberately make the domain contradictory; reachable witnesses for n=0 and n=4 are part of validation. The current harness obtains them by asking for counterexamples to `assert(n != BF_WITNESS)`, with normal unwinding checks retained; it does not treat a tool crash as coverage.

## PR-T33-03: nondeterminism and the environment

Use actual arrays, nondeterministic lengths 0..4, initialized values 0..10 and protected output storage. Enumerate loop identifiers with `--show-loops`; retain both insufficient and sufficient `--unwind` runs with unwinding assertions and bounds/pointer/signed-overflow checks. Four body iterations require a fifth terminating guard evaluation in the inspected convention. Reachability/cover mode is separate from assertion proof. Keep the full counterexample before replay/reduction; a failed unwinding obligation is not necessarily a memory counterexample.

For allocation, branch on NULL versus a live owned object; release only successful acquisition, once. A writer at remaining=4 can return -1 (error), 0 (no progress), 1 (partial), or 5 (malformed adapter). Validate the return before changing offsets. Record the committed prefix on partial-then-error and never replay it. Read-to-clear hardware needs mutable device state: two reads are correlated. Sequential proof excludes asynchronous interrupts, reset and power loss unless a new transition model explicitly includes them. Adding a requirement reopens the claim.

## PR-T33-04: goal provenance

Maintain rows for loop initialization, preservation, result, frame and each runtime error. Give each one `proved`, `unproved`, `assumed`, `unsupported` or `skipped` plus source dependencies. Do not sum these into a success percentage. A lemma asserted inside the body must be proved; an external callback contract remains trusted until its body is analyzed. Preserve the original failing goal while separating false property, missing invariant, solver limit and unsupported model. Raising the value bound invalidates the accumulator bound; switching to float invalidates integer/Real reasoning about IEEE order, exceptions and NaNs.

## PR-T33-05: abstraction and composed evidence

For x in [-1,2], x+1 is [0,3], and x=-1 is a concrete zero-divisor witness. Only a caller that enforces x>=0 justifies [0,2] and [1,3]. Correlation y=x+1 is stronger than independent intervals: y-x is exactly 1 even though Cartesian interval subtraction contains zero. Track machdep, library specifications, volatile reads and unsupported interleavings. A length check followed by an ISR changing length before access defeats a purely sequential proof.

Give bounds, sum, lifetime, latency and physical energy their own claim rows. CBMC is bounded; WP depends on contracts and prover; Eva overapproximates its admitted environment; tests cover actual executions; timing instruments and calibrated energy measurements answer separate questions. Record compiler, solver, parser, library and hardware model as trusted dependencies. Source/model/tool/ABI changes invalidate dependent rows without erasing independent arithmetic derivations. Expand length or alias policy only after revising all affected contracts, unwinds, invariants, bounds and tests.

## PR-T33-06: finite publication

`models.py` enumerates all six order-preserving placements of two producer and two consumer events. Consumer reads only if its observation saw publication. Explain each trace, including the one bad early-publication trace. A second epoch must distinguish stale generations; boolean ready is insufficient. In C17, a release store read by an acquire load connects earlier payload writes to later reads, provided lifetime is stable and no concurrent rewrite races. That refinement is separate from the SC enumeration. Two writers need ownership arbitration; finite safety exploration supplies neither fairness nor eventual scheduling.

## PR-T34-01: workload and cost boundary

Specify exact modulo-2^32 addition for live disjoint n-element uint32 objects. Tiny lengths 0..9 test dispatch and tails; measured strata include 8, 4096 and 65536 elements. Record units and workload weights before comparing throughput, per-call latency, memory and energy. A bimodal workload needs per-stratum tails; its most common tiny requests cannot be represented by a huge-buffer average. Floating reductions have a separate order/exception/error contract: strict left-to-right cancellation, NaN and signed-zero cases do not authorize reassociation.

At n=64 there are 64 additions and 64*(4+4+4)=768 logical bytes: 1/12 operation/byte. UInt64 outputs change this to 1024 bytes and 1/16. At a declared 100 Gop/s compute and 12 GB/s bandwidth boundary the synthetic roof is 1 Gop/s. Cache-line traffic, write allocation, dependency latency and dispatch costs need separate evidence; the arithmetic is neither an observed rate nor WCET.

## PR-T34-02: locality and dependencies

Count synthetic 16-byte lines explicitly. Contiguous addresses 0,4,8,12 touch one; 0,16,32,48 touch four; offset 4 makes the contiguous set touch two. A page-stride experiment tests a different TLB/cache hypothesis. AoS-to-SoA keeps stable IDs through deletion, and blocked traversal must preserve padded edges. Reading every cold label changes the value-only workload and can erase a locality benefit.

Both clamp implementations produce [0,0,3,4] for [-2,0,3,5]. Compare their final dependency graphs rather than source branch counts. A subtract-and-mask shortcut can overflow at INT_MIN/INT_MAX; comparisons and selection avoid that arithmetic. Time constant, alternating and seeded inputs separately. Compilers may emit identical instructions. Unroll by 1/2/4/8, preserve tails, and record linked code bytes, stack-use files and actual spills; neither code size nor an assembly guess proves instruction-cache misses.

## PR-T34-03: reports and valid promises

Retain the compiler's optimized/missed reports, selected IR, object and linked instructions for each exact flag set. Match loop source locations to guards, vector bodies, tails and helpers. Independent addition, recurrence and strict floating reduction have different dependencies. A prefix sum requires a different parallel algorithm, not an alias hint.

Only promise restrict when the owning caller proves disjoint object extents. Exact in-place output uses `bf_add_inplace` without the disjoint promise; partial overlap remains forbidden. An untrusted adapter should pass handles and validated offsets within one known allocation. General ordering comparisons between unrelated pointers do not validate arbitrary memory. Reinspect final code after -O changes and LTO. Keep contraction/reassociation separate from the strict floating gate.

## PR-T34-04: ISA and dispatch boundary

SSE2 processes four uint32 lanes per full 16-byte load/store and uses a scalar tail. The typed interface admits offsets 0,4,8,12 modulo 16, not arbitrary misaligned uint32 pointers. Each vector access still needs 16 valid bytes. Stride 12 cycles through those offsets; aligned allocation does not align every access. Exact allocation-end tests at n=0..9 catch short-tail overreads under sanitizers.

Compile scalar/selector and specialized code in separate units. On the declared x86-64 Linux ABI SSE2 state support is baseline; the selector nevertheless tests feature values 0,1,8, OS support and forced scalar. Feature truth means positive, not exactly one. Inspect final targets under plain/LTO builds. A different ISA or masked load requires its own lane, extent and fault-suppression contract. AVX needs OSXSAVE/XCR0/state support. An ifunc resolver executes before ordinary initialization; explicit dispatch avoids assuming constructor state is ready.

The separate AVX2 transfer interface uses eight32-bit lanes, full32-byte unaligned accesses and scalar tails. `avx2_test.c` checks CPUID AVX/OSXSAVE, XCR0 XMM/YMM state and CPUID AVX2 before calling that separately compiled specialization. It exercises lengths0..17 and all typed offsets modulo32. The Cortex-M4 firmware build excludes the specialized translation units entirely, demonstrating a scalar-only target build without claiming execution on that board.

## PR-T34-05: measurement integrity

Initialize data outside timing, warm explicitly, alternate baseline/variant pair order, consume results and preserve raw nanoseconds/checksums. Per-iteration input changes and separately compiled kernels stop simple invariant-hoisting without making payload volatile. Record the checksum/input-update overhead: this measures an end-to-end loop, not isolated additions. A cold-start objective requires a separate protocol. Affinity is uncontrolled unless the actual call succeeds; do not change host security settings.

The synthetic samples [100,110,90,105,95] have median 100 and spread 20. Actual timing rows need units/configuration/strata and must retain regressions. Hardware counters are raw count plus enabled/running durations; running=0 is invalid. 1000*20/10=2000 only estimates unsampled intervals under stationarity. Sampler addresses must match the linked binary; inlining requires machine-level attribution. Permission/support failures stay open, and Valgrind simulated cache events are not hardware PMU observations.

The hosted runner separately collects software cpu-clock samples, a gprof periodic histogram/call-count view and Linux getrusage minor-page-fault deltas when supported. These have distinct boundaries: gprof instrumentation changes the program, CPU samples exclude blocked time, and process minor faults are not hardware cache/TLB events. Preserve raw sampling data and exact ELF identities, not only a formatted percentage table.

## PR-T34-06: whole-program and target decisions

Use hosted plain/LTO/PGO builds with identical semantic gates. Preserve every compile/link flag, profile training distribution and opposite held-out distribution, compile time, peak resident memory, linked bytes and raw runtimes. Archive extraction needs plugin-aware tools. Source/profile or compiler-version mismatches require new profiles; preserve diagnostics rather than suppressing them.

Audit undefined helpers, maps, gcov collection/file/exit support and strict firmware separately. `BOARD_CAPTURE.md` is the required physical record, currently empty. Synthetic variants (1000,80,200), (1400,112,160), (1250,96,175) respectively fail latency, fail Flash and pass budgets (1300,128,180). Energy remains an independent physical objective. Acceptance requires semantic AND resource gates. Record regression thresholds and the rule for overlapping/noisy timing distributions before optimizing; compiler/workload/board changes require relevant revalidation even when source is unchanged.
