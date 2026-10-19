# Assessor analysis: object, process and device boundaries

Keep this file out of learner packages. Assess all six original PR-T35-02 leaves
and their exact delayed IDs separately; a working child launcher alone is not
coverage of path identity, policy or MPU reasoning.

For T35.05a the original controlled reader checks one inode, acknowledges the
adversary's replacement, and opens another. EVIL is the harmless violation.
An open descriptor acquired before replacement still reads SAFE. O_EXCL rejects
the preexisting synthetic entry instead of treating a previous stat as a lease.
In the changed intermediate-link case final O_NOFOLLOW still reaches EVIL; the
actual imported pn_open_confined rejects resolution of every link. Its BENEATH
rule does not ban hard links, mount traversal or later changes to file contents.
Directory/name ownership assumptions are indispensable. A regular-file
replacement within an otherwise permitted tree can still be a different object;
identity-sensitive authorization must use the acquired descriptor/object policy.

For .05b both source handles are duplicated above destinations before fork.
Sequential dup2 without this staging can overwrite another source when numbers
coincide. CLOEXEC is attached during each acquisition; post-open fcntl leaves one
of the three legal orderings exposed to another thread's fork/exec. This precise
logical counterexample satisfies the changed schedule task without claiming
stress is exhaustive. This launcher itself admits only a single-threaded parent.
The child marks5..UINT_MAX CLOEXEC, removes1/2 and maps the original fixture0/4;
the test intentionally keeps a parent fd>=900 to reject low-number-only scans.
The worker's full proc inventory excludes only its own probe descriptor.

The error writer must remain usable until failed exec can report; CLOEXEC turns
successful exec into EOF. The parent owns each staged handle and exact child,
reaps reported failures and publishes child/output ownership on channel EOF.
EOF alone cannot distinguish exec from unexpected pre-exec child death; the
admitted setup excludes asynchronous termination and the caller must still wait
for final status. It is not a service-readiness handshake. Current
failure injection covers four acquisition points and actual ENOENT inside the
owned tree. Child setup errors normalize to EIO; syscall failures before fork
retain their errno. A foreign blocking child or continuously delivered signals
would need additional cancellation/retry bounds and are not admitted here.
Permission bits requested at creation may be restricted by ambient mask/ACL;
the test does not alter system-wide security configuration.

For .05c execve transports explicit strings and an explicit environment. No shell
interprets the semicolon or dollar-parenthesis text. The worker's own --admin
parser nevertheless changes its mode until -- ends option parsing. Valid quoting
at one boundary says nothing about the next parser. Require exact argument
lengths, boundaries and absence of PATH rather than accepting one visually
similar command string. The synthetic mode has no OS privileges.

For T35.08a O_RDONLY constrains operations through that descriptor. It is still
possible for the same user to open the owned pathname O_WRONLY. Actual worker
reopen changes OPEN through a new handle; this is the explicit residual authority,
not a failure of the original EBADF claim. No network or external same-user
process is probed to fill the authority table. The evidence is narrower than a
sandbox. For .08b the application gate denies write-config and revoked read-log
before pread: invalid descriptor -1 still yields policy EACCES on denial, not
kernel EBADF. Output remains KEEP. Offset alone would be an inadequate discriminator
because pread does not change it even if an unauthorized read occurred.

For .08c ordinary C module opacity is programmer/API discipline, with diagnostics
and runtime checks providing limited enforcement. The original fork control
demonstrates private-memory separation, not shared-memory or inherited-authority
isolation. The independent MPU table checks two aligned nonoverlapping regions;
AP6 gives code read-only, AP3 gives RAM read/write, RAM XN denies instruction
fetch. Its exact B06 source-bound prior MPS2 controls test RAM execution and code
write faults in privileged mode, not unprivileged mode or physical STM32 behavior.
Privilege can reprogram the MPU; privileged background is not a sandbox. CPU
permissions do not constrain DMA; protecting DMA registers and controlling the
transfer owner are separate. Retained guest evidence is labeled prior execution,
never fabricated fresh execution.

Withhold these transfer cases: reorder descriptor sources to collide with0/3/4
and require staged ownership; supply a missing intermediate directory and forbid
fallback resolution; deny read-log on an invalid descriptor and ask which result
must win; supply a double dash after an already interpreted option and explain
why it cannot undo the effect. In the MPU design move RAM base off its size
alignment and require rejection before register programming. These complement,
not replace, the original delayed variants. No finite observation proves universal
security, independent learner competence or complete source inspection.
