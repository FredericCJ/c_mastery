# P2 exact writer assessment analysis

## T10.07.b

The correct normal offset trace is0,3,3,5, with remaining9,6,6,4. The EINTR
fixture reports no accepted bytes; a positive short return does advance the
prefix. `cpw_step` invokes at most once, including zero progress. The actual
pipe experiment repeats three EAGAIN/readiness cycles for both raw and libc
adapters; each ready call accepts exactly three bytes because the adapter caps
its request, not because Linux guarantees short writes of that size. The
short/retry mutants discriminate accepted bytes from requested bytes. Actual
asynchronous EINTR is not claimed by this packet.

## T23.04.c

The implementation invokes actual `in_drain` and retains its exact `struct in_transfer`.
The wrapper adds terminal-state admission rather than relying on an implicit
retry as recovery. Queue count falls2→1→0, while pending65 and66 survive their
respective retry/error. Accepted65 must not be requeued: the compiled control
really produces65,66,65. The copied-span transaction stages a ring value whose
representation is self-contained, validates all byte values, then commits both
owners. Accepted3 survives failure; recovery sends positions3..8. `struct cwf_span`
cannot be relocated by raw structure copy while active because its nested
transfer borrows its own `bytes` array. That address-stability precondition is
essential; require the learner to identify it in the dossier.

## T23.04.b

Raw-2 is ENOENT, teaching-2 is retry, libc-1 requires captured errno. Mixing
them changes both retry policy and causal detail. Actual raw/libc pipe calls
observe EAGAIN, successful transfer and EBADF; value injection separately
tests EINTR before progress. Hosted `poll` readiness does not reserve capacity
for another concurrent writer, and this harness deliberately owns both pipe
ends exclusively. Strict images contain only the selected queue/transfer call
graph, explicit entry and byte helpers; libc diagnostics do not enter them.

## T21.02.b

`cwf_error` copies category/operation/accepted/first/cleanup without calling any
writer or logger. The logging-enabled and logging-absent binaries preserve the
same core result. Cleanup9 never replaces primary5; repeat cleanup10 leaves9.
Actual `er_transfer` returns a progress product whose code alone is insufficient
to retain backend cause, so `struct cpx_progress` carries the extra record. Equal trace
and callbacks justify this bounded comparison. Its static conditional-branch
sites and actual structure sizes are not a universal speed ranking. There is
no runtime frequency claim or automatic ISO C ownership enforcement.

All exact original/delayed tasks remain source-bound in the successor mapping.
Do not award a learner or independent-review pass from this author analysis.
