# T53.09 — generation-qualified invalidation and retained frame authority

## 1. Identity and exercise families

This additive contract implements `AX-PR-T53-09-T53.09.a`, `.b`, `.c`, their
original `-DELAYED` variants, and the adjacent-page batching transfer in
`PR-T53-09`. Families A/B/C/D/E/F/G/H/I/J apply: the C publication argument,
actual target adapter and independent changed-condition analysis are distinct
deliverables. Historical project and assessment records remain historical.

## 2. Objectives and inspected references

For `.a`, derive the order PTE publication → each required local invalidation →
generation-qualified acknowledgement → permission to reclaim. Distinguish
cache coherence from translation-cache validity. For `.b`, construct the
bounded controller and interrupt adapter, including membership and timeout
ownership. For `.c`, exhibit and repair a synthetic stale-mapping violation,
then repeat with two address spaces and a permission change.

The source register is
`research/v5/expansion/legacy_smp_shootdown/resources.json`. Actually inspected
Intel 253668-060US, September2016, §§4.10.4.1 and4.10.5 specify invalidation
scope and propagation; the bounded N1570 §5.1.2.4 passages support the C
publication vocabulary. These are different authorities. Exact published C17
text remains unavailable; neither a C atomic nor a compiler barrier alone
establishes x86 translation invalidation. The retained CMK handler's CR3 reload
is inspected prerequisite material, not evidence for all this successor work.

## 3. Prerequisites and learner artifacts

Provide completed `PR-T53-05` locks, `PR-T53-02` startup, `PR-T53-08` ownership
analysis and `PR-T48-04` page-table/frame-lifetime artifacts. The original
`PR-T56-01`/CMK guest supplies the boot substrate. Explicit paths to completed
startup/kernel/lock workspaces are required by `guest.py`; solutions are never
copied into the learner export. Explain the lock rank, interrupt stack and
register-preservation ABI before implementing the adapter. A virtual mapping
does not transfer the physical frame's ownership.

## 4. Environment and dependency profile

Hosted C17 uses GCC and Clang debug, optimized and ASan/UBSan builds. The native
pthread campaign additionally uses both ThreadSanitizer builds. Host threads
represent protocol participants; their callbacks do not invalidate a real TLB.
The strict freestanding target is the original x86-64 CMK guest under isolated
QEMU TCG, two and four CPUs, `qemu64`, no network. It imports completed startup
and TAS locks, uses C atomics plus explicit x86 instructions and assembly, and
links no hosted runtime or storage service. Audit the final ELF for undefined
symbols and implicit helpers. Host Python, pthreads and sanitizers are verifier
dependencies only. The selected target has 4KiB pages, PCIDE=0, and an optional
PGE/global-page condition; no PCID, huge-page, hotplug or arbitrary ISA claim.

## 5. Problem, inputs and outputs

Implement `shootdown.c`, `shootdown_guest.c` and `shootdown_frames.S` against
`shootdown.h`. A stable world contains one to four online CPUs, two positive
distinct address-space identities, and at most one pending request per space.
Inputs are actor/space, aligned first address, one to four adjacent pages,
PAGE or GLOBAL scope and a stable port. A request contains the complete
identity, nonwrapping generation, target mask, range and scope. Outputs are a
published request or a completed `StPermit`; rejected operations preserve them.
`ST_SIGNAL` is an exception to ordinary failure: PTE/request publication already
committed and notification must be retried, without replaying publication.

The supplied test starts with generation9 and acknowledgements9/8/9 on CPUs0/1/2.
CPU1 remains outstanding. Other supplied inputs include partial notification,
failed local invalidation, generation exhaustion, range overflow, concurrent
membership entry, two independent permission domains, and four-page batching.
The guest maps VA0x40000000 to private synthetic frameA (100), remaps it to
frameB (200), delays CPU1's handler, and retains A until every required ACK.
After completion A may contain999; the old VA must resolve to B's200. The
intentionally flawed guest condition reuses A before acknowledgement and is
accepted only as the specifically observed local stale-read control.

## 6. Mechanisms and misconceptions

An interrupt being sent, a PTE being globally visible, and a CPU reporting an
old acknowledgement are three different events. A timeout is a diagnostic,
not proof that a CPU stopped using a mapping. A membership snapshot requires a
protocol with concurrent entry and exit. Global mappings can survive a CR3
reload; the architecture permits over-invalidation, so failure to observe a
stale global entry does not make a CR3-only implementation correct.

## 7. Required reasoning and invariants

All membership changes and request publication use one lock. Entry executes
locally before any address-space use, flushes the selected context, and is
refused while its space or a global request is pending. Entry before the
snapshot is included; entry after completion flushes before use. Leaving a
targeted CPU is refused until completion. The fixed online set excludes CPU
hotplug: a missing CPU causes retained authority, not mask deletion.

Publication failure promises no PTE change. Successful publication precedes the
release publication of the immutable request snapshot. Local invalidation must
finish before a release ACK; completion acquire-observes the exact generation
from every target. Old or unrelated ACKs cannot authorize reuse. Handler code
never acquires the membership/kernel lock: notification may synchronously
invoke the local handler while the sender holds that lock. All shared snapshot
fields are atomic; a stale handler may harmlessly over-invalidate but cannot
acknowledge a different tuple. The handler and context callbacks execute on the
declared CPU and must not be invoked concurrently as two independent local CPU
contexts. Port callbacks and their context remain live and unchanged.

PAGE targets current members of that address space. GLOBAL targets every online
CPU, including CPUs currently using the other space. A global operation
serializes with every pending operation. Permission reduction in one space
does not remove the other space's mapping or physical-frame ownership. A
completed permit says only this request completed: an owner must separately
prove no other mapping or owner permits access before freeing/reusing a frame.

