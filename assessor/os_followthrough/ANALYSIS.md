# T49 assessor derivations

Author observations do not establish independent or delayed learner performance. Hosted Linux,finite models,source comparisons and owned guest observations have separate scopes.

## AX-PR-T49-01-T49.01.a

Original task: Draw write's ISO C call, libc wrapper, selected Linux machine ABI, kernel copy and device boundaries. Classify raw -9 versus wrapper -1/errno9, and specify which successful calls leave errno uninterpretable.

The C expression calls a declared interface; write is not an ISO C library primitive. libc maps the Linux raw negative error to -1/errno; successful errno is uninterpretable. Kernel copying and device completion are later boundaries. The actual raw invalid descriptor gives -9 and both wrappers give -1/errno9.

Changed condition: Move the wrapper to a second ABI and derive argument/error/clobber changes before reuse.

AArch64 changes the trap to SVC, number to64 inX8 and arguments/result toX0–X2/X0. Both O0/O2 static freestanding images execute under qemu-aarch64 with actual disassembly and empty undefined-symbol lists; this is a second machine ABI observation, not a physical ARM device test.

Evidence selectors: gcc-O2-syscalls, arm64-O0, arm64-O2, arm64-O2-assembly.

## AX-PR-T49-01-T49.01.b

Original task: Implement a bounded original raw write wrapper for the actual selected Linux ABI; compare it with libc on a disposable pipe and invalid descriptor. Inspect disassembly and record rcx/r11/memory or the selected ABI's actual clobbers, retries and partial progress.

The pipe contains exactly abcabc from raw and libc writes. The actual x86-64 code and assembly retain RAX/RDI/RSI/RDX plus RCX/R11,memory,flags effects at O0/O2. A separate invalid-pointer request returns EFAULT before user-buffer access.

Changed condition: Use a nonblocking pipe with less free capacity than the requested transfer.

Filling a nonblocking pipe, draining one page and requesting two pages produces a real positive short count and then EAGAIN. The accepted prefix and remaining count agree between raw and libc routes; strace retains the actual syscall results.

Evidence selectors: gcc-O0-syscall-assembly, clang-O2-syscall-assembly, gcc-O2-syscall-trace, clang-O2-syscalls.

## AX-PR-T49-01-T49.01.c

Original task: Inject EINTR before progress, short positive progress, EAGAIN and terminal failure in a deterministic transfer adapter, then correlate at least one real short-transfer trace. Retain done and remaining lengths after every step.

The deterministic raw sequence -4,2,-11,2,-5 for wanted7 gives done0,2,2,4,4 and remaining7,5,5,3,3. Retryable no-progress states preserve the prefix; terminal error retains done4. PTRDIFF_MIN refuses before negation or unsigned conversion. Real strace short writes supply separate correlation.

Changed condition: Change the destination to a record-oriented contract and explain why byte-stream recovery no longer suffices.

With a record contract, a partial positive count records the actual consumed prefix and ends in protocol error71. It cannot be silently retried as a byte-stream suffix; the recipient protocol must define an atomic record/reassembly mechanism.

Evidence selectors: gcc-O2-core, clang-O2-core, gcc-O2-syscall-trace, gcc-O2-io.

## AX-PR-T49-02-T49.02.a

Original task: Build an ownership graph separating process address spaces, MAP_SHARED objects, open-file descriptions, thread stacks and join/wait records. Explain fork's private counter versus explicit shared counter and reference sequence2,3,2,1,0.

Separate private address spaces from MAP_SHARED storage and shared open-file descriptions. Private7 remains7 in the parent while the child writes9; shared42 is observed after publication/wait. The declared abstract references2,3,2,1,0 reach zero only at step5, independent of thread stacks/join records.

Changed condition: Open a descriptor before fork and distinguish shared offset from private memory.

The actual pre-fork descriptor shares its open-file-description offset: after baseline5/5/4, the child reads f and parent offset becomes6. Private counter memory still diverges. A shared offset is not evidence of shared private objects.

