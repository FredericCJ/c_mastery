# T33/T34 assessor derivations

Withheld from normal learner export. Exact task and variant identities are unchanged. Native tests, deductive goals, finite models, host measurements and physical observations have separate authority.

## AX-PR-T33-01-T33.01.a

The abstract input is the entry sequence A, length n<=min(extent,4), with live unsigned elements <=10 and a writable output. The only successful write is *out=sum(A); rejection leaves it unchanged. n=0 writes zero. Maximum total is 4*10=40.

Changed condition: Allow output to alias one input element and revise access-order/frame obligations.

Alias a+1 for A=[3,4,5]: read all three values before publishing 12. Frame permits only a[1] to change; postcondition refers to old A, not the overwritten array.

Evidence selectors: formal, cbmc-unwind-5.

## AX-PR-T33-01-T33.01.b

A volatile fixed-cell polling loop can be bounds-safe and nonterminating. Returning zero on [3,4,5] is safe and terminating but functionally false. bf_sum separately establishes access, result and decreasing n-i.

Changed condition: The routine waits for callback progress and requires an environmental liveness assumption.

Callback zero forever defeats unbounded progress. A budget of three calls returns BF_PENDING with committed=0; termination of each callback itself remains an environmental assumption.

Evidence selectors: formal.

## AX-PR-T33-01-T33.01.c

n,extent,i use size_t; a[i] and acc use unsigned. Admitted ranges make unsigned addition equal integer addition. i increments only while i<n<=4. The single final store implements commit; 3+4+5=12.

Changed condition: Input uses signed values allowing cancellation and requires new intermediate bounds.

INT_MAX,1,-1 has fitting final mathematical value but its second intermediate overflows. bf_signed checks before addition and preserves output; 10,-10 succeeds with zero.

Evidence selectors: formal.

## AX-PR-T33-01-T33.02.a

acc<=40 admits acc=0 at every iteration and cannot prove sum=12. The stronger relation acc=sum(A[0:i]) plus acc<=10*i connects state progress to semantics.

Changed condition: Replace sum with maximum requiring nonempty or explicit empty result.

For maximum, use BF_EMPTY without writing when n=0; for n>0 invariant m=max(A[0:i]) after initialization from the first element, or m>=0 with all inputs nonnegative. [2,9,4] yields 9.

Evidence selectors: formal, wp-goals.

## AX-PR-T33-01-T33.02.b

Initialization has i=0,acc=0; one step adds A[i] and advances i; exit i=n establishes total. Double-add violates preservation; omitted increment violates the decreasing variant, even if bounds remain safe.

Changed condition: A loop processes two elements per step and requires tail handling.

bf_sum_pairs validates first, consumes pairs while n-i>=2 and then one tail. n=3 requires one pair and a tail; exhaustive 0..4 sequence checks exercise every odd length.

Evidence selectors: formal, cbmc-double-sum.

## AX-PR-T33-01-T33.02.c

Under i<=n, n-i is a mathematical nonnegative value also representable in size_t. An unchecked i>n would wrap subtraction. One input pass costs n reads/adds with constant local storage.

Changed condition: Add retry on zero-progress and supply a finite retry budget or declare liveness unresolved.

The writer budget is a second decreasing natural. Three zero returns consume three calls without advancing the prefix; no unbounded retry is hidden in the sum proof.

Evidence selectors: formal, cbmc-environment.

## AX-PR-T33-02-T33.03.a

On this exact-width environment uint8_t 250 and 10 promote to int; 260 is representable then converts modulo 256 to 4. Output validity, lifetime and baseline nonoverlap are independent obligations.

Changed condition: Use unsigned int at its maximum and derive where modular addition actually occurs.

UINT_MAX+1u is already unsigned-int arithmetic, so wrap occurs in addition, not just in assignment. The native test executes only this defined case.

Evidence selectors: formal.

## AX-PR-T33-02-T33.03.b

Logical INT_MAX+1 exists as an integer, but the corresponding signed C addition is undefined. Use b>0 && a>INT_MAX-b and the symmetric negative guard before evaluating a+b.

Changed condition: Use C23two-complement representation and explain why signed overflow still does not become wrapping.

Two-complement representation in C23 does not authorize signed overflow. The same guard remains necessary; no invalid signed operation is executed for demonstration.

Evidence selectors: formal.

## AX-PR-T33-02-T33.03.c

