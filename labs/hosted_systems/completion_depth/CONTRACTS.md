# Bounded completion I/O contract

Required successor to `PR-T24-07`, criteria `AX-PR-T24-07-T24.12.a`, `.b`, `.c`
and their existing `-DELAYED` IDs. Historical elective labels/hours are unchanged.
Families B/C/F/G/H/I/J apply to construction, boundary diagnosis, falsification,
recovery, resource accounting, backend changes and component reuse.

Use hosted C17, GCC or Clang, Linux x86-64, an unprivileged process in the teaching
guest, installed `linux/io_uring.h`, libc/POSIX and the retained T62 `CMRawRing`
contract. The raw ring's `__atomic` publication operations are compiler extensions
over the Linux shared-ring ABI; they are not portable ISO C queue semantics.
No liburing library is permitted: implement the operation and ownership adapter.
Python models and report tools are host assessors, never target dependencies.
C23 does not provide these Linux operations or make their lifetimes statically
safe. A changed compiler/header/kernel requires renewed operation probing and
generated-object inspection, not just changing a language flag.

Prerequisites: byte spans; integer bounds; partial I/O and EOF; descriptor ownership;
T24 readiness, cancellation/quiescence and stale-event work; T62 raw-ring source
and learner artifact implementing `cm_ring_init/push/submit/wait/close`. Initially
test `completion.c` independently; then link your actual T62 artifact. Record its
hash. Author integration uses the separate assessor T62 implementation, never
claims it as a learner artifact. Read `COMPLETION.md` and the exact primary-source
locators in `research/v5/expansion/legacy_completion_io/resources.json` before native
operations. Missing operation support is exit77/unavailable, never fallback/pass.

## Pool and operation algebra

`CoPool` owns four caller-supplied live, disjoint byte arrays, each of the same
capacity1..4096. Their extent and nonoverlap are caller preconditions; equal base
pointers are additionally rejected. Initialization rejects a null pointer, invalid
capacity or generation limit outside1..0x7fffffff without modifying the pool.
Serialize all calls; this API supplies no mutex. Buffers and pool outlive every
submission and completion. `co_acquire(slot,1..capacity)` returns1 and a fresh
token,0 for unavailable/retired slots,-1 for malformed arguments. Outputs are
unchanged on failure. Token=(generation<<8)|(slot+1); high bit distinguishes the
cancel request. Retire a free slot at generation exhaustion, never wrap.

States: FREE→BUSY at acquisition; BUSY→DONE only on a target's terminal CQE;
DONE→FREE only at explicit release after any cancellation CQE has also arrived.
RETIRED has no outgoing transition. `co_cancel` records at most one pending
cancellation while BUSY and returns its separate token. Caller must reserve SQ
space before recording a cancellation; if a publication error leaves uncertainty,
retain the loan and drain/reconcile the ring rather than pretending nothing ran.
This adapter does not offer transactional rollback of an already published SQE.

`co_complete(token,res,flags)` returns1 for a current admissible event,0 for stale
or duplicate events,-1 for unsupported flags or an oversized nonnegative target
result. The ordinary contract accepts only flags0 and single-shot READ. The
changed contract uses READ_FIXED with the same terminal-event ownership rule.
Store negative target/cancel results separately. Cancellation acknowledgment,
including failure, never grants buffer reuse. Release returns1 exactly once when
both obligations are settled; otherwise0. Quiescence requires no BUSY/DONE slots
or pending cancellation. An unknown token cannot affect a current slot.

Representations are exposed for diagnosis; callers who mutate fields can violate
the contract. ISO C checks neither token freshness nor loan uniqueness. Runtime
checks reject specified mistakes; pointer validity, serialization and actual
kernel completion truth remain external obligations. This is not a language-native
borrow checker or structured-concurrency guarantee. Fixed storage gives O(1)
operations and O(4) quiescence scan; no heap service belongs in the pool core.

## Exact work and deliverables

