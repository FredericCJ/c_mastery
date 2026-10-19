# Checked ownership from boot data to durable services

These are bounded teaching contracts, not a general operating-system ABI.
`criteria.json` preserves every original task and changed condition verbatim.
For each task submit the implementation or explicit premise analysis, a causal
prediction, the specified counterexample, raw observations and a limit statement.
Use the mechanism's own negative control; a generic final success string is not
evidence for all criteria. Keep the original baseline sources and receipts intact.

## T48.02 — untrusted descriptions do not allocate memory

Treat the boot-information bytes as a borrowed span. Validate the supplied
extent, declared extent, each raw tag size, alignment rounding and mandatory
terminal tag before publishing any result. Unknown tags are skipped using their
validated extent. The mmap entry stride is data, not `sizeof` your preferred C
struct. All integer additions need an overflow argument before pointer formation.
Borrowed command-line bytes remain valid only while the boot span is retained.

Build a page-set oracle independently of the production traversal. A usable
interval must cover the whole page; any reserved overlap wins. Explicit kernel,
module and boot-info reservations are separate inputs. Permute map order and
move a module across page boundaries. Late malformed input must preserve the
caller output and allocator state. An optional validated tag does not make a
required tag optional. Use `tags`, `reservations` and `oracle.py` independently.

## T48.03 — the frame is an assembly/C contract

Draw each saved qword in order, including synthesized versus hardware error
codes. Derive every C offset from the actual push sequence, and derive return
cleanup from that same diagram. Check alignment at the C entry before its
prologue. Changing metadata requires changing the matching assembly boundary,
not merely adding a C member. Distinguish the user RSP stored in the frame from
the kernel RSP currently executing the handler.

Record vector, error, RIP,CS,pid and relevant fault address before optional
diagnostic text. Preserve the first record on nested faults. Compare separate
user-fault and ring0-fault guests: one process may be terminated while peers
continue, but a broken kernel cannot safely continue as if only a user failed.
For an unavailable ordinary stack, identify the owned IST range and show the
actual double-fault frame lies there. Serial output is bounded evidence, not
permission to recurse indefinitely in a damaged handler.

## T48.04 — address, mapping and execution ownership

Give every frame a state transition: reserved/free/allocated. Check the entire
request before mutating a mapping; track only newly acquired frames during
construction. On the kth acquisition failure, clear new PTEs and release those
frames without disturbing older mappings. Zero all newly exposed bytes. Test
every prefix, not just the first failure and the success case.

Separate death from reclamation. Before releasing user frames, prove no current
CR3/TSS/process owner still requires them. Static kernel stacks and page-table
arrays have different lifetimes from allocated user frames. A dynamically reused
task slot additionally needs execution-stack quiescence and a new generation.
At generation exhaustion retire the slot. The terminal no-task path still needs
a permanent address space and exception stack; it cannot reclaim the stack on
which it is presently executing.

## T48.05 — effective permissions and guard assumptions

For this four-level profile, combine P/U/RW permissions across every level and NX
across the path. State CR0.WP and EFER.NXE assumptions, and explicitly exclude
unimplemented protection mechanisms. Test a dedicated harmless sentinel page
with intentionally permissive and corrected mappings. Run two private values at
the same virtual address to distinguish virtual identity from physical sharing.
Record the actual error bits and CR3 evidence, not just that a process stopped.

A guard detects an access to its unmapped range. An instruction can adjust RSP
across it into another mapped range without accessing the guard. Compare that
counterexample with sequential page probes, and state the maximum unprobed step
your argument allows. Data-write permission does not imply instruction-fetch
permission; execute from NX data in its own controlled guest.

## T48.06 — policy is separate from context preservation

Test the round-robin selector as a pure function with no live tasks, one live
task and holes. Then demonstrate timer preemption using workers that never call
yield. Save all promised GPRs and preserve each worker's private memory. Missing
EOI and corrupted saved registers are different bugs and need different
observations. A weighted selector may change who runs next without changing the
frame format or the assembly save/restore contract.

Distinguish logical ticks, poll iterations, emulator TSC and host wall time.
The synchronous kernel interval has interrupts disabled; shortening the timer
quantum does not make that interval preemptible. Vary quantum and device delay
independently. State a bound in work units when justified, and keep physical
latency or worst-case scheduling claims unassessed.

## T48.07 — executable subsets and versioned archives

Validate all program headers before mapping any segment. Filesz,memsz,file
offset,address,entry point, page rounding and permissions are separate checks.
An entry must lie in admitted executable storage. Byte-disjoint segments may
still share a page with conflicting permissions. Loading bytes and zero-filling
the remainder must be checked across page boundaries.

CMK is a strict ELF subset. Explain why a general ELF file with TLS, interpreter,
dynamic metadata or other permitted alignment can still be outside this loader's
contract. Removing section headers should not remove usable program headers.
For archives, validate names, table extents, payload overlaps and output
preservation. A new read-only entry type needs a new declared format version;
it does not silently enable dynamic linking or change CMKA1's reserved fields.

## T48.08 — user bytes and process-local authority

Specify syscall registers, signed error representation and which registers are
preserved. Validate the entire copy range and every affected page before output,
allocation or device submission. Decide zero-length pointer validity explicitly.
Refusal must preserve the state promised by that syscall; it need not imply that
every unrelated counter is globally unchanged.