n<=4 and n<0u admits no state. Reachable cover witnesses for n=0,n=4 disappear, even though implication-style assertions have no counterexample. Preserve the vacuous and ordinary outputs separately.

Changed condition: Assume all callback returns full success and locate the hidden failure paths.

An always-full writer excludes zero, short progress, error and oversized count. The environment harness admits all four, and the partial/error test observes exactly one committed byte.

Evidence selectors: reachable-witness, vacuous-witness, cbmc-environment.

## AX-PR-T33-02-T33.04.a

proof.c defines a finite logical sum expression without an assumed axiom and binds requires, ensures, assigns, loop invariant and variant to source. Public checked_sum validates n/extent/range separately; unsupported aligned/function-pointer guards remain explicitly skipped.

Changed condition: Move a global invariant into an explicitly supported contract rather than silently relying on unsupported WP feature.

Instead of a global invariant silently ignored by WP, encode the required range in proved_sum requires and prove it at checked_sum call. Caller validation discharges the precondition rather than declaring the whole program globally valid.

Evidence selectors: wp-goals.

## AX-PR-T33-02-T33.04.b

BF_MUTANT=2 writes out[1] while leaving the total correct. The native neighbor sentinel and CBMC frame assertion reject it. checked_sum failure never reaches the output assignment.

Changed condition: Caller allows aliased output and input; revise both frame and postcondition.

When out aliases a[k], allowed footprint is that one element and result equals the pre-state sum. The ordinary separated WP wrapper does not cover this variant; native alias evidence and the read-before-single-commit argument are separately labeled.

Evidence selectors: formal, cbmc-frame.

## AX-PR-T33-02-T33.04.c

An inner requires a[k]<=10 deletes bad-range calls from its proof. Public checked_sum first rejects n>4,n>extent and range failure, then calls the inner kernel. Its failure postcondition preserves old output.

Changed condition: A caller passes a short extent and require a separate validation proof.

Logical extent=0,n=1 rejects before reading. Actual backing-storage lifetime/validity cannot be tested from a pointer; the proof harness requires real live storage and separately models short admitted extent.

Evidence selectors: formal, wp-goals.

## AX-PR-T33-03-T33.05.a

sum_cbmc uses actual four-element arrays, initializes every value, constrains n0..4 and checks output plus a neighbor. Negated-witness assertion counterexamples verify empty/maximal calls remain reachable.

Changed condition: Allocation may fail; represent failure and ownership explicitly.

environment_cbmc chooses NULL or an owned four-byte automatic object as the allocator outcome abstraction. NULL rejects with no callback and unchanged committed sentinel; ownership release is outside this nonallocating component and remains caller responsibility.

Evidence selectors: cbmc-unwind-5, reachable-witness, cbmc-environment.

## AX-PR-T33-03-T33.05.b

Four body iterations require five guards in the recorded CBMC convention. --show-loops captures identities; unwind4 must fail unwinding and unwind5 must pass the original harness.

Changed condition: Nested loop bounds change and require separate counts plus feasibility reasoning.

The nested harness has two outer bodies (three guards) and four inner bodies per outer (five guards), eight total visits. Bounds are per loop rather than a single product8 substituted into a guard count.

Evidence selectors: cbmc-loop-ids, cbmc-unwind-4, cbmc-unwind-5, nested-complete.

## AX-PR-T33-03-T33.05.c

The double-add model produces a functional assertion counterexample in live valid storage; replaying [3,4,5] gives24 instead of12. Frame mutation is a separate property class. An unwind failure says completeness was not established.

Changed condition: The counterexample relies on a permissive stub outcome absent from real contract.

A count5 response at remaining4 is allowed only as a malformed-adapter containment challenge, not a conforming writer outcome. The implementation rejects before pointer advance; keep that model scope in the trace interpretation.

Evidence selectors: cbmc-double-sum, cbmc-frame, cbmc-environment, formal.

## AX-PR-T33-03-T33.08.a

Writer outcomes at remaining4 classify -1 error,0 pending,1 committed prefix,5 invalid count. Every callback sees base+accepted and remaining4-accepted in the CBMC model; frame permits only context fields.

Changed condition: Hardware status read clears a bit; encode the side effect rather than nondeterministic independent reads.

Status=3 read-to-clear returns3 then stores2; the second read returns2. Two independent nondeterministic reads could wrongly permit3 twice and model a different device.

Evidence selectors: formal, cbmc-environment.

