# Exact T56 private assessor derivations

The original contracts and narrower historical receipts remain valid for their stated scope. Each cited guest case is an actual mechanism; physical and independent observations are separate.

## AX-PR-T56-01-T56.01.a

Case26 joins the exiting thread with result17 and shared value123 while its sibling remains blocked and two TCBs remain. Closing the queue later releases the sibling independently.

Changed condition: Case13 fixes the parent to CPU0, observes its child actually current on another CPU, kills it, and checks that kill does not reap the remote mapping. Wait succeeds only after quiescence; frames/references return to baseline.

Exact guest selectors: 26, 13.

## AX-PR-T56-01-T56.01.b

Case12 has four live threads in the first process and refuses the fifth; a consumed join is stale. The historical formatter limit is a real domain mismatch, not evidence that32 scheduler slots are unusable.

Changed condition: Eight actual processes each hold four live threads. At32 total,31 are blocked and one runs; a ninth process and33rd thread are refused with unchanged frames/references/ready/wait counts. Closing wakes all31 and actual joins/waits restore one process/thread.

Exact guest selectors: 12.

## AX-PR-T56-01-T56.01.c

Case1 observes the current process and thread share the same low32 numeric token. Passing the tagged thread to process wait and the tagged process to thread join is refused.

Changed condition: SF adds high-bit type tags and separate32-thread/64-capability encoders, validates recipient/owner/incarnation, and refuses wrap. The retained16-slot formatter and old ABI evidence remain untouched. Case12 reaches the previously unencodable slots.

Exact guest selectors: 1, 12.

## AX-PR-T56-02-T56.02.a

Case14 deterministically waits before exit7, then observes quiescence before a second wait7, rejects repeated/nonchild/stale waits, and reuses the slot with a new generation.

Changed condition: Case18 submits actual virtio I/O from a grandchild, observes the hardware used entry while holding only the observer, kills its supervisor and lets the orphan exit. Waited/quiescent/reaped can all be true while io_pending remains true; slot admission excludes that incarnation until actual observation.

Exact guest selectors: 14, 18.

## AX-PR-T56-02-T56.02.b

Case16 executes the retained checked loader at every allocation failure, including the third frame. The full SF probe additionally compares complete task and thread tables before/after the SDK frame/ref bitmap checks.

Changed condition: An already-live child is present during every cut. Public resource ledgers remain equal and that child subsequently exits7 through the wait-before-exit protocol. No candidate becomes a live task before the last allocation succeeds.

Exact guest selectors: 16.

## AX-PR-T56-02-T56.02.c

Case14 checks default child disk denial, explicit inheritance from a capable parent, and refusal when an incapable child requests inheritance. Case27 shows actual COW frames differ after a write; SPAWN uses a separate archive image.

Changed condition: Case12 fills all eight process slots, rejects a ninth and preserves the32-thread resource ledger. Every admitted child then completes and is reaped; the immutable archive remains the same pinned input.

Exact guest selectors: 14, 27, 12.

## AX-PR-T56-03-T56.03.a

Case2 reserves two absent pages and checks no present PTE, reads all8192 bytes and writes only page2. It verifies independent values and materialized mappings.

Changed condition: The allocation injector seeds every new frame with0xa5 before the real materializer clears it. All4096 bytes in each page are independently checked as zero; this controlled allocator premise is distinct from a physical remanence measurement.

Exact guest selectors: 2.

## AX-PR-T56-03-T56.03.b

Case15 pins two actual threads to different CPUs and gates both saved page faults before either lock acquisition. Two arrivals produce one physical frame; both writes survive. This is an explicit test import, not an uncontrolled scheduling hope.

Changed condition: The256 cause/PTE combinations distinguish read/write/fetch, user/supervisor, reserved bits, zero and COW entries. Case16 executes NX fetch and supervisor/out-of-region faults; already-present permission does not excuse a forbidden access.

