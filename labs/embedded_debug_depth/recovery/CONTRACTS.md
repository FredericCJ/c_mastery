# Interrupted update, bounded recovery and irreversible effects

## 1. Identity and exercise families

Required additive support for PR-T31-05, objectives T31.09.a–c/T31.10.a–c and their six original delayed IDs. Families A/B/C/D/E/F/G/I/J apply. Preserve the original eight-hour historical milestone and acceptance; additions have no learner-time ceiling.

## 2. Objectives and source bindings

Assess image/handoff authority (.09.a), byte-level interrupted update and shared erase domains (.09.b), checksum/authentication/freshness distinctions (.09.c), recovery's own failure contract (.10.a), persistent bounded retries (.10.b), and detection versus consequence containment (.10.c). The scoped register in `research/v5/expansion/legacy_embedded_debug_recovery/resources.json` identifies actual original criteria, the frozen lifecycle/signature/handoff contracts and inspected OpenSSL primary return/one-shot rules. Exact C17 text remains inaccessible; a C11 draft is labeled as such. Source/title metadata alone is not inspection.

## 3. Prerequisites and learner artifacts

Bring PR-T31-01 connection identity, PR-T31-02 exact fault capture, PR-T31-03 peripheral ownership, PR-T31-04 release/startup evidence and PR-T27-01 startup/handoff reasoning. Import completed PR-T59-02/03 signature/lifecycle/handoff components only with their current contracts. This successor uses actual `lce_record/valid/choose`, `lce_fault_begin/commit`, `lce_admit`, `lce_signature_bridge` and `fb_handoff`; no invented baseline C API is implied. Learner-owned implementations are supplied by explicit paths; assessor implementations remain separate.

## 4. Environment, language and dependencies

ISO C17 portable core, fixed objects, no allocator, no OS/library implementation of the update/recovery mechanism. Hosted tests use six GCC/Clang debug/release/sanitizer profiles. OpenSSL Ed25519 is explicitly permitted only as an external host signature primitive; the assessed mechanism is binding/policy, not Ed25519 arithmetic. The synthetic signing fixture is host-only and uses a deliberately public seed. The Cortex-M4 guest imports the existing original handoff implementation and its exact assembly/linker/MPU contract, with explicit C runtime helper source, no hidden libc/libgcc/compiler-rt. It is QEMU AN386, not F439 Flash or clock hardware. Frozen lifecycle code retains two known single-line indentation warnings: compilation preserves those diagnostics and scopes their nonfatal classification to that unchanged translation unit. New code remains warning-clean under strict flags.

## 5. Concrete inputs, outputs and operations

Implement `recovery.h` in the supplied unsolved `recovery.c`. All objects are valid, live, initialized as stated, disjoint and exclusively serialized. Outputs do not overlap inputs/control/storage. The only permitted callback reentry is the explicitly bounded logging path below. No callback retains a borrowed buffer.

`struct du_slot` is64bytes. LE32 fields at offsets0/4/8/12/16/20/24 are magic31554944, version, payload extent, base, Thumb entry, stack and clock. Payload occupies28..51, unused bytes are zero; generation is52, reflected CRC32 of0..55 is56, and commit wordC04D17ED is60. CRC uses polynomialEDB88320, initial all-one state/final complement. Payload10..24 starts with matching LE32 stack/entry. Version/generation/clock are nonzero; base is512-aligned; the checked image range contains at least two entry-instruction bytes after the first eight vector bytes. Stack is8-aligned in(20000000,20020000]. These bounds are this small teaching format, not general firmware admission.

`du_select` accepts one to three records and trusted floor/maximum/clock. Validate integrity/commit/layout before compatibility/selection; return highest compatible version. Conflicting byte-distinct records of an equal compatible version refuse. The original fixture has committed1, uncommitted2 and committed3; select3. Failure preserves selected output. Changing an immutable trusted floor requires a separate policy argument.

`du_install` operates on two logical slots with the active index and explicit physical erase-sector identifiers. Admit only valid newer version/generation, a distinct destination and distinct erase domains. Reject shared sectors unchanged before erasing; changing logical indices cannot repair a shared physical erase unit. The declared digital model atomically erases only the inactive sector, then programs64bytes in ascending four-byte units. The cut supplies0..16 completed units and an arbitrary bit-subset mask in the next unit. Programming only changes1→0. Return interrupted unless all units were issued, but recovery decides validity from actual bytes: a partial final program can already equal the complete marker. The old slot remains recoverable at every admitted cut. This model does not assert atomic physical erase, real Flash retention or arbitrary analog-fault coverage.

