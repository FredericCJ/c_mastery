# Interface authority and a declared non-Linux port

Target of the analysis port: a POSIX.1-2008/TC2 hosted implementation with the
selected threads, mapped-file/shared-memory and monotonic-clock facilities.
No non-Linux machine was run. The exact published POSIX pages returned403 during
this inspection; the following bounded ownership claims are supported by the
listed Linux interface manuals and must be checked against that target's primary
documents before claiming a completed port. POSIX.1-2024 adds some interfaces
that were extensions under the 2008 contract, so "not in2008" does not mean
"never standardized".

| Interface | Declaration/library | Contract/optional boundary | Linux mechanism / port decision |
| --- | --- | --- | --- |
| fork | unistd.h, libc | child identity, inherited process context and private address-space behavior; threaded-child restrictions | libc can use clone internally; retain interface, do not issue raw clone in the portable core |
| execve | unistd.h, libc | replace image using explicit argv/env, retaining selected process attributes | do not infer all mappings survive; use a prepared absolute helper path |
| waitpid | sys/wait.h, libc | collect the owned child, separate exit/signal and wait errors | retain API, handle EINTR; no raw wait4 ABI dependency |
| pipe2 | selected glibc exposes with _GNU_SOURCE | absent from2008 baseline; current manual records POSIX.1-2024 | port2008 uses pipe+fcntl before concurrency or under process-wide descriptor-creation coordination; naïve separated CLOEXEC setup is not equivalent in an arbitrary threaded launcher |
| /proc/self/fd | Linux procfs path, not a C function declaration | mount/permission and OS-specific observation | replace with an application-owned descriptor ledger and explicit file actions; do not invent a portable enumeration API |
| clone/clone3 | Linux-specific libc/raw-kernel interfaces with different signatures | flags select sharing and child-exit behavior, not a portable fork substitute | exclude from port API; wrapper internals remain implementation observations |
| openat | fcntl.h with selected2008 feature visibility | directory-relative resolution, ownership/flag contract | retain on declared target, verify close-on-exec creation support; no assumption that a visible declaration selects a kernel syscall |
| epoll | sys/epoll.h, libc/Linux | level/edge/one-shot modes and supported handle types differ | use poll only for this packet's single-pipe zero-timeout level contract; reject stronger mode requirements |
| pthread_create | pthread.h, thread library integration | selected thread facility, identity/lifetime/synchronization | do not treat a thread as a process or equate pthread_t with Linux TID |
| mmap | sys/mman.h | verify selected mapping facilities, sharing/backing/lifetime rules | MAP_ANONYMOUS is not the chosen2008 portable dependency; a prepared shared backing object can supply the byte-mapping experiment |
| flock | sys/file.h, BSD/Linux contract | cooperating participants; network filesystem semantics may differ | retain only as a declared Linux/BSD backend; a POSIX fcntl lock backend requires its own process/open-description and close semantics, not textual substitution |
| spawn fchdir action | GNU _np action in this selected backend | no2008 portable working-directory action assumed | either start from already-correct process cwd before concurrency, use a verified target action, or keep restricted fork path; never process-global chdir around concurrent launches |
| monotonic/CPU clocks | time.h and selected feature macros | monotonic, CPU and suspend-aware clocks are distinct | CLOCK_BOOTTIME is Linux-specific; a portable lease must state whether suspend counts and reject unsupported semantics |

The executable probes separately record clean-header visibility, link/runtime
environment, injected old-kernel errors and current actual epoll/poll behavior.
They do not execute an older kernel. Capability epoch2 uses ENOSYS after epoch1
success; an old cached choice is invalid unless its epoch/lifetime contract still
holds. A successful new-header compile is unchanged by that modeled transition.

The inspected Ubuntu glibc2.43 source has a shared child error field and waits/reports a
pre-exec error to the caller. The observed Ubuntu2.43 missing-image return agrees
with that bounded mechanism. The general manual also describes implementations
whose child exits127 instead. The learner must explain this discrepancy, record
both possible interface outcomes, and never use numeric127 alone as proof of
exec failure. The vendor binary is recorded separately from packaged source;
no bit-identical source-to-binary provenance is asserted.
