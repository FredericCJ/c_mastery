# P1: storage, effects, and resource accounting

This is additive support for **PR-P1-01 and PR-P1-02**, all nine retained
criteria and their changed-condition tasks. Read the parent assessment,
`labs/checkpoint_depth/CONTRACTS.md`, and `labs/checkpoint_depth/transfer/CONTRACTS.md`.
Their interfaces and prerequisite dossiers remain binding. No historical hour or
accepted receipt changes. Submit your own providers; assessor providers are
reference executions, not your work.

Use C17. The byte arena, typed pool and ring form the strict freestanding core.
The allocated-record, asynchronous-registration and ownership comparisons are
separate hosted programs. `struct ow_owner` reuse occurs only in the later revisit after
PR-T66-01, not as a forward prerequisite for the first P1 attempt. Host Python,
sanitizers, malloc-backed ledgers and clocks never become target dependencies.
N1570 is an inspected C11 draft; it does not establish access to published C17.
These exercises use no C23-only feature. A C23 compiler mode alone supplies no new
lifetime, ownership or effect guarantee.

## Storage derivation and construction

Keep three representations distinct. An arena is declared character storage and
returns a checked `(offset,length,generation)` byte view. A pool contains actual
`struct cpm_record` objects, with occupancy, generation and pin arrays. A ring contains
handles rather than pointers into either backend. Copy an integer's bytes into
the arena, then back into a declared integer before reading it. Increasing the
arena's alignment does not change its declared type. For the allocated variant,
write actual `struct cpx_record` and `struct cpx_node` objects into suitably aligned allocated
storage; release that storage only after every dependent access has ended.
See WG14 N1570 §6.5 paragraphs 6–7, including its distinction for objects with
no declared type, in the source register below.

The 32-byte arena has an encoded three-byte header, eight-byte payload alignment
and an honestly supplied base alignment. From used offset `u`, first check that
three bytes remain. Derive `s=u+3`, then padding `(8-s%8)%8`; check padding and
payload against the remaining capacity before writing any header or output.
Requests 5 and 9 produce starts 8 and 16, ending at 13 and 25. The next one-byte
request would start at 32 and is refused. A refused request preserves used=25,
all bytes and the output sentinel. Separate tests use payload24 in capacity32
(exact fit) and capacity31 (one short), plus `SIZE_MAX` and insufficient base
alignment. Do not form a pointer beyond the storage while checking bounds.

The typed pool has four slots. A successful allocation publishes one initialized
record and one handle. A queued handle owns a pin where the adapter contract
requires it; pop transfers responsibility to its consumer. Validate slot, live
state and generation **before** producing a pointer. Generations 1,2,3 deliberately
exhaust quickly: release at 3 retires the slot. Wrapping to 1 can resurrect an old
handle. Handles do not identify different owner objects; keep owner identity and
lifetime in the caller's contract. Relocation preserves handles, not raw borrows;
reject relocation while pinned, allocate/copy before releasing old backing, and
do not compare an expired pointer after success.

For FIFO capacity4, trace 11,22,33,44,refused55,pop,pop,66,77. The removed pair is
11,22 and the remaining sequence is 33,44,66,77. With two free slots a batch of
three commits two and reports a retry position of two. The all-or-nothing adapter
stages both pool and ring; on failure neither changes. It costs copies of both
bounded control objects. It cannot undo an arbitrary externally effectful
allocator by copying a struct. The linked variant instead acquires a record,
then a node/pin, publishing only when both exist. Node failure releases the
unpublished record but retains the earlier accepted prefix.

## Lifetime and failure investigation

Represent queue ownership and callback ownership as separate references. Removing
the queue item does not finish the callback. Cancellation requests shutdown;
only a matching quiescence acknowledgement releases the callback reference.
Reject stale cookies and duplicate completion before changing the ledger. Arena
reset is refused until both references end, then advances a nonwrapping generation.
An alternative copying design is valid only with an explicit new owner and costs.

Hosted initialization acquires record backing, node backing, then registration.
Inject failure at ordinals1,2,3 and require zero retained allocations on returned
construction failure. Growth acquires replacement records and nodes before its
commit; fail each allocation, preserving old owner fields and record bytes.
For each of four node-admission cuts, compare accepted records, live slots, pins,
queue length and FIFO contents. Repeat the original four-record workload at
capacity2. The valid result is changed acceptance, not a smaller workload.

Keep an independent live-allocation ledger recording base, extent, required
alignment, owner and release. Check nonoverlap only for live allocations under
the chosen native flat-address model; ISO C does not supply arbitrary unrelated
pointer ordering. Reconcile bytes and counts after each operation. A zero count
alone misses an incorrect copied value or a pointer published too early.