`du_prepare` consumes a current trusted `struct du_image` descriptor and machine-state evidence. Validate bounds, privileged thread state, no active exception or outstanding DMA/peripheral ownership, and exact current clock agreement before returning vector/stack/entry. Reject a16MHz application contract when the selected environment is25MHz. A C struct is not an unforgeable authority; immutable current image ownership and truthful state inputs are coordinator premises. The actual guest starts no DMA/IRQ/peripheral operations and invokes the unchanged `fb_handoff`: disable maskable interrupts/SysTick, replace MPU mappings, set VTOR/CONTROL/MSP and branch in assembly without returning through an obsolete C stack. Original app assembly checks those exact registers. No pending NMI, lazy FP frame or foreign exception/stack owner is permitted; PRIMASK alone cannot establish those premises. A different vector is rejected by the actual application check in the target mutant. No claim of hardware reset or board clock transition follows.

For the authorization counterexample, change synthetic payload42→99 and recompute CRC: integrity-only decode still accepts. In the repaired boundary, `lce_admit` authenticates immutable LE32 version/key-ID/payload via the actual signature bridge and trusted key policy. The host fixture signs version3, confirms its Ed25519 signature remains valid, then raises the trusted floor to4 and requires rejection. Altered bytes, revoked key and truncated signature also reject without publishing version. A stronger unkeyed checksum does not fix authorization. Current rollback floor/key storage are trusted premises, not implemented tamper-resistant persistence; signing and verifying with one provider is integration evidence, not independent cryptographic correctness.

`struct du_retry` contains two separate24-byte lifecycle records. Explicit `du_retry_initialize` is trusted fresh provisioning, never automatic recovery of corrupt/empty media. Records encode sequence and consumed attempts. Before granting retry permission, write the incremented count to the inactive independent record in byte order and commit it; only a complete24-byte write returns permission. A cut grants nothing. On restart choose the highest valid sequence and retain consumed count. Commit-before-crash can lose availability but cannot issue an extra retry. All-invalid, conflicting, exhausted or out-of-range state conservatively stops. Five failures with limit2 issue exactly two retries; resetting volatile state cannot reset the persisted budget. Counter exhaustion refuses while old records remain readable. The memory-backed persistent model must be mapped to a device's real durability/erase guarantees before hardware use.

`struct du_recovery` starts all-zero. `du_effect(1)` publishes the synthetic output and increments an irreversible emitted-effect count, refusing counter overflow. First `du_fault(reason,step,log)` latches failed/first-reason/detection step before invoking the logger. One nested logger call may report another fault; it records a secondary reason/nested flag without invoking the logger recursively or replacing the first payload. Logger failure produces bounded fallback metadata, never normal-output recursion. `du_recovery_input` checks status/count before dereferencing: status0 progress,1 timeout,2 device failure; unknown status, reported>offered or nonzero count on failure refuses. Accepted bytes have offered≤64, reported≥2 and a length byte≤reported−1. A count such as SIZE_MAX never enters pointer arithmetic.

After a fault no new output1 is admitted. `du_contain` invokes a reliable bounded safe-output callback at a step not preceding detection. Exact return1 acknowledges containment and sets current output0; other results retain conservative unknown/not-contained authority. Repeating successful containment has no new callback effect. The prior emitted count is never erased. In the fixture, output1 precedes detection step10, failed shutdown step12, and successful containment step13: prior effect1 and latency3 remain explicit. This is synthetic logical time, not measured actuator safety or physical shutdown latency.

## 6. Necessity and counterexamples

Refute newest-record-implies-safe-jump, commit-last-implies-all-Flash-safety, checksum-implies-authenticity, valid-signature-implies-freshness, logging-cannot-fail, volatile-retry-count-bounds-reset-loops, and detection-erases-prior-effects. Each requires its own observable control. An invalid image and an ambiguous shutdown are different failure states; do not flatten both into a generic error code with no retained authority.

## 7. Required invariants and costs

Derive byte format and every extent before decoding. Prove old-slot preservation from disjoint erase domains and inactive-only writes. Identify physical assumptions explicitly. Show that all successful retry grants correspond injectively to durable consumption, while committed-but-undelivered permission can only reduce availability. Prove first-fault publication precedes callback reentry and nesting does not allocate/grow recursively. Preserve external-effect history independently of current safe output.