| Criterion | Ordinary task and evidence | Changed-condition package |
|---|---|---|
| `.a` | Implement `completion.c`; implement `native.c` modes `cancel`, `boundary`, `failure` using actual T62. Capture READ/READ_FIXED/ASYNC_CANCEL probe support, separate CQE identities, blocked pipe cancellation, stale cancel against a newer request, delayed application dispatch, short read3, EOF0, EBADF, and five prepublication acquisition cuts. | Register four anonymous4096-byte buffers; read from a valid subrange+17, reject a range crossing the registered region, drain target and cancel completions, unregister synchronously before unmapping. Explain registration lifetime versus each operation's loan. |
| `.b` | Implement the specified file/socket workload for ordinary pread/write and sendfile; compare payload, complete-workload cost, allocated and borrowed bytes, immutable source lifetime, mapping/bookkeeping, retries and registered pin interval. | Implement `stall.c`: fill an owned nonblocking socket while its receiver is idle, observe EAGAIN, stop admission, half-close, drain/verify the exact queued prefix, then release source resources. Derive a storage bound and explain why measured cancel-to-drain latency is not a scheduler-independent upper bound. |
| `.c` | Implement `workload.c` and run the matrix below after sealing a decision. Retain every latency, whole/setup/workload/process-CPU time, cache snapshot, resource field, compiler command and failure trace. | Repeat the same contract with64-byte scattered requests instead of4096-byte sequential requests. Seal a fresh cost prediction before seeing the alternate backend, retain disagreement and revise the explanation. |

Supply contract/invariant derivation, actual code, object imports/size and stack
reports, exact primary-source identities, raw logs, independent model comparison,
one minimized semantic mutant, and an evidence card naming limitations. The
public files are intentionally unfinished learner scaffolds. Reference solutions,
timing results and delayed assessor packet are excluded from learner exports.

The workload has64 requests against a new immutable1MiB synthetic file. Byte at
offset x is `(17*x + floor(x/251)) mod251`. Large request i starts at4096*i;
small request i starts at64*((1619*i) mod16384). Each response is an exact byte
span delivered to its owned nonblocking AF_UNIX stream peer. Admit batches of
depth1 or4. Readiness uses synchronous `pread` on the regular file, then
nonblocking `write`/`poll`; ring/fixed batch file reads through T62 then use the
same socket continuation; sendfile uses the identical source spans and receiver.
File reads in the readiness baseline can block: do not call this an asynchronous
storage comparison or hide batch head-of-line delay. No TCP or storage durability
claim follows from this local experiment.

Each first batch's receiver is held until10ms after setup to expose retention;
record this deliberate pause separately from steady requests. Whole timing starts
before adapter/socket setup and ends after shutdown/unregistration/unmapping/close;
fixture creation and cache preparation are outside it. Use4 backends ×2 depths
×2 sizes ×2 cache states ×3 trials, on both optimized compilers. Rotate backend
order by trial; retain unaggregated records and distributions, including the first
batch. Report both full and post-first-batch samples rather than hiding the pause.
Cold means an owned-file `fsync`, DONTNEED request and zero resident pages in a
`mincore` admission snapshot; warm means read every page and verify all resident.
Neither proves physical-device cache state. A failed snapshot leaves that matrix
cell unavailable; do not silently rename it cold. Never write `/proc/.../drop_caches`.

## Pass/fail and scope

Run `python run.py --cc gcc` for the public core smoke check. Functional reference
acceptance additionally requires6 GCC/Clang debug/optimized/ASan+UBSan profiles,
the independent720-permutation/4320-transition event-set model, compiled token,
early-release and pending-ack mutants rejected for their semantic error, actual
native modes and compiled learner/provider stubs rejected. Timing acceptance has
no universal speed threshold: byte correctness, complete matrix, honest cache
labels and independent cost explanation are mandatory. A sanitizer report/crash
does not substitute for the intended semantic rejection. Finite permutations do
not prove all concurrent kernel histories; the pool assumes a serialized issuer.

Rubric0: unsafe lifetime, false backend/copy claim or missing artifact. Rubric1:
ordinary output only, unable to explain which CQE releases what. Rubric2: distinct
criterion evidence, invariants, resource bound, real operation support and a
discriminating counterexample. Rubric3: independent changed-condition solution
after the original minimum seven days, including revised assumptions and a
defensible new decision. Author tests do not award these learner levels.

Security boundary: original local processes/files/socketpairs and synthetic bytes
only; no external address, privilege, host-policy change or operational data.
Stale-token failures demonstrate a local ownership violation in the original
state model, not arbitrary code execution. Do not unmap live kernel buffers to
manufacture a crash. No physical observation or independent review is supplied.
