# Mapping extents, publication and participant death, version 1

This required successor addresses LRUN-06 and the nine existing criteria in
PR-T23-03. It inherits their stable `AX-PR-T23-03-T23.xx.y` IDs, original leaf
rubrics, supplied fixture and seven-day independent variants from the legacy
assessment shared contract. It supplies actual construction/observation support
for T23.08.a–c, T23.09.a/c and T23.10.a–c. T23.09.b's equivalent-contract
mapped-versus-explicit-I/O workload comparison remains a separate required
experiment; these tests do not claim its performance results.

Families A, B, C, D, F, G, I and J apply. The misconception is that an address
range alone establishes physical residency, permanent file bounds, cross-process
pointer validity or synchronized publication. Predict which contract each changed
condition violates before execution. This is original local teaching software.

## Prerequisites and inspected sources

Bring PR-T23-03's mapping ownership ledger, T23.08/09/10 teaching, bounded byte
decoder, error-channel/reap implementation from PR-T23-01, and a descriptor to
open-file-description ownership graph. Primary Linux mmap, mincore, record-lock
and remap passages actually inspected are registered as `R-LMAP-*` under
`research/v5/expansion/legacy_posix_mapping/resources.json`. ISO C17 governs
local objects and byte arithmetic; Linux/POSIX governs mappings, fork/exec,
file locks, process termination and page faults. No claim of portable ISO C
interprocess atomics or physical durability is made.

## Environment and supplied artifacts

Use an owned disposable working directory, unprivileged Linux, GCC/Clang and
libc. `map.h`, `map_stub.c`, `tests.c` and the original `fault_helper.c` are
learner-facing. Implement the stub; keep the tests and deliberately faulting
helper intact. Only the author runner explicitly selects separated assessor
sources. No physical equipment, network share, host policy or external file is
part of the target. Use a local filesystem supporting process record locks.

All pointed-to objects and buffers are live/disjoint as specified. Mapping
controls have one nonreentrant owner, are not copied while active, and are
initialized inactive. There is one thread per participating process. No other
thread closes any descriptor for the locked inode; no external writer, truncator,
reaper or signal policy interferes. Advisory locks constrain cooperating peers,
not a malicious process that ignores the protocol. Parent test deadlines are
15 seconds and always indicate failure, never a passing negative control.

## Owned mappings and borrowed views

`pm_map(fd, offset, mapped, logical, out)` borrows an open regular-file descriptor
and acquires a read-only shared mapping. `mapped` is 1..16 discovered pages;
`logical<=mapped`; offset is nonnegative and page-aligned. Current file size
must include `offset..offset+logical`, checked by subtraction without overflow.
Zero/unaligned/overlarge/overflowing requests fail before acquisition. Failure
leaves output unchanged. Success publishes base, both extents, offset, device,
inode, active state and generation1. The caller may close its fd immediately;
the mapping remains owned until successful `pm_unmap`, which is idempotent on an
inactive control. An unexpected unmap error retains ownership for diagnosis.

`pm_copy(map, borrow, out, capacity)` accepts a generation plus offset/length,
not a retained raw pointer. It rejects an inactive owner, stale generation,
insufficient output or a range outside the logical extent before forming a view.
Zero length at logical end is valid and performs no dereference. The output is
unchanged on these validation failures. The current file extent remains stable
under the caller's coordination contract; bounds checks cannot prevent another
process from truncating the file between validation and use.

`pm_remap` acquires a replacement mapping of the same device/inode before
unmapping the old one. The two mappings coexist until commit, so the base moves;
no MAP_FIXED address overwrite is used. Successful replacement increments the
generation, invalidating every previous borrow even if logical fields still fit.
UINT64_MAX refuses further replacement without wrapping. A wrong file or failed
acquisition leaves the old mapping usable. This is a map/replace operation, not
an assertion that Linux mremap always moves. Compute resource cost: temporarily
two mappings, O(length) copy-out, constant control storage and syscall count.

Concrete checks use two file pages, `abcdef` as the six logical bytes, an exact
end zero-length borrow, one byte past logical end, SIZE_MAX offset, a closed
original fd, wrong-inode remap and repeated cleanup. A delayed variant moves
the base and rejects its old generation rather than testing a dangling pointer.

## Relocatable record and synchronization

The object is exactly128 bytes. Bytes0..3 are `PMR1`; byte4 is state (0 uncommitted,
1 committed); byte5 version1; byte6 scalar byte order (1 little,2 big); byte7
reserved zero; bytes8..15 nonzero session64;16..19 payload offset32;
20..23 length32;24..27 a modulo-2^32 payload check;28..31 reserved; encoder
payload begins at32 with length0..64. The check starts0 and updates `v=v*33+b`.
It detects the supplied corruption, not malicious forgery. Every scalar is
decoded bytewise. Validate offset>=32, offset<=extent and length<=extent-offset
before forming a view; no native struct, padding, pointer or unaligned load is
the wire contract. The codec writes a full staged128-byte result only on success;
decode similarly publishes a copied `struct pm_record` only after validation.

`pm_publish(fd, session, payload, length, cut, context)` and `pm_replace` use an
exclusive whole-file **process-associated fcntl record lock**. A zero-sized new
file becomes128 bytes while locked. A complete existing record wins initialization
and produces EEXIST; the second initializer must not silently overwrite it.
Malformed committed objects are errors. An uncommitted object may be replaced by
an authorized new session after acquiring the lock: owner death has released
the old process's lock. The external owner supplies a never-reused increasing
session; PID alone is never a session identity.