Evidence selectors: certificates, gcc-O2-lifecycle, gcc-O2-descriptors.

## AX-PR-T49-02-T49.02.b

Original task: Implement the real process/thread tree with bounded counters and child reports. Force failure after the first successful worker creation, join exactly created workers and reap all children; keep argument storage alive until completion.

Two actual workers contribute2000 atomic increments and are joined. Injected EAGAIN after one creation produces1000 and exactly one join; only successful creations acquire ownership. All actual children are reaped and final waitpid reports ECHILD.

Changed condition: Move worker arguments from static to scoped storage and prove the scope encloses every join.

Argument records live in a parent local array enclosing every join. Each worker returns its own record address; joining verifies that identity before the scope ends. Moving them into a helper that returns before join would invalidate their lifetime; the corrected scope is exercised at O0/O2.

Evidence selectors: gcc-O0-lifecycle, gcc-O2-lifecycle, clang-O2-lifecycle, gcc-memcheck-lifecycle.

## AX-PR-T49-02-T49.02.c

Original task: Compare thread exit, process exit, child failure before result publication and duplicated stdio buffering around fork. Use explicit reports and exit statuses; diagnose a deliberately missing close that blocks EOF.

pthread_exit returns one worker result while other work and the process continue. Child exit7 before publication gives an empty report with an explicit failure status. Unflushed pre-fork stdio yields prefixprefix12; the repaired pre-fork flush yields prefix6. The inherited-writer handshake produces EAGAIN until its last close, then EOF.

Changed condition: Replace the result pipe with shared memory plus explicit synchronization and revisit visibility.

MAP_SHARED plus a process-shared semaphore publishes value42 before observation. sem_post/sem_wait provides the handoff; waitpid supplies lifecycle completion. Merely sharing storage without this ordering would not establish when the parent may read the result.

Evidence selectors: gcc-O2-lifecycle, gcc-O2-descriptors, clang-O2-lifecycle.

## AX-PR-T49-03-T49.03.a

Original task: Specify total event order for arrivals, completion, RR requeue and dispatch. Define positive service/quantum, maximum16 jobs/256 ticks and rejection output semantics; derive conservation and waiting identities.

Inputs are bounded16/256 with positive service/quantum. Refusal leaves the complete output unchanged. Completion, arrivals ordered by time/index, expired RR requeue, and selection form the declared total boundary order. For zero overhead, finish-arrival=waiting+service and first-arrival=response.

Changed condition: Add explicit dispatch cost and revise identities rather than silently charging it as service.

The successor emits -2 dispatch ticks and reserves its chosen job through them. Waiting includes charged overhead; queued=waiting-dispatch_charge. The independent oracle and a second trace scan verify service, idle, overhead, first/finish and dispatch counts under nonzero costs.

Evidence selectors: gcc-O2-core, gcc-oracle, clang-oracle.

## AX-PR-T49-03-T49.03.b

Original task: Implement FCFS,SJF,SRTF,RR and compare against an independent tick oracle over small arrival/service sets. For A(0,8),B(1,2),C(2,1), derive every dispatch; include idle gaps, singleton and invalid zero quantum.

FCFS,SJF,SRTF,RR are compared on19680 full tick traces against an independent deque oracle, including empty/singleton/idle cases. For A0/8 B1/2 C2/1, SRTF selects A,B,B,C,A… and finishes11/3/4 with first0/1/3. Service totals remain eleven.

Changed condition: Use simultaneous arrivals and a job completing exactly at a quantum boundary.

Exhaustive arrival/service sets include simultaneous arrivals, positive quantums1/2/4 and completion exactly at quantum boundaries. Completion removes the job before any RR requeue, while arrivals at the same time precede a surviving expired job.

Evidence selectors: gcc-oracle, clang-oracle, gcc-O2-core.

## AX-PR-T49-03-T49.03.c

Original task: Compute response, waiting, turnaround and dispatch counts from the emitted trace with a second implementation. Mutate the tie break and idle accounting; show which metric catches each defect.

