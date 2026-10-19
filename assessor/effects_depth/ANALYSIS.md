# T13 technical analysis

## Effect closure and interception

The checksum and cache both return 294 for `abc`; only the latter retains a
32-byte allocation. Cold and post-reset calls request storage, while warm calls
reuse it. Moving acquisition to startup moves the effect in time and ownership;
it does not remove it from process initialization or failure analysis. For the
test allocator, requests and successful acquisitions are separate counters;
pool takes add requests without touching the general heap. A rare refill must
be counted even if all sampled steady-state calls hit.

GNU `--wrap` rewrites unresolved references during that link. The separately
linked original DSO contains its own malloc relocation; its positive allocation
therefore bypasses the executable's hook. The pool bypasses it for a different
reason: it implements allocation from owned storage. Empty hook logs cannot
establish that no storage was acquired. An unresolved dynamic callback extends
the call graph with an unknown effect set; a finite, audited callback set can
instead be enumerated. Neither inspection nor one process trace proves all
future dynamic targets satisfy the same contract.

The syscall log includes loader and runtime startup. Attribute the explicit
temporary-file writes/seeks and nanosleep using source order and trace contents,
and keep uncertain loader/libc attribution marked unknown. First touch of an
already mapped region can add faults and resident pages with no new allocation
syscall at that touch. User time, system time, elapsed time, faults and virtual
bytes are different dimensions. Process CPU sums threads and can exceed elapsed
time when multiple cores execute; this is a possibility, not a required observed
inequality on a quota-constrained host.

## Stack, locking and startup

The source-byte example 32+64=96 is a simultaneous array-storage subtotal, not an
ABI frame result. Compile-generated `.su` and disassembly account for alignment,
spills, red zones, inlining and tail calls. The VLA probe has bounded admission
but may retain a dynamic compiler qualifier. Recursion needs a depth bound;
unknown indirect callees prevent a closed transitive bound. The simple fixture
24+16+8=48 leaves 16 under a 64-byte budget; adding nested contexts 20+12+8 gives
88. Dedicated interrupt stacks require separate budgets and transition frames.
The guarded pthread pattern scan includes thread startup and is an observation
of writes, not proof of untouched worst-case stack or an interrupt measurement.

The staged mutex test ensures another thread owns the lock before a contender
attempts it. It observes waiting and an `EBUSY` trylock result without assuming a
scheduler latency. The three-vertex cycle is a graph witness of circular wait.
Calling a nonrecursive protected service again from its own callback is a
potential self-wait; separate front-end instances still share backend locks.
Source evidence for FILE, allocator, locale and once locks is required beyond
the wrapper's visible mutex. Unexamined internals remain unknown.

Run cold observations in new processes; resetting a component is narrower than
resetting a process or libc. The cache test checks both retry and latched failure,
and refuses reset while a borrower exists. The integer/floating configuration
study changes a real Picolibc build selection. A changed locale can change
formatting and runtime initialization effects; the two available C locales
tested here happen to agree on the admitted numeric output.

## Global state, progress and replacement

The intentionally flawed parser returns the same static buffer twice; reading
the still-live first pointer observes the second byte. This is a state isolation
failure with defined accesses, not evidence of arbitrary code execution. The
replacement's two instances preserve each other's view; same-instance reentry
refuses while a borrow exists. Caller discipline enforces unique view ownership;
ISO C does not prevent copying the struct. The seeded generator provides explicit
state, while locale and FILE position remain separately owned dependencies.

The signal handler performs only assignment to its `volatile sig_atomic_t` flag.
That specific handler does not establish safety for the logger, allocator or
stdio. Individually synchronized read/write operations still permit the
check-then-act lost-update trace. The atomic flag logger rejects busy ownership
instead of spinning in C, but its target instruction sequence and interrupt
priority relation determine whether a hard timing contract is justified.
Bounding retries does not bound an arbitrary callback; the nonblocking backend
precondition is essential and must not be inferred from a function name.

The fixed sorter accepts all lengths through 16 and rejects 17 before mutation.
Key comparisons avoid signed subtraction overflow. Stable records carry identity
and payload together; the third mutant reorders equal keys and must fail. An
equivalent qsort benchmark compares ascending keys and permutation only, since
qsort has no general stability promise. Reporting the replacement's stronger
stable contract explains any difference in work instead of hiding it.

## Tagged source and configuration boundaries

glibc 2.41's selected qsort source chooses temporary byte needs 256 for 64
four-byte items, 1200 for 300 such items, and 224 for ten 64-byte items on an
eight-byte-pointer model. The last becomes 144 with four-byte pointers. Its
1024-byte threshold selects the heap only for the middle case; allocation failure
selects a sorting fallback and preserves errno. The probe observes those exact
paths in a privately built 2.41 runtime. It does not assign that algorithm to ISO
C or another libc version.

musl 1.2.5's inspected implementation follows a different smoothsort structure
with automatic temporary storage and comparator/copy dependencies. Its static
hook probe is supplemented by that source inspection; the hook alone is not
proof of a transitive resource bound. Picolibc native configurations retain host
OS services; integer and floating format selection must be described as those
specific builds. Omitting a backend operation such as seek changes which
file-position operations can be implemented; printing one console byte does not
prove complete FILE support.

The 1.8.10 documentation's `posix-io=false` option is not accepted by the actual
tagged Meson build. Do not substitute the similarly named test option or claim
the requested configuration ran. The supported custom-console build uses
`posix-console=false`; its original FILE provides character callbacks and
returns the documented nonseekable failure for `fseek`. The POSIX-backed file
probe requires the seek backend and its negative link removes that capability.

## Assessor decisions

For each original leaf criterion, require the exact ordinary/transfer pair and
its evidence locator. A table of effects without a reachable call graph is
superficial. Executable correctness plus accurately bounded measurements meets
construction, not independent mastery. Independent assessment requires deriving
the altered closure, failure/ownership argument and changed measurements without
copying this analysis, after the original delay. External review and physical
interrupt claims remain separate dimensions.