Publication invalidates state, writes header/payload, then commits state1, all
while the lock is held. The optional nonreentrant teaching hook runs after
invalidation, data completion and commit, numbered1/2/3. Its descriptor ledger
must not close another descriptor for this inode. The test pauses at each hook,
tries a reader, kills only the confirmed owned initializer, reaps it, and recovers.
Before death, even the committed-but-locked stage must deny the reader. After
death, stages1/2 remain uncommitted; stage3 is readable. No timeout alone permits
stealing a live owner's state, and no partially initialized pthread mutex is
reinitialized as a purported recovery technique.

`pm_read_record` takes a nonblocking shared lock, copies and validates the full
record, checks the expected session, unmaps and unlocks. Busy returns EAGAIN or
EACCES, uncommitted EAGAIN, stale session ESTALE. Output remains unchanged on
failure. `pm_replace(fd, expected, replacement, ...)` requires an exact current
session and strictly larger new session; mismatch is ESTALE, nonincrease EINVAL.
It fences future operations carrying the old session. Already returned snapshots
are ordinary copied values tagged with their original session.

All peers open read/write for this narrow adapter; every operation closes its
mapping before unlocking. The outer creator unlinks the object only after all
owned peers are reaped. Record locks are automatically released on process death
and are not inherited as locks through fork. Closing **any** descriptor for the
same inode releases that process's locks; this is why the single-owner descriptor
precondition is substantive. Locks are advisory and do not synchronize two
threads in one process. No NFS lost-lock behavior is claimed.

The fresh-exec reader opens only the pathname passed by its controller: its
address space contains no inherited mapping, and all data fds were close-on-exec.
The two simultaneous mapping/array cases separately test different bases. Two
initializers are released through a gate: exactly one publishes and the other
observes EEXIST, regardless of scheduling order. Process-death cuts preserve
kernel page-cache state; they are not abrupt machine-power or storage-flush tests.

## Residency, private sharing and fault observations

The supplied investigation maps3 and16 anonymous pages, samples mincore and
getrusage, writes only the middle page, samples again and prints each field.
Only the least-significant residency bits are interpreted. No exact resident
count, fault delta, overcommit conclusion or future latency bound is asserted.
For the delayed page-size/huge-page analysis, recompute rounded extents and
vector lengths and identify which observed accounting fields must be repeated;
do not change host huge-page or overcommit policy.

The private/shared experiment sets11, forks one child that writes22, reaps, and
expects11 for MAP_PRIVATE and22 for MAP_SHARED. A private file mapping's write
is compared with a second private mapping and pread of the backing file. These
are visibility observations, not a physical-page identity inspection.

The separate fault helper disables core dumps **only in itself**, maps the
owned two-page file, announces readiness and waits for a parent token. In the
nontruncated control it exits normally. One case makes its first page PROT_NONE
and accesses that valid mapping address; another has the parent truncate the
file to one page before it accesses the second mapped page. The parent records
SIGSEGV and SIGBUS respectively. The helper is deliberately compiled without
sanitizer signal interception so the Linux termination is what is observed;
the surrounding reference code still runs sanitizer profiles.

This is an explicit local fault specimen. The competing state is the owned
parent truncation or the helper's protection change. A harmless success condition
is the expected contained child termination plus exact descriptor/file cleanup.
The repair is a stable-file/protection ownership protocol and checked logical
extent; regression includes nontruncated control and mismatched expected-access
mutants. A crash here is not arbitrary code execution or privilege escalation,
and no handler attempts to resume invalid accesses.

## Deliverables, verification and rubric

Compile in a new owned temporary directory using absolute source paths:

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 fault_helper.c -o fault-helper
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -I. tests.c map_stub.c -o mapping-tests
./mapping-tests views ./fault-helper
./mapping-tests codec ./fault-helper
./mapping-tests publication ./fault-helper
./mapping-tests sharing ./fault-helper
./mapping-tests residency ./fault-helper
./mapping-tests faults ./fault-helper
```

Resolve paths relative to the copied mapping directory, or copy the public files
into the temporary directory. Replace only the stub with the learner's source.
The first three are learner implementation tests and must reject the unsolved
stub. The last three are supplied platform investigations, requiring analysis
of observations; they are not falsely counted as rejected learner implementations.
The author runner `tools/run_posix_mapping.py` records its exact six-profile
commands, source pins, raw data, semantic mutants and separate fault-helper flags.

D1: mapping/descriptor/session ownership diagrams and predicted failure cuts.
D2: original control, codec and publication implementation plus byte-order and
lock-lifetime derivations. D3: raw outcomes, observed accounting samples, all
three death cuts, two-initializer winner, fresh-exec reader, generation refusal
and semantic negative controls. D4: independent delayed changed assumptions from
the original criteria, with changed source/target obligations stated explicitly.
For .08.a/.08.c the residency report must state its sampling limits; .08.b/.09.c
must classify the original child faults and private/shared visibility; .09.a
must demonstrate stale-borrow refusal; .10.a–c must each account for version,
offset/session and owner-death behavior separately. Passing only the codec does
not assess publication or ownership.

Level0 violates lifetime/containment or invents a guarantee. Level1 matches an
ordinary value but conflates mapped/resident, pointer/offset or lock/commit.
Level2 supports the leaf's mechanism, changed boundary and negative control with
the exact contract and independent expected result. Level3 supplies the delayed
unassisted transfer and identifies which proof/observation changes. External
review, independent learner performance and physical evidence remain separate.
First hint: a mapping extent and a current file extent are different quantities.
Second hint: a process-owned kernel lock can disappear on death; bytes left in
the mapped file still need a version/state/session recovery decision.

Assessor analysis and implementation are separated under the restricted assessor
tree. Finite author tests establish only these cases. General adversarial writers,
thread sharing, persisted epochs, physical power loss, filesystem benchmarking
and absolute learner mastery are outside the claimed evidence, not silently
declared complete.
