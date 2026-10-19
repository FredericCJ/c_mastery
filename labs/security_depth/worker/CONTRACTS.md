# Descriptor and worker authority, version 1

This required successor implements the six unchanged PR-T35-02 criteria
`AX-PR-T35-02-T35.05.a/b/c` and `AX-PR-T35-02-T35.08.a/b/c`, plus their exact
seven-day `-DELAYED` tasks. Use the original supplement and its shared assessment
contract for D1–D4, level0–3 rubrics and source-bound learner evidence. This is
additional author support, not rewritten historical acceptance or workload.

## Prerequisites and environment

Bring the learner's T23 process/descriptor artifacts, T35 threat ledger and
PR-T35-02 fixture predictions. Import the actual learner-owned
`pn_open_confined` interface from
`labs/posix_lifecycle_depth/namespace/path.h` and its CONTRACTS.md. Its existing
resolution/type/failure contract remains authoritative; do not substitute a
weaker final-component check. The author adapter composes that frozen reference
implementation, while exported learners receive its unfinished stub.

Use C17, parent profile A, nonprivileged Linux with openat2, close_range CLOEXEC,
`/proc/self/fd`, fork/execve and a local owned temporary directory. The parent is
single-threaded and owns all child lifecycle. No signal handler concurrently
reaps children. Child executable and bounded output are the supplied original
worker; accepting an arbitrary blocking executable would require another
cancellation/deadline contract. Record actual compiler/libc/kernel, user identity
and descriptor limit. The high-descriptor test requires capacity above900; an
unavailable prerequisite fails explicitly. Do not change host limits or policy.
Compare GCC/Clang debug, optimized and ASan/UBSan builds. Host tools are not
freestanding target dependencies. ISO C provides no descriptor, process or MPU
isolation; the active implementation edition is distinct from OS/ISA contracts.

## Problems and exact contracts

Implement `worker.h`. Return zero or a positive errno; preserve published outputs
on failure. Input strings and arrays remain live and stable for the call, outputs
are distinct, and borrowed descriptors are not closed by the component. No shell
or PATH lookup is permitted. All syscall results must be examined; unsupported
full-resolution behavior is not permission to weaken the security policy.

`sd_exclusive` creates an owned read/write temporary with O_CREAT|O_EXCL and
atomic O_CLOEXEC, requested mode0600. The leaf is nonempty, at most32 characters,
contains no slash and does not start with a dot. The supplied directory descriptor
is a retained anchor. An existing entry, including an owned symlink, returns
EEXIST and preserves the output handle. This is exclusive creation, not an
assertion that a separately checked pathname has retained identity.

`sd_request(read_allowed,operation,borrowed_log,out)` permits only read-log when
the boolean policy is true. All other valid requests return EACCES before any
read/write, including when the supplied descriptor is invalid. An admitted read
uses a four-byte pread at offset0; short data returns EIO, syscall failure its
errno, and no partial output is published. It never writes the configuration.
The application policy is separate from the kernel's rights on a descriptor.

`sd_spawn` creates an exact original child with explicit executable, argv and
environment. Before fork, acquire CLOEXEC pipes and duplicate borrowed sources
above the fixture destinations to avoid descriptor alias collisions. In the
child, fd0 is the owned report pipe, fd4 the read-only log, fd3 a temporary marked
CLOEXEC; close1/2 and mark the entire range5..UINT_MAX CLOEXEC. Preserve the
internal error pipe until exec, where CLOEXEC closes it. The parent publishes an
owned child PID and report-read descriptor after the error channel closes. Under
the admitted normal child setup this follows successful exec; unexpected child
termination before reporting is not distinguishable from EOF by this channel,
so final wait status remains mandatory and no ready-service claim follows. Exec/setup
failure is reported through the error channel, the child is reaped and every
new parent descriptor is closed. Fault1–4 inject failure before first acquisition,
after the first pipe, before fork and in child setup respectively. No borrowed
handle is consumed. The caller reads bounded output, closes it and waits exactly
that child. This launcher is not a process sandbox or general multithreaded fork
library. Marking descriptors in this child does not fix another concurrent
thread's independent fork; atomic creation prevents that separate leak window.

## Supplied experiments and necessary distinctions

For .05a, inspect a synthetic link referring to SAFE, release an owned child to
replace it with another owned object EVIL, await acknowledgement, then perform
the flawed later open. Show the wrong object and compare the retained SAFE
descriptor and exclusive-creation rejection. In the changed case, replace an
intermediate link after checking `middle/item`: O_NOFOLLOW on only the final
component still reaches EVIL; the actual full-resolution producer rejects ELOOP.
No content provenance, immutability, mount exclusion or hard-link prohibition
follows from that producer. State directory/name authority assumptions.