Exact guest selectors: 15, 2, 16.

## AX-PR-T56-03-T56.03.c

Case16 executes actual fault allocation exhaustion, outside-range and supervisor access. Each child dies with fault status while the parent and complete resource ledger survive.

Changed condition: Case9 admits the first destination page and fails the second. The first is zero-materialized but the destination bytes and message count stay unchanged. Close and a short-capacity retry still do not consume; successful whole retry receives the original two bytes.

Exact guest selectors: 16, 9.

## AX-PR-T56-04-T56.04.a

Case27 observes refs1 before clone and refs2 while the child is held. Child write99 obtains a different frame withrefs1; the parent retains41 and returns torefs1 after child exit.

Changed condition: Case3 lets the child exit before a write, disables allocation, then writes42 through the sole COW owner. The physical frame address is unchanged, proving permission restoration without a copy.

Exact guest selectors: 27, 3.

## AX-PR-T56-04-T56.04.b

Case15 rejects multithreaded private clone; case10 rejects the presence of PTE_SHARED before any private-clone mutation.

Changed condition: Case10 separately uses the retained explicit shared-fork profile and new SF mapping tracking. Cases5/11 enforce mapping rights, creator/reference lifetime, actual read-only hardware rejection and unmap acknowledgment before last-reference release.

Exact guest selectors: 15, 10, 5, 11.

## AX-PR-T56-04-T56.04.c

Case27 fails the actual child private-copy allocation after its stack is materialized. The child faults; the parent still has41 in its original frame and one reference. Remote invalidation remains the retained actual provider operation, never inferred from a native model.

Changed condition: Case3 temporarily saturates the selected actual page reference, snapshots512 PTEs and task/thread counts, and requires SDK_FULL with no clone mutation before restoring the synthetic saturation. No wrap or partial clone is accepted.

Exact guest selectors: 27, 3.

## AX-PR-T56-05-T56.05.a

Case6 fills exactly eight messages, observes one blocked sender, closes the queue, joins the sender with CLOSED and lets a surviving receive-only process drain all eight in FIFO order.

Changed condition: Case8 runs a three-process pipeline until both queues hold16 accepted messages and two sends remain pending. Downstream release delivers all20 in order; close/drain/join leaves zero messages/waiters. Case7 also preserves accepted bytes after owner exit.

Exact guest selectors: 6, 8, 7.

## AX-PR-T56-05-T56.05.b

Case4 receives one byte at the last admitted address despite spare capacity, then rejects an invalid destination without consuming the next message. Case9 spans two materialization pages and rejects a too-small span.

Changed condition: After the first failure, case9 closes the channel and repeats a short-output rejection. The original queued payload survives both events, destination bytes stay unchanged, and a later full copy drains it exactly once.

Exact guest selectors: 4, 9.

## AX-PR-T56-05-T56.05.c

Cases4/6 reject513-byte input, a receive-only send, nonowner close and a destroyed stale token. The kernel copies only validated lengths.

Changed condition: Case8 delegates receive-only and send-only capabilities through the process pipeline. The intermediate process explicitly attempts to amplify each direction to both and is refused; actual downstream flow succeeds with the admitted subsets.

Exact guest selectors: 4, 6, 8.

## AX-PR-T56-06-T56.06.a

Case5 maps one object twice, drops its creator handle, then unmaps one while the other remains readable; the final unmap restores the frame/reference ledger.

Changed condition: Case10 runs1000 release/acquire SPSC transfers across inherited shared mappings, closes the creator, unmaps the parent while the child continues reading, then releases the final child mapping and reaps it. Every reference is accounted.

Exact guest selectors: 5, 10.

## AX-PR-T56-06-T56.06.b

The retained CMK2 map always grants its fixed profile. SF adds explicit one-page RO/RW rights, no execute authority, protected-region exclusion and checked address/length/offset admission.

