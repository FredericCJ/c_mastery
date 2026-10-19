"""Reviewed clause-specific evidence selection; never infer closure from topic count."""
PROVIDERS={n:'research/v5/expansion/'+p for n,p in {
'debug':'legacy_debugging_followthrough/support.json','memory':'legacy_memory_followthrough/support.json','vm':'legacy_vm_verifier/completion_dispositions.json','storage':'legacy_storage_followthrough/durability_completion_dispositions.json','os':'legacy_os_followthrough/support.json','mcu':'legacy_mcu_followthrough/support.json','wait':'legacy_concurrency_wait/support.json','once':'legacy_concurrency_once/support.json','response':'legacy_realtime_response/support.json','shoot':'legacy_smp_shootdown/support.json','amp':'legacy_amp_followthrough/support.json','grace':'legacy_advanced_grace/completion_dispositions.json'}.items()}
def ref(provider,project,*objectives):return [(provider,'AX-'+project+'-'+o) for o in objectives]
def c(text,*checks,providers=()):return dict(rationale=text,checks=list(checks),providers=list(providers))
CLAUSES={
'X-T43-001':{
'R1':c('Actual s_paired signed differences and lower median,32 consumed workload pairs with raw times and rank-count oracle.','b045_systems paired','paired execute','bootstrap'),
'R2':c('Outlier fixture reverses mean sign while lower median remains -8; percentile resampling is explicitly conditional on dependence/stationarity premises, never WCET.','b045_systems paired','bootstrap'),
'V1':c('Seed7449826 balanced shuffled16AB/16BA; actual equal-sum separately compiled calls, warmup16, MONOTONIC clock resolution and separate order medians.','paired execute','bootstrap','retained benchmark calls'),
'V2':c('1999 paired resamples, multiply-high index mapping, lower median and explicit percentile rounding; independent frequency-rank expectations and exact small binomial distributions.','bootstrap')},
'X-T44-001':{
'R1':c('Exact original failure transition, PASS/FAIL/UNRESOLVED child replay identity; twelve actual mixed FAIL/PASS pairs retain UNRESOLVED.','b045_systems reduce',providers=ref('debug','PR-T44-02','T44.03.a')),
'R2':c('Actual ordered subset/complement reducer and bounded budget/cache; actual grammar frontend deletes whole balanced children before running original C parser.','b045_systems reduce',providers=ref('debug','PR-T44-02','T44.03.b')),
'R3':c('Every remaining single deletion checked; selected candidate may be1-minimal while another failure has lower cardinality; same failure is not causal repair.','b045_systems reduce',providers=ref('debug','PR-T44-02','T44.03.c')),
'V1':c('Retained executable nested A/N tree reducer reaches [[A]], enumerates every whole-child neighbor and keeps token versus grammar neighborhoods distinct.',providers=ref('debug','PR-T44-02','T44.03.b','T44.03.c')),
'V2':c('Actual controlled alternating fresh children exercise repeated classification; first-only acceptance is refused. Finite agreement cannot prove deterministic replay.','@actual repeated mixed replay')},
'X-T45-001':{
'R1':c('Original B4 failed nonzero resize retains old accounting; successful native moved resize allocates/copies while old object is still live.','b04_depth memory','transfers execute',providers=ref('memory','PR-T45-07','T45.07.c')),
'R2':c('Invalid release modeled using integer identities; generation-qualified aliases, original allocator identity and single release checked without invalid pointer evaluation.','b04_depth memory','transfers rows execute'),
'V1':c('Native MwOwner preserves two aliases after refusal and invalidates both after forced move; new bounded ledger checks generation and retires exhausted slots.','transfers rows execute',providers=ref('memory','PR-T45-07','T45.07.c')),
'V2':c('2560 allocation/resize/owner/allocator combinations; transfer moves attribution while preserving total live bytes and allocator; duplicate owner release rejected.','transfers rows execute')},
'X-T46-001':{
'R1':c('Original all-record validation followed by reachable height worklist; actual CVM decoder marks every variable-width instruction including unreachable code.','b05_depth verify',providers=ref('vm','PR-T46-02','T46.02.a')),
'R2':c('Conflicting join and late backedge heights reject rather than keep first predecessor; nearest-valid paired programs retained.','b05_depth verify',providers=ref('vm','PR-T46-02','T46.02.b')),
'R3':c('Accepted balanced loop still reaches FUEL and accepted tag/aggregate-stack examples reach TYPE/STACK_LIMIT on actual unchanged VM.','b05_depth verify',providers=ref('vm','PR-T46-02','T46.02.c')),
'V1':c('Actual CVM widths and boundary map reject branch into immediate operand; SPLIT lowering relocates real boundaries and executes both arms.',providers=ref('vm','PR-T46-02','T46.02.a','T46.02.b')),
'V2':c('Actual checked function descriptors derive call arity, return height, local range and function boundaries; cross-function branch and malformed/valid arity pairs execute.',providers=ref('vm','PR-T46-02','T46.02.b','T46.02.c'))},
'X-T47-001':{
'R1':c('Original committed-prefix checksum/generation validation stages result; actual CST2 raw plus structural prepublication guard preserves homes on rejection.','b05_depth recover',providers=ref('storage','PR-T47-09','T47.09.a')),
'R2':c('Uncommitted suffix terminates replay; committed malformed record rejects unchanged. Actual malformed committed pages and selected short reads preserve image.','b05_depth recover',providers=ref('storage','PR-T47-09','T47.09.c')),
'V1':c('Actual CST2 full32-page after-images survive37x37 interrupted recoveries, identical homes and byte-idempotent reopen; poisoned/indeterminate outcomes retained.',providers=ref('storage','PR-T47-09','T47.09.b')+ref('storage','PR-T47-07','T47.07.b')),
'V2':c('Canonical32-byte domain/store/generation/value/commit framing, opaque synchronous MAC callback and failure-atomic publication; actual existing HMAC provider checked independently. Trust/key/rollback/truncation limits explicit.','authentication execute','MAC oracle')},
'X-T48-001':{
'R1':c('Original checked extent64-address before pointer indexing, present/user/write permission preflight and all-or-nothing publication.','b05_depth copy','transfers oracle'),
'R2':c('2376 address/count/page schedules compare every synthetic page byte; rejected copy preserves unrelated kernel/peer bytes; callback remap effects distinguished.','b05_depth copy','transfers oracle'),
'V1':c('Snapshot versions and staged actual copy, callback remap then touched-version recheck before publication under external exclusion; generation exhaustion refuses.','transfers execute','transfers oracle'),
'V2':c('Explicit copied/status pair with exact confirmed prefix, cross-page fault and out-of-range0 count; resumption begins after returned prefix.','transfers execute','transfers oracle')},
'X-T49-001':{
'R1':c('Unique bounded resident set, FIFO birth/LRU recency/OPT future-distance/CLOCK hand-bit state; future information remains offline premise.','b045_systems replace',providers=ref('os','PR-T49-06','T49.06.a')),
'R2':c('Actual four-policy trace adapter compares CLOCK predecessor and FIFO/LRU/OPT on identical references; per-step victims/live/dirty sets checked independently.','b045_systems replace',providers=ref('os','PR-T49-06','T49.06.b')),
'R3':c('FIFO anomaly9/10 at3/4frames with every eviction and42648 LRU stack-prefix checks; totals alone are insufficient.','b045_systems replace',providers=ref('os','PR-T49-06','T49.06.b','T49.06.c')),
'V1':c('Equal-cost farthest-future exchange argument and LRU prefix induction; no assertion that online policies know future references.',providers=ref('os','PR-T49-06','T49.06.c')),
'V2':c('Dirty writecost10/no final flush: FIFO29 versus miss-optimal OPT37; independent cost optimum17 proves changed objective.',providers=ref('os','PR-T49-06','T49.06.c-DELAYED'))},
'X-T50-001':{
'R1':c('Clock sampling/horizon and timer ownership are separate from queue policy; modular elapsed check cannot discover an unseen complete wrap.','b05_depth timer',providers=ref('mcu','PR-T50-05','T50.05.a')),
'R2':c('Rollover expiry before/at/after threshold and exact half-range refusal; hidden full-wrap histories retain information limit.','b05_depth timer',providers=ref('mcu','PR-T50-05','T50.05.a')),
'V1':c('Actual16-slot queue orders equal deadlines by insertion sequence, returns generation tickets, rejects stale cancellation and retires exhausted identities.',providers=ref('mcu','PR-T50-05','T50.05.b')),
'V2':c('Actual phase-preserving10,20,30 versus delay-after-work10,23,39; phase30/period10/now57 gives next60/skipped2 using bounded division.',providers=ref('mcu','PR-T50-05','T50.05.c'))},
'X-T51-001':{
'R1':c('Predicate/enrollment/runnable metadata share an actual exclusion domain; native mutex and target saved-mask are explicitly different mechanisms.','b045_systems wait',providers=ref('wait','PR-T51-01','T51.01.a')),
'R2':c('Actual protected CMX recheck/enrollment and explicit ownership, stable registration/generation and one outstanding node.','b045_systems wait',providers=ref('wait','PR-T51-01','T51.01.b')),
'R3':c('Omitted-enrollment control exposes lost wake; notification only creates eligibility, predicate recheck remains required after spurious wake.','b045_systems wait',providers=ref('wait','PR-T51-01','T51.01.c')),
'V1':c('Actual two-waiter FIFO/credit accounting and163840 oracle transitions; each linked node becomes runnable exactly once under protected arbitration.',providers=ref('wait','PR-T51-13','T51.13.c')),
'V2':c('All24 ready/timeout/cancel/close orders, current task/object/operation generations, first terminal winner and stale callbacks preserve successor ownership.',providers=ref('wait','PR-T51-13','T51.13.a','T51.13.b'))},
'X-T51-002':{
'R1':c('Actual MB owner-specific state transitions and once adapter separate attempts, publications and effects; stable FAILED differs from retry EMPTY.','modern_bridge once','once execute',providers=ref('once','PR-T51-11','T51.11.a')),
'R2':c('Actual CMX blocking enrollment/poll/cancellation and eight-entry/checksum484 permanent publication;11 failure cuts include entry3, released waiters and source-bound M4 execution.','modern_bridge once',providers=ref('once','PR-T51-11','T51.11.b')),
'R3':c('Three actual native contenders, stable delayed readers and policy comparisons. Registered Rust OnceLock/Go Once panic rules remain source comparisons, not executions or implicit rollback.','modern_bridge once',providers=ref('once','PR-T51-11','T51.11.c')),
'DELAYED':c('Sixty owner/cut/policy combinations: journal append before publication, retained exact ID/tag dedup,2retry attempts/1publication/1effect and permanent1attempt; conflict/overflow refusal.','once execute')},
'X-T52-001':{
'R1':c('Fixed-priority preemptive single CPU independent non-self-suspending jobs; cost/blocking/jitter/overhead are explicit modeled inputs, not measured WCET.','b05_depth response',providers=ref('response','PR-T52-03','T52.03.a')),
'R2':c('Checked original bounded ceiling iteration and full-width successor statuses; no floor, overflow wrap or iteration-limit success.','b05_depth response','response oracle',providers=ref('response','PR-T52-03','T52.03.b')),
'R3':c('Independent direct-integer ceiling/term comparison;1728 switch-cost cases plus retained full-width oracle; sufficient rejection is distinct from concrete deadline miss.','response oracle',providers=ref('response','PR-T52-03','T52.03.c')),
'V1':c('Charge each job a conservative dispatch/return pair2s exactly once; recurrence3,6,9,12,12 and D11 rejection; omitted/double counting controls.','response execute','response oracle'),
'V2':c('Concrete one-job execute[0,1),suspend[1,5),execute[5,6) missesD3 although originalC2 bound2 passed; no general suspended-task theorem inferred.','response execute')},
'X-T53-001':{
'R1':c('Unpublish/invalidate/ACK/free authority and frozen target membership; actual StWorld entry flush preserves snapshot premise.','b045_systems shoot',providers=ref('shoot','PR-T53-09','T53.09.a')),
'R2':c('Exact generation-qualified ACK, retained pending frames and nonwrapping generations; actual guest invalidation checked separately.','b045_systems shoot',providers=ref('shoot','PR-T53-09','T53.09.b')),
'R3':c('Early reclaim changes synthetic OLD frame label toNEW; actual two-/four-CPU guest separately demonstrates stale A999/B200 and permission faults.','b045_systems shoot',providers=ref('shoot','PR-T53-09','T53.09.c')),
'V1':c('Actual adjacent4page batch versus4single requests, per-CPU space/extent binding and actual equal-page guest comparison.',providers=ref('shoot','PR-T53-09','T53.09.c')),
'V2':c('All256 target masks preserve pending on offline request; trusted final execution/context quiescence or exact ACK permits reclamation; stale incarnation cannot drop rejoined CPU. Model only, no native hotplug claim.','transfers execute','transfers rows execute')},
'X-T54-001':{
'R1':c('Original epoch/sequence identity and trusted quiescence distinction; actual survivor receipt separates transport from application session.','b045_systems peer',providers=ref('amp','PR-T54-08','T54.08.b')),
'R2':c('One interrupted request remains uncertain until separately established outcome; actual queued ledger retains old epoch and requires peer plus device quiescence.','b045_systems peer',providers=ref('amp','PR-T54-08','T54.08.c')),
'R3':c('Old ACK(1,1) cannot close new(2,1); unknown effect is not absent effect, full reset persistence unclaimed.','b045_systems peer',providers=ref('amp','PR-T54-08','T54.08.b')),
'V1':c('Actual one-entry AfReceipt deduplicates latest same-session ID/payload after before-effect/after-effect/lost-reply cuts; outside horizon returns stale/unknown, survivor ledger retains unresolved work.',providers=ref('amp','PR-T54-08','T54.08.a','T54.08.c')),
'V2':c('Actual H745 supervisor/pipeline progress watchdog, park/device-quiet/new-ready route and separate hashed images; AN386 executes same task pipeline. Dedicated-board reset/heartbeat traces remain unobserved.',providers=ref('amp','PR-T54-08','T54.08.c'))},
'X-T55-001':{
'R1':c('Captured-ticket grace ownership protects old readers without requiring all future readers stop; permanent stalls bound progress separately.','b05_depth grace',providers=ref('grace','PR-T26-02','T26.07.a','T26.07.b')),
'R2':c('Actual immutable AGStore publication retains the old version until its captured reader leaves while a new-version reader remains; serialization and borrowed-reference premises explicit.','b05_depth grace',providers=ref('grace','PR-T26-02','T26.08.a')),
'V1':c('Actual immutable17-version store and two-writer mutex adapter reject competing/unowned writes; old/newversion readers and retained callback references.','b05_depth grace',providers=ref('grace','PR-T26-02','T26.08.a','T26.08.c')),
'V2':c('Equal three-node first-retirement pools and identical native/M4 pause schedule retain1 hazard versus3 epoch, or2 with adjacent protections; retained old reader is never forcibly cleared.','reclamation execute','threads execute')},
'X-T55-002':{
'R1':c('Original MB publication/removal/retirement/protection/collection and no-prevalidation-payload argument; new stable-domain container enforces lease/section refs.','modern_bridge hazard','reclamation execute'),
'R2':c('Original10 SC partial orders plus40 actual gate-serialized container histories, both-reader scans, observed-link revalidation, native pthread/TSan and actual M4 contexts.','modern_bridge hazard','reclamation oracle','threads execute'),
'R3':c('Nine hazard generations then FULL; participant generations1..3 never reused; stalled hazard retains only protected nodes. Gate BUSY and pool exhaustion are not lock-free guarantees.','modern_bridge hazard','reclamation execute','threads execute'),
'DELAYED':c('Depart requires an inactive participant; stale registration cannot clear its successor; same3-payload pool and pauses compared in native and actual M4 runs; epoch one-use policy kept distinct.','reclamation execute','threads execute')},
'X-T55-003':{
'R1':c('Actual serialized MBEpoch composed under acquire/release gate; entry section precedes access, removal precedes retirement, active departure forbidden.','modern_bridge epoch','reclamation execute'),
'R2':c('Same append/read/remove container and three-node capacity; actual two-reader CMX and native pthread contexts, gate-history oracle, hazard retains1 versus epoch3 under identical pause.','reclamation oracle','threads execute'),
'R3':c('Distance0/1 cannot collect; current cohort allows one advance then blocks; permanent epoch3 FULL, active-owner retention and finite-pool failure distinct from progress.','modern_bridge epoch','reclamation execute'),
'DELAYED':c('Long scan leaves and invalidates all references then reacquires; old reader1/new cohort1 after wrap is ambiguous, retain proved permanent exhaustion policy; equal-pool hazard comparison.','reclamation execute','threads execute')}
}
