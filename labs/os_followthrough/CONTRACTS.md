# T49 operating-system followthrough

This additive package retains the original ten project briefs, fixtures, criteria and unfinished anchors. `criteria.json` identifies every original task and seven-day variant. Implement `os.c`; use the complete hosted probes as instrument examples and extend their changed conditions from your frozen prediction. The assessor implementation is separate. `build.py --output /tmp/os-learner` builds an intentionally failing scaffold. A successful author campaign supplies worked examples, not an independently assessed learner result.

## 01 Calls cross distinct boundaries

An ISO C function call describes evaluation, object lifetimes and function types. `write` itself is a POSIX/Linux interface, not an ISO C library function. Its libc wrapper translates the selected machine return convention to `ssize_t` plus `errno`; kernel validation/copy and the eventual device operation are later boundaries. A successful count does not make an old `errno` meaningful or promise persistence. For this Linux x86-64 specimen, syscall number 1 is in RAX; descriptor, pointer, count are RDI, RSI, RDX. SYSCALL changes RCX/R11; the compiler must also know about memory and flags. `raw.h` declares those effects. The AArch64 comparison uses SVC, number64 in X8 and X0/X1/X2 arguments, with X0 the signed result. Do not transplant registers or assume the C calling convention is the syscall convention.

Predict raw -9 versus wrapper -1/errno9, then observe a real pipe at both optimization levels. On a positive short count, advance only that prefix; retryable errors keep it unchanged. The deterministic adapter exposes done/remaining after EINTR, short progress, EAGAIN and terminal failure. Its record variant treats a partial record as a protocol error: replaying the suffix is not automatically valid. The real nonblocking pipe supplies a separate short-write trace.

## 02 Ownership survives creation failures

Draw separate nodes for each process address space, explicitly shared mapping, open-file description, thread stack, argument object, join record and child wait record. Fork copies the private value; descriptors can still reference a common offset. A worker may share an atomic counter while its argument remains in a scoped parent array. The parent owns every successful creation until join/wait returns, including a prefix interrupted by a later creation failure. A direct pthread error is not an errno result.

In `lifecycle.c`, one forced second-creation failure leaves exactly one worker to join. A child can exit before publishing: empty report and exit7 together explain that case. Shared-memory publication uses a process-shared semaphore before the parent reads. Buffered stdio state copied across fork produces two flushes unless flushed first. EOF needs every writer reference gone; `descriptors.c` holds one inherited writer behind an explicit release handshake to expose that mistake without hanging.

## 03 Time is a stated model

Accept at most16 jobs, positive service and quantum, arrivals below256 and a completed trace of at most256 ticks. Reject before publishing output. At each boundary, completion precedes new arrivals; arrivals enter in time/index order before an expired RR job rejoins. SJF/SRTF ties use arrival then index. FCFS/SJF run selected jobs to completion; SRTF reevaluates each service tick. RR completion exactly on its quantum needs no requeue.

For zero overhead, turnaround=finish-arrival=waiting+service and response=first-arrival. Derive these again from the emitted trace. The successor marks idle -1 and dispatch -2. A selection is reserved throughout its dispatch delay, with intervening arrivals admitted before the next decision. Waiting includes a job's charged dispatch cost; queued delay subtracts that cost. More frequent switching can improve first response while reducing useful throughput. `oracle.py` independently compares tick/deque traces, and `mutants.c` kills changed tie selection and idle-as-wait accounting.

## 04 Fairness policies require different state

MLFQ has level, consumed allotment and queue order. Budgets1/2/4 and boost every16 ticks are explicit. Yield requeues a running job but preserves consumed allotment. Demotion is processed before a boundary yield can erase it. Sleep preserves residual allotment; the stride model clamps waking credit to the active minimum rather than granting unbounded catch-up. A boost resets all level/allotment state at its declared boundary.

Stride selects the smallest pass and increments by floor(scale/weight). Scale28 with weights1/2/4 gives exact increments28/14/7 and service2/4/8 in14 ticks. Floor1024 arithmetic is a separately labeled policy. A weight change changes future increments while retaining accumulated pass. Normalize by the active minimum before addition would overflow; verify relative ordering and dormant-credit handling.

Linux EEVDF compares eligibility and virtual deadlines, not MLFQ levels or simple stride pass. The selected7.0.14 source additionally has protected slices, buddy choice, hierarchy and delayed dequeue. A task with negative lag cannot erase excess service through a brief sleep. The inspected6.6 comparison uses different tree augmentation and dequeue paths; changing release requires renewed source/configuration evidence. These teaching models are not Linux scheduler replicas.

## 05 Pages, mappings and measured costs

Divide each address by the declared page size before computing reference sets. At4096 bytes the six supplied addresses map to0,0,1,1,3,0. Equal total traffic can have one-page grouped windows or three-page interleaved windows. These sets do not specify RSS, faults or device bytes: mapping type, shared zero pages, copy-on-write and huge-page choices change the path.

`locality.c` uses fresh private anonymous mappings, actual page size, allocation/touch timestamps and observable per-page checksums. Sequential, coprime stride and seeded shuffled orders are repeated in randomized phase order. Read-first and write-first runs are distinct. Record global huge-page policy and process smaps context read-only; do not change host policy. The bounded competing reader operates on an original unlinked file and reports its own bytes. Compare sample distributions and ranges; do not attribute every timing difference to the memory access order. Warm file reads can return many logical bytes with zero major faults. Cold-cache control and host isolation remain unobserved.