Changed condition: Case5 rejects X/protected/wrapping extents and observes RO PTEs; case11 grants a child read-only authority, refuses amplification, then executes a write fault through the actual read-only PTE and verifies final reference reclamation.

Exact guest selectors: 5, 11.

## AX-PR-T56-06-T56.06.c

Two unsynchronized reads of10 can produce two writes of11; for C threads conflicting non-atomic accesses are undefined. The author does not execute that undefined program and treat its output as an oracle.

Changed condition: Case10 uses separate cache lines and release/acquire for publication and slot reuse, including counter wrap. The new H745 images declare SRAM3 noncacheable/shareable; the frozen AMP cache/doorbell/DMA package and physical capture plan retain the noncoherent assumptions separately.

Exact guest selectors: 10, 24, 25.

## AX-PR-T56-08-T56.08.a

Case17 opens the exact10-byte abcdefghij file twice, reads3 through the first and4 through the second, and checks their byte content.

Changed condition: It duplicates the first, reads2 through the duplicate, then reads offsets5 through the original alias and4 through the second independent open. All references close independently.

Exact guest selectors: 17.

## AX-PR-T56-08-T56.08.b

Case17 rejects negative and overflowing displacements without changing offset and verifies exact EOF after10 bytes.

Changed condition: The additive SF_SEEK_EXTENDED admits0..1048576, preserves the historical SDK domain, admits position100, returns zero at EOF without changing100 and rejects1048577. The ten-byte byte hash is independently fixed.

Exact guest selectors: 17.

## AX-PR-T56-08-T56.08.c

The actual parser probe rejects duplicate names, a traversal entry later in the archive, malformed payload extent and traversal query. File close invalidates its generation-bound handle.

Changed condition: Case17 rejects destination copy crossing USER_LIMIT and observes offset5 unchanged. Case9 independently shows byte-channel message publication remains unchanged when two-page copy admission fails.

Exact guest selectors: 17, 9.

## AX-PR-T56-09-T56.09.a

Case19 sends actual encoded programs from two processes to one VM owner. Malformed input is refused without advancing its ID; a loop exhausts its declared fuel while another client receives42.

Changed condition: The native/guest core checks two simultaneously retained pair results whose raw heap slot/generation collide. An instance/session/request wrapper rejects cross-instance access while the correct instance reads19. Each arena has its own heap and budget.

Exact guest selectors: 19.

## AX-PR-T56-09-T56.09.b

Case20 holds the owner unbound until the actual client process exits, executes its accepted request, observes the closed client-owned reply queue and releases it. The core rejects results larger than64 bytes and retains a valid response when caller capacity is too small.

Changed condition: Core tests cancel before admission, immediately after admission, after one instruction and after result production. Every matching cancel suppresses reply; a stale cancel cannot target another request. There is no external host callback in this profile, so cancellation does not invent device quiescence.

Exact guest selectors: 19, 20.

## AX-PR-T56-09-T56.09.c

The new wire contains a verified container, session/ID, fuel, GC and response capacity. Case19 exercises arbitrary bytecode rather than the old fixed service selector. The old adapter remains valid for its narrower purpose.

Changed condition: The core tests UINT64_MAX request admission followed by refusal of zero/older IDs, strictly increasing sessions, same-ID/different-payload conflict and delayed old-session reply/handle rejection. Current session and ID are checked before publication.

Exact guest selectors: 19, 20.

## AX-PR-T56-10-T56.10.a

Case21 uses two actual clients and one store-owning process. Each PUT reply follows the real store commit/flush callback; a client never mutates the owner cache.

Changed condition: One serialized batch reads left then right after both PUT replies and returns7/9. The native service executes200 durable effects,200 exact retries and200 conflicts plus11 reopenings; these are explicit per-operation checks.

Exact guest selectors: 21.

## AX-PR-T56-10-T56.10.b

Case21 commits increment42, exits the actual owner before reply and reopens its disk in a new process. Retrying the same ID returns42 without adding again; an empty reply queue leaves the caller uncertain before retry.