## AX-PR-T33-03-T33.08.b

The sum proof excludes interrupts/reset/power loss and dynamic allocation because its component has no such transitions. The environment model explicitly includes nullable acquisition and bounded callbacks, but not real allocator internals or scheduling.

Changed condition: User requirement includes power loss and forces a new model.

Power loss introduces a transition discarding volatile state at every relevant commit point. A RAM output store cannot prove persistence; the new requirement needs storage/reset/recovery state and is open, not waived.

Evidence selectors: cbmc-environment.

## AX-PR-T33-03-T33.08.c

For write1 then error, committed stays1 and second pointer is base+1. A loop that advances by requested4 skips bytes1..3; a loop restarting base duplicates byte0. Mutant3 is rejected by the exact-prefix oracle.

Changed condition: Callback returns an oversized count; require validation before pointer advance.

An oversized return5 at remaining4 is rejected with committed0 before any pointer arithmetic. A valid positive count may advance only after comparison to remaining.

Evidence selectors: formal, cbmc-environment.

## AX-PR-T33-04-T33.06.a

The actual WP inventory retains initialization, preservation, postcondition, frame and runtime-error goals plus false_bound. 43/44 is not blanket success: false_bound(x=10) contradicts result<=9, and skipped alignment/function-pointer guards remain disclosed.

Changed condition: Increase value range and predict arithmetic goal failure.

Widening element bound to UINT_MAX invalidates acc<=10*i and the unbounded functional equality after modular addition. The separate widened run preserves its unproved goals; do not repair by hiding input states.

Evidence selectors: wp-goals, wp-widened-range.

## AX-PR-T33-04-T33.06.b

sum4 is a finite definition, not an axiom. The proven accumulator bound depends on n<=4 and value<=10. Frama_C_interval specification is trusted environmental input for Eva. An asserted lemma still generates a goal.

Changed condition: Replace callback axiom with actual body analysis and reevaluate dependencies.

Replacing an external full-success assumption with environment_cbmc and actual bf_write body exposes partial/error/zero/oversize paths. This shrinks the trusted stub but does not prove real hardware/OS callback implementation.

Evidence selectors: wp-goals, cbmc-environment.

## AX-PR-T33-04-T33.06.c

false_bound is actually false with witness x=10. Unknown preservation under Qed alone can instead require a solver/invariant; unsupported aligned guards are neither false nor discharged. Retain each cause distinctly.

Changed condition: Switch from integer to float and reject aReal-model-only claim of IEEE correctness.

Float left-fold [1e20,-1e20,3] gives3, whereas reassociation to 1e20+(-1e20+3) can give0. A Real equality ignores rounding, NaN and exceptions and cannot certify the strict floating contract.

Evidence selectors: wp-goals, performance.

## AX-PR-T33-05-T33.07.a

[-1,2]+1=[0,3], with actual x=-1 a zero-divisor witness. Eva reports that alarm; its final states omit executions already stopped at undefined behavior, so a displayed nonnegative final x does not erase the alarm.

Changed condition: Track correlation y=x+1 using a relational domain and compare precision.

For y=x+1, paired states make y-x exactly1. Independent interval subtraction allows -2..4 and includes zero. models.py compares the exact relation with intervals; relational precision is a separate configured-analysis obligation.

Evidence selectors: eva--1.

## AX-PR-T33-05-T33.07.b

The restricted Eva entry models a caller that admits only0..2 and removes the zero alarm. This is a changed calling contract, not a proof that the original caller cannot pass-1.

Changed condition: A new caller admits-1 and must reopen the claim.

A new caller passing-1 reopens the original zero alarm immediately. Run both inputs and retain both results; never use an unjustified assume as an alarm suppression strategy.

Evidence selectors: eva--1, eva-0.

## AX-PR-T33-05-T33.07.c

Eva uses x86_64 machdep and the supplied builtin interval specification. Sequential control flow admits no interrupt transition; volatile value changes alone do not provide an ISR schedule or atomicity proof.

Changed condition: Add an ISR modifying length and construct the missing interleaving.

Interleaving: main checks length<=4; ISR changes length to5; main reads a[4] using reloaded length. Snapshotting, synchronization or exclusion must be part of a new environment/refinement proof.

Evidence selectors: eva--1, eva-0.

## AX-PR-T33-05-T33.10.a