Generation UINT64_MAX is usable once, never wraps, and refuses the next begin;
cleanup/context entry and exit remain legal. Reinitialization requires a new
externally distinct world lifetime with no surviving handle/request/caller.
Output and control objects do not overlap. No raw control-field mutation is
allowed outside explicit counter-boundary test setup. Controller storage is
O(C+S); publication/completion and global membership checks are O(C+S), local
range invalidation O(P), and a synchronous waiting caller may wait indefinitely.
Neither fairness nor bounded completion follows from a finite spin limit.

## 8. Implementation and experimental deliverables

Submit the core, the exact ISR/page-fault frame layout, and a dependency map.
For the target, create two original page-table roots and private frames, enable
CR0.WP, and record controlled write-protection faults with the expected CR2,
faulting instruction and present/write bits. Unexpected faults fail the run;
they are not a valid success marker. Use dedicated IST storage and preserve
the interrupted register set. Record per-CPU invalidations, handler counts and
the actual target masks for both spaces.

Compare one four-page request with four one-page requests under the same
correctness contract, CPUs and page work. Record repeated sender-local virtual
TSC intervals plus request and invalidation counts. Do not require a timing
speedup or label virtual ticks physical latency. Separate the deterministic
translation-cache model, real pthread protocol stress and actual guest evidence.

## 9. Verification and pass/fail evidence

Run supplied host tests, independent `oracle.py`, native stress and the actual
guest through explicit completed prerequisite paths. The host reference must
pass, the compilable learner stub must fail a semantic assertion, and five
distinct controls must be detected: early ACK, any-nonzero ACK, timeout release,
entry during the chosen freeze policy, and incomplete GLOBAL target mask. A
crash, compile failure, generic error or sanitizer report is never a successful
semantic-control rejection. The entry-policy mutant tests this contract; a
different proven entry protocol could be safe and must be assessed separately.

Actual guest acceptance requires timeout retention;200 after acknowledgement
despite999 in retired A; exact alternating two-space target masks; permission
faults only in the revoked space, then both; and four versus one request for
equal four-page work. The local flawed condition must report the exact999
stale-read witness. Tests do not establish arbitrary execution, ISA conformance,
physical timing or learner mastery. Missing execution remains missing evidence.

## 10. Criterion-specific rubric

For `.a`: level0 confuses PTE coherence and invalidation; level1 predicts the
ordinary path; level2 derives the full happens-before/ISA chain and rejects
generation8; level3 also justifies entry-before/during/after and counter limits.
For `.b`: level0 merely sends an IPI; level1 passes a two-CPU happy path; level2
implements bounded snapshots, failure retention and nonblocking handler lock
discipline; level3 transfers to GLOBAL scope and derives why CR3 alone gives
insufficient guarantees. For `.c`: level0 calls any crash an exploit; level1
shows a model-only symptom with unclear ownership; level2 supplies the bounded
synthetic stale-read witness and repaired regression; level3 independently
defends exact two-space permissions, shared-frame retention and batching costs.
All three criteria require their own arguments and evidence, not a shared PASS.

## 11. Graduated hints and assessor separation

Hint1: mark the point at which a new entrant may use its first translation.
Hint2: distinguish the generation in a request from the highest generation
already acknowledged by each CPU. Hint3: draw a waiter holding the membership
lock while its local interrupt handler attempts to take that same lock.
Hint4: enumerate all mappings that still own the physical frame after one space
becomes read-only. Private technical analysis and implementations remain under
`assessor/legacy_smp`; learner export contains only the supplied unsolved work.

## 12. Changed conditions and delayed transfer

The original `.a-DELAYED` variant requires actual concurrent entry while
requests execute, with a separately reasoned target-membership trace. The
`.b-DELAYED` variant chooses GLOBAL mappings: preserve exact selected ISA
scope, test PGE on/off and explain why over-invalidation is not a guarantee.
PCID remains an alternative requiring a new implementation and derivation,
not a falsely claimed completed variant. The `.c-DELAYED` variant uses two
address spaces sharing a frame and reduces permissions separately. Retain the
original seven-day unaided assessment delay. The author campaign is readiness
evidence only. Independently extend adjacent batching and revisit sparse CPU
identities, request range boundaries and a permanently missing participant.

## 13. Security scope and containment

Target: original CMK teaching guest and original array-based translation-cache
model only. Environment: disposable no-network QEMU TCG with synthetic100,
200,999/777 values; no disk or production data. Attacker capability: the test
schedule delays its own CPU1 handler and requests accesses through its own
private teaching VA; the deliberate defect is premature frame reuse, violating
revocation/reclamation isolation. The exposing transition is PTE remap →
delayed invalidation → prematureA reuse → old translation access. Exploitability
requires a retained stale translation and reuse before acknowledgement; no
arbitrary code execution, privilege escalation or third-party exploit is claimed.
Harmless success is precisely the synthetic999 read (and777 write in the
explicit model), confined to original live teaching storage. Repair retains A
until all current-generation invalidations and retains shared-frame ownership
across both spaces. Regression checks include timeout, stale ACK, GLOBAL scope,
permission reduction and unexpected-fault rejection. Residual limits include
physical microarchitecture, arbitrary speculation and unmodeled PCID/hotplug.
No host security settings, hardware flashing or external targets are involved.

## 14. Validation status and limits

The source is authored; focused core/native/QEMU pilots are recorded separately.
Only a current passing `reviews/v5/expansion/legacy_smp_shootdown/campaign.json`
and guard establish the declared executed matrix. The screening interruption is
retained in the research register with exact available text and source hashes;
it is not a successful execution. Independent human review, delayed learner
assessment and physical observation remain unattempted. Existing historical
receipts and the retained kernel implementation are unchanged.
