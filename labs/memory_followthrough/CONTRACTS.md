# T45 memory and lifetime followthrough

Implement `memory.c` from `memory.h`; the distributed bodies deliberately return
`MW_UNIMPLEMENTED`. Run `python3 labs/memory_followthrough/build.py --output /tmp/memory`
in a Linux export. An assessor may explicitly supply the two reference source
arguments. No reference source is exported. C17 is the baseline. `checked23.c`
is an optional, separately compiled installed-header comparison, scoped to the
actually inspected April 2023 N3096 draft clause 7.20; it is not evidence that
the published C23 standard was obtained. Host experiments may use POSIX and GNU
toolchain features; those are not dependencies of the portable core.

Freeze predictions before execution. `criteria.json` preserves every original
task, expected relation, and delayed task. The sections below supply mechanisms;
each exact criterion names its own evidence selectors. A single green native
test cannot discharge an unrelated detector, debugger, linker, or hardware claim.

## Observers and negative controls

Write a property/observer table before building `observer.c`. An instrumented
ASan access can expose physical bounds and ended allocations. It need not reject
index 6 of a logical four-byte object carved from a live eight-byte pool.
`pool` therefore needs the logical capacity check even when ASan is silent.
`dependency.c` is original local code: compile it once with ASan and once without,
then link both into otherwise identical instrumented executables. Record setup,
ENTRY, tool configuration, exit, and report. Missing ENTRY is inconclusive.
Instrumented detection and uninstrumented silence measure coverage, not repair.

Memcheck follows definedness through the missing mode assignment, record copy,
and branch in `init`. Compiling that dependency without source instrumentation
does not disable Memcheck's binary instrumentation. Missing debug information
reduces attribution. ASan silence does not establish initialization. The negative
C program has undefined behavior: never prescribe its uninitialized value,
crash address, or signal as a portable golden result. Rebuild O0 and O2 to record
symptom changes; only a restored field/range/lifetime invariant supports repair.
The campaign uses separate bounded children, with original input held constant.

## Extents, strings, and integer stages

`MwSpan` describes an offset into one actual backing array, a logical capacity,
and an initialized prefix. Validate these before forming an access. Reject a
negative signed index before conversion. One-past with count zero is permitted;
index beyond one-past is rejected even for zero count. Every rejection preserves
all backing bytes and published length. Scanning stops inside the initialized
prefix; missing initialization differs from a fully initialized missing NUL.
`mw_string` reserves one byte for NUL and refuses embedded NUL and overlapping
source/output. `mw_move` is the versioned binary operation: embedded zeros are
ordinary payload, overlap is supported, and reverse copy is required when the
destination starts later. Numeric offsets are comparable because they belong to
one array; this is not permission to order arbitrary allocation pointers.

For count*stride+header+terminator, check the multiplication, each addition, then
alignment padding before evaluation. Alignment must be a positive power of two.
The declared limit can be below SIZE_MAX. 9360*7+12=65532; 9361 fails addition;
9363 fails multiplication under limit65535. Add a terminator or round-up only
after its own check. Reject zero allocation. Save output until all stages pass.
Use a counted allocator that refuses every accepted request; rejected arithmetic
must make zero allocator calls. An initialization failure after allocation must
release the unpublished object. Mathematical negative conversion, unsigned wrap,
signed overflow, and narrowing are different analyses. The optional ckd probe
uses a temporary because an overflow indication also writes its result object.

## Stack evidence and caller work storage

Recollect compiler `.su` output for the selected optimization and frame-pointer
configuration. Preserve `static`, `dynamic,bounded`, and unbounded `dynamic`
qualifiers. A function frame report is not a whole-program bound. State recursive
depth, feasible callbacks, indirect targets, interrupt nesting, exception frames,
FP policy, and MSP/PSP selection. The fixture's foreground144 plus IRQ152 is296;
changing one exception frame32 to104 gives368. A feasible logging callback24
during fault handling consumes the original margin24. Unknown indirect/dynamic
paths leave the whole bound unknown.

`stack.S` measures an actual owned64-byte frame, changes32 bytes, restores8,
and records a64-byte reservation. The remaining24 changed bytes understate the
reservation; untouched or restored bytes cannot establish minimum SP.
`bounded.py` sets stack/core/CPU limits only in a new child and verifies readback
and ENTRY. A bounded native exhaustion signal is a local observation. There is
no signal handler on the exhausted stack and no host policy change.

`mw_tree` replaces recursive traversal with caller-owned work storage. For depth
d<=6, DFS needs d+1 pending entries and BFS needs 2^d. Both produce 2^d leaves.
Refusal preserves published result and peak; the supplied work array is scratch
and may change. Compare semantic results, capacity denial, and actual `.su`
profiles. A local buffer overrun is a different defect from aggregate stack
exhaustion. Instrumented stack size is not the production frame bound.