Bounds/result rows cite CBMC/WP; live-storage assumptions remain caller premises; native tests check finite behavior; latency/physical energy require actual instruments. Incompatible evidence is not added into a percentage.

Changed condition: A source change affects only one function; derive dependency-based invalidation.

Changing bf_write invalidates callback-prefix proof/tests but not the independent sum4 arithmetic definition. Changing unsigned width invalidates sum/vector machine refinements and ABI-dependent code, even with identical mathematical values.

Evidence selectors: wp-goals, cbmc-unwind-5, formal.

## AX-PR-T33-05-T33.10.b

Trusted base includes GCC/Clang translation, CBMC lowering/solver, Frama parser/WP/Eva, Z3, exact annotation and builtin contracts, x86_64 ABI and host runtime. Agreement between tools reduces correlated defects without eliminating trust.

Changed condition: A cross compiler uses different integer sizes and requires a new machdep/ABI review.

A cross compiler with16-bit int changes promotions and bounds, requiring new machdep and exact-width-type availability checks. Do not transplant x86_64 solver results as ARM/other ABI proof.

Evidence selectors: wp-provers, gcc-version, clang-version, cbmc-version.

## AX-PR-T33-05-T33.10.c

Supported claim is finite length0..4, live stated storage and exact value range, bounded native/CBMC behavior and selected WP goals. It is invalid to conclude all firmware safe or physical energy bounded from this record.

Changed condition: Expand length bound and change alias policy; identify every invalidated argument.

Expanding n invalidates sum4 definition,acc bound,loop unwind,tests and extents. Allowing alias invalidates separated-frame proof and requires old-state sum and delayed commit; native alias evidence alone does not update the WP result.

Evidence selectors: wp-goals, formal.

## AX-PR-T33-06-T33.09.a

models.py enumerates choose(4,2)=6 histories. Correct write-before-publish gives zero bad reads; early publish permits publish,observe,read,write as the sole bad trace under conditional read.

Changed condition: Add second epoch and stale generation observations.

Epoch1 observation followed by epoch2 publication is accepted by a boolean but rejected by generation equality. A generation check alone does not prevent a concurrent payload rewrite/data race; lifetime/ownership remains required.

Evidence selectors: models.

## AX-PR-T33-06-T33.09.b

Publication linearizes at ready release. If the consumer acquire reads that release, producer payload writes sequenced before release happen-before reads sequenced after acquire. This needs stable lifetime, single writer and no later racing modification.

Changed condition: Two writers require a new ownership/publication protocol.

Two writers can interleave payload writes before one ready store, yielding a mixed object. Require mutual exclusion or separate immutable buffers plus atomic owner/publication; merely two release stores do not serialize ordinary writes.

Evidence selectors: models.

## AX-PR-T33-06-T33.09.c

The enumerator is finite sequentially consistent event order, not an axiomatic C checker. Reordering, interrupt extensions and arbitrary epochs are absent. Its assertion is safety only.

Changed condition: A liveness claim needs a fairness assumption absent from finite safety enumeration.

An execution where the consumer is never scheduled remains possible without fairness. No finite absence of a bad read proves eventual observation or completion.

Evidence selectors: models.

## AX-PR-T34-01-T34.01.a

The immutable gate is uint32 modulo equality with live nonoverlapping n-element objects. Tiny0..9 and measured8/4096/65536 strata separate dispatch from streaming cost; resources have explicit bytes and time units. Energy has no inferred value.

Changed condition: The application changes from batch throughput to single-request tail latency.

Single-request tail latency needs per-request timestamps, queue/load and percentile policy; batch nanoseconds/element cannot answer it. Keep this new protocol separate from the current warm repeated-throughput record.

Evidence selectors: performance, measurement.

## AX-PR-T34-01-T34.01.b

Native scalar/disjoint/SSE2/fallback/unroll variants share exact wrap/tail vectors. Float is strict left-to-right; the cancellation, NaN and zero tests are separately defined and not compared using the integer oracle.

Changed condition: Add floating exceptions or bitwise reproducibility to the required contract.

Bitwise reproducibility or floating exceptions forbids treating a tolerance-only result as equivalent. Reassociation/contraction must be disabled or newly justified and exception flags explicitly captured.

Evidence selectors: performance.

## AX-PR-T34-01-T34.01.c

Native tests stratify n0..9 and legal modulo16 offsets0,4,8,12; timings separate sizes8,4096,65536. No global speedup claim is made from aligned lane multiples.

