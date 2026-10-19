# Actual QNX8 build and capture protocol

This step is required evidence for the alternate-OS variant, currently **not built
and not executed**. The host seam is not its substitute. QNX's current installation
guide requires a valid myQNX account and deployed SDP8 license; the QEMU quick-start
image has the same prerequisite. No SDK, sysroot, guest image or license is supplied
by this repository. Do not obtain an unauthorized image or change host security.

Use an authorized SDP8 environment and a disposable private QNX8 x86-64 guest.
Record SDK package versions, compiler output, guest image digest and exact local
launch command. Provision through the cited vendor process; licensing/account
acceptance is an external dependency, not a command hidden in this laboratory.
In that environment run:

```sh
python3 qnx_build.py --source /absolute/path/to/completed/close_owner.c --out qnx-build
```

The supplied producer uses `qcc -Vgcc_ntox86_64 -std=c17 -D_QNX_SOURCE`; it records
failures instead of falling back to Linux headers. `-pthread` is omitted because
QNX8's qcc documentation explicitly calls it unnecessary. The `__QNXNTO__` guard
must reject all four target translation units with ordinary host compilation. Transfer
the four digested binaries by the already authorized local guest mechanism and
verify their hashes inside the guest. No command in this exercise contacts a third
party service or downloads a proprietary image.

Start `manager` inside the disposable guest and capture stdout/stderr. It registers
one fresh `/dev/cmastery-close-<pid>` name with mode0600, using the guest's explicitly
permitted PATHSPACE and PUBLIC_CHANNEL abilities. It does not grant itself permissions; failure to attach
is recorded. Run `probe` once with the exact READY path. No other client, signal
sender, descriptor duplication or concurrent opener is admitted in this experiment.
The manager is single-threaded. Real client `_IO_CLOSE` request1 returns EINTR
before default cleanup; request2 invokes `iofunc_close_dup_default`. Synthesized
rcvid0 disconnect cleanup always uses the default handler. Stop the owned manager
after capturing both results, and discard the guest.

The probe calls real libc `close`, records its ownership state, verifies the still
open descriptor and matching fstat identity, explicitly retries once, then checks
EBADF without any intervening descriptor allocation. These are **expected** results,
not observations from this repository. Framework behavior or compile failures must
be investigated against the actual SDK, and retained as failures. Do not change a
failed result into a passing host-seam run.

The controlled resource-manager return tests the actual close error/descriptor
path. It does **not** demonstrate a signal interrupting a blocked close. Capture
this pair using `--mode injected`; keep its logs separate from the following pair.

## Separate blocked-close signal experiment

`qnx_signal_manager.c` and `qnx_signal_probe.c` supply the separate source-level
experiment. They are authored from the current QNX8 interfaces registered in
`resources.json`; they remain **not SDK-compiled and not guest-executed** here.
The build emits `signal_manager` and `signal_probe`. Start a fresh signal manager
and record its `QC_SIGNAL_READY:/dev/cmastery-signal-close-<pid>` path. Run exactly
one original signal probe against that path, under the same guest user identity.
No other client or signal sender is admitted. Both binaries operate entirely in
the disposable guest; the manager never accepts a user-supplied signal target.

The probe installs a SIGUSR1 handler with flags0, explicitly unmasks SIGUSR1, and
disables cancellation before acquiring the descriptor. The handler only sets a
`volatile sig_atomic_t`; it never closes, logs, allocates or changes ownership.
QNX8 documents return from this handler and the channel-unblock exception; this
experiment does not assume Linux `SA_RESTART` behavior or require that macro.

The first received `_IO_CLOSE` is retained without default close cleanup or a
reply. Receipt establishes REPLY-blocked state in the documented message model.
The manager records `rcvid_t`, OCB and kernel-provided process/thread/connection
identities; `SignalKill(0,pid,tid,SIGUSR1,SI_USER,0)` targets only that exact original
client thread. It then returns `_RESMGR_NOREPLY`. This needs no sleep or guess that
the client probably reached `close`. The library's `_NTO_CHF_UNBLOCK` channel
causes an actual kernel unblock pulse while the signal remains pending.

The I/O unblock callback calls `iofunc_unblock_default` first. Only its
`_RESMGR_DEFAULT` result permits the custom search. The callback checks the saved
rcvid and OCB, current `MsgInfo` identities and `_NTO_MI_UNBLOCK_REQ` before
`MsgError(rcvid,-1)`. This selects the kernel's stored interruption reason; it does
not inject numeric EINTR. Calling that function with -1 outside an unblock request
would violate its API contract. Unmatched or stale pulses receive no reply;
synthetic disconnect clears a matching retained pointer before default cleanup.
One dispatch thread serializes this state. This laboratory does not use file
locks, duplicate descriptors, timeouts, multiple clients or concurrent handlers.

Require the manager trace in this order: `held`, `sent-exact-thread`,
`unblock-pending` with matching identities, `kernel-error-replied`, `default-close`.
Require the probe's installed-handler marker, caught1 plus retained EINTR and live
matching descriptor, then the explicit second close's success/EBADF observation.
The final default-close marker belongs to that second request, not the interrupted
first request. The capture checker binds the manager's client pid to the probe.
Cross-process wall-clock ordering alone is not evidence; kernel request identity,
single-server order and the client result establish the proposed causal chain.

Capture this run with `--mode signal` and the signal pair's binary hashes in the
identity record. An injected trace, missing handler, missing kernel-pending flag
observation, reused request identity or missing reply must reject. Failed signal
permission, unexpected framework dispatch, build error, missing pulse or a hung
client is a failed/blocked experiment. Use the authorized guest console to stop
these two original processes and discard the guest; a watchdog terminating them
does not demonstrate successful close or cancellation cleanup. Never change host
security or signal an external process to obtain a result.

The unavailable licensed SDK/guest prevents checking target headers, framework
dispatch and libc outcomes. No missing API declaration was papered over with host
shims: all APIs and signatures above have current QNX8 primary passages. Actual
compilation, any resulting repair and both target captures remain required. The
source and protocol are no longer deferred merely because target execution is
blocked, and no synthetic capture used by a checker self-test counts as execution.

Supply `identity.json` with `os`, `release`, `image_sha256`, `sdk_packages`,
`compiler_version`, `guest_command`, `probe_sha256`, and `manager_sha256`, alongside
the actual build report and unedited logs. Invoke `capture.py` with `--mode`, `--build`,
`--probe-log`, `--manager-log`, `--identity`, `--exit-code`, and `--output` paths.
It checks the expected ordered observations and binary binding, but cannot itself
prove an operator's provenance claims or establish independent assessment.