## Fixed layout, startup, and retained bytes

Use checked aligned half-open intervals. The 1024-byte fixture places data96,
bss400, retained64 through560. Heap160 leaves48 before a stack beginning768;
guard32 fits. Heap192 leaves16 and fails the reservation contract. Bss900 ends
fixed placement1064, overflowing40. Zero-size sections are legal intervals;
overflowing arithmetic publishes no report. A representable but too-large report
sets a false fits flag and never authorizes use.

`image.ld` separately constrains Flash load and RAM run placement. The campaign
actually links the good image and rejects oversized RAM and Flash images. The
QEMU image initializes data96/bss400 twice and retains64 sentinel bytes. That is
software warm-initialization behavior, not physical reset retention. The hosted
checked startup refuses a bss end13 that intersects retained byte12 before any
write. Select retained validity by reset cause plus schema/checksum; cold or
invalid retained contents require typed initialization. Neither a legal wrong
write into `cells[1]` nor a hosted virtual mapping makes a fixed section grow.
Hosted mmap/allocator placement cannot be assumed to collide with a contiguous
descending MCU stack.

## Ownership, leaks, resize, and initialization

`resources.c` imports the existing three-resource constructor. Its callback
returns zero on success. Inject failures before and after each acquisition and
after all three before publication. Track32/48/16 bytes, reverse releases, first
cause, and exactly-once cleanup over repeated cycles. A transferred child owns
its resource even when its former parent subsequently fails. Epoch retirement
needs an admission bound while old readers remain; no finite reclamation promise
follows from a reader that never leaves. The graph analysis distinguishes a lost
root with indirect child, an unreachable cycle, an interior root, strong roots,
and weak edges. Native conservative leak classifications are recorded separately.
A reachable24-byte cache violates budget16 even if a leak detector calls it
reachable. Live payload, retained storage, peak, and RSS are distinct measures.

Use a temporary result for nonzero realloc. Refusal retains the owner and both
borrow handles. On success use only the returned pointer; never compare or
evaluate the old pointer or its aliases, even when an integer model says the
address token is unchanged. Initialize the grown tail before publication and
advance generation. The allocate-copy-commit-free alternative copies while the
old object is live. A pinned borrower denies resize/drop; a long-lived caller
must release that pin or choose a separate copy. Maximal generations retire rather
than wrap. Null release, repeated release, interior/foreign release, and stale
generation have separate model outcomes; original native invalid-free controls
run only in isolated children.

Initialize records by typed staging. Track acquired storage, typed fields,
semantic validity, and publication separately; every failure prefix cleans up
without changing the output object. A tagged union requires its active member,
not every inactive field. Compare semantic fields and serialize explicit bytes;
padding is not an equality contract. Version1 gives the optional field its
declared default; version2 serializes it. Vary surrounding fill patterns without
pretending arbitrary byte zero is a universal pointer representation.

## Async lifetimes and diagnosis

The core imports the existing FAsync transition provider, adding two operation
slots, request identity, generations, submit-return and callback leases. Timeout
does not authorize reclaim. Cancellation intent is not device quiescence. Exactly
one terminal event queues a callback; reclaim waits for submission return and
callback return. This handles completion before submit returns. A callback can
start a successor sharing context; that successor keeps context live. Nested
callbacks require distinct scratch leases, with a finite bound of two. Duplicate,
stale, conflicting, and early-reclaim operations preserve ownership. Retire both
max-generation slots and report capacity exhaustion rather than reusing tickets.

Host transitions do not model independent DMA. Retain the F439 SRAM/CCM source
scope and requalify the exact M7 part, controller, bank, bus route, MPU attributes,
cache-line rounding, clean/invalidate direction, barriers, interrupt priority,
error handling, and confirmed stop. AN4839 Rev2 sections3.2/4 supplies the bounded
cache-coherency comparison; its example is not a universal H7 bus map. The cache
certificate is an explicit software model, and physical measurements remain open.

The diagnostic histories share a final symptom but differ in first invalid
transition. Minimize the first-bad prefix, then select a range, initialized-field,
generation, or reservation discriminator. Changing symptom delay leaves cause
unchanged. GDB observes live `cells[1]`, removes an old object watch before free,
then binds a new watch after relocation and generation change. Missing memory and
an invalid BFAR remain unknown. A canary can detect damage after the extent was
violated; it does not replace prevention. The assessor-only caller-timing case
tests shared-context reclamation. Its author execution does not satisfy the
independent unseen or minimum-seven-day delayed assessment.