Close changes OPEN to CLOSING, calls unregister once, and retains backing until
quiescent completion. A second close remains pending without repeating the call.
A wrong cookie or nonquiescent acknowledgement cannot free memory. A successful
acknowledgement releases two backings once. In the later OwOwner comparison, its
three resources also remain live across pending unregister; measure the adapter's
extra control bytes and calls. C checks none of move-only ownership, automatically
invoked destruction, or callback effect promises. Explain the runtime checks,
external tests and programmer obligations separately.

## Effects and generated-code investigation

Produce a call/effect ledger for the core, allocator, comparator, logger, callback
registration and test harness. The core's bounded operations do not allocate or
wait. Hosted allocation and clocks are not pure. A comparator signature does not
exclude malloc; a logger can wait even if it returns no value. Supply clean
callbacks and original locally bounded violating callbacks. Observe exactly four
comparisons and log calls for four records, including a comparator that allocates
and releases four one-byte blocks and a logger that calls a one-millisecond empty
poll four times. This is a wait-capable control, not a latency upper bound.
Runtime counters do not prove arbitrary callbacks pure; inspect their bodies and
transitive providers. No external targets or data are involved.

Build the actual core with GCC and Clang for Cortex-M4 at O0 and O3, using
`target_build.py` with explicit core, header, original runtime and target driver
paths. The script links five objects directly; inspect every object's undefined
symbols and the final defined symbols, map and disassembly. The emulator driver
uses semihosting for bounded test completion; it is not firmware performance
measurement. Original memory helpers and their ABI wrappers must be identified.
No libc, libgcc, compiler-rt, libatomic or HAL archive is allowed. GCC's link
documentation explains why suppressing libraries does not suppress helper calls;
Arm RTABI32 describes the helper calling contracts. See the inspected register.

Separately enable native LTO and stack protection on inspection images. Resolve a
teaching allocator locally as a negative control: an empty undefined-symbol list
does not establish the no-allocation contract. Also retain an unresolved blocking
provider and the emitted stack-check dependency. These inspection images lack
startup/guard initialization and must not be executed. Explain what must be
initialized before a protected executable could run.

## Paired workload and measurement protocol

Use four offered records in each of three distributions: lengths `[1,1,1,1]`,
`[16,16,16,16]`, and `[1,4,8,16]`. Payload byte j of record i is i+1+j. Compare
an arena with capacity64/header3/alignment1 with a pool of four 16-byte-payload
records. The pool additionally reserves typed record metadata and node backing;
report these actual sizes and each control object separately. This is a fixed
backend-capacity comparison, not a claim of equal gross memory.

Predict accepted counts, checksums, headers, padding, live payload, backing and
retention before execution. In the all16 distribution the arena accepts three
and the pool four. For the mixed distribution both accept four with payload29
and checksum251. Payload bytes logically released from the arena remain reserved
until reset; pool slot slack differs from metadata and backing retained for reuse.

Run instrumentation on/off for both backends. For each configuration discard
three warmups and retain31 samples, each summing128 individually timed workloads.
Construct inputs and allocate/initialize backing outside each interval. Time
admission, copy/read and logical release; record clock calls/assertions as included
overhead. Reset/destruction are outside and must be reported separately. Publish
all raw samples, compiler/CPU/OS/flags, executable hashes, minimum, median, p95
(nearest-rank30/31) and observed maximum. The all16 timings have different accepted
work and must not be ranked as equivalent performance. No observed maximum is WCET.

## Submission, hints and assessment

Deliver providers, an effect/ownership diagram, sealed predictions, a source-bound
build manifest, independent ledger/oracle, minimized failing cases, raw measurements
and a criterion-indexed explanation. Use `run.py --source YOUR_CORE --include
YOUR_HEADERS --build BUILD` for the public core check. Then run the retained
ordinary and transfer tests with your explicit implementations. A test harness is
supplied work; completing it does not replace implementing the provider.

Hints are graduated: (1) write the state immediately before each publication;
(2) separate byte extent, declared type, pin and generation checks; (3) stage only
state you own, and retain callback context until acknowledgement, not cancellation.
For a timing mismatch first compare accepted work and retained bytes, then generated
code. Assessor solutions and transfer procedures are packaged separately.

Per retained criterion: level0 is an incorrect contract or missing evidence;
level1 is a happy-path demo with unexplained failure; level2 requires its specific
ordinary mechanism, independently predicted boundaries and intended semantic
rejection; level3 additionally requires an actual unaided changed-condition attempt
after at least seven days with assistance provenance. Author reference success
does not award that level. Finite checks do not establish absolute mastery.

Source register: `research/v5/expansion/legacy_checkpoint_memory/resources.json`.
Current validation: `reviews/v5/expansion/legacy_checkpoint_memory/`.
