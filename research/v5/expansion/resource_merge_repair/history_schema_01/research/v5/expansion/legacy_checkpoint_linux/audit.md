# Exact P5 successor audit

Starting evidence: eight ordinary and eight V1 tasks in PR-P5-01/-02; original
FAsync/in_shutdown anchor; actual CPAQueue, CPLReader and CPOperation reference
providers; public async/linux/operation tests; original shared framing contract.
Those providers already implement substantially more than the synchronous anchor.
They are retained, not recreated or relabelled as new learner achievements.

Missing discriminators found by reading actual sources: old fd reuse used slot7
without forcing descriptor7; competing consumers did not deterministically force
the post-wake predicate-loss schedule; copied versus shared mutable workload lacked
an executed paired implementation; cleanup order lacked an explicit per-transition
trace; the real callback adapter did not import the original FAsync contract. New
private fixtures and public bridge contract close these precise gaps. Allocation
failure/128-byte copy, LT/ET resumption, graceful/abort, retirement and reporter
reentry were already meaningful and are actually rerun against unchanged providers.

The bridge uses actual local poll/read/wake-pipe/pthread execution. Its ack gate,
thread-creation failure and late-close failures are declared injection seams. The
descriptor reuse is real; EIO/EINTR are injected outcomes. No target outside the
original local processes is involved. A construction-failure inspection found
that destruction initially could retain queued work forever without a successful
retry; the final implementation explicitly abandons never-submitted work once
worker absence/join is known. No historical provider was changed.

Initial GCC bridge/waiter pilots rejected misleading-indentation under Werror;
statements were separated without suppressing diagnostics. Reports remain in
build/p5-bridge-pilot and build/p5-owner-pilot. The first full pilot at
build/p5-full-pilot.json passed255 commands/127 checks before report-order and
construction-abandon/TSan additions. It is historical pilot evidence only. The
current report must pin the final sources and exact matrix; no earlier pilot is
used to close the final packet.

Inspected source scope is explicit in resources.json/source_access.json. Actual
Issue8 condition-wait body was read from the prior legitimate primary capture
after the browser returned403; the capture title/version was checked. Linux6.19
epoll/close passages were read, not merely cited. No published ISO edition or
physical target observation is invented. Learner artifacts and seven-day attempts
remain external prerequisites even when author-side references pass.
