"""Exact retained criterion-to-concrete author evidence, not learner acceptance."""
BINDINGS={
'T23.02.a':('reap','reap','Retain unchanged prior launcher/error-channel evidence and add owned-child hard-wait recovery, actual ECHILD authority loss and original signal-termination transfer.'),
'T23.01.a':('process','process','Private/shared address-space and two-thread identity observations; fresh exec image retains selected identity/context and replaces image state.'),
'T23.01.b':('process','process','Explicit argv/env/cwd/limits reporter and missing-configuration allowlist result, no PATH search.'),
'T23.01.c':('process','analysis','Interface authority table with actual private-memory/offset observations; declared non-Linux port analysis names replacement/preconditions and source gaps.'),
'T23.02.c':('process','process','Prepared restricted child path while two workers and copied locked mutex exist; selected GNU spawn actions/failure comparison.'),
'T23.03.a':('process','process','Actual duplicated description offsets/status and independent second-open transfer.'),
'T23.14.a':('capability','capability','Version/optional-support authority inventory and actual equivalent single-pipe poll/epoll controls.'),
'T23.14.b':('features','capability','Actual clean feature macro diagnostics and separately modeled older-kernel/changed-epoch capability errors.'),
'T23.14.c':('capability','capability','Explicit per-epoch choice, failed/weaker fallback rejection and partial nonidempotent progress classification.'),
'T23.12.a':('storage','storage','Actual buffering/visibility/sync calls with injected EIO and pipe synchronization rejection.'),
'T23.12.b':('storage','storage','Original stable-lock replacement protocol, transition failure records, old-inode split and synthetic uncooperative writer.'),
'T23.12.c':('storage','analysis','Actual owned-writer process cuts contrasted with logical fixture; exact delayed VM/block-fault and independent recovery-oracle design, no host-storage fault execution.'),
'T23.13.a':('timing','timing','Monotonic absolute-deadline/CPU-time observations plus explicit wall-jump and active/suspend-aware clock models.'),
'T23.13.b':('timing','timing','Read-only settings, child-only lowered resource limit, EPERM/EINVAL request adapter and changed CPU availability model.'),
'T23.13.c':('timing','models','Raw planned/wake/dispatch/completion observations and independent synthetic interference counterexample; no scheduling bound.')}
ARTIFACTS=['labs/posix_context_depth/CONTRACTS.md','labs/posix_context_depth/PORTABILITY.md','labs/posix_context_depth/PERSISTENCE_EXPERIMENT.md','assessor/posix_context_depth/ANALYSIS.md']
def checks(commands,kind):
 if kind=='analysis':return sorted(x for x in commands if x=='models' or x.endswith('.process') or x.endswith('.storage'))
 if kind=='features':return sorted(x for x in commands if x.startswith('feature.') or x.endswith('.capability'))
 if kind=='models':return ['models']
 return sorted(x for x in commands if x.endswith('.'+kind) and '.learner.' not in x and '.mutation.' not in x)