Metrics are recomputed from the trace by a second implementation, not copied from reference fields. OF_TIE_BAD reverses equal-arrival SJF selection and is killed by the first service slot. OF_IDLE_BAD charges prearrival idle to waiting and is killed by a job arriving at2; fixed witnesses pass.

Changed condition: Compare quantum choices with nonzero switch overhead and report the fairness/throughput tradeoff.

For A/B/C with dispatch cost1, RR quantum1/2/4/8 yields total ticks17/15/15/14 and dispatches6/4/4/3. First response differs as shown in complete oracle traces; faster completion of the whole workload need not improve the late short job response.

Evidence selectors: gcc-mutant-fixed, gcc-mutant-tie, gcc-mutant-idle, clang-mutant-tie, clang-mutant-idle, gcc-oracle.

## AX-PR-T49-04-T49.04.a

Original task: Derive MLFQ residual-allotment and stride-pass invariants. Distinguish module-required yields/sleep accounting from the retained CPU-only SPEC; define those added transitions before implementation.

MLFQ retains used allotment on yield and demotes before a boundary yield can erase it. Levels1/2/4 and boosts every16 are state transitions distinct from the retained CPU-only provider. Stride normalizes all passes by the active minimum before overflow and preserves order.

Changed condition: Add sleep/wakeup with bounded credit and analyze whether it rewards a yielding hog.

A sleep at20 for30 ticks excludes the worker through49; wakeup retains its residual MLFQ allotment, while stride clamps stale credit to the active minimum. This is a bounded-credit policy, not an entitlement to unlimited catch-up. Full per-tick level/used traces accompany the finite sleep/yield examples.

Evidence selectors: gcc-O2-core, gcc-oracle, clang-oracle.

## AX-PR-T49-04-T49.04.b

Original task: Add explicit yield events to a three-level1/2/4-budget MLFQ with boost every16 ticks. Compare a hog and a task yielding just before demotion; separately derive the scale28 stride fixture service2,4,8 and the executable floor(1024/weight) policy.

The fixed yield model differs from a mutant that resets used allotment: repeated short yields cannot avoid the same accumulated demotion threshold. Exact scale28 with weights1/2/4 yields2/4/8 over14 ticks. Floor1024 arithmetic is separately named, and independent stride calculations compare128 weight/scale/seed/length/change combinations.

Changed condition: Change weight mid-run and preserve a declared pass-credit policy without overflow.

A boundary weight change retains accumulated pass and changes only future increments. Independent arbitrary-precision Python arithmetic checks the actual C timeline/final passes near UINT64_MAX, including normalization before addition rather than wraparound ordering.

Evidence selectors: gcc-O2-core, gcc-oracle, clang-oracle.

## AX-PR-T49-04-T49.04.c

Original task: Compare the teaching policies with inspected Linux EEVDF lag, eligibility and virtual-deadline rules. Record the observed kernel/source revision and give a sleeping-task counterexample to resetting all negative lag.

The selected7.0.14 source uses weighted eligibility and earliest virtual deadline with additional buddy/protected-slice paths; it is not MLFQ or this simple stride loop. With V10, vruntime12/8 and deadlines13/20, task0 has lag-2 and is ineligible, so task1 wins; resetting sleeping task0 to zero lag incorrectly lets it win.

Changed condition: Change the selected Linux release and identify which source observations must be renewed.

The inspected Linux6.6 tag uses min_deadline augmentation and immediate dequeue bookkeeping, versus selected7.0.14 min_vruntime augmentation and delayed dequeue. Renew eligibility precision, lag placement, protection, dequeue/wakeup, features/configuration and actual runtime identity on release change. Linux6.6 is source comparison only; host6.18.33.1 and guest7.0.14 are independently identified.

Evidence selectors: certificates, host-kernel, gcc-guest-run.

## AX-PR-T49-05-T49.05.a

Original task: Derive page-reference and working-set sets for addresses0,4095,4096,8191,12288,0 at4096-byte pages. Separate requested bytes, distinct touched pages, RSS, minor/major faults and physical I/O.

