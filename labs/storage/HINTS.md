# Staged storage hints

Reveal one stage only after recording the failing input and your predicted state.

| Milestone | Stage 1: invariant | Stage 2: observation | Stage 3: repair direction |
|---|---|---|---|
| Byte codec | Disk bytes have no native struct type | Decode a literal page with a bad length/checksum | Validate identity, count, extents and checksum before following references |
| Cache | Cached bytes belong to one physical page and generation | Compare repeated lookups with actual backend read count | Include backend identity in the tag and invalidate at publication/checkpoint |
| Lookup/scan | Separator equals the right subtree minimum | Search the exact separator key and its immediate prefix | Equality descends right; scan follows checked leaf links |
| Insertion | Overflow is local until a promoted separator reaches the parent | Draw eight entries and nine child slots before split | Copy leaf separator, promote internal separator, allocate a new root if needed |
| Deletion | A removed minimum can invalidate ancestors even without underflow | Delete first keys, then search every surviving key | Borrow before merge, shrink empty root, update minima and free only unreachable pages |
| Transaction | No uncommitted byte reaches home storage | Compare stable home pages before/after staged updates | Keep a bounded private workspace and abort whole transaction on allocation failure |
| WAL | Success implies a durable commit marker after durable images | Kill the no-flush mutant with an acknowledged commit followed by a crash | Separate accepted writes, successful flush and response delivery |
| Checkpoint | Log evidence outlives every incomplete home write | Interrupt after the first home page and inspect both generations | Flush home images, then alternate metadata, then reclaim the log |
| Recovery | Replaying an after-image twice changes nothing further | Crash midway through recovery and reopen again | Validate the whole batch before write-back; retain replay evidence |
| Fault harness | Each outcome compares complete semantic state | Inject each cut into a known three-operation transaction | Permit old/new for unknown outcome, require new for acknowledged success |
| POSIX adapter | A byte count below 4096 is unfinished work | Trace `pread/pwrite` progress and EINTR/zero/error cases | Continue positive short transfers; stop on zero/error; report failed fsync |
| Integration | An application retry is not automatically safe | Inject an indeterminate VM host-call commit | Return an explicit error, reopen/recover and inspect state before deciding |

Delayed challenge: replace whole-tree separator refresh with path-local repair.
Predict the changed CPU/page-read bounds. Preserve the same public API and compare
byte-key behavior, crashes and objective evidence with the accepted implementation.
Do not read the assessor implementation to pass this challenge.
