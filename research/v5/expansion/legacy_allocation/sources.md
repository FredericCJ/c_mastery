# T16 source and depth audit

Inspected the retained T16 teaching, all42 original supplemental tasks and their
42 exact delayed tasks, all nine project contracts, and the actual retained
`algo_pool` declarations/implementation. The retained implementation supports
16 int32 slots, index/generation identities and UINT32_MAX retirement, but selects
the owner through the caller's pool argument. No embedded owner token exists.
The new three-slot wrapper therefore composes rather than edits that producer.
The old arena uses uintptr_t address alignment; the new portable policy instead
states a caller-proved base-alignment precondition. Neither byte arena grants
arbitrary typed lvalue access.

The new authored mechanisms fill concrete gaps: validated nested marks and
commit, LIFO resource cleanup, boundary-tag structural checking, deterministic
first/best placement, deferred collection, failure-preserving migration, bounded
classes, whole-sibling buddy coalescence, independent histories, real host
mapping/thread adapters and exact ordinary/delayed reasoning dossiers. Existing
headings were not treated as executable evidence. Historical estimates,
acceptance and fixture expectations were preserved.

Source records contain the exact inspected locators. Wilson's primary PDF was
downloaded and pages6–9,27–29,33–35,38–39 read. Historical policy observations do
not establish contemporary performance. N1570 effective-type, allocation and
selected synchronization passages were read, with N3096 realloc comparison.
These are drafts; exact published C17/C23 access remains a separate gap. A guessed
Hanson `mem.pdf` endpoint was unavailable, so this batch makes no fresh memory-
chapter inspection claim. Existing held-edition records remain historical.

Open Group2024 mmap/mutex/join pages returned403. Actual Linux man-pages and the
explicit POSIX2017 mutex reproduction were then read. Do not relabel that
reproduction as the unavailable2024 original. Private source PDFs/extractions
are under ignored build storage and are not learner artifacts.

One initial independent-oracle run generated a second allocation for an already
live client ID and overwrote its descriptor. That violated the oracle driver's
single-owner-per-ID trace grammar and produced a real mismatch after101 actions.
The correction restricts a live ID to release/resize/collect; it does not alter
allocator results. The final full campaign reruns every trace. Early TSan output
identified the intended payload race but lacked its external symbolizer; the
actual diagnostic and binary offsets remain evidence, not a fabricated source-
resolved stack. Final reports retain exact command output and limits.

The byte-class capacity accounting is intentionally explicit: activating the
8/16/32 logical classes accounts224 bytes, whereas embedded32-byte arrays reserve
384 payload bytes plus typed metadata for the entire containing object's lifetime.
The separate four64-byte-slab model cannot be mistaken for actual munmap or a
measurement of that structure. Similarly, finite lifetime diagrams and buddy
node bounds do not claim physical scheduling bounds or independent mastery.
