# T28.10 assessor analysis

These original reference implementations are assessor-only. The public files deliberately remain unfinished. Preserve original criterion IDs/tasks and seven-day interval. A reference run is not a learner attempt or non-author review.

<a id="10.a"></a>
## 10.a — Admission and representation

Use subtraction bounds rather than address+capacity comparisons that can wrap. Validate all4 descriptors before enable, including later descriptors; copy to private state before the caller may alter the descriptor array. The maximum total is4*4096 and fits uint32_t. Pairwise comparison is O(n^2) with n<=4; a poll is O(1) except a single bounded configure; submit/finalize cache work is proportional to total whole lines. There is no heap allocation. Equivalent alternatives include a prevalidated immutable descriptor object kept borrowed, but then the descriptor lifetime contract must change and its physical accessibility must remain CPU-only for software SG. A hardware linked list requires a different hardware source and representation.

The six core controls respectively remove bank rejection, line rejection, destination clean, generation check, quiescence check and post-invalidation. Each compiles; assertion failures are semantic observations, not translation tests. Stubs must compile and fail the first obligation. Big-endian PPC64 runs the integer core with original minimal output helpers, not an STM32 board. Exact source/command/raw/output hashes bind claims to one run.

<a id="10.b"></a>
## 10.b — Ownership and uncertain progress

The useful invariant is: phase!=0 implies all submitted payloads remain borrowed, and `done<=touched<=total` after a terminal calculation. Running done counts whole completed extents. Stop retains ownership through FIFO drain. Cancel with remaining8/16 has a known8-byte prefix only after EN0/FIFO-empty/TC; on TE the current extent is uncertain even if a stale NDTR says0. On configure failure for the next extent, no new extent was published and the completed prefix is exact. Repeated polls of READY may succeed, but only one finalize commits a result and changes phase0. Failed/repeated finalize must preserve the caller's sentinel result.

The prior `fw_dma` function remains valid under its abstract quiescence input. `composition.c` deliberately binds that input incorrectly to EN and observes early retirement while the model FIFO is still flushing; the new adapter preserves the lease. Do not call this a discovered defect in every possible baseline adapter. The independent probe's expected rows are calculated from enumerated premises in Python, not by calling the C implementation.

Delayed: each cut k of three16-byte extents leaves48-k bytes when k is width-aligned. For k=16, drop the first descriptor; k=20 advances the second by4. At k=48 return an empty vector only after validating every original extent. Preserve count/output on refusal. Non-idempotent peripheral replay can duplicate effects, so a valid memory-copy suffix is a counterexample to an overstated general resume claim, not a generic retry implementation. Cached k=1 is mathematically a suffix but fails the whole-line transfer contract. Replay the owned whole extent only after quiescence and only under immutable-source copy semantics.

<a id="10.c"></a>
## 10.c — Cache and dual-core integration

`cache_test.c` composes the actual B06 cache-range emitter with a declared two-line RAM/write-back model. Source dirty7 must reach RAM before DMA; destination tail dirty55 must survive partial cancellation. A clean speculative fill sees old9 before DMA writes42; post-invalidation exposes42. Omit destination clean: the untouched tail is lost. Omit post-invalidation:42 remains invisible. Independently invalidate a dirty neighbouring77: it falls back to RAM55, demonstrating why whole-line exclusive ownership is necessary. These are defined synthetic cache-model results, not portable outputs of undefined C races.

`notice_test.c` calls actual `fw_peer_step`; outstanding BEGIN blocks HEALTHY after TAKE and only END after ACK permits it. A pure four-action transition-table oracle explores all1024 length5 histories and compares5120 transitions. The port model separately simulates metadata RAM vs local cache: remove pre-invalidate, payload invalidate or metadata clean and the selected test fails. Test both COREIDs, failed lock, saved PRIMASK0/1, exact HSEM addresses, no cache operation on failure and one release. The port model's primitives stand in for target MMIO and CPU instructions; actual emitted objects additionally verify addresses, word accesses and barriers. The M4 cache-provider build intentionally differs from M7.

Cold-start initialization requires a global reset/lease premise. HSEM ownership alone cannot prove another DMA channel stopped or another CPU released a payload. Do not accept any repair that clears the mailbox after timeout while the receiver may still read. Peer liveness, physical FIFO completion/cache maintenance and MPU/cache attribute setup are board obligations with complete protocols but no fabricated observations. ES0445 write-through erratum means selected write-back configuration must not silently become FORCEWT/SIWT. Exclude concurrent HASH DMA from this bounded setup; do not generalize across other H7 DMA engines or the USART TRBUFF workaround.

## Scoring and changed-condition separation

Per objective score representation/contracts0–3, implementation/invariants0–3, falsification/diagnosis0–3, independent changed-condition reasoning0–3. A score3 requires a correct derivation and a counterexample to a plausible false explanation. Any premature lease return, stale generation acceptance, silently lost dirty byte, unrestricted library dependency or invented physical observation fails the relevant criterion regardless of total. Valid alternative cache strategies include an explicitly noncacheable shared bank with revised MPU/alias and performance arguments, not merely deleting maintenance calls. Reference closures remain author-complete only; independent performance and delayed assessment are pending external dimensions.

Give the scatter/gather, suffix and H745 packets at least7 days after the corresponding first assessment, without reference implementation. Require the learner to choose a new bank/width or request source and explain why the proof must be revisited. Do not award general peripheral retry, automatic C ownership enforcement, cross-core wakeup latency or physical throughput from this bounded evidence.
