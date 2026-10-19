# Path identity, full-resolution policy and metadata

## Identity and families

This required successor supplies executable reference support for the six existing
criteria `AX-PR-T23-02-T23.05.a` through `.05.c` and `T23.06.a` through `.06.c`.
It does not allocate new accepted units or replace their task/rubric/history.
Families B/C/D/E/F/I/J apply to identity, confinement and changed assumptions;
semantic reasoning and observation apply to metadata and permission models.

## Objectives and inspected sources

Use the exact primary-source passages registered in
`research/v5/expansion/legacy_posix_namespace/resources.json`: Linux `openat2(2)`,
`path_resolution(7)`, `stat(2)`, `stat(3type)`, `umask(2)` and `acl(5)`.
The Linux system-call policy is not an ISO C guarantee. C17 remains the
implementation baseline; no C23 feature changes the pathname authority problem.
The ACL example models only owner/group/other entries. It does not claim an
implementation of the abandoned POSIX.1e draft or a complete Linux ACL evaluator.

## Prerequisites and learner artifacts

Bring the learner-owned `PR-T23-02` descriptor and namespace implementation,
`PR-T23-01` exact-child ownership/cleanup work and the six original criterion
prediction sheets. Understand open file descriptions, borrowed descriptors,
`fork`, `waitpid`, `errno`, short transfer/error distinctions and object lifetimes.
Submit the changed implementation in `namespace/path_stub.c`; keep the supplied
controlled flawed reader and semantic controls intact. Host verification uses
Python and compilers; these are not dependencies of a freestanding target.

## Environment and dependencies

Hosted C17 on Linux with `openat2`, local temporary storage, `/proc/self/fd`,
`/dev/shm`, `fork`, descriptor-relative operations and system libc headers.
No `libacl` is needed: the creation/authority functions are explicit small models.
The runner requires two actually different filesystem types for the changed
metadata observation. Unsupported syscalls/filesystems fail the campaign with
their reason. Do not silently use weaker `openat` resolution. Compiler profiles
are GCC and Clang debug, `-O3`, and AddressSanitizer/UndefinedBehaviorSanitizer.
The parent shell's mask, credentials, mounts and security settings are untouched.

## Problem and supplied inputs

Implement the declarations in `path.h`. The tests create an owned outer directory,
two child directories, and literal `SAFE`/`OTHER` files. Every tested absolute path
also names that owned tree. The ordinary and changed experiments are:

1. Open `old`, rename it to `saved`, create another `old`, add a hard link to
   `saved`, remove both names and continue reading the original through live
   handles. Report `(device,inode,link-count)` and distinguish name from object.
2. Retain a directory descriptor across `chdir` and rename. Create/read through
   that descriptor; separately show an absolute `openat` path ignores an invalid
   directory descriptor. Do not call this confinement.
3. Check the original symlink target, release an owned child to replace that link,
   await its acknowledgement, then let the intentionally flawed reader read
   `OTHER`. Compare final `O_NOFOLLOW` with an intermediate-link counterexample
   and a full-resolution implementation. No timing luck or uncontrolled racing
   is necessary for the counterexample.
4. Copy metadata from file, link and directory without changing its meaning;
   repeat on sparse data and two filesystem types. Output raw size/block/time
   values and units, including requested versus returned timestamp fractions.
5. Model creation with/without a three-entry default ACL, compare the no-ACL
   prediction with an actual disposable child's `umask(027)` creation, and
   distinguish successful lookup, file read/write and entry replacement rights.

## Mechanisms and misconceptions

A dirfd is an anchor, not an assertion that every name stays beneath it. A final
component check cannot constrain intermediate links. A successful lookup is not
authorization for a later operation, and `(device,inode)` is not a permanent
identifier after the object's lifetime. A hard link is deliberately accepted by
the full-resolution policy: denying symlinks does not establish content provenance.
`st_size`, allocated 512-byte blocks and preferred I/O size have different units.
A timestamp structure's precision does not promise the filesystem stores every
nanosecond. `ctime` is status-change time, not creation time.

## Required contracts and reasoning

`pn_open_confined` borrows a valid directory descriptor and a live NUL-terminated
path. It returns a new owned `O_CLOEXEC` descriptor only for an existing regular
file reached by `RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS`.
It uses `O_NONBLOCK` so rejecting an owned FIFO does not wait for a peer. Empty
input is `EINVAL`; other errors, including `ENOSYS`, `EXDEV`, `ELOOP`, `EAGAIN`
and `EINTR`, propagate without changing the output or weakening the policy.
There is no unbounded retry. Close an acquired descriptor if type inspection
fails. Callers must retry only under a separately stated bounded policy.
This does not ban mount traversal, hard links, changes to file contents after
open, or a peer already possessing another authority. Directory type/permissions
remain kernel-checked. A live handle pins an object, not immutable bytes.

