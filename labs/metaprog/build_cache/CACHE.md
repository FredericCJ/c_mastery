# Required T68.10 cache, build invalidation and publication investigation

Apply X-T68-010 R1-R4 and X-T68-010-DELAYED. Complete X-T68-009, retain your
PR-T20-01 codec.records implementation and PR-T32-02/U-T32-02-02 testing.faults
schedule contract. Supply the completed api_depth workspace explicitly. This
module calls actual ar_encode version2 and retained mp_build/mp_emit_table;
implement src/build_cache.c and src/cache_file.c from these permitted producers.
The testing.faults artifact specifies a schedule, not a C ABI: MpbBoundary is
the explicit adapter that supplies stage and accepted prefix after each effect.
Ninja, Python, sanitizers and compiler tools are host verification dependencies.

Use hosted ISO C17, exact uint64_t/int32_t, an octet unsigned char, Linux local
filesystems and POSIX threads. The allocation-free core has no hosted I/O, but
this packet does not claim a strict freestanding link or physical execution.
Input objects are live, aligned, initialized and immutable during each call;
controls and outputs are disjoint from inputs and each other. Public structures
are contract-governed representations, not immutable or unforgeable C types.

## Declared action inputs and canonical representation

There are0..8 nodes. Logical IDs must be exactly0..count-1, unique regardless
of storage order. Each dependency mask may refer only to those IDs; self and
mutual cycles are rejected. Each node has signed32 revision plus six independent
0..8-byte fields: source, tool, configuration, environment, include and schema.
These are exact declared toy inputs, not eight-byte truncations of real compiler
executables or a claim to discover all environment dependencies. Reject an
oversize field. Unmodeled compiler inputs remain the caller's invalidation gap.

For each logical ID encode id and dependency mask as one byte each, revision
through actual ar_encode(2) as four big-endian bytes plus the XOR check byte,
then each field as one length byte and its full bytes. The descriptor is at
most61 bytes. XOR detects only its specified simple corruption classes and
does not authenticate the revision. Pass these descriptors to actual mp_build
in logical-ID order with its tool/configuration words zero, since both are
already in the descriptor. Use the retained little-endian-word FNV recurrence
and lowest-ready-ID topological order. Cycles and invalid input leave the whole
output unchanged. The empty plan succeeds without dereferencing a null node
pointer. No pointer arithmetic on fabricated native objects is a test case.

Each action's complete equality vector consists of version1, its ID, the
transitive closure mask including itself, then every closure node in ascending
ID order as descriptor-length byte and full descriptor. At most499 bytes fit
the512-byte entry. This includes dependency topology and every declared field;
changing an independent node must not invalidate this action. Plan entries
are indexed by logical ID, while order[] contains execution order.

The lookup key is the retained64-bit hash. force_collision=1 substitutes zero
for every lookup key while keeping the actual generated artifact unchanged.
Generate each artifact with actual mp_emit_table and its real computed key.
The artifact includes the complete64-bit constant and deterministic C text.
mpb_match requires equal lookup key, full input length/bytes, and full generated
artifact length/bytes. All are required even when keys collide. It returns false
for null entries or invalid lengths. This bounded model recomputes expected
artifacts cheaply, so its cache saves publication and recompilation; it does
not claim to avoid the generator's work. FNV plus exact local equality is not
cryptographic authentication or authorization of a shared remote cache.

mpb_encode stages a canonical record: MPB, version1, node ID, key as8little-endian
bytes, input length as2big-endian bytes, input bytes, artifact length as2big-endian
bytes and artifact bytes without NUL. Maximum length is784 in an800-byte scratch
buffer. Entry input length is<=512, artifact length<256, ID0..7. INVALID/FULL
preserve every output byte and used count. Null bytes is permitted only for
capacity0, which returns FULL for a valid nonempty encoding. There is no unsafe
native-struct serialization or general untrusted C-source decoder.

## Dependency admission, ownership and finite epochs