At4096 bytes the references are0,0,1,1,3,0 and unique set{0,1,3}. Requested byte counts, pages,RSS,fault counters and device traffic have different units/scopes; shared-zero/COW/huge-page behavior prevents a universal conversion.

Changed condition: Use equal total bytes with interleaved windows and compare working sets by window.

Nine equal one-byte requests grouped as000111333 give one-page windows, while013013013 gives three-page windows under the same window width3 and same total unique pages. The certificate computes each window rather than equating total bytes to working-set size.

Evidence selectors: certificates, gcc-O2-locality.

## AX-PR-T49-05-T49.05.b

Original task: Build sequential,strided and fixed-seed randomized touch phases with observable checksums. Retain allocation/touch timestamps, page size, mapping mode, huge-page context and before/after resource counters; inspect optimized code for retained accesses.

Fresh private anonymous mappings are touched sequentially,with coprime17 stride,and with seeded permutations. Every retained access contributes to a verified checksum. Rows bind allocation/touch timestamps,page size,AnonHugePages,minor/major deltas,CPU/wall and maxRSS; optimized of_touch disassembly is retained.

Changed condition: Compare read-only first touch with write first touch and explain differing allocation paths.

Read-first mappings return zero-byte values through the kernel-selected zero-page/allocation path; write-first materializes dirty private pages with the expected nonzero checksum. Actual fault/RSS/huge context is observed per phase, not asserted equal across environments.

Evidence selectors: gcc-O2-locality, clang-O2-locality, gcc-O2-touch-assembly, clang-O2-touch-assembly, host-environment.

## AX-PR-T49-05-T49.05.c

Original task: Repeat runs with randomized phase order and record environmental interference. Explain a warm-file run with zero major faults but many logical reads; document when cold-cache control is unavailable without changing global host settings.

Nine repetitions randomize the three pattern positions; each first-touch mode has its own distribution. Guest warm file reads retain logical read totals and actual major-fault deltas; zero major faults does not mean zero read traffic. Host exclusive scheduling and cold-cache control remain explicitly unobserved.

Changed condition: Introduce a bounded competing reader in a disposable environment and compare uncertainty intervals.

A bounded concurrent reader repeatedly reads an original unlinked file, with checksum-verified byte count capped512MiB. Baseline and competing groups each retain nine sample min/median/max ranges per pattern/write mode. Overlap or differences are descriptive uncertainty, not a controlled causal proof.

Evidence selectors: gcc-O2-locality-summary, clang-O2-locality-summary, gcc-guest-observations, host-environment.

## AX-PR-T49-06-T49.06.a

Original task: Define FIFO queue, LRU recency order, CLOCK hand/reference bits and optimal future-distance state over page IDs. Specify capacity0 refusal and empty-trace behavior; prove resident uniqueness/capacity after each transition.

FIFO birth order,LRU last touch,CLOCK bits/hand and OPT future distance are distinct states. Each hit preserves membership; each full miss replaces exactly one slot; live masks enforce capacity and residents remain unique. Capacity0 refuses without publication; empty trace succeeds.

Changed condition: Add dirty state without conflating writeback cost and fault count.

Dirty bits remain attached to resident identities and clear on replacement before a new write marks the slot. Dirty eviction increments writeback separately from faults. No final flush is counted, and that premise is part of the cost contract.

Evidence selectors: gcc-O2-core, gcc-oracle, clang-oracle.

## AX-PR-T49-06-T49.06.b

Original task: Implement all four policies, retaining the existing CLOCK anchor and adding independent FIFO/LRU/optimal implementations. Print per-reference hits, victims and resident sets for the supplied anomaly string at three/four frames.

All four actual policies emit complete per-reference hit,victim,resident and dirty sets for the anomaly at3/4 frames. FIFO gives9/10. New CLOCK counts are compared directly with the retained CLOCK anchor; independent Python transitions compare every row.

Changed condition: Use exhaustive short strings over three page IDs and compare to an independent optimal oracle.