Changed condition: The source-bound retained receipt campaign executes all76 cuts with write/reboot pairs (152 actual guest executions),37-event host fault cuts and256 sector masks. Its exact source/receipt guard is rerun; unchanged receipt/store providers are consumed here, with clear narrower-value and failure-model provenance.

Exact guest selectors: 21.

## AX-PR-T56-10-T56.10.c

Case22 keeps the owner unbound, queues one configuration plus seven requests, rejects the next as FULL, then admits it safely after draining. Case21 separately loses the postcommit reply.

Changed condition: Case21 retries the committed ID with delta43 instead of42 and receives conflict while the stored result remains42. The native receipt tests repeat this payload-binding check200 times without new writes.

Exact guest selectors: 22, 21.

## AX-PR-T56-11-T56.11.a

Case23 boots write, records the disk/image hashes, reboots verify against the same disk and reads42. A separately reset fresh-file control observes zero before creating its own42.

Changed condition: Case22 damages or zero-truncates an actually written uncommitted WAL page while the commit marker is durably inactive. A fresh owner recovers the previously committed42. This does not permit ignoring corruption under an active valid commit marker.

Exact guest selectors: 23, 22.

## AX-PR-T56-11-T56.11.b

Case21 waits for actual owner exit, closes/destroys its endpoint, creates a new endpoint and process, mounts the retained disk and rejects both the old token and old request session. VM core tests likewise reject delayed results for a newer session.

Changed condition: Case18 kills a supervisor while its child owns actual submitted I/O, then lets the orphan exit. CPU/page ownership resolves before its pending request slot can be reused; device completion observation resolves the remaining retention.

Exact guest selectors: 21, 19, 18.

## AX-PR-T56-11-T56.11.c

Cases21/22 provide actual rejected-before-accept, committed-before-lost-reply and completed observations. The retained journal-cut campaign distinguishes old/new durable outcomes under its explicit flush/sector model.

Changed condition: After three newer receipts evict ID5, case21 refuses delayed ID5 as stale instead of reapplying42. The durable high-water mark preserves no-reapply safety; a client needing the original result beyond that window needs longer retention or an explicit reconciliation policy.

Exact guest selectors: 21, 22.

## AX-PR-T56-12-T56.12.a

The frozen AMP task images provide M4 acquisition and M7 compute with explicit cache/epoch/doorbell/DMA assumptions. The new paired polling images use the identical bounded synthetic workload and static M7 boot/MPU authority.

Changed condition: New normal/swapped image pairs exchange acquisition and compute on M4/M7. Source and map audits identify private memories, shared SRAM3, role code and dependency closure. Cases24/25 execute both assignments on coherent SMP; physical H745 roles remain unobserved.

Exact guest selectors: 24, 25.

## AX-PR-T56-12-T56.12.b

Host and actual two-CPU guest execute the same128-item body. Burst1 yields128 admitted/completed and checksum8384; host CLOCK_MONOTONIC nanoseconds and QEMU TSC counts are retained separately. No physical timing sample exists.

Changed condition: Burst4 identically yields47 admitted,81 rejected,45 completed,2 uncertain and checksum1980. Join/phase acknowledgment precedes final uncertainty classification. The audited H745 images implement the same policy; their physical ledger/timing remains an explicit capture obligation.

Exact guest selectors: 24, 25.

## AX-PR-T56-12-T56.12.c

Both M7/M4 images are built for each role/burst and compiler/optimization, audited for unresolved/runtime dependencies and architectural ordering. Physical observation stays not-run.

Changed condition: BOARD_CAPTURE.md gives the concrete later authorized capture procedure, equal-workload expected ledger and retained AMP scheduler/DMA/restart routes. Missing hardware is an external observation gate; historical narrower receipts remain unchanged rather than being promoted.

Exact guest selectors: 24, 25.