Changed condition: A bimodal workload has mostly tiny calls and rare huge calls.

For99% tiny calls and1% large calls, request-weighted latency differs from byte-weighted throughput. Report both populations, p99 boundary sensitivity and explicit weights rather than average away the common tiny path.

Evidence selectors: performance, measurement.

## AX-PR-T34-01-T34.02.a

64 adds with two4-byte loads and one4-byte store equals768 logical bytes and intensity1/12 op/B. This does not include write allocation, cache evictions or instruction fetch.

Changed condition: Use uint64 output while inputs remain32bit and recalculate bytes.

Widening output to8 bytes changes useful traffic to64*(4+4+8)=1024 and intensity1/16. It also changes the value contract if wrap is no longer uint32.

Evidence selectors: models.

## AX-PR-T34-01-T34.02.b

min(100G,12G/12)=1Gop/s at the declared boundary. Dependent pointer chasing can be latency limited despite low traffic and cannot be predicted by the same streaming ceiling.

Changed condition: Reuse input in cache and revise the relevant bandwidth boundary.

In-cache reuse changes the relevant bandwidth level and traffic per reuse. Recompute bytes at that boundary rather than keep DRAM bandwidth while counting cache hits as external traffic.

Evidence selectors: models, measurement.

## AX-PR-T34-01-T34.02.c

Tiny calls plausibly favor dispatch cost; large independent adds plausibly favor bandwidth. Actual paired measurements retain every sample and can reject a universal faster-disjoint hypothesis; absent PMU evidence leaves causal diagnosis bounded.

Changed condition: The compiler removes the work; treat this as invalid measurement before explaining speed.

A compiler-removed kernel invalidates measurement. The checksum is independently recomputed from exact seeded inputs and per-call changes; a no-work function fails before any speed claim.

Evidence selectors: measurement.

## AX-PR-T34-02-T34.03.a

Synthetic addresses touch1 versus4 declared16-byte lines; offset4 touches2. The line model is exact arithmetic, not a CPU cache observation. Page strides additionally change translation locality.

Changed condition: Offset the contiguous set by4 so it crosses a line boundary.

Offsetting a contiguous 4-element set to4 crosses at address16. A physical line size different from16 requires recomputation and supported events for an observed cache claim.

Evidence selectors: models.

## AX-PR-T34-02-T34.03.b

Stable IDs travel with values during SoA conversion and deletion; deleting20 leaves30 associated with300. The blocked3x5 transpose uses input stride7/output stride4/block2 and preserves padded cells.

Changed condition: The workload now reads cold labels with every numeric record.

Reading each32-byte cold label adds label traffic and either an ID lookup or parallel label storage. The value-only timing cannot be reused as an all-fields performance result; identity still must survive deletion.

Evidence selectors: performance.

## AX-PR-T34-02-T34.03.c

Typed uint32 pointers admit four residues modulo16, not arbitrary byte misalignment. Allocations end at the active extent so sanitizer tests catch a full-vector tail read beyond it.

Changed condition: Stride is12bytes and vector alignment is16bytes.

Stride12 yields residues0,12,8,4,0. Aligned base alone cannot justify aligned vector loads for every record; use unaligned loads only when full16-byte extent is present.

Evidence selectors: performance.

## AX-PR-T34-02-T34.04.a

Clamps give[0,0,3,4]. Comparison/select does not form x-lo or hi-x, avoiding overflow. Final disassembly decides whether source alternatives actually differ.

Changed condition: Use extremaINT_MIN/INT_MAX to falsify a subtract-and-mask shortcut.

At INT_MIN/INT_MAX, subtraction-based masks can overflow signed int. The extrema tests use defined comparison/select operations and reject semantic shortcuts without executing undefined arithmetic.

Evidence selectors: performance, gcc-object-disassembly, clang-object-disassembly.

## AX-PR-T34-02-T34.04.b

Constant/alternating/mixed distributions require separately retained raw data and identical consumed outputs. Generated code can make both forms identical, rejecting source-level branch-count predictions.

Changed condition: A different compiler emits identical code for both sources.

Identical linked code is a concrete failed optimization hypothesis; remaining timing differences are noise/context candidates, not evidence of a source-level branch advantage.

Evidence selectors: clamp-measure, gcc-object-disassembly, clang-object-disassembly.

## AX-PR-T34-02-T34.04.c

