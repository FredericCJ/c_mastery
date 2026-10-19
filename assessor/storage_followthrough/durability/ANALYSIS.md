# Technical analysis: persistence and recovery

The public contract retains the exact original criterion IDs. It adds implemented support for the twelve ordinary criteria in PR-T47-07..10 and ten delayed criteria; operation-ID service and logical-redo variants are separate obligations. Do not credit those two from this campaign.

## Authority and expected results

For each of seven commit persistence modes, cuts 1..35 precede publication and return STORE_IO with the old map after reboot. Cuts 36/37 return STORE_INDETERMINATE. The complete new map survives those cuts only in even-subset or all-persist modes: marker page 66 is even. No mode gives a partial application map. All failures poison the original live handle. A final successful flush is the basis for acknowledged durability, conditional on the medium premise.

A lying final flush acknowledges the new transaction while leaving the old marker durable. A lying after-image flush followed by persistence of marker 66 alone produces a valid marker without its batch and must reject. These are explicit counterexamples to using successful return values without their persistence contract.

The actual late-ack worker completes persistence and waits inside the callback. The caller cannot safely reopen the same backend concurrently, even though it can observe the gate's synchronized fact that the marker is stable. It keeps all owners live, releases and joins the worker, receives uncertainty and then reconciles. This is real pthread synchronization, not simulated concurrency or a cancellation implementation.

The 37 checkpoint transitions include all 32 home writes before flush, alternate checkpoint publication before flush, and only then marker reclamation. Thus an intact new marker remains sufficient while homes are mixed. After new metadata is durable, it names complete homes. An obsolete valid or torn marker need not be cleared by a later open. Consequently different interrupted histories need not have identical inactive marker bytes, while each recovered image is byte-idempotent under another open. The test first compares all 32 canonical homes with clean recovery, then compares all 67 pages before/after another successful open and requires no writes. An initial author assertion equating every byte across distinct interruption histories failed; the corrected obligation distinguishes authority from canonicalization rather than suppressing a recovery defect.

## Actual retained limitation and repair

The original source's raw batch validation checks every page's CRC, ID, generation and marker CRC entry. Its tree/root/bitmap validation runs after installation. Resealing a zero root, overfull leaf, self-cycle or unreachable bitmap bit demonstrates STORE_CORRUPT after home and checkpoint publication. That original comparison is a positive observation of a defect, not a positive repaired test.

The additive adapter runs `dg_validate` at the already verified pre-home boundary. A fresh local `store` uses the candidate generation and actual WAL reads; metadata validates the root and allocation bitmap, and the actual `store_check` verifies all reachable nodes and leaf ordering/reachability. Any error returns before a home write. Both passes require an exclusively stable backend. An attacker able to change bytes after validation can defeat this time-of-check premise; the implementation makes no concurrent hostile-backend claim.

Space is one actual store plus one 4096-byte metadata buffer and the actual bounded traversal/cache costs, not 32 after-image pages. Read amplification is measurable: the raw pass reads the whole batch, and structural traversal rereads reachable pages through the candidate's own cache. Original handle read counters omit some candidate-view reads; medium counters observe them. Report both rather than conflating them. With fixed32 pages this is bounded; a scalable generalization is O(P) raw work plus checker traversal and O(cache+height/visited-set) auxiliary state under that checker's representation.

## Generation selection and changed format

CST6 is generated from the exact pinned original source, with guard insertion, three metadata slots, moved home/WAL/marker addresses, `generation % 3` publication, and highest-valid selection. The public header advertises68pages and the magic differs. Six checkpoints cycle1,2,0twice. Torn replacement leaves an older valid checkpoint plus intact newer WAL. Neither two nor three slots allows generation wrap: an attempted commit at UINT64_MAX aborts its volatile transaction before I/O. The lowest-generation mutant fails after real checkpoint advancement. A third slot improves metadata redundancy under the same model; it is not authentication or a proof against arbitrary simultaneous device corruption.

## Independent oracles and falsification

The Python oracle derives259 commit rows, twelve raw medium rows and both exact37-event traces without using the C medium. The map checks inspect all16 keys, absent-output preservation, ordered scan and structural check. There are259 checkpoint cuts,1369 double-recovery cuts and259 second-transaction cuts, plus259 three-slot cuts. Second-transaction recovery permits only the first or second committed prefix, never the initial map. The profile runner repeats actual compiled observations; report profiles separately rather than multiplying a claim of distinct schedules.

The finite model enumerates135 states and332 transitions, with16 terminal states and at most2 pending pages. Cuts recursively restart recovery and visited-state exploration reaches closure for this finite abstraction. The three defective programs yield shortest BFS witnesses. This is an actual explicit-state model checker, not an SMT solver or a proof that the C implementation refines it. The separately checked C event ordering provides a concrete correspondence; whole-byte structural cases and CRC collisions remain outside the three-valued page abstraction.

For the CRC counterexample, toggle the leaf value byte at192, compute32 basis deltas from bits in reserved bytes1024..1027, and solve the full-rank GF(2) system to restore the original CRC. The C decoder returns values165 and164 with the same stored checksum2329055228. This is a local integrity-property counterexample under attacker-controlled synthetic bytes. A MAC would require a secret and explicit authenticated identity/generation/replay policy; a checksum and tree checker cannot authenticate who chose a structurally valid value.

Controls are separately compiled and must exit1 at DUR_CHECK: omitted commit flush, early reclamation, bypassed structural guard, lying successful flush and lowest-of-three selection. A sanitizer abort or failed compile cannot count. Learner stubs compile with actual provider imports and refuse each exercised mechanism; exported workspaces contain public assets only and require explicit provider selection. All generated compiler sources are reproduced byte-for-byte and pinned to the recipe and untouched original.

## Assessment boundaries

Use every ordinary/delayed criterion's original rubric and ask for its specific authority implication and counterexample. Reject an answer that calls every error an abort, treats a checksum as authentication, claims a poisoned handle can retry, frees callback storage before join, reformats a bad image, or calls finite fault schedules exhaustive hardware coverage. Strong work transfers the order proof to three slots and explains which assumptions a changed backend invalidates. No external human, independent learner or physical durability observation is implied by this author packet.
