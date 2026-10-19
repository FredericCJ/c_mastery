# Resumable I/O, positioned records and close ownership

## Stable objectives and families

This successor serves original `AX-PR-T23-02-T23.04.a`, `.04.b`, `.07.a`,
`.07.b` and `.07.c` through construction, diagnosis, failure injection,
verification and changed-condition transfer (B/C/D/F/G/I/J). It does not replace
the original `.04.c` request/deduplication exercise or imply that successful byte
transfer is durable effect publication. Original IDs, units, hours and receipts
remain unchanged; new evidence is linked separately.

## Sources and learning objectives

Read the registered bounded Linux `read(2)`, `write(2)`, `pread(2)`, `close(2)`,
`connect(2)` and `fcntl_locking(2)` passages. Explain partial progress versus
interrupted-before-progress, readiness versus completion, position versus shared
offset, and ownership versus success. C17 defines the local arithmetic and object
rules; POSIX/Linux defines these calls. Linux `pwrite` with `O_APPEND` appends
despite its explicit offset, so this interface rejects that flag. Linux manual
`close(2)` describes the difference between POSIX.1-2008's unspecified interrupted
descriptor state, POSIX.1-2024's retained descriptor and Linux's continuing
release behavior. The direct POSIX2024 page was inaccessible during this source
inspection; the edition comparison is explicitly attributed to the Linux manual,
not falsely recorded as inspection of the standard text.

## Prerequisites and artifacts

Bring `PR-T23-02` learner descriptor/offset work, `PR-T23-01` exact-child
wait/retry ownership, and the original per-leaf prediction sheets. Understand
unsigned subtraction guards, signed representability, object lifetimes, borrowed
descriptors and a resumable state machine. Keep the deliberately flawed
shared-offset and split-record examples intact. Replace only `io/io_stub.c`.

## Environment and dependency profile

Hosted C17, Linux local temporary files, libc/POSIX calls, process helpers and
record locks. GCC/Clang debug, `-O3`, AddressSanitizer/UndefinedBehaviorSanitizer
are the core profiles. Linux ABI assumptions: signed `off_t`, no padding bits,
width no greater than `uintmax_t`. Check the actual sizes and maximum before
conversion. The optional actual32-bit probe is a distinct toolchain experiment;
absence of headers/libraries is an unavailable result, not a pass. A five-GiB
logical sparse file contains about one MiB of explicit data, never five GiB of
allocated payload. No devices, networks, credentials or host settings change.

## Problem and supplied inputs

Implement `io.h`. For `PCCopy`, a read fills its64-byte owned buffer; a write
consumes only the pending suffix. Execute the exact script
`read3, write1, EINTR, write2, read4, write2, EAGAIN, write2, EOF` on `abcdefg`.
Return a readiness state after `EAGAIN`, preserving `fg`, then resume without
re-reading or duplicating bytes. Change the sink to fail after five committed
bytes. Run the same cursor against real nonblocking pipes: initially empty
source, saturated sink, later drained sink and closed producer.

For offsets, convert only after checked representability and selected-file
extent tests. Exercise zero, maximum, maximum+length overflow and beyond-end
extent. Create data at0/1MiB, inspect a hole, then fill it with explicit zeros;
interleave `pread` and sequential reads through `dup`. Probe a single byte beyond
5GiB on the native large-file ABI. Have two owned workers write64 disjoint
eight-byte records. Then demonstrate a forced `lseek`/`write` interference and
a two-part overlapping record, and implement the stated cooperative lock policy.

## Why these mechanisms need separate assessment

Returning the correct result after one full read/write does not establish
resumption. The source has already advanced while unwritten bytes remain owned
by the cursor. `EAGAIN` is not EOF and zero write is not useful progress. Explicit
offsets avoid a shared file-position interaction; they do not serialize a record
made from multiple calls. Identical zero bytes need not have identical allocation.
Blindly retrying Linux `close` can close a different object after descriptor reuse.

## Representation, invariants and failure semantics

`PCCopy` owns its inline buffer; `0 <= next <= end <=64`. The half-open range
`[next,end)` is exactly the read prefix not yet committed to the sink. `committed`
counts only successful writes; it is bounded by `UINTMAX_MAX`. `pc_resume` borrows
its callbacks/context for the call, permits no reentrancy or concurrent access,
and uses at most `call_budget` callback invocations. Budget exhaustion yields,
including repeated `EINTR`. EAGAIN/EWOULDBLOCK returns the appropriate wait state.
EOF becomes done only when no bytes remain. Zero write is sticky `EIO`; an
adapter returning more than requested is sticky `EPROTO`. Hard errors retain the
pending suffix and prohibit further callbacks until the owner starts a new
copy. A new destination may deliberately recover that suffix, but the old
operation is not rolled back. Every callback obeys the stated buffer bound,
reports negative results with an error, and never reports ambiguous positive
progress. Arbitrary device errors with uncertain effects need another contract.