All page strings of lengths0..7 over three IDs and capacities1..3 produce39360 policy runs. OPT miss counts are compared with a separate recursive dynamic-programming optimum; online policies never receive future-distance data.

Evidence selectors: gcc-oracle, clang-oracle, gcc-O2-core.

## AX-PR-T49-06-T49.06.c

Original task: Prove the LRU stack-inclusion relation under identical deterministic history and provide FIFO's violating witness. Compare optimal fault bounds and dirty writeback counts under equal capacity/correctness contracts.

LRU capacities are prefixes of one recency stack. A hit moves the same page to the front; a miss adds it and truncates each prefix, preserving inclusion inductively. The finite check independently verifies42648 prefix inclusions. FIFO9/10 supplies the non-stack counterexample; farthest-next-use has the standard equal-cost exchange argument.

Changed condition: Assign unequal read/write costs and show miss-optimal need not minimize total cost.

With dirty writes every third reference and writeback cost10, FIFO has9 misses+2 writes=29 while miss-optimal OPT has7+3=37. The independent cost optimum is17 with no final flush. Miss minimization therefore does not minimize this changed cost objective.

Evidence selectors: gcc-oracle, clang-oracle.

## AX-PR-T49-07-T49.07.a

Original task: Model descriptor integers separately from shared open-file descriptions. Trace open a,dup b,reads3+2,separate open c/read4, then close and numeric reuse; identify which flags and offsets belong to which layer.

Actual reads give shared offsets a5/b5 and independent c4. O_NONBLOCK follows the description; FD_CLOEXEC follows the descriptor. After close,the same integer is reused for /dev/null,demonstrating why numeric equality cannot validate lifetime.

Changed condition: Add a descriptor transfer and extend the reference ledger before using it.

SCM_RIGHTS sends another reference over an owned Unix socketpair. recvmsg with MSG_CMSG_CLOEXEC yields a private descriptor flag but a shared description; reading g advances the original description to7 after the fork read advanced it to6. All transferred endpoints are closed.

Evidence selectors: gcc-O2-descriptors, clang-O2-descriptors.

## AX-PR-T49-07-T49.07.b

Original task: Implement the temporary-file/dup/fork/pipe experiment with deterministic handshakes. Close every unused inherited writer and show EOF only after the last writer closes; inject setup failure after each acquisition.

The deterministic child handshake holds one inherited writer,so the parent sees EAGAIN rather than EOF until release and last close. Failure after each of five acquisition stages closes its owned prefix,unblocks/reaps any child and restores the descriptor count.

Changed condition: Use close-on-exec at creation and verify the executed child cannot inherit the private endpoint.

pipe2 O_CLOEXEC establishes the private endpoint atomically. An exec self-check fails for the deliberate inheritance control and succeeds for close-on-exec; it inspects the actual child descriptor table after exec,not just parent flags.

Evidence selectors: gcc-O0-descriptors, clang-O2-descriptors, gcc-memcheck-descriptors.

## AX-PR-T49-07-T49.07.c

Original task: Compare Linux close error/blocked-I/O behavior with the selected POSIX edition. Reproduce numeric reuse in a bounded synthetic syscall adapter and show why blindly retrying Linux close can close a replacement resource; mark exact POSIX2024 text unavailable until inspected.

The inspected Linux close manual separates descriptor invalidation from an I/O-held description reference and warns against retry after errors. In the synthetic Linux adapter,close reports EINTR after invalidation,the number is reused for object2,and a blind retry closes object2. Exact POSIX2024 text remains unavailable; its comparison is attributed to the manual.

Changed condition: Port the cleanup adapter to a platform retaining the descriptor after EINTR and revise ownership transitions.

The alternate adapter explicitly retains object1 after EINTR and requires retry while ownership is still held. Both policies have different release transitions; one generic retry loop cannot be called portable without the selected platform/edition contract.

Evidence selectors: gcc-O2-io, clang-O2-io, gcc-O2-descriptors.

## AX-PR-T49-08-T49.08.a