## 06 Replacement state and objective

FIFO owns insertion order, LRU owns recency order, CLOCK owns a rotating hand and reference bits, and OPT examines next uses. Capacity zero refuses; an empty trace succeeds with no transitions. A hit preserves resident membership; a miss fills a vacant slot or replaces exactly one resident. Dirty state attaches to the resident and counts a writeback only on dirty eviction; no implicit final flush is included.

Inspect every supplied-reference row at capacities3 and4, including victim and live/dirty sets. FIFO's9/10 anomaly is not an LRU counterexample. The LRU stack orders all pages by last use; capacities select prefixes of that same order, so inclusion survives each move-to-front operation. Under equal miss costs, replacing a nearer-next-use victim can be exchanged for the farthest-next-use victim until their next divergence without adding a miss. This argument does not optimize unequal dirty-write costs; compare the independent dynamic-programming cost oracle.

## 07 Descriptor names are not object identity

Descriptor integers name table entries. Duplication, fork and SCM_RIGHTS can create new references to one open-file description, sharing offset/status flags. FD_CLOEXEC belongs to the descriptor. Separate opens have separate offsets. The fixture gives5/5/4; fork and descriptor transfer extend the actual ledger. A closed number can immediately name another object.

Use close-on-exec at creation, and confirm the executed child cannot see the endpoint. Release all acquired descriptors and reap the child on every injected prefix. Linux close errors normally invalidate the descriptor even when reporting an error; a blind retry can target its replacement. A blocked Linux I/O operation may retain the underlying description. The synthetic cleanup adapter separately represents a platform that retains ownership after EINTR. The inspected Linux manual's POSIX2008/2024 comparison is attributed to that manual; the exact published POSIX2024 text remains uninspected.

## 08 Visibility, budgets, readiness and completion

Keep four columns: namespace views govern names/visibility; cgroups govern selected resources; readiness hints that an operation may progress; completion resolves an operation identity and its lifetime contract. Poll does not reserve bytes. Another reader can consume the byte first, so this reader must retain EAGAIN as retry state. Bound partial-record storage, preserve accumulated bytes across interruption, and reject EOF in the middle of a record.

The real pipe and self-pipe probes are separate from synthetic completion delivery. Generation4 rejects event(3,12), accepts request7 once and rejects event(4,8). The retained memory-followthrough lifecycle also checks request identity, pending/active callbacks and synchronous completion before submit returns. Cancellation requests leave ownership live until terminal delivery; shutdown stops admission and waits for quiescence. A maximum generation retires the slot. Only an owned networkless guest creates private PID/mount namespaces; none of those views implies a resource budget.

## 09 Budgets need an external observer

Five aligned100000-unit periods with20000 service each have a100000 ceiling in the stated no-burst single-CPU model. A500000-unit window starting midway intersects six periods and can admit120000 under independent boundary budgets. Neither number is a latency promise or an owned core. A tighter parent changes the result even with identical child settings.

`guest.c` acknowledges placement before starting bounded workers. Its supervisor remains in the guest root; workloads alone enter new groups. Record limits, ancestry, membership, raw counters, status/signal and cleanup. Compare ungrouped/unlimited, CPU quota, memory limit, two weighted children under a tighter parent, and changed parent ancestry. CPU throttling is not slower instructions. RSS is a process view while memory.current includes charged group resources. A distinct worker-only32MiB RLIMIT_AS trial makes the64MiB allocation return ENOMEM and exit12, while the memory.max trial reports a killed process. These require different evidence. Keep the supervisor outside the budget it must diagnose and recover.

## 10 I/O needs several ledgers

Application accepted writes7168, reads2048 and acknowledged prefix6144 are different quantities. The last1024 bytes are uncertain after the model's crash point, not known lost. An overlapping rewrite adds application traffic without increasing final size and can invalidate earlier persistence assumptions for the rewritten region. Record-oriented completion, file size, cached traffic and device counters must not be substituted for one another.

The host adapter injects bounded short113-byte callbacks, EINTR and EAGAIN while checking complete contents. The owned guest mounts a fresh64MiB ext4 image and a separate2MiB tmpfs that actually reaches ENOSPC. Its I/O cgroup records exact device identity/limits/io.stat. Concurrent readers retain separate logical counters. Paired buffered/fdatasync runs preserve ordering and uncertainty; tmpfs is a changed backend requiring new interpretation. QEMU fsync acknowledgment is not a physical power-loss durability observation. Device/application discrepancies may involve cache reuse, metadata, writeback ownership and measurement boundaries; preserve alternatives not resolved by these counters.

## Evidence and assessment

Run the narrow author producer only with explicit reports under `reviews/v5/expansion/legacy_os_followthrough`. Keep every failed attempt. The guard binds source, tool commands, raw observations, original criterion identities and public exports; it also rejects stale or inflated receipts. Hosted Linux probes intentionally use libc/POSIX. The C17 algorithm core has a separately audited freestanding closure. External review, independent unseen repair after at least seven days, exact unavailable standards and physical observations retain separate gates.