`pc_extent` uses unsigned subtraction checks before evaluating a signed sum or
cast. Both start and exclusive end must fit the chosen `off_t`; the selected
file size is the caller's stable extent contract. Even zero length cannot be
beyond that extent. Errors leave the converted output unchanged. This is a
snapshot, not protection against later truncation by another actor.

`pc_write_record` borrows a seekable, non-append descriptor and live disjoint
input. It returns positive errno and an exact committed prefix for execution
failure; validation failures leave the progress output unchanged. It retries
EINTR and accounts for short `pwrite` results, using at most4096-byte requests.
It has no hard real-time guarantee under endless interruptions. The caller owns
the synchronization/timeout policy. No automatic rollback or durable commit is
claimed. The changed overlapping-record policy requires every cooperating reader
and writer to hold a process record lock around the whole record; no other close
on that inode may discard the process's locks while the operation is active.

`pc_close_transition` is a pure ownership model: with Linux-release policy the
owned bit is cleared even on injectedEINTR; with retained-on-EINTR policy it is
kept only for that error. Errors remain observable rather than converted to
success. The experiment performs actual release and deliberate number reuse for
the first model, and injects an error before release for the second. It does not
claim to have caused the running Linux kernel to implement another OS's close.
Never generalize the copier's EINTR retry rule to connect/close. For nonblocking
connect, readiness needs `SO_ERROR` interpretation; other failed connection
states require the selected socket contract, not restarting effects blindly.

## Deliverables

Submit implementations, cursor transition table, exact callback trace and
pending-byte/committed counts at each return. Supply the off_t derivation and
actual ABI probe, logical/allocation observations,64-record contents, forced
shared-offset counterexample, overlapping-publication lock trace and both close
ownership traces. Explain syscall-specific retry decisions for read, write,
waitpid, connect and close. Reuse the separately executed exact-child wait work;
do not pretend a copy test assesses wait semantics.

## Verification and evidence-based gate

Run `stream`, `extent`, `position`, `close_policy` and supplied platform case
`sparse`. Correct targets return0 with `PC_PASS`. Each named semantic control
must compile then fail with `PL_CHECK:`; crashes and compiler diagnostics are
not semantic detection. `stream` verifies exact seven-byte output, five-byte
failure progress, retained suffix, zero write, source wait, overflow refusal,
budgeted EINTR and real pipe readiness. `position` compares all records and the
unchanged shared offset, rejects append descriptors, and checks a child cannot
acquire the overlapping read lock before publication. `sparse` compares bytes
before/after explicit zeros and reports both block counts without assuming a
specific allocator. Retain compiler/environment, command and log hashes.

## Objective-specific rubric

Original levels0–3 remain authoritative. `.04.a`: level2 requires every script
transition and the real-pipe integration; level3 derives hard-error suffix
ownership. `.04.b`: distinguish retry rules and both close models, plus exact
child wait evidence; no direct-standard or alternative-OS execution claim.
`.07.a`: prove guards before casts and regenerate actual boundaries; narrower
ABI remains unvalidated unless that compile/run succeeds. `.07.b`: distinguish
shared offset and byte equivalence from allocation equivalence. `.07.c`: verify
disjoint contents, show the forced lseek interference, and justify whole-record
locking under overlapping writes. A trace without the causal argument earns at
most level1. Level3 still requires the separate delayed unaided learner variant.

## Hints and separate assessor material

Start with two cursor indices; do not read again while an unwritten suffix
exists. Subtract before adding an extent. For positioning, draw two descriptors
pointing to one open file description. For close, track the resource ownership
bit separately from the returned error and the numeric descriptor value.
Private reference and control analysis are under `assessor/posix_lifecycle_depth/io`.

## Changed assumptions and composition

The supplied variants are hard sink failure after five bytes; retained-on-EINTR
close policy; an actual narrower off_t probe when its toolchain exists; explicit
zeros replacing holes; and overlapping records with cooperative lock publication.
Compose the copier with a future event loop by retaining the cursor until the
required readiness event, rather than busy-spinning. Compose positioned writes
with the later storage contract only after adding explicit persistence and
recovery semantics. Request deduplication/effect atomicity remains a separately
bound exercise and is not inferred from byte-transfer completion.

## Security and containment

These are owned local synthetic process/file experiments. The deliberate
shared-offset/split-record counterexamples reveal incorrect data placement and
inconsistent records; no arbitrary code execution or privilege gain is claimed.
No external path/device/server, real credentials, other users' files or host
security changes are permitted. Reap exact owned children; the runner deadlines
bound failed experiments and removes only its own temporary tree.

## Validation status and limitations

Authored, independently compilable learner stubs and separate reference source.
Exact execution evidence resides in the source-bound report; this contract does
not invent a passing campaign. Native hosted results, changed-ABI availability,
author review, external review, learner independence and physical observations
remain separate. Finite schedules do not prove all interleavings or mastery.