Original task: Separate namespace views, resource budgets, readiness and completion in a four-column contract. For generation4 events(3,12),(4,7),(4,8), derive one terminal acceptance and stale/duplicate rejection.

The four-column certificate separates namespace visibility,resource budgets,readiness hints and terminal completion identity. At generation4,the stale(3,12) rejects,request7 completes once,and(4,8) rejects. The retained MwAsync provider also holds pending/active callback and submit-return leases.

Changed condition: Add cancellation before a late completion and preserve generation identity.

Cancellation before the late completion is accepted as pending; reclaim remains busy. Terminal delivery still occurs for the same ticket,and only after callback completion plus submit return can reclaim advance the generation.

Evidence selectors: certificates, gcc-O2-completion, clang-O2-completion.

## AX-PR-T49-08-T49.08.b

Original task: Build a nonblocking poll reader with bounded partial-record storage; force another reader to consume readiness before this one reads, inject EINTR and partial EOF, and preserve EAGAIN as a retry state. Observe only permitted disposable namespace views.

A second descriptor reader steals a polled byte; the actual nonblocking operation returns EAGAIN. The bounded8-byte record accumulates4+4 exactly once,and a2-byte partial EOF rejects. Atomic unmask through ppoll produces an actual EINTR. Owned guest PID1 and mount namespace identities differ while a private mounted file stays absent in the parent.

Changed condition: Deliver synthetic completions out of order while real pipe reads remain separate.

Two synthetic requests complete in reverse submission order with identity-preserving callbacks. Those modeled completions are labeled separately from real pipe progress and real namespace observations; neither substitutes for physical DMA delivery.

Evidence selectors: gcc-O2-readiness, clang-O2-readiness, gcc-O2-completion, gcc-guest-observations.

## AX-PR-T49-08-T49.08.c

Original task: Diagnose early buffer reuse and duplicate completion in the bounded request model, then test corrected ownership with stale generations and exhaustion. Compare the observation scope to the actual self-pipe probe.

Reclaim is rejected before terminal delivery,during pending callback,during active callback and before submit returns. Stale and duplicate tickets cannot release the next request. Maximum generation retires the slot rather than wrapping. Actual self-pipe notification is only readiness evidence.

Changed condition: Compose completion delivery with a task shutdown protocol and require quiescence before reclaiming context.

Shutdown stops admission,cancels the two live operations,delivers the second before the first and waits for zero live slots/callbacks/scratch leases before context reclamation. Reclaim of the remaining first operation is refused after only the second finishes.

Evidence selectors: gcc-O2-completion, clang-O2-completion, gcc-O2-readiness.

## AX-PR-T49-09-T49.09.a

Original task: Derive the finite quota fixture:20000 per100000 over five complete periods. State phase alignment, burst, hierarchy and scheduler assumptions; distinguish service ceiling from guaranteed latency or core ownership.

Five complete aligned periods20000/100000 give100000 service ceiling only under the no-burst,independent-period,single-CPU,always-runnable and no-tighter-parent model. It is neither guaranteed service nor latency/core ownership.

Changed condition: Start measurement midway through a period and derive the possible boundary contribution.

A500000-duration window starting50000 into a period intersects six budget intervals. Each boundary half can contain the full20000,so the declared model admits120000. Tighter hierarchy and burst settings require a different bound; real counters are not forced to equal the model.

Evidence selectors: certificates, gcc-guest-observations.

## AX-PR-T49-09-T49.09.b

Original task: Author a bounded delegated-subtree experiment with an external supervisor, baseline/CPU/memory trials, fixed workload limits, recorded ancestry and cleanup. Capture cpu.stat,memory.events,exit status and actual limits; execute only in an already authorized disposable environment.

The fresh guest acknowledges cgroup membership before release; the supervisor remains root outside its children. Ungrouped,unlimited,CPU and memory runs retain limits,raw counters,status/signals,placement and successful removal. The memory worker alone can be killed; host controllers are never changed.

Changed condition: Add two weighted children beneath a tighter parent quota.