Initialize MpbSchedule to zero in fresh storage. A plan passed to mpb_begin must
come from successful mpb_plan and remain live and unmodified until all workers
join; copying a raw public struct does not validate an arbitrary forged plan.
The scheduler core is externally serialized. begin rejects pending/running work
with BUSY, refuses epoch UINT64_MAX with EXHAUSTED, otherwise increments epoch
and makes every node pending. Restart requires all prior callers joined and
no live ticket borrow; an epoch change is not a substitute for object lifetime.

claim chooses the lowest pending ID whose dependencies are done, moves it to
running and returns owner pointer, epoch and ID. No ready node gives BUSY while
work remains, DONE when quiescent, or FAILED after a worker failure. Failed
claims preserve the output ticket. One ticket owns one private output. finish
requires exact owner/epoch and a currently running ID, otherwise STALE. A null
or mismatching produced entry marks failure and removes all unstarted pending
work. Already-running work still must finish and release its ticket. Successful
finish clears running and marks done. Duplicate finish cannot transfer twice.
publish first requires no pending/running work, then rejects any failure,
otherwise copies the plan. All refusal paths preserve its output.

The supplied two-worker pthread harness protects claim/finish with one mutex,
waits on a condition variable, computes into separate private entries outside
that mutex and joins both workers before publication/destruction. A first-work
barrier demonstrates two simultaneous owners. Node1 depends on0; node2 is
independent. A node2 failure cancels pending work and drains work already admitted.
Core policy, thread implementation and progress assumptions are distinct. The
test harness treats OS thread creation failure as a failed experiment, not a
demonstration of portable transactional thread startup. No asynchronous pthread
cancellation or lock-free scheduling contract is provided.

## File replacement and uncertainty

Use a private directory owned by this process, with no competing writer,
namespace changes, symlink manipulation or untrusted filesystem. Open its
directory descriptor and keep it live. Final names are node0.cache through
node7.cache; temporary names are node0.stage through node7.stage. ID0..7 and
length1..800 are admitted; input bytes and callback context remain live. All
operations on a given slot are serialized. A stage file from an interrupted
writer requires explicit mpb_file_recover after writer quiescence.

mpb_file_put opens the stage exclusively with mode0600, close-on-exec and
no-follow, writes in positive prefixes of at most7 bytes, synchronizes the
file, closes once, renames within the same directory, then synchronizes that
directory. It retries interrupted read/write calls; a failed close is not
retried against a potentially reused descriptor. The Linux local-filesystem
premise matters: the documented NFS rename ambiguity is outside this contract.

The optional nonreentrant callback runs after OPEN, every positive WRITE,
FILE_SYNC, CLOSE, RENAME and DIRECTORY_SYNC. It receives the total accepted
prefix, may return nonzero to inject refusal, or terminate the original child
teaching process. It must not retain buffers, close descriptors, modify the
namespace or mutate the current operation. Before rename, failure returns IO
and attempts stage cleanup; the prior final file remains authoritative. After
rename, failure returns UNCERTAIN: the new file may be visible and no rollback
is promised. An error after directory synchronization remains conservative
UNCERTAIN. This API never reports that a failed durable operation had no effect.

mpb_file_match compares the complete file against recomputed encoded expected
bytes, including exact EOF. Absent or unequal content is MISS; an I/O failure
is IO. A hash/header prefix is insufficient. It never executes cached text.
mpb_file_recover removes only the fixed stage name, tolerates absence and must
not race an active writer. Its successful unlink is process recovery, not a
claim that the removal is durable through an arbitrary later power cut.

For80-byte old/new synthetic entries,12 positive writes produce17 boundaries;
rename is boundary16. Kill the child at each boundary: final must be old before
16 and new after, while a stage may remain. Also inject callback refusal at each
boundary and check IO versus UNCERTAIN. Change a byte after the first8 bytes
to kill prefix-only verification. Compare against an original in-place truncate
defect. These are actual process-exit experiments on an owned local target,
not physical power cuts, kernel failures or universal filesystem proofs.

## Actual incremental-build investigation

Compile tests/generate.c with the retained producers and compile/run its emitted
C table through tests/consumer.c. The supplied experiment uses actual Ninja,
not a simulation: omit the schema implicit input and observe stale output;
repair it with an implicit dependency; replace it with order-only and observe
that ordering does not invalidate; repair again, then change a command argument
and observe command-tracked regeneration. Compare every observed table key with
the independent Python canonical-byte/hash model. Repeat generation byte-for-byte.
The generator executable itself must be a declared input in a real graph; this
fixed-executable experiment changes only the named dependencies and arguments.

