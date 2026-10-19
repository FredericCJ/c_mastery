# Restricted launcher analysis

The original native-int error path correctly loops over successful short writes,
read EINTR and exact-child wait EINTR. The followthrough wrappers call the real
kernel functions after a single injected interruption, cap transfers at one
byte, and deliberately stop one failing child's record after one byte. That
child is still reaped and the sentinel output remains untouched on EPROTO.
Early exit126/SIGTERM verifies the weaker EOF interpretation directly; no image
authentication claim is warranted. The stage extension validates record extent,
stage, code and final wait classification before publishing an entire result.

The two snapshots are the representation that preserves a cycle. For sources
[a,b] and destinations[b,a], sequential dup2(a,b),dup2(b,a) would make both
refer to the original a. Snapshots prevent the first replacement from destroying
the second source. Even source==destination is duplicated from a distinct
snapshot so FD_CLOEXEC is deliberately cleared. Fail ordinal4 illustrates the
nontransactional boundary: destination0 changed, destination1 retained. The
reference releases temporary resources but cannot undo an already observed
external stream replacement; the declared setup caller terminates instead.

The ready-channel EOF supplies a real synchronization fact. A ready byte alone
could precede the intermediate helper closing its own writer; waiting for EOF
rules out that alternative. Then EAGAIN versus EOF distinguishes whether the
grandchild still owns a write reference. Both branches are intentional specimens;
the flawed branch must remain flawed until the release token, then clean up.

The separate real-signal probes require observed EINTR counters and a live
controlled delay; a successful ordinary transfer alone does not demonstrate the
retry branch. The write probe fills a pipe, blocks on one more byte and releases
capacity from its owned child. Mutants1/2 respectively drop a successful short
write suffix and accept incomplete records;3/4 accept an unknown stage and turn
any EOF into successful exit0;5/6 destroy a descriptor cycle and retain CLOEXEC
on equal descriptors;7/8 break the repaired and deliberately unrepaired
grandchild ledgers;9/10 reject real read/wait EINTR. Original mutants retain
their meanings (ignore exec error, conflate127 with exec failure).

The first optimized wrapper experiment correctly failed its own observation
check: glibc had emitted __read_chk, bypassing a read-only linker wrapper. The
corrected injector wraps both entry points and delegates the fortified call to
the real checked implementation. Neither fortification nor strict warnings was
disabled. The archived failed attempt is useful diagnosis evidence, not a pass.

All fork paths in these tests start from a single-threaded owner. The production
pre-exec child path uses local scalars/bytes, close, execve, write, kill/getpid
for the explicit early-death specimen, and _exit. A source audit supports the
restricted path; this does not execute arbitrary allocator-lock inheritance or
prove a generic multithreaded launcher. General hard wait failure, external
reaping, cancellation and deadlines remain separate protocols. The original
learner criterion for posix_spawn comparison is not completed by this package.