For .05b, execute the original worker and enumerate its actual descriptors through
`/proc/self/fd`, excluding only the inventory's own directory descriptor. Require
exactly0 and4 and absence of3 and an intentionally open parent descriptor>=900.
Run four acquisition failures and an actual nonexistent executable inside the
owned tree; outputs and parent descriptor count remain unchanged. Analyze all
three orderings of open, later fcntl and fork/exec with open preceding fcntl:
identify the leaking interval and compare atomic CLOEXEC. This changed task is
an explicit logical schedule, not a claim that a few thread races are exhaustive.

For .05c, arguments include `a b`, `semi;colon` and `$(synthetic)` as exact bytes.
The worker reports lengths and boundaries and never invokes a secondary command.
It also has a deliberately tiny option parser: `--admin` changes a synthetic
mode before `--`, but becomes literal data after `--`. This is child argument
interpretation, distinct from shell interpretation; quote-free execve does not
validate the child's application grammar. Environment is explicit `SD_ONLY=yes`
and optionally the owned reopen path; PATH is absent.

For .08a/.08b, execute read-log and denied write-config in the actual worker.
Reading the inherited O_RDONLY log gives SAFE and writing that same descriptor
returns -1/EBADF without changing bytes. Remove read permission and require
EACCES even for an invalid descriptor, proving the gate precedes reads. In the
changed case the worker reopens its own user-writable path, writes OPEN through
that new handle and observes it through the original reader. This expected
residual power does not contradict the original descriptor-right guarantee.
Inventory filesystem/network/same-user authority without probing external targets.

For .08c, compare the earlier original same-process/private-child experiment
in `labs/security_depth/command/process_boundary.c` with this descriptor check.
Attribute each enforcement to ISO C, diagnostics, runtime checks, OS or discipline.
For the MPU changed design use the separately source-bound B06 configuration:
aligned nonoverlapping2MiB code RO/executable and128KiB RAM RW/XN, with privileged
background enabled. Verify layout and access table, then inspect its prior actual
MPS2 fault controls for executing RAM and writing code. The model revalidates that
producer's exact source receipt; it does not pretend to run the board again.
Privileged software can reconfigure the MPU, and CPU MPU permissions do not cover
DMA transactions. Protecting DMA configuration and managing transitions remain
separate obligations. No physical STM32 observation is inferred.

## Deliverables, verification and assessment

D1 records the original task prediction, wrong explanation and exact trust/owner
diagram. D2 implements the three operations and imported resolver composition,
or supplies the inspectable MPU/schedule analysis where that criterion calls for
analysis. D3 includes ordinary and adversarial results, acquisition ledger,
before/after bytes, raw descriptor/argv observations and versions/hashes. D4
retains each exact original delayed task and new predictions under its changed
condition after the required interval. Do not substitute a fresh author run for
the learner's independent transfer.

Public native cases are `race`, `policy`, `inheritance <absolute-worker-path>`.
Use owned `SD_TMP_ROOT` when running mutations; failed assertion targets may leave
their owned synthetic directory for the supervising campaign to remove after all
children are joined. Positive cases clean themselves and return to their initial
descriptor count. Reproduce the independent process-policy fixture and model;
compile/execute semantic mutations and unfinished learner rejection separately.
No assertion disabled by NDEBUG is an acceptable gate.

Level0 includes weakened resolution, inherited forbidden handles, denied effects,
unreaped child or an invented isolation claim. Level1 reproduces one trace but
confuses a name with an object or an argument vector with application policy.
Level2 meets the exact original leaf result and an independently chosen boundary,
explaining which contract enforces it. Level3 transfers under that leaf's original
intermediate-directory, concurrent-open, option-parser, ambient-reopen, revoked-
read or MPU change. Passing finite experiments cannot prove absolute mastery.

Hint1: draw names, open descriptions and descriptor numbers separately. Hint2:
locate each authority-bearing operation and ask whether it repeats resolution.
Hint3: mark ownership on every branch between pipe creation and successful exec;
then remove the child-executable assumption and identify what new cancellation
mechanism would be required. For the MPU compare CPU privilege with DMA ownership
rather than treating every access as a CPU instruction.

## Security scope and evidence limits

Targets are original local programs, synthetic four-byte files and owned children.
The teaching adversary may replace only named entries within its private tree;
the harmless success is EVIL instead of SAFE. The argument target may interpret
only its own synthetic mode, never execute the embedded text. Repairs bind
operations to descriptors/full resolution, atomic inheritance policy and an
application gate; regression tests verify both ordinary success and rejection.
The worker is nonprivileged but retains ambient user authority, deliberately
demonstrated only on its owned log. No network, external path, credentials,
host-security change or flashing is authorized by this exercise.

Current source-bound author status is separate from independent review/learner
and physical evidence. Linux-specific calls, the current producer contract and
the selected MPU source/receipt constrain portability. Original published C17/C23
source gaps remain. This successor closes no hardening, secret-lifetime or
artifact-authenticity criterion merely by mentioning those concerns.