The four constant unroll wrappers retain tail correctness and permit nm/objdump/stack-use comparison. Code byte growth and observed spills are distinct from measured runtime and do not establish instruction-cache events.

Changed condition: Use a larger surrounding program to change instruction working set.

A surrounding program changes instruction working set and call locality. A standalone kernel result cannot predict that application; rerun within the larger image and retain its identity.

Evidence selectors: performance, gcc-symbol-sizes, clang-symbol-sizes.

## AX-PR-T34-03-T34.05.a

GCC vector reports and Clang pass/missed remarks are bound to exact source/object. The recurrence has loop-carried dependence; strict float has order constraints. Inspect guards and tail in final code.

Changed condition: Link-time optimization changes the final loop after object inspection.

LTO final disassembly can remove/internalize/clone a loop after object output. Archive and LTO artifacts are separately captured; old object-only conclusions are invalidated.

Evidence selectors: gcc-vector-report, clang-vector-report, archive-linked-code.

## AX-PR-T34-03-T34.05.b

Selected GCC optimized-tree/Clang LLVM IR, object and linked code give distinct stages. Nonrestrict add may need overlap guards while disjoint promises simplify admission; helper calls must appear in undefined-symbol audit.

Changed condition: Change optimization level and repeat the mapping instead of reusing old assembly.

Changing optimization level requires a new report and code mapping; the O0/O3/sanitized semantic matrix preserves behavior but does not let one assembly file stand for all profiles.

Evidence selectors: gcc-ir, clang-ir, gcc-linked-disassembly, clang-linked-disassembly.

## AX-PR-T34-03-T34.05.c

bf_add permits potential aliasing, bf_add_disjoint has explicit restrict promises, and bf_add_inplace uses a separate contract. No forbidden partial overlap is executed as a valid restrict call.

Changed condition: Permit exact in-place output but forbid partial overlap.

Exact out==a is valid for bf_add_inplace when b is disjoint. Partial offset overlap remains excluded because early stores can change later input; the ownership boundary must distinguish equality from overlap.

Evidence selectors: performance.

## AX-PR-T34-03-T34.06.a

Element add iterations are independent. r=33*r+a[i] carries a true recurrence. Float addition is nonassociative under the strict gate. These are different barriers, not interchangeable missed-optimization causes.

Changed condition: Use prefix sum requiring a different parallel algorithm rather than a hint.

Prefix sum output[i]=sum(a[0:i+1]) has a recurrence; a parallel scan algorithm needs intermediate-prefix combination and fresh numerical/overflow semantics, not restrict or vector hints.

Evidence selectors: gcc-vector-report, clang-vector-report, performance.

## AX-PR-T34-03-T34.06.b

Caller allocates separate actual arrays and validates n against extents; residue tests follow typed alignment. An owner can compare offsets within one allocation; unrelated-pointer ordering is not a general validator.

Changed condition: Pointers arrive from an untrusted adapter and require a span/handle representation.

Untrusted input must be an owner-issued handle plus offset,length checked using subtraction-based bounds before deriving a pointer. No raw pointer value establishes lifetime or allocation provenance.

Evidence selectors: performance.

## AX-PR-T34-03-T34.06.c

The five outputs[1,0,9,10,10] are exact modulo32. Float cancellation3, NaN propagation and zero equality have separate expectations; zero sign and exception behavior need declared host floating model.

Changed condition: Enable contraction/reassociation and identify invalidated expectations.

Reassociation may turn cancellation3 into0; contraction can change rounding and exceptions. A relaxed build invalidates strict bit/exception expectations and must use a separately approved gate.

Evidence selectors: performance.

## AX-PR-T34-04-T34.07.a

SSE2 adds four32-bit lanes modulo2^32 using full16-byte unaligned accesses. The nine-value fixture yields[1,2,3,4,5,6,7,0,10] and two groups plus one scalar tail.

Changed condition: Move to another supported vector ISA and inspect its lane/overflow contract first.

The separate AVX2 transfer uses8 modulo32 lanes, full32-byte load/store extents and scalar tails. Its actual host test checks lengths0..17 and typed offsets0..28 only after CPUID AVX/OSXSAVE, XCR0 XMM/YMM bits and AVX2 support; no masked-load claim follows.

Evidence selectors: performance, gcc-specialized-unit, clang-specialized-unit, avx2-transfer.

## AX-PR-T34-04-T34.07.b