All portable operations are bounded: decode/CRC O(64), selection O(3×64), install O(64), retry O(24), fault/containment O(1) plus bounded callback cost. Report actual target code/stack/link dependencies. Host cryptographic costs belong to the permitted provider and must not be credited to a first-principles implementation.

## 8. Deliverables and experiments

Submit completed source, byte/state transition tables, ownership ledger, independent predictions, source/tool manifest and raw logs. Execute `tests.c`, complete `replay.c` against `oracle.py`, actual signature fixture and the dedicated handoff guest through explicit provider paths. Retain positive and deliberately incorrect executions. Show that the shared-sector control invalidates the old-slot argument before writing. Demonstrate callback-count refusal with null data under an invalid-count result, nested logging failure, every retry prefix cut and commit-before-permission loss. Keep actual emulator architecture evidence separate from the digital persistence model.

## 9. Verification and pass/fail criteria

Require six host profiles, the full core/control/stub matrix,561 next-word bit-prefix cuts,256 byte-subset cuts and50 restart/permission-loss cases per profile against an independent byte/CRC oracle. Require actual valid old Ed25519 verification plus current-floor refusal and distinct fail-open/wrong-key/revocation controls. Require three M4 toolchain profiles, unchanged provider provenance, actual register-check success and wrong-vector/clock-bypass/stub failures. A compiler failure, generic process crash or sanitizer error does not satisfy a semantic mutant. Recompute source/log identities and exact source/flag commands. Changed dependencies invalidate affected evidence until rerun.

## 10. Objective-specific rubric

Each row uses level0 missing/incorrect; level1 ordinary example only; level2 the named implementation, boundary proof and independent result; level3 the original changed condition after at least seven days without copying a worked solution.

* .09.a: distinguish metadata selection, truthful live state and actual architectural handoff; transfer changes clock/vector and revisits transition requirements.
* .09.b: derive byte-valid commit and old-slot preservation, including early-valid torn marker; transfer rejects shared erase domains and redesigns storage placement.
* .09.c: execute the harmless recomputed-CRC counterexample and real signature-policy composition; transfer rejects a validly signed old image under an independently trusted floor.
* .10.a: admit callback counts before access, preserve first fault through logger failure/nesting, and state bounded fallback ownership.
* .10.b: derive two retries from five failures and exact record validity; transfer retains consumption across volatile reset and stops conservatively on corrupt persistence.
* .10.c: retain prior effect and failed-containment uncertainty, then acknowledge a distinct safe transition; transfer cannot invent rollback of an already emitted effect.

## 11. Hints and assessor separation

Hint1: write the byte-validity and permission predicates separately. Hint2: a failed writer call and recovered valid bytes can coexist. Hint3: consume permission durably before using it and latch first-fault authority before callbacks. Hint4: represent effect history separately from current output. Reference implementations and detailed analyses are withheld under assessor paths; no learner export may include them.

## 12. Changed conditions and transfer

All six original delayed variants remain required, with their original IDs/minimum_days7. Implement them using the concrete clock/vector, shared-sector, signed-replay, nested-fault, persisted-retry and irreversible-output tasks above. New compiler, physical program width, reset class or interrupt ownership changes invalidate the corresponding prior argument. Do not claim a scripted callback or emulator establishes physical persistence, NMI safety or board fault tolerance.

## 13. Security scope and containment

Target only original synthetic slot bytes, original host signature fixture and the dedicated original QEMU image. The attacker may alter image bytes/recompute an unkeyed CRC or replay a previously signed synthetic image; it cannot modify the trusted key/floor. Harmless success is acceptance of payload99 by the integrity-only teaching decoder, followed by rejection at the explicit authenticated/current-policy boundary. No host security changes, external service, production key, deployed system or hardware flashing. Repair is authenticated exact-byte/key binding plus current rollback policy; regress all substitutions and state limits of trusted policy/provisioning. A crash is not arbitrary code execution or privilege escalation.

## 14. Validation status and limitations

Current author campaign/guard paths are under `reviews/v5/expansion/legacy_embedded_debug_recovery/`; source and logs determine whether authored work was actually executed. Independent review, unaided delayed learner assessment and physical Flash/power-cut/actuator observations remain separate and unclaimed. Tests/oracles cover finite stated schedules, not all faults or absolute mastery. No existing historical acceptance receipt is rewritten.
