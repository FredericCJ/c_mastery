# Bounded timer-cost investigation

`resilience_timer_cost_validation.json` records four current GCC/Clang O0/O3
configurations, fifteen equal-contract modes per configuration, seven measured
timings after one warmup, and 122,880 per-advance samples per configuration.
Counter-free builds execute the independent 71,877-case timer oracle first.
Separate counter builds expose ordering/linkage work. The runtime-selected object
is actually848 bytes in all four configurations and all three modes; no smaller
specialized representation is claimed. Report individual handles and node layouts
from the build, not an invented machine-independent ABI.

The initial four-workload observation is retained as
`resilience_timer_cost_initial_validation.json`. It revealed that the coarse due63
burst reverses the list twice, leaving it in admission order before fine expiry.
The new dated `resilience_timer_decision_followup.json` therefore added a direct
fine-bucket due7 burst, after recording the expected quadratic ordering behavior.
Every configuration observed120 key comparisons for that wheel burst versus15
for its coarse burst. The new list's corresponding insertions also use120 key
comparisons; the heap's complete workload uses72. These counts cover different
maintenance operations under the same end behavior; they do not establish equal
instruction cost per comparison.

For example, the current GCC O3 due7 medians over3,000 repetitions are5,669,328ns
(list),5,493,711ns(heap),5,224,544ns(wheel); Clang O3 gives2,863,597ns,
2,894,805ns,3,100,054ns respectively. This changed ordering is a reason to retain
compiler/configuration and workload in the conclusion. These are host run totals,
not per-operation latency guarantees, and the full raw repetitions remain in the
receipt. No isolated timer-container superiority follows from this finite N16
comparison. The sample maxima of per-call runs include scheduler and clock-reader
noise; they are not WCET or physical interrupt jitter.

The inspected GCC O3 core assembly shows initialization using `rep stos` and
vector stores, checked schedule addition branching on carry, and an advance frame
with a stack protector. The static core frame report is944 bytes for advance,
40 for schedule and16 for cancel in the inspected build. These are individual
compiler frame reports, not a whole-thread high-water measurement. The undefined
core symbol is `__stack_chk_fail`; hence this hosted object is not evidence of a
strict freestanding helper-free image. The linked size also includes the test
harness and imported CMX and must not be reported as the timer abstraction alone.

The original PR-T50-05 accepted source and receipts remain unchanged. The real
CMX integration tests intersecting deadline/cancel outcomes; the new alternatives
compare their stronger stable expiry order and generation identities with each
other. The initial author prediction and follow-up are not an independent learner
decision. Compilation/incremental-build measurements for the complete T75 successor
family, a complete13-dimension dossier, final source-bound family campaigns and
per-rubric readiness are separate completion obligations. Physical timing and an
unaided delayed learner attempt remain external evidence requirements.