n0..9 includes every residue and exact end-of-object allocation. The vector guard n-i>=4 prevents a full load of a short tail; preceding sentinel detects an unrelated write.

Changed condition: A masked-load ISA has fault-suppression rules that must be inspected separately.

Masked-load fault suppression is ISA-specific and does not follow from SSE2 loadu semantics. Keep short tails scalar until the new mask contract and page-boundary tests are established.

Evidence selectors: performance.

## AX-PR-T34-04-T34.07.c

force_scalar actually invokes the scalar path for every semantic case. Baseline unit uses x86-64 with AVX disabled and vectorization disabled; final code is retained for instruction inspection.

Changed condition: Target has no vector support and the baseline remains usable.

On a no-vector target use only the portable scalar translation unit and a selector that cannot reference the specialized symbol. x86-64 execution itself does not prove that different minimum ISA.

Evidence selectors: performance, gcc-baseline-unit, clang-baseline-unit.

## AX-PR-T34-04-T34.08.a

feature0,1,8 with OS true and feature8 with OS false select0,1,1,0. Detection/selection itself executes no specialized operation; the recorded target is x86-64 Linux with SSE2 ABI support.

Changed condition: Add AVX only after inspecting OSXSAVE/XCR0 requirements and state-management evidence.

The AVX2 transfer tests CPU AVX/OSXSAVE then XCR0 bits1 and2 before checking AVX2 and invoking the separately compiled function. The current host reports XCR0=7. Baseline detection uses -mno-avx; no unsupported vector instruction executes before admission.

Evidence selectors: performance.

## AX-PR-T34-04-T34.08.b

Separate baseline and SSE2 compilation commands retain exact ISA flags. Baseline minimum is x86-64/SSE2, not pre-SSE x86. Scalar forced-path behavior and final linked target are independently checked.

Changed condition: LTO crosses translation-unit boundaries and requires renewed isolation inspection.

LTO may inline across units and propagate specialized operations. The archive/LTO final-code inspection must be repeated; translation-unit separation alone is no post-link guarantee.

Evidence selectors: gcc-baseline-unit, clang-baseline-unit, archive-linked-code.

## AX-PR-T34-04-T34.08.c

Plain linked symbols/disassembly identify bf_dispatch,bf_sse2 and fallback. LTO may internalize/integrate symbols, so actual final instructions and executed semantic gate are the evidence.

Changed condition: Resolver executes before normal constructors and requires explicit initialization rules.

An ifunc resolver runs before ordinary constructors and cannot depend on constructor-populated state; explicit call-site selection uses already-established CPU/OS facts. No loader-independent ifunc claim is made.

Evidence selectors: gcc-linked-symbols, clang-linked-symbols, archive-linked-code.

## AX-PR-T34-05-T34.09.a

Each paired sample regenerates identical seeded inputs outside timing, changes one input per iteration, consumes a per-result word and final checksum. Python independently derives all checksums. Timed scope includes input-update/checksum overhead.

Changed condition: LTO proves repeated inputs constant and hoists the kernel.

Under LTO keep output dependence on per-call changed data and inspect final code. A purely constant repeat can be hoisted and cannot measure repeated kernel execution, even if one final checksum is correct.

Evidence selectors: measurement, lto-disassembly.

## AX-PR-T34-05-T34.09.b

The current protocol uses10 warmups, CLOCK_MONOTONIC and alternating path order. Affinity/frequency/interrupts/cache state are uncontrolled beyond stated warmup; raw observations retain this uncertainty.

Changed condition: Cold-start latency becomes the objective and warmup is no longer representative.

Cold-start latency must remove the steady-state warmup assumption and specify process/image/cache startup conditions. Existing warm samples are not recycled as cold observations.

Evidence selectors: measurement.

## AX-PR-T34-05-T34.09.c

Sorted samples90,95,100,105,110 yield median100 and max-min20. Real samples are integer nanoseconds, retained as pairs/strata with medians and ranges rather than only a ratio.

Changed condition: Use a bimodal distribution and report strata/tails.

For bimodal tiny/huge calls, report stratum weights and tail quantiles; a single median can entirely hide the slow population, especially near the percentile weight boundary.

Evidence selectors: models, measurement.

## AX-PR-T34-05-T34.10.a

The runner captures software cpu-clock addresses and matching ELF, a gprof histogram and actual Linux getrusage minor-fault deltas. Hardware cycles are unsupported in WSL and retained separately; no process-fault count is called a cache miss.

