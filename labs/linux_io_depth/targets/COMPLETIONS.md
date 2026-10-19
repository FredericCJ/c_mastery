# T62 actual completion ownership continuation

This required B/C/D/F/G/I/J work retains PR-T62-01–08 and the B06 shared contract.
Prerequisites are the learner progress cursor, readiness loop and completion
state model. Use C17 plus explicitly identified Linux UAPI/system calls and
GCC/Clang atomic builtins; no liburing implementation may supply the ring. The
host verification program owns anonymous pipes and ring mappings only.

Implement `raw_ring.h` from stubs for one issuer, no SQPOLL, eight requested
entries. Respect the kernel-returned capacity/masks and SINGLE_MMAP feature.
Acquire-load the consumer head before reusing SQ storage; publish the completed
SQEs/index array with release-store to tail. Acquire-load CQ tail before reading
completion fields, and release-store CQ head after copying. Ordinary ISO C
thread semantics alone do not describe kernel-shared mappings. Initialization
failure unwinds every acquired mapping/fd. Queue-full refusal leaves state
unchanged. Submit handles EINTR and partial submitted counts without duplicating
entries. A consumed SQE is not a released payload. Payload and timespec storage
remain alive until the corresponding terminal event is consumed.

The supplied probe submits an empty-pipe read with generation1/slot3, cancels it,
and accepts either ordering of cancel-success and target-ECANCELED. After terminal
consumption, canceling the old ID must return ENOENT. A generation2 operation in
the same slot must survive an old-generation cancel and read exactly K. A linked
5ms timeout cancels an empty-pipe read: expect target-ECANCELED and timeout-ETIME
with distinct identities. A multishot poll must produce MORE for two separately
drained events; cancellation must eventually yield one terminal target CQE
without MORE. Additional event CQEs before terminal are allowed. The table bound
is an experiment watchdog, not a kernel guarantee of a fixed cancellation cost.

Also fill a nonblocking pipe, release4096bytes, then write two3000byte vectors
separated by a zero-length vector. Verify positive partial progress and exact
prefix ordering. Splice synthetic bytes between own pipes, verify source drain
and destination equality; this does not measure internal copies. Queue saturation
and repeated ring-wrap cycles must preserve every NOP identity.

D1: adapter and mapping/unwind proof; D2: operation ownership table and exact
CQE transcript; D3: independent progress-prefix oracle; D4: generated-code and
dependency audit plus same tests under changed compiler/optimization. Run
`python3 tools/run_b06_io_completion.py`. Require three profiles, full reference
success, both mutants rejected (queue over-admission and lost MORE), and compiled
stub rejection at B06_IO_CHECK. IO1 maps T62.01.a/b/c to D3; IO2 maps T62.03.a/b/c
and T62.04.a/b/c to D1/D2; IO3 maps T62.05.a/b/c to D2; IO4 maps T62.08.a/b/c to
D4. Level0 releases on cancel acknowledgment or assumes CQ order; level1 only
ordinary completion; level2 supplies these invariants and exact observed results;
level3 independently revises ownership for a changed opcode or multiple issuers.

Hints: name the target and cancel request separately; distinguish MORE from a
successful result; list storage whose lifetime extends past SQ consumption.
Delayed variants add multi-producer publication, completion racing cancellation,
EALREADY and direct-I/O alignment fallback. EALREADY must still await the target
terminal event; the current actual schedule does not force that result. Filesystem
power-loss and cross-directory durability remain separate. Assessor analysis is
`assessor/linux_io_depth/targets/ANALYSIS.md`. No network service, credentials,
host configuration changes or privileged operation is involved. Current receipt
is `io_completion_validation.json`; finite schedules are not exhaustive proof.

Inspected reference bindings: R-B06-FOLLOWUP-URING-SETUP, R-B06-FOLLOWUP-URING-ENTER, R-B06-FOLLOWUP-URING-CANCEL.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.
