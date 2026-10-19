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
must reject both target translation units with ordinary host compilation. Transfer
the two digested binaries by the already authorized local guest mechanism and
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
path. It does **not** demonstrate a signal interrupting a blocked close. A signal
investigation requires a separately observed blocked request, non-restarting
handler, controlled unblock and retained ownership; until implemented/executed it
must remain a distinct trigger limitation, not inferred from the numeric errno.

Supply `identity.json` with `os`, `release`, `image_sha256`, `sdk_packages`,
`compiler_version`, `guest_command`, `probe_sha256`, and `manager_sha256`, alongside
the actual build report and unedited logs. Invoke `capture.py` with `--build`,
`--probe-log`, `--manager-log`, `--identity`, `--exit-code`, and `--output` paths.
It checks the expected ordered observations and binary binding, but cannot itself
prove an operator's provenance claims or establish independent assessment.