`pn_info` uses `fstatat`, follows the final link exactly when requested, stages all
fields locally and publishes output only after success. No raw `struct stat`
serialization is permitted. The source's concurrent field snapshots need not
describe one atomic moment. The supplied tests serialize their metadata edits.

`pn_creation_mode` accepts only bits `0777` in its three unsigned mode inputs.
Without a default ACL use `requested & ~mask & 0777`; with a declared three-entry
default use `requested & default_permissions`. This latter restriction omits
named users/groups and the ACL mask. `pn_authorize` is a serialized policy
adapter: all operations need directory search; read/write need their respective
file permission; replacement needs parent write. No sticky bit, privilege,
immutable attribute, read-only mount or concurrent credential change is modeled.
Rejected inputs leave output unchanged. No dynamic allocation is required in
these four functions; state space is bounded, and path traversal cost belongs to
the kernel/path length, not a false constant-time claim.

## Deliverables and experiments

Submit four implementations, the six objective-specific causal arguments,
predicted and observed identity/metadata tables, exact compiler/kernel/filesystem
versions and commands, plus descriptors/children cleanup accounting. Retain both
the flawed-reader observation and each repaired result. Explain why the tested
hard-link case reads `OTHER` without violating the stated resolution policy.
Produce the complete delayed variants in the existing criterion records after
their seven-day interval; an author's reference run is not that learner attempt.

## Verification and acceptance

Run each case independently: `identity`, `anchored`, `confined`, `metadata`,
`policy`; repeat metadata in `/dev/shm` and ordinary temporary storage. The author
runner is `tools/run_posix_namespace.py --report
reviews/v5/expansion/legacy_posix_namespace/namespace_validation.json`.
Correct runs return zero, print `PN_PASS`, and end with their initial descriptor
count. Each semantic mutant/stub must compile and fail with `PL_CHECK:`; crashes,
sanitizer findings and compile failures do not count as discriminating evidence.
The two-error syscall adapter injects `ENOSYS` and `EAGAIN` only into the owned
reader, proving fail-closed behavior without changing host syscall support.
The real umask observation checks absence of a default ACL first. Its separate
`EACCES` authority adapter is explicitly injected/model evidence, not a claim that
the host denied a root or privileged process. Compare size and block values with
an independent raw `fstat`; do not demand a fixed sparse allocation or timestamps
that differ when the filesystem resolution cannot express the requested change.

## Assessor rubric

Use each original leaf's levels0–3. Level0 includes reading outside the owned
tree, fabricated denial/delay, weaker resolution fallback, leaked ownership or
claiming all paths/permissions are safe. Level1 reproduces a trace but cannot
explain its object/permission boundary. Level2 must satisfy the concrete cases
for that leaf and distinguish its two semantic controls. Level3 derives the
hard-link, renamed-dirfd, intermediate-link, changed-filesystem, default-ACL or
replacement-authority variant independently. No finite sample proves all
possible kernel interleavings or absolute mastery.

## Hints

First draw separate columns for strings, directory entries, inode identities and
live handles. Next identify every distinct pathname resolution; look for work
between check and use. Finally compare the kernel's whole-resolution flags with
the final-component flag, and compare requested mode with the two distinct
creation rules. Assessor implementation and analysis remain under `assessor/`.

## Variants and composition

The actual hard-link and directory-rename experiments support `.05.a/.05.b`;
the intermediate link and accepted hard-link counterexample support `.05.c`.
The two filesystem types with sparse/timestamp observations support `.06.a`;
the default-ACL model supports `.06.b`; the changed operation from write to
replacement and its inverse permission state support `.06.c`. Compose a later
parser with this acquired descriptor rather than re-resolving a checked path.
Adding content authentication, mount exclusions or concurrent mutation requires
a new explicit policy and independent tests; none follows from this adapter.

## Security scope

Target: the original teaching reader and its owned temporary tree. Environment:
unprivileged local Linux helpers under runner deadlines. Attacker capability:
the designated child may replace exactly one synthetic symlink when signalled;
the variant installs one intermediate link within the outer owned tree.
Defect/property: a previous name check does not bind later resolution to the
same permitted object. Exposing transition: check, acknowledged replacement,
later open. Harmless success: the flawed reader returns literal `OTHER` where
the prediction was `SAFE`. No code execution, privilege escalation or external
file access is asserted. Repair: full-resolution policy plus type/ownership
checks. Regression: final/intermediate/preexisting links, `..`, owned absolute
path, missing/nonregular input, syscall failures, hard-link residual and cleanup.
Limits: the policy concerns resolution, not authenticity or immutable contents.
Do not substitute production paths, secrets, mount operations or host policy
changes. Stop all owned children before removing their private test directory.

## Validation status

Authored successor with separate reference, ten semantic controls and unsolved
learner stubs. Current execution status and exact source/log bindings live in the
campaign report and `support.json`; this text makes no pre-execution pass claim.
External review, learner independence/delay and physical observation are separate
dimensions. Original assessment and acceptance evidence remains historical.
