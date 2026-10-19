# T57 guest continuation: ownership, removal and execution context

These required continuations retain PR-T57-01–08 and the B06 shared contract at
`curriculum/v5/contracts/b06.md`. Families B/C/D/E/F/G/I/J apply. Prerequisites
are the learner kernel_lifecycle portable core, its acquisition ledger, RCU versus
reference-count argument, and the T57 entry recall. C17 is the implementation
baseline; Linux7.0.14 uses its kernel C dialect and APIs, not ISO C threading.

## Concrete target and environment

Implement two original guest modules against the exact headers/config in the
runner receipt. The supplied `probe.c` is learner-visible; implementations remain
in assessor storage. The owner device is `/dev/cm_b06_owner`, mode0600. A write
contains version1/2, payload length0..32, flags and reserved byte, then exactly
that payload. Version1 flags are zero; version2 permits bit0 as ignored metadata.
Reserved bits/lengths reject. Copy faults reject the entire record before
publication. Reads return six native-endian u32 values: writes, work executions,
payload size, closed, hardirq-timer observations, first payload byte or0. This is
a named x86 Linux teaching ABI, not a portable serialized ABI. ioctl0xB060
requires CAP_SYS_ADMIN and unpublishes/revokes the device; old fds retain storage
but writes reject ENODEV. The module owner blocks unload while handles remain.

The second module creates its own software IRQ descriptor, one primary handler,
one threaded handler and a managed device payload. A hardirq hrtimer triggers
the original descriptor once. Task and threaded contexts may take the mutex and
allocate GFP_KERNEL; the primary handler records its event and wakes the thread.
On the debug-kernel negative variant, `might_sleep()` intentionally diagnoses
the hardirq context. It does not deliberately block, corrupt memory, or exercise
a physical interrupt controller. The corrected run must have no such diagnostic.

## Reasoning and deliverables

D1: provide the module sources, exact kernel config, build commands, module map,
and a ledger for registry, open and accepted-work references. RCU protects lookup
storage until the attempt to acquire a reference; neither RCU nor kref replaces
the other. D2: specify close linearization under the object mutex, prevent every
new enqueue after it, cancel/drain accepted work, cancel the timer, wait for RCU
readers, then release registry ownership. Account separately for a canceled
pending-work reference and a running callback reference. D3: explain why staged
user-copy failure is atomic and why version2 flags cannot grant authority. D4:
implement managed cleanup after allocation and after IRQ publication failures;
quiesce callbacks before device-managed memory is released. A self-owned IRQ
descriptor pins the module while requested: a one-shot exercise must free the
IRQ before returning from initialization, or expose a separate explicit close
operation before unload. An exit callback cannot break its own entry condition.
D5: retain process/thread/hardirq context traces, actual copied-byte fault,
4000 successful writes, concurrent remove/open/write attempts,100reloads and
held-fd unload refusal. Analyze lock ordering, callback stack/resource bounds,
failure cleanup and the difference between finite observations and a proof.

## Verification and assessment

Run `python3 tools/run_b06_kernel_guest.py --debug-kernel --report
research/v5/expansion/b06/kernel_debug_validation.json` in WSL. Provision/build
the private debug kernel with `tools/build_b06_debug_kernel.py`; this never
installs a host kernel. Require KASAN, LOCKDEP, PROVE_LOCKING and
DEBUG_ATOMIC_SLEEP in the actual configuration. Corrected stages must have no
BUG/WARNING/KASAN/atomic-context reports. The separately marked negative stage
must diagnose an invalid sleeping context. Require exact terminal markers and
balanced allocated/freed counts, not just QEMU exit0.

| Criterion | Objectives | Level2 evidence | Level3 independent transfer |
|---|---|---|---|
| KG1 | T57.01.a/b/c, T57.04.a/b/c | failure-stage cleanup and real three-context trace | replace managed payload lifetime and explain IRQ-owner pin cycle |
| KG2 | T57.02.a/b/c, T57.07.a/b/c | protected-page copy fault preserves record; untrusted flags fail to authorize | change ABI extent and preserve atomicity without copying unchecked sizes |
| KG3 | T57.03.a/b/c, T57.05.a/b/c, T57.06.a/b/c | explicit RCU/kref/enqueue/removal proof plus concurrent traces | introduce registry replacement and distinguish old-reader versus new-owner lifetimes |
| KG4 | T57.08.a/b/c | instrumented lifecycle, held fd and concurrent removal results | independently diagnose an unseen teardown ordering defect after seven days |

