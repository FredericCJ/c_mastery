# Real service adapters: explicit successor contracts

These required adapters extend X-T56-002 through X-T56-005. The portable
`struct sd_memory`, `struct sd_channel`, `struct sd_files` and `struct sd_journal` exercises retain their original
contracts. This layer implements actual checked images, x86-64 mappings, protected
kernel tables, and transactions in the retained page store. Read the parent
exercise and its D1–D4 rubric first. The author adapter imports **reference**
components from `assessor/kernel_smp` and `assessor/storage`; these executions
are not evidence that any learner's prerequisite components were integrated.

## Environment and ownership boundary

The kernel target is the existing `pc-i440fx-10.2` QEMU TCG profile, x86-64,
128MiB RAM, modern virtio block, no network, and a fresh ordinary 4MiB image
in a native Linux temporary directory recorded in the receipt. C17 plus the retained GCC/Clang x86 assembly and ABI
extensions are required. Link directly with `ld -nostdlib`; audit the complete
kernel and every user ELF for unresolved helpers and dynamic dependencies.
No libc, allocator library, libatomic or implicit runtime is permitted in these
images. Host Python, GRUB and QEMU are verification/build dependencies.

The retained global kernel lock serializes metadata and mapping transitions.
User threads still run concurrently. This is not a scalable fine-grained kernel.
The adapter stages explicitly identified reference sources and replaces only the
loader and named integration hooks; it leaves retained source files and historical
receipts unchanged. A changed import anchor is a build failure, not permission
to silently use the old implementation. Record compiler, flags, all import
hashes, staged source, maps, objects, image hashes and raw guest output.

## Transactional loader and restricted COW

Import the retained `cmk_archive_find`, `cmk_elf_parse`, frame allocator,
`materialize`, `fork_single`, shootdown and reaping mechanisms. Their static
page-table/control slots are not dynamically allocated page-table frames.
For a checked ELF, acquire each actual data/stack frame into a bounded transaction
list. Zero the frame, establish its reference, populate the unpublished PTE and
copy the admitted file extent. Publish a runnable thread only after all frames
are acquired. Any allocation failure releases this invocation's frames in reverse
order, clears them and restores the previously unowned control descriptor.
It must neither panic nor alter another image's ownership. The number of
allocations is the sum of page-rounded admitted segment extents plus32 stack
pages. Exercise every failure index, not only the first allocation.

`SDK_CLONE_PRIVATE` is an additive restricted operation. Before changing any
reference/PTE, reject shared mappings, absent/saturated references, and a caller
with more than one live thread. The retained CMK2 clone continues to support its
different shared-map policy. A successful private clone uses the retained COW
publication and acknowledged remote invalidation before returning. The page-fault
path rechecks the current PTE under the kernel lock: a second CPU can arrive with
a fault recorded before another CPU resolved it. One child is held alive while
two parent threads fault/write the same page, preserving the shared backing
premise. Waiting for child completion includes the retained remote-address-space
quiescence before its frames can be reaped.

Derive the frame/reference conservation equations and distinguish rejection
preflight from the no-failure publication phase. Report O(mapped pages) clone
and rollback work, O(page size) copied data, static transaction/control storage,
kernel stack use, and shootdown synchronization cost. No timing bound follows
from a finite QEMU run.

For X-T56-002 V1, `SDK_SHARE_IMMUTABLE` converts one page-aligned, present,
non-executable, non-COW mapping into shared read-only storage and acknowledges
remote invalidation. `SDK_CLONE_IMMUTABLE` permits this kind while retaining the
private clone's admission checks. Both processes retain the same frame; neither
can write it. Process teardown releases each reference, with actual reuse only
after remote quiescence. Ordinary private COW is an incorrect substitute: it
would allow a child's write by making a new page. V2 uses the held-live child
and two affined faulting threads described above; serialized metadata does not
mean those threads cannot arrive with the same stale fault premise.

## Protected directional byte channels

Eight two-message queues hold messages of0..32 bytes. A separate32-entry kernel
capability table records process ownership, nonwrapping generation, queue
incarnation and rights SEND/RECEIVE/CLOSE. Numeric userspace handles are lookup
keys, not authoritative rights. Attenuation creates another capability with a
nonempty subset; it cannot amplify rights. This profile grants within one
process. Cross-process delegation needs a separate authenticated recipient
contract; merely copying a number does not grant it.

Send copies the admitted bytes before publication. If the queue is full and
blocking was requested, the pending sender owns a kernel copy, not a user
pointer. A successful wake publishes that copy once; cancellation removes it
before waking with `SDK_CANCELLED`. A cancellation that loses to completion
returns an error rather than falsely promising rollback. The caller controls
the input lifetime during the initial copy; concurrent mutation of its source
has no atomic-snapshot guarantee.