Changed condition: Inlining merges attribution and requires machine-level interpretation.

Inlining can attribute multiple source routines to one machine region. Inspect inline debug information and final addresses; source-level percentages cannot be naively added or treated as separate instructions.

Evidence selectors: perf-counter, perf-sampling, gprof-report, clamp-measure.

## AX-PR-T34-05-T34.10.b

1000*20/10=2000 is a stationary extrapolation. running0 is rejected. If the sampled half covers only warmup, scaling does not reconstruct an unseen steady-state phase.

Changed condition: Group counters cannot schedule together and require a different experiment.

Unschedulable counter groups require smaller groups or separate matched experiments and uncertainty about phase alignment. Zero-running data never becomes a valid total by substituting one.

Evidence selectors: models, perf-counter.

## AX-PR-T34-05-T34.10.c

Changed stride affects address order, working set and dependency latency. Code accesses plus timing need relevant supported cache/TLB events before assigning cause; denied PMU evidence remains a gap.

Changed condition: Event definition differs on another CPU model.

Another CPU can assign different event encodings/semantics. Recheck model-specific event definition and enabled/running behavior; preserve a competing pointer-dependency explanation.

Evidence selectors: locality-measure, perf-counter.

## AX-PR-T34-06-T34.11.a

GCC plain/LTO/PGO share the exact kernel and Python checksum gate. The profile-generation executable is distinct from the optimized use executable. Archive commands use gcc-ar and LTO at compile and final link.

Changed condition: Put a function in an archive and verify plugin extraction/optimization.

The archive variant retains map and final disassembly proving extraction and participation. Native correctness runs after plugin optimization; archive contents alone cannot show final participation.

Evidence selectors: measurement, archive-run, archive-link.

## AX-PR-T34-06-T34.11.b

time records build elapsed seconds and peak RSS KiB; size gives final text/data/bss; held-out samples keep opposite branch bias. Training advantage is not accepted as representative workload benefit.

Changed condition: Deployment distribution drifts and triggers profile reassessment.

Distribution drift invalidates profile representativeness even with unchanged source/compiler. Retrain using a declared new corpus, retain old regression evidence and remeasure held-out strata.

Evidence selectors: plain-build, lto-build, pgo-generate-build, pgo-use-build, measurement.

## AX-PR-T34-06-T34.11.c

The mismatch experiment trains copies then changes a branch at the same source/output identity. GCC must emit coverage-mismatch failure. Symbols expose hosted gcov collection/file support; strict target must not link it.

Changed condition: Upgrade compiler and regenerate version-bound profile/LTO artifacts.

Compiler-version changes require regenerated profiles and LTO objects. Suppressing mismatch diagnostics does not create compatible data or valid evidence; keep the original rejected build.

Evidence selectors: mismatch-diagnostic, pgo-generate-symbols, pgo-use-undefined.

## AX-PR-T34-06-T34.12.a

Synthetic candidates fail cycles200>180, failFlash1400>1300, and pass1250/96/175. The fastest160 candidate is rejected for Flash. No physical energy number is inferred.

Changed condition: A lower energy variant misses latency and requires an explicit requirements decision.

A lower-joule candidate over latency remains rejected unless the requirement owner changes the latency bound. Record that tradeoff explicitly instead of merging objectives into an unexplained score.

Evidence selectors: models.

## AX-PR-T34-06-T34.12.b

BOARD_CAPTURE records exact image/readback, board revision, clock/interrupt/memory placement, stack method and raw timing/energy requirements. It is a prepared protocol with physical fields unobserved.

Changed condition: Hardware revision or memory placement changes and invalidates timing assumptions.

Board revision or memory placement changes invalidate physical timing assumptions; retain the old image and capture as historical evidence and perform a new paired measurement before acceptance.

Evidence selectors: board-protocol.

## AX-PR-T34-06-T34.12.c

Semantic vectors, modulo/float contracts, resource budgets, target assumptions and raw comparison policy belong to one candidate identity. Passing time alone cannot hide semantic failure; noisy overlaps do not establish a winner.

Changed condition: Compiler or workload changes while source stays fixed.

Source unchanged is insufficient identity: compiler/flags/runtime/workload/board/profiles are dependencies. Invalidate affected rows and rerun their gates; pure arithmetic fixture derivations remain valid.

Evidence selectors: performance, measurement, board-protocol.