Two same-CPU children with weights100/400 run beneath parent quota30000/100000. Both child and parent snapshots show the hierarchy and actual service. Finite ratios are observations,not guaranteed exact4:1 shares or exclusive CPU ownership.

Evidence selectors: gcc-guest-observations, clang-guest-observations.

## AX-PR-T49-09-T49.09.c

Original task: Analyze throttled intervals, memory.current versus RSS and allocation failure versus killed process using trial traces. Include a deliberately wrong supervisor-inside-budget diagram and repair its ownership/control placement.

Actual CPU trials pair throttled counter growth with reduced worker CPU time; memory trials pair SIGKILL with oom_kill growth. A distinct worker-only32MiB RLIMIT_AS trial returns malloc ENOMEM and exit12 without an OOM kill. memory.current includes group charges unlike process RSS. A supervisor inside the exhausted group could lose recovery/control,so the corrected ownership graph keeps it outside.

Changed condition: Change ancestry while keeping child limits constant and predict the new confounder.

The child remains20000/100000 while parent changes max to10000/100000. Both ancestry snapshots and parent throttle counters are retained; lower observed service is consistent with the tighter ancestor. Instruction speed and other timing effects are not inferred from throughput alone.

Evidence selectors: certificates, gcc-guest-observations, clang-guest-observations.

## AX-PR-T49-10-T49.10.a

Original task: Build separate byte ledgers for application reads/writes, cached traffic, device traffic and acknowledged durable prefix. Derive writes7168,reads2048,prefix6144 from the fixture without treating the final1024 as necessarily lost.

Actual short-callback writes and fsync boundaries reproduce7168 accepted writes,2048 requested reads and6144 acknowledged prefix. The trailing1024 is uncertain rather than necessarily lost. Cache/device ledgers remain distinct and QEMU acknowledgment does not establish physical persistence.

Changed condition: Add overlapping rewrites and explain why byte count differs from final file size.

A1024-byte overlap rewrite raises application accepted bytes to8192 while final file size remains7168. Rewriting a previously acknowledged range also requires renewed persistence reasoning for that range; cumulative write count is not durable unique content.

Evidence selectors: certificates, gcc-O2-io, clang-O2-io.

## AX-PR-T49-10-T49.10.b

Original task: Implement bounded disposable-file generate/write/fsync/read phases with short-I/O and ENOSPC injection. Retain device identity, placement, raw io.stat when available, CPU/wall times and free-space guard; keep controller trials within authorized disposable scope.

The original host adapter checks full bytes under short113 callbacks,EINTR/EAGAIN. The private guest checks contents,free-space guard,device identity,CPU/wall,io.max/io.stat and actual2MiB tmpfs ENOSPC with accepted2097152/remainder2097152. Every original temporary file and cgroup is removed.

Changed condition: Add a concurrent bounded reader and preserve separate per-process and device observations.

Concurrent bounded readers each report their own4194304 logical bytes using independent opens. Device io.stat belongs to the shared cgroup/device scope,and /proc/self/io belongs to the reporting process; these are retained separately and cannot be assigned to one reader without further evidence.

Evidence selectors: gcc-O2-io, gcc-guest-observations, clang-guest-observations.

## AX-PR-T49-10-T49.10.c

Original task: Compare warm-cache and explicit-fsync trials with paired ordering and uncertainty. Explain a device-byte count exceeding or below application bytes using only supported mechanisms; preserve unresolved alternatives.

Three paired order-varied buffered/fdatasync trials retain min/median/max wall times with complete checksums. Warm reads may have zero major faults and device bytes below logical traffic. Metadata,writeback ownership and timing windows remain possible explanations for other discrepancies; the finite samples do not prove a unique cause.

Changed condition: Change filesystem/backend and identify which prior results and assumptions must be discarded.

The same content contract is rerun on tmpfs after ext4. Its device identity,memory-backed storage and absent equivalent block durability invalidate transfer of ext4 I/O-controller/persistence conclusions. The changed backend is recorded explicitly rather than averaging incompatible trials.

Evidence selectors: gcc-guest-observations, clang-guest-observations.