Receive preflights/materializes the entire destination while holding the kernel
lock, copies, then dequeues exactly once. A rejected destination leaves the
message and user bytes unchanged. There is no unmap syscall in this profile;
mapping transitions serialize under the same lock. This makes the admitted
extent stable for the copy, not race-free against another user thread writing
the same bytes. The caller must own its destination for the call. A blocked
receiver stores no pointer: wake returns the distinct `SDK_RETRY=256`, and the
caller resubmits and revalidates. This value cannot be confused with a0..32-byte
successful receive. Close wakes waiters and permits draining admitted messages.
Process exit closes owned queues, removes pending calls and invalidates its
capabilities. Report fixed table limits and O(thread limit) wake/cleanup scans.

For X-T56-003 V1, `SDK_RECEIVE_PARTIAL` returns the copied prefix length plus
`SDK_PARTIAL_END` only when that message ends. The head stores a consumed-byte
offset and reserves the remainder to the first receiving thread. A rejected
copy changes neither offset nor destination. A zero-capacity call cannot advance
a nonempty message; an empty message returns the end marker with zero bytes.
The same thread retries the exact suffix. Another reader is denied until the
owner ends or exits; exit releases the reservation, preserving the remainder.
For V2, a closed/drained queue may be destroyed and reused only after waiter
removal. Enrollment carries the old queue incarnation. The diagnostic delayed
wake operation rejects a reused slot's old incarnation. It is a teaching test
selector, not a production authority-grant API. Reusing a slot index alone is
insufficient even if the original sender is already dead.

## Archive-backed file descriptions

Import the actual retained checked archive parser; a fresh arithmetic name
fixture is insufficient. The immutable boot archive remains reserved for the
guest lifetime. Eight open descriptions contain a borrowed backing span and
offset;16 protected generation handles own references to descriptions. `open`
creates an independent offset, `dup` shares it, and the last close releases only
the description, never the permanent archive storage. Process exit closes all
its handles. No user pointer to a backing object is returned.

Names are exact nonempty1..23-byte names accepted by the retained archive parser.
Read requests are capped at256 bytes. Preflight the entire actual output extent
before changing the offset. Zero-byte reads need no nonnull destination and
perform no pointer arithmetic on null. Seek uses checked signed addition from
beginning/current/end and admits only [0,length]. Invalid, foreign or stale
handles fail without changing another description. The parser's fixed archive
profile excludes empty archive entries; that differs from the portable SDFiles
profile and must be recorded as an adapter constraint.

For X-T56-004 V1, a cloned process receives ownership of the same numbered
handles and descriptions, with additional references. Offsets are shared;
closing a handle removes only that process's ownership bit. A slot is reusable
only after its last process owner closes it. The inherited-bitset model is
bounded by the retained process limit; it is not a general POSIX descriptor
table or a new permission-grant operation.

For V2, one trusted process incarnation owns a mutable32-byte file. Two backing
versions permit publication while an old snapshot is pinned. `open_snapshot`
retains the current version; `open_live` retains only the logical description
and consults the current root during each serialized read. Mutate allocates a
free version before publication and fails unchanged if both are pinned. Truncate
preserves existing offsets: a live reader beyond the new end sees EOF until it
seeks to an admitted position. A zero-length version is valid. The root has
kernel lifetime, and the sole writer's process generation prevents accidental
authority transfer on PID reuse. No implicit ownership migration is provided.

## Durable effect and receipt transaction

Implement `receipt.h` on the actual `store` API. Provision only a fresh formatted
store under its sole trusted owner; initialization is not a remote reset command.
Two client IDs1/2 have fixed epoch1. Each client has a durable high-water request
ID, a three-entry circular receipt window, and a cursor. New IDs are positive,
strictly increasing for that client but need not be consecutive. Each receipt
stores the ID, complete signed32 delta and resulting signed32 effect. The
metadata, overwritten receipt and effect are written in **one store transaction**.
Use explicit little-endian bytes; do not write a padded C structure to storage.

A retained identical retry returns its saved result. A retained ID with a
different delta is a conflict. An evicted or otherwise older ID is stale and
must not reapply its effect. A different epoch is stale; this bounded profile
requires an explicit externally authorized epoch migration, which is not
implemented by wrapping an integer. Overflow is rejected before beginning the
transaction. An output value is published only after a successful commit.
An indeterminate commit leaves the output unchanged and requires reopening the
store before retry. The owner serializes all access; it does not promise
distributed consensus, unbounded receipt retention or exactly-once delivery.

The invariant is atomic recovery of `(effect, receipt, high-water/cursor)`.
Neither a successful send nor an in-memory deduplication table establishes it.
On retry, inspect the durable identity before computing a new effect. Test
the hand-derived sequence: client1 ID5 +7→7; client2 ID9 +3→10;
client1 ID10 −2→8, ID20 +1→9, ID30 +1→10; old ID5 is then stale;
client2 ID9 returns its original10. A conflict, epoch mismatch and overflow
must preserve a sentinel output and the final10.

## Experiments, evidence and limits

