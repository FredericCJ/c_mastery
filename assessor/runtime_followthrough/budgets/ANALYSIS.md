# Assessor-only resource accounting and host-wait analysis

Use the three original criteria and unchanged delayed tasks; do not award one
global mark from a green binary. The adapters observe actual components whose
implementations must already exist. They do not manufacture independent review
or establish learner ownership of those components.

<a id="09a"></a>
## T46.09.a — charge units and changed collector

The isolated examples pay fuel2, GC0, host0; fuel3, GC16, host0; and fuel3, GC0,
host1 respectively. The composed image pays fuel9, GC19, host1, allocation1,
collection1 and returns18. The root local appears at STORE before GC; GC sees
one root and two fields, then reserves16 units for sweep. If initial GC is0,1,2,
the attempted pass pays exactly that many units; if it is3..18 it pays3 then
refuses the all-or-none sweep. Fuel5 has already been paid. Failed GC leaves the
allocated object occupied, and no host call occurs. A later fuel failure after
the successful HOST retains one side effect and reports no guest result.

The independent Python oracle is a piecewise derivation over instruction
positions5,8,9, not a second interpreter: all11×21×3=693 budget triples. It also
checks event counters and callback effect count, not only final status. Control1
swaps fuel/GC,2 excludes a paid failing instruction,3 hides a paid host attempt
on later failure,4 publishes a failure value,5 pretends each collection costs16
regardless of roots and edges. All must compile and fail their semantic checks.

On the changed graph, original cost=1+2×2+16=21. Actual ORVM cost is
1+2×3+16+2=25: it validates both fields of the unreachable used pair and counts
marked objects separately. Budget24 refuses atomically with statistics sentinel91;
budget25 moves the two surviving objects and updates the root. The original
`gc_budget` remains100 because ORVM takes a separate explicit budget. Neither
cost is cycles, bytes copied, or host time. The same equivalent reachable-value
contract permits the comparison, but identity/relocation and failure atomicity
also changed and must be stated.

Seven-day variant: use the unchanged task “Change collection algorithm and
recalibrate its work units without calling them cycles.” Give an unreachable
cycle and duplicate roots so root slots, unique live objects and validated fields
cannot be conflated. Require an independently predicted ledger before running.
Accept a different correct collector only with explicit root/identity and cost
contracts; renaming the old counter does not implement the task.

<a id="09b"></a>
## T46.09.b — independent limits and first refusal

The supplied tests isolate all seven resources. Self-recursive CALL0 at initial
depth1 fills16 frames and the16th paid CALL refuses. The operand case preserves
63 caller values, enters a function that needs two operands and refuses the
second push at global64; its per-function verifier bound is valid. Fuel paid is
63 literal instructions+CALL+two attempted literal instructions=66. This shows
why a verifier's local height proof is not a global stack-capacity proof.

For heap exhaustion, preload a16-node rooted chain. Pair's two scalar temporary
roots make three examined roots,32 reached fields and16 sweep cells: GC51
completes, but no space remains, so HEAP_LIMIT follows. Arena mode refuses with
GC0. With seven explicit scalar roots instead, the second temporary push refuses
and restores root_count7 before any GC; this STACK_LIMIT comes from explicit
roots, not operand slots. Preserve the diagnostic distinction in the dossier.

A zero-GC explicit collection pays one guest instruction. Host budget0 refuses
after the paid HOST but before callback. At the same HOST instruction, fuel0
refuses before the host-budget check; one additional fuel unit reaches HOST_BUDGET.
Changing the operand to BOOL reaches TYPE before checking that host budget.
An invalid image is rejected by verification without dispatch. Result sentinel91
does not imply other state was rolled back. Reinitialize before replay, because
IP, stacks, marks and consumed budgets can differ after refusal.

Delayed assessment retains “Compose two near-limit resources and identify which
refusal occurs first under the specified order.” Give a different boundary pair,
such as full explicit roots plus insufficient collection budget. Require a
minimal fixture, exact state at first refusal and a one-condition change that
reaches the second limit. Randomly increasing every budget until the program
works demonstrates no diagnosis and earns at most level1.

<a id="09c"></a>
## T46.09.c — a native wait outside the instruction model

`blocking.c` actually executes the original guest in an owned child. The callback
announces entry, then reads its empty release pipe while the parent keeps the
write end open. Fuel3 covers INT,HOST,RET but the callback's wait has no guest
instruction boundary. The parent observes no result for100ms, releases one byte,
receives18 and reaps the child. The no-read control must fail the before-release
condition. This experiment supports the observed blocking mechanism, not an
infinite-duration claim. The source has no callback deadline; for an arbitrarily
late release its native wait can exceed any guest-fuel-derived time bound.

The faulty callback violates the retained CVM requirement for a finite native
I/O/deadline policy. Say so explicitly: a compliant embedding must supply that
separate bound. Compilation and verification execute before dispatch, and GC
and native callbacks can perform work inside one instruction. A fuel budget
therefore bounds only the stated dispatch count, not these elapsed times. Even
an abstract finite work count needs a hardware/scheduler model before it can
bound wall time. Never report the observed maximum as WCET.

For the pending variant the parent actually uses frozen Vs: take the request
once, start a local provider, retain the pending session and run an unrelated
two-instruction guest to47 while the provider waits. Restart and wrong epoch
completion refuse. On normal completion, consume the record, reap the provider,
complete the matching request and execute RET to18. On cancellation, SIGKILL is
sent only to that child; waitpid confirms termination before publishing failure.
The session output stays91. This is concrete quiescence for an owned process,
not a generic cancellation theorem for threads, DMA or async store operations.
Those need their own producer-lifetime proof.

POSIX source scope is narrow: read's empty-pipe blocking and interrupted/short
transfer semantics; poll's readiness/timeout/EINTR distinctions; waitpid WNOHANG
and termination-status predicates. The harness uses absolute monotonic deadlines
and closes all ends on every ordinary path. External scheduling can exceed the
requested wait, so its deadline is a containment policy rather than a hard-time
guarantee. No other process, external service or host security setting is touched.

Delayed task: replace the callback with an explicit pending/resume protocol and
derive retained-state ownership, withholding Vs/reference files. Give late or
duplicated completion and cancellation before producer quiescence. Require a
state diagram, identity checks, ownership intervals, an independently progressing
second request and exact terminal publication. A flag that immediately frees the
provider's borrowed state fails even if ordinary serial tests pass.

## Evidence and grading

Level2 requires the ordinary proofs and observations above. Level3 additionally
requires the original changed mechanism and transfer to an unfamiliar boundary.
Negative controls must reach execution. Native sanitizer success is bounded
diagnostic evidence; object/map/closure checks address a different target claim.
Hosted process experiments never appear in the strict target closure. Author
reference execution, external review, independent learners, seven-day attempts
and physical measurements remain separately tracked.