Handle lookup first validates representation, then slot and generation in the
calling process's table, then rights. A matching integer in another process is
not authority. Closing a token and destroying the underlying resource are
different lifetime events. Delegate only a subset of rights under an explicit
resource-lifetime premise. Test stale reuse, high bits, exhaustion and a valid
handle paired with an invalid destination. Retain device-submission and free-frame
observations to establish refusal before consumption.

## T48.09 — discovery and queue ownership

The fixed PCI profile validates a numeric configuration snapshot against trusted
BAR apertures before authorizing MMIO. Capability lengths do not establish BAR
sizes. The owned guest sizes/restores BARs before DMA begins. This lab refuses
duplicate selected capability kinds as a simplifying profile restriction; general
virtio permits multiple interfaces of a kind. Missing negotiated features and
invalid notification offsets must refuse before DRIVER_OK. Configuration64-bit
fields use aligned32-bit low/high accesses; capacity is fixed during this profile.

Track header, payload, status, descriptors and rings as one transferred lifetime.
Timeout retains that storage. Two requests require separate per-request storage;
local ticket generations cannot add a generation field to a wire used-ring ID.
Write an explicit countertrace for a late old completion after reuse. Publishing
descriptors, exposing index and notifying the device need ordered boundaries.
Inspect the compiler and CPU barriers in the actual linked driver. The coherent
identity-DMA premise must be replaced with cache maintenance and translation
authority on a noncoherent/IOMMU target; a TCG pass does not establish those.

## T48.10 — completion is not durability

Keep submitted,completed,error,unknown and flush-confirmed states distinct. A
device error may follow a persistent partial effect. Reset acknowledgement is
external evidence of quiescence before reclaiming quarantined buffers. It cannot
be manufactured by clearing a local busy flag.

Derive page-to-sector units and validate the narrow store slice independently of
raw disk capacity. A changed sector-size ABI is refused unless explicitly
implemented; virtio's sector field remains in512-byte units. Compare all bytes
after writing/flushing and starting a fresh guest on the same ordinary disk file.
Record cache options and clean emulator shutdown. A no-flush write may survive
that shutdown. Use the separate byte-prefix cut model to show why survival is
not a persistence guarantee, and do not call that model a physical power loss.

## T48.11 — stream state and teardown

The64-byte line includes its NUL. Overflow enters discard-until-newline so a tail
cannot become another command. Distinguish empty,maximum,overlong and partial
lines. CR/LF pairing spans input chunks. The shell protocol admits printable text,
CR/LF and an explicit Ctrl-D termination event; unavailable serial input is not EOF.

Drive the actual serial path with recorded chunks and output offsets. After exit,
check revocation, address-space switch and frame reclamation. The stats syscall
returns a bounded versioned struct of copied numbers, never kernel pointers.
Unknown version/size or invalid destination preserves the caller's sentinel.
An incomplete line terminated with the process must not execute as a complete
command, and must not bypass the ordinary cleanup path.

## T48.12 — same core, explicit budgets

Link your same hashed CVM core hosted and inside a user ELF; the supplied
`runtime/probe.c` is test infrastructure, not a replacement VM implementation.
`fixture.h` produces the exact container bytes from the public opcode enum.
Compare six semantic result/error/fuel/allowance/callback rows. Truncation of an
archive span must fail before running a VM instruction or invoking a callback.

Audit static storage, maps, helper imports and complete simultaneous native call
paths. The VM's explicit16 frames do not mean16 recursive C calls. Baseline
verifier scratch is automatic; the explicit changed profile borrows caller-owned
static scratch with a nonreentrancy rule. A callback cannot reacquire it. User,
kernel and exception stacks are different resources. Run a one-page stack only
after its compiler-specific native path bound fits, and retain the baseline
counterexample. Instruction fuel and host-call count bound different work;
an internal callback loop needs its own finite bound and timing premises.

## T48.13 — errors cross the entire service chain

Draw every owner from bytecode through the named service, opened store, page
adapter, syscall copy, bounce buffer and device. All target dependencies must be
explicit. One store owner serializes calls; read-only clients receive only the
declared service rights. A successful source-level signature is not native
memory isolation.

Observe the same callback error both before commit and after committed effects
with a lost reply. Reopen/query durable state before deciding how to retry. A
non-idempotent increment needs the effect and id/payload/result receipt in one
transaction. Specify the exact replay window, sequential-client assumption and
ID exhaustion behavior. A latest-receipt design does not promise unlimited
exactly-once history. The separate service cut campaign exercises74 modeled
write/flush failures; the actual two-boot guest demonstrates the real adapter
chain. Both retain their own observation scope.

## T48.14 — a dossier that can be disproved

Bind every observation to source,compiler,flags,image and raw-log identities.
Keep malformed ELF,stale handle,missing EOI,register damage and resource-capacity
changes separate. For each diagnosis state the first divergence and an alternate
experiment that would distinguish your explanation from another one. A timeout,
old log or missing marker alone is not a successful test. Preserve failed campaigns.

Revisit these assumptions in the later T56 process/thread system: process-owned
address spaces versus TCB-owned execution frames; per-CPU current/CR3/TSS stacks;
mutual exclusion and acquire/release order; reference counts and acknowledged TLB
invalidation; DMA lifetime under blocking and process exit; one durable store
owner. The separate CMK2 provider supplies actual1/2/4-vCPU transfer experiments.
That evidence does not make this one-CPU successor SMP-safe. Independent delayed
learner work, external review and physical observations remain separate gates.