The host experiment reuses the retained five-mode storage fault model: discard,
reverse-order partial dirty pages, addressed-page prefix/suffix half tears, or
all pending writes persisted despite an error. It enumerates all37 transaction
write/flush boundaries for the three-key receipt operation. This is a declared
finite model, not arbitrary-device proof. A torn or maliciously recomputed
checksum is not authenticated storage.

The separate sector model enumerates all256 old/new512-byte masks for each
addressed write, and every reverse-order dirty-page prefix plus one such torn
boundary page at each flush. Its37 event sites produce18,176 cases; masks for
all dirty pages are not jointly exhaustive. State the protocol argument as
well: acknowledged flushes separate prepare data from commit publication, so
unacknowledged write order cannot be treated as a completed transaction. CRC
validation is a bounded corruption detector, not an impossibility proof against
arbitrary corruption or maliciously recomputed valid metadata. X-T56-005 V2
requires deriving this assumption boundary and a counterexample when it is
removed, in addition to implementing and running the stronger sector model.

The guest experiment uses actual virtio requests and acknowledged flushes into
its synthetic raw file. It stops the **whole VM** immediately before and after
each operation, and after durable commit but before reply. A new VM recovers
the same file and retries the same request. A separate no-cut run recreates
only the owner store object while the VM/device stays alive. Distinguish these
two experiments. QEMU exit and host file caching are not a physical power cut;
before-flush writes may already be visible. Therefore recovery may expose
either the old0 or new7, but retry must produce7, never14, and effect/receipt
must agree. The explicit model supplies additional reorder/tear counterexamples.

The retained guest poisons the device after a100-tick completion deadline.
Initial campaigns using Windows/DrvFS image files observed this timeout during
flush and failed; those receipts remain failed. A subsequent native Linux image
campaign retained the same deadline and passed all75 interruption boundaries
and the no-cut run, each followed by recovery in a new VM. This environment
change supplies reproducible bounded execution, not a guarantee that all host
filesystems finish within that deadline. Never count a timeout as a crash-cut
success or silently extend the guest deadline to make a run pass.

For each test retain positive evidence and a compiled, executed incorrect
implementation rejected for the intended property. A compile failure is not
a semantic mutant rejection. A marker without the matching successful exit,
source/image identity and complete required case set is insufficient.

## Assessment progression and local security scope

First derive ownership and publication diagrams; then implement the portable
core; then integrate the learner's own named prerequisites using their actual
headers, source fingerprints and target build. Compare those imports to the
author-reference campaign without treating reference artifacts as learner work.
Submit an independent expectation before inspecting withheld controls. Diagnose
an early offset update, missing rollback, rights-check omission, uncleared
cancelled waiter, payload-unbound receipt or effect/receipt split transaction.
Explain why suppressing the diagnostic or retrying blindly does not repair it.

Only the original guest, synthetic archive/messages and disposable disk are
targets. The adversary controls admitted handles, byte spans, request IDs and
declared interruption schedules. Harmless success conditions are unauthorized
byte receipt, a changed offset after rejection, leaked frames, resurrection of
a cancelled send, or a doubled synthetic integer effect. No arbitrary code
execution, host privilege, external service, credentials or physical device
claim is made. Compare repaired and faulty behavior under the same conditions.

At level1, a correct trace without its invariants is superficial. Level2 needs
the complete named contract, original implementation, generated-image audit,
all applicable boundary checks and explained failures. Level3 additionally
requires independently adapting a changed assumption and defending the revised
proof/cost model after the prescribed delay. Author executions can establish
reference readiness, never level3 learner performance. Keep external review,
independent/delayed attempts and physical evidence pending until actually obtained.

## Building the actual learner adapter

`load.inc`, `services.inc` and `receipt.c` are deliberately incomplete learner
files. `guest.py` has no assessor discovery or fallback. Supply your actual
prerequisite source directories explicitly, using the retained public ABI/layout
contracts, and retain their fingerprints. Example from a source checkout:

```sh
python3 labs/service_depth/adapters/guest.py \
  --kernel-source /path/to/your/kernel \
  --kernel-lab labs/kernel_smp \
  --adapter-source labs/service_depth/adapters \
  --store-source /path/to/your/store.c --storage-lab labs/storage \
  --build-root build/my-service-adapter --suite services --cpus 2 \
  --report build/my-service-adapter/services.json
```

Repeat with `--suite receipts` to enumerate all crash/restart cuts. A selected
`--cut N` is a diagnostic subset, never the complete campaign. Run both compilers,
O0/O2 and two/four CPUs for service mechanisms. The builder stages explicitly
checked integration hooks, the modulo-32-bit Multiboot checksum representation
and ISO atomic tick increments; it does not edit imported sources. Divergent
import anchors fail explicitly and require a reviewed adapter change. Public
ring3 probes are tests, not implementations of the kernel/store mechanisms.
The separate author wrapper `tools/run_service_kernel.py` deliberately selects
assessor references and labels that provenance in every receipt.