Level0 is unsafe code or unsupported claims; level1 is ordinary success without
failure/lifetime reasoning. Hints: first enumerate each reference producer;
then mark the no-new-work point; finally classify every callback context before
choosing a lock or allocation mode. Separate assessor analysis is
`assessor/kernel_lifecycle/guest/ANALYSIS.md`.

## Scope and status

Only original modules, synthetic bytes, and a diskless networkless disposable
QEMU guest are authorized. No host insmod, deployed device, credentials or
external target. The debug negative condition is an expected local diagnostic,
not privilege escalation. Current author receipts are source-bound; learner,
external review, real device IRQ and physical validation remain separate. The
replacement-reader epoch variant below complements the actual two-entry registry.
New workload has no historical-hour ceiling.

Inspected reference bindings: R-B06-FOLLOWUP-KREF, R-B06-FOLLOWUP-WORK, R-B06-FOLLOWUP-IRQ-SOURCE.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.

## Required lookup, generation and replacement transfers

KG5 (PR-T57-03/05/06, every a/b/c objective): extend the registry to two entries,
keys0/1, with one owned reference per published entry. Open uses key0 and the
module parameter `mutex_lookup` selects a mutex-protected or an RCU-protected
lookup followed by get-unless-zero. Both must acquire lifetime before releasing
lookup protection. ioctl0xB062 takes four native-endian u32 words: key, mode0RCU
or1mutex, output generation, output closed. It requires trusted CAP_SYS_ADMIN;
invalid key/mode rejects EINVAL, missing entry ENODEV, copy fault EFAULT.

ioctl0xB061 publishes a newly allocated empty key0 object and closes the former
one under registry/object locks. On allocation or exhausted generation it leaves
the registry unchanged. Generations start1; closure increments, and the new
object inherits that next generation. Refuse replacement at UINT_MAX-1 to retain
one final close generation without wrapping. Existing fds retain the closed old
object, whose writes reject ENODEV. The replacement has fresh counters and no
armed timer; key1 is unchanged. Capture the generation when submitting work,
require equality and open admission before self-requeue, and drain after closure.
Never wait for the worker while holding its mutex. The generation is an identity
check, not a substitute for locking, references or synchronous cancellation.

D6: implement both paths and submit a comparison of lock contention, read-side
restrictions, acquisition lifetime, and reclamation costs. Run `probe replace`
for64 replacements with20000 concurrent lookups, old-fd reads and work quiescence;
repeat with both open modes. Count balanced destruction only after all fds close
and RCU callbacks complete. A completed grace period does not revoke an fd.

D7: implement `epoch.h` from initial `{.current=1}` using serialized calls and
eight reader IDs. Enter stores current epoch in an unused ID; leave clears an
occupied ID. Replacement retires current epoch, increments it, and rejects a
second outstanding retirement, closed state or UINT32_MAX. Reclaim succeeds only
when the retired epoch has no reader, leaving newer readers untouched. Close
is idempotent, increments once, and refuses UINT32_MAX unchanged. Requeue succeeds
only for an open state and the exact captured current generation. All rejected
transitions preserve state. The live struct, outputs and IDs follow `epoch.h`;
no allocation or target synchronization primitive is permitted in this model.
Its O(8) scan and fixed storage make the starvation counterexample inspectable.

Run `python3 tools/run_b06_epoch.py`. Require every old-reader position, held new
reader,64 successive new-epoch entrants, stale work and wrap refusal; the
wait-for-all-readers mutant must fail although it avoids early free. Also kill
the stale-generation mutant and reject the unsolved scaffold. Explain why this
model is an argument about admission and epochs, not an implementation of Linux
RCU ordering. Level2 KG5 requires both real registry paths, all semantic controls
and reference conservation; level3 requires an unseen replacement/cancellation
schedule and proof that every successful requeue precedes its close point.
Level0 releases live storage or wraps identity; level1 merely passes a serial
happy path. Hints: separate pointer publication from final ownership; identify
the oldest relevant epoch; test a new reader that deliberately remains active.
After seven days independently change to multiple simultaneous retired epochs,
specify a bounded reclamation queue and reject capacity exhaustion atomically.
This delayed assessment remains unperformed by author testing.