## Deliverables, rubrics and changed conditions

- D1/R1: implement the complete declared-input model and distinguish readiness,
  invalidation, collision checking and trust. Demonstrate independent-node
  stability and changed transitive input propagation for all six field classes.
- D2/R2: derive canonical lengths, topological termination, exact equality and
  ticket invariants. Compare actual Ninja explicit/implicit/order-only inputs
  and tracked commands. State every omitted filesystem/compiler input assumption.
- D3/R3: execute retained buildgraph and six successor anchors; reject ten compiled
  semantic controls and six compiling stubs; match an independent oracle over
  all three-node dependency graphs/permutations plus seeded boundary cases.
  Compile and execute the generated C table, rather than inspecting its text alone.
- D4/R4: implement local cache publication/recovery, forced-key collisions and
  parallel ownership. Run all17 process-exit/refusal boundaries and both worker
  outcomes, then complete the changed-condition assessment and all13cost rows.

Run run_metaprog_build_cache.py, verify_metaprog_build_cache.py,
run_metaprog_cache_investigation.py and check_metaprog_build_cache.py --self-test.
Retain exact commands, sources, tool versions, compiler dependency lists,
generated artifacts, raw timing and first divergences. Six GCC/Clang
debug/release/ASan+UBSan profiles are author observations, not proof of arbitrary
interleavings. Compile each defect and require ordinary MPO_CHECK rejection,
not undefined behavior or sanitizer termination. Public tests alone earn at most
level1; each explicit criterion is needed for level2. Level3 requires an unaided
changed case, with assistance recorded, at least seven days later.

Hints: (1) normalize logical IDs before hashing; (2) treat a hash as lookup, not
equality; (3) pending cancellation cannot reclaim a worker's live output;
(4) a rename changes which failure promises are still possible. The private
solution and sealed case are under assessor/, excluded from learner exports.

## Cost and enforcement table

| Dimension | Required evidence |
| --- | --- |
| Representation | Actual entry/plan sizes; bounded full input vector, artifact and epoch. |
| Reachable storage | Eight entries, staged plan, descriptors and one800-byte encoded scratch. |
| Lifetime | Plan outlives tickets; directory and callback context outlive each call; joins precede reuse. |
| Mutation/aliasing | Disjoint controls and immutable inputs; externally serialized scheduler and per-slot namespace. |
| Allocation | Core allocates no heap; thread stacks/stdio and OS file objects are hosted costs. |
| Movement/copying | Full canonical vectors and plan copies; collision safety is paid for with exact comparisons. |
| Destruction | Running work drains before control destruction; descriptor close and explicit stage recovery. |
| Dispatch | Actual retained hash/codec/emitter calls; indirect injection callback only at named boundaries. |
| Concurrency | Two actual pthread workers, mutex admission, private outputs and joined publication. |
| Failure | Output transaction, cancelled pending work, post-rename uncertainty and cleanup limitations. |
| Compilation | Real generated consumer, hidden-input controls, command change, clean/unchanged build timings. |
| Binary/stack | Six-profile object/stack/size logs and separate file-adapter disassembly/undefined imports. |
| Boundary | Educational declared inputs; no authenticated shared cache, universal filesystem or physical timing claim. |

For N<=8, topology scanning and transitive closure are O(N squared); copying
each reachable bounded descriptor also costs O(N squared times descriptor
length). Matching is O(input+artifact), encode/file comparison O(record length),
and claims scan O(N) bitmasks. Native synchronization and storage latency are
additional costs. Seven10000-plan CPU-time samples use identical three-node
inputs and full validation, zero separate warmup and checksum210000. Record
clock resolution/noise; no universal speedup or constant-time security claim.

Validation statuses remain source-bound. Published C17/C23 and the attempted
POSIX2024 normative pages remain access gaps; the registered Linux/Ninja
passages support only their stated scope. Independent human review, unaided
learner assessment and physical durability evidence are separate obligations.
