"""Mechanism-specific interpretation of the observed T75 cost dossier.

This data is authored analysis, never fabricated execution output. The caller
attaches exact raw report records and current source identities.
"""

DIMENSIONS = ['Representation', 'Reachable storage', 'Lifetime',
              'Mutation/aliasing', 'Allocation', 'Movement/copying',
              'Destruction', 'Dispatch', 'Concurrency', 'Failure',
              'Compilation', 'Binary/stack', 'Boundary']

# Representation and the two build dimensions are generated from actual report
# records; the ten remaining entries are mechanism-specific contract arguments.
DETAILS = {
1: [
    'Each cell retains its immutable parent and allocator context. The explicit maximum depth is8; a borrower is an additional reference, not a copied cell.',
    'Create retains the parent only after successful admission; creator release does not acknowledge work. Final borrower acknowledgment can trigger the bounded parent-chain release.',
    'The cell address is stable. Copying a pointer does not retain it; a caller must already own a live reference before retain/borrow. This is a caller rule, not an ISO C affine type.',
    'One requested sizeof(scope) allocation per successful create. Each injected failure preserves output and parent reference count; allocator-internal overhead and blocking are outside the count.',
    'No control-object move is permitted. The duration/transit certificate is16 octets and contains no process pointer; rebasing creates a new local deadline rather than copying an absolute timestamp.',
    'Release traverses at most the admitted depth8; allocator callbacks may block. Reference exhaustion refuses before mutation, and missed acknowledgments intentionally retain storage.',
    'GCC O3 object inspection observes lock cmpxchg plus indirect allocator/disposer calls. Atomic reference operations protect the counter; they do not establish safe acquisition from a dangling pointer.',
    'Cancellation publication precedes borrower acknowledgment. Two actual paused pthread workers retain storage after creator release; bounded schedules/TSan are distinct from the arithmetic deadline proof.',
    'Allocation/depth/reference refusal leaves the requested output unchanged. Cancellation wins the stated deadline tie but never rolls back an already committed effect.',
    'The16-octet duration certificate assumes equal-rate clocks and bounded transit; repeated hop-relative timeout resets and unbounded transit violate the end-to-end argument.'] ,
2: [
    'The queue owns at most four values. Pump credit partitions QUEUED/TRANSPORT/LEASED/retired slots, plus two registered producer records and borrowed struct msg_hub/scope objects.',
    'A BLOCK/REJECT offer retains caller ownership; DROP/OVERWRITE/DEGRADE have explicit consumed-value rules. A transport receive does not return credit until the exact lease is released.',
    'Owned policy queue operations require external serialization; pump operations use its mutex. Borrowed callback contexts and hub must outlive close, drain, producer collection and joined callers.',
    'The pump itself uses caller-provided storage. Payload allocation belongs to the supplied producer; policy-specific rejection/discard invokes or preserves the supplied disposer exactly as contracted.',
    'Copied queues move bytes; owned queues move descriptors. struct msg_hub carries the slot identity. Forwarding is transactional on actual accepted channel send and does not duplicate the payload owner.',
    'Close/drain visits at most four values and two producer records. Disposer and joined-worker latency is unbounded by this finite count; held leases prevent destruction.',
    'The observed pressure object calls actual struct msg_hub, pthread operations and indirect value disposers; direct policy branches implement the five different admission contracts.',
    'Admission, registered waits, close and lease return share the mutex. Actual native blocked producers demonstrate the wait-for cycle; forwarding alone cannot create missing end-to-end credit.',
    'Closed/rejected/stale/exhausted transitions preserve or consume ownership exactly once. Generation MAX retires the slot instead of allowing an old lease to return a new credit.',
    'The lease includes finite slot/generation identity and is local to the owning pump. Neither a serialized integer nor a copied pointer grants remote ownership or process-safe synchronization.'],
3: [
    'Sixteen nodes retain full due timestamps, stable sequence IDs, generations, list links or heap positions. The wheel also retains eight fine and eight coarse bucket heads.',
    'Schedule publishes one live handle; cancel/expiry removes it and changes generation or retires the slot. Reinitialization requires old handles to be discarded under the caller contract.',
    'All three structures are externally serialized and controls may not be byte-copied while live. Callback-free returned event descriptors do not borrow internal node addresses.',
    'No dynamic allocation occurs in these timer cores. Full capacity, sequence exhaustion and invalid horizon refuse before changing output or structure.',
    'Heap reordering moves internal indexes, preserving public slot/generation identity. Wheel cascade relinks nodes without changing their original due time; late processing reports lateness explicitly.',
    'Cancel is O(1) for the indexed list/wheel and O(log N) for the indexed heap. Expiry/cascade performs work proportional to visited buckets/nodes and stable-order bookkeeping; a coincident burst is not constant work.',
    'Direct structure operations appear in each compiled timers object; actual disassembly and undefined symbols include stack-protector support in the observed hosted build. No timer callback is invoked inside these cores.',
    'The producer/catch-up test serializes admission at observed100 while processing remains0. Native exclusion proves only that schedule; retained CMX is compared only for common due/cancel outcomes.',
    'Handle, sequence and timestamp boundaries reject rather than wrap. Mutants lose stable order or accept bucket coincidence without checking the full due timestamp.',
    'Tick units and maximum admitted horizon are explicit. A host timing sample or CMX logical tick is not an MCU interrupt-latency/WCET measurement; no stronger CMX event identity is inferred.'],
4: [
    'One fixed bucket stores integer tokens, fractional remainder, rational rate, capacity and last time. Paired admission stages two whole bucket states before publishing either debit.',
    'Initialization establishes a valid denominator and bounded credit. Migration exports exact credit; FREEZE or trusted-duration REFILL explicitly determines how time away is treated.',
    'Callers serialize each bucket and jointly serialize paired admission. Copying a migration snapshot does not retire the source or establish exclusive issuer authority.',
    'No heap allocation in refill/admit/migrate. Temporary staged state is automatic; malformed state and overflow checks are performed before arithmetic that would lose the invariant.',
    'A bounded state copy is used for transactional paired debit and migration. Requests do not copy payloads. Remainders survive idle splitting; truncating each refill changes the accepted language.',
    'No resource destructor is needed for the arithmetic value. Closing an associated queue or pending request is a different owner obligation, not implied by discarding bucket state.',
    'The inspected optimized object uses unsigned division with no undefined external symbols in this build. This observation is specific to the host compiler/data model; it is not a helper-free target claim.',
    'The bucket supplies admission arithmetic, not mutexes, fairness or waiting. Tests compare exact Fraction models and joint debit; a concurrency limiter imposes a different in-flight bound.',
    'Denied work publishes elapsed-time refill but spends no token. Time reversal/malformed input refuse transactionally. The time-to-full bound prevents an overflowing rate-times-time product.',
    'A duration, not a foreign absolute clock reading, authorizes migration refill. Authenticated/durable handoff is external. The burst-plus-rate bound assumes the declared monotonic time scale.'],
5: [
    'Retry state retains one real struct net_stack flight, inherited scope, checked attempt/deadline state and finite PRNG draw state. The struct net_stack is larger than the retry controller itself.',
    'Beginning a flight borrows the scope. A refusal to schedule more attempts does not end the outstanding transport; finish requires ACK processing or explicit reset before returning the borrow.',
    'The retained struct net_stack and retry control are serialized, stable and not copied while active. A retry does not independently own the effect resource; idempotency is a receiver contract.',
    'Retry planning and retained transport state use caller storage. The enclosing scope/response allocator is separately counted; retry amplification is a work count even when no new heap block is requested.',
    'Each dispatch serializes the same18 application bytes through the actual retained transport. It is another backend attempt, not an ownership transfer of the logical request.',
    'At most the configured attempts are considered, but safe finish can wait indefinitely for an uncooperative backend unless reset establishes quiescence. Cleanup does not follow from reaching the retry cap.',
    'Direct retry calls compose actual net_receive/retransmission APIs and scope/bucket operations. The measured object and link map bind the real imported transport, not a replacement integer model.',
    'Scope cancellation stops new work while outstanding transport lifetime remains active. Independent populations test correlated versus independent jitter seeds; no random schedule proves absence of overload.',
    'Exponential/deadline arithmetic and64-draw rejection bound refuse before publishing an attempt. Retry-After equality with the deadline is rejected, while a later ACK can still complete the old flight.',
    'The established-flight fixture omits handshake/congestion conformance. Byte ACK parsing is actual, but network delay/loss is synthetic. PRNG rejection sampling is not cryptographic unpredictability.'],
6: [
    'The gate retains finite tenant reservations, active permits, epoch/generation identity and bounded health-window observations. Old permit cleanup authority survives a new health epoch.',
    'Admission reserves active cost before work; complete releases that exact permit once. Closing asks for cancellation but cannot destroy a permit owner before completion acknowledgment.',
    'Core gate calls are externally serialized; native service/hedge use their declared mutex. An integer permit copied to another gate is not a valid owner; control identity remains a precondition.',
    'The gate uses fixed caller storage. Backend allocations are external and remain owned while the permit is active; an allocation failure is an outcome, not implicit permission to skip cleanup.',
    'Health records and permit descriptors are bounded copies. Reclassification of a stale completion may not overwrite a current epoch observation or move tenant cost to another owner.',
    'Finite permit enumeration bounds local bookkeeping, not backend completion time. A backend that never acknowledges can keep cost reserved indefinitely; force-free would violate lifetime.',
    'Compiled direct gate operations implement policy-specific window scans and checked cost arithmetic. The object audit separates this code from the larger mutex/decoder service wrapper.',
    'Both completion orders for two permits are tested. Neutral cancellation and backend health observations are distinct; slot/epoch validation prevents an old result publishing into half-open recovery.',
    'Full tenant reservation refuses even if another tenant is idle. Cooldown/epoch/counter exhaustion retires safely; stale or double completion must not restore capacity twice.',
    'The tenant and cost model is local admission policy, not remote authentication or scheduler fairness. Different consecutive/window rules are compared as different health contracts.'],
7: [
    'The arbiter retains two attempt tickets, owned response descriptors, callback counts, request generation, gate permit and scope borrow. Result visibility can precede losing-owner quiescence.',
    'Start grants exact attempt ownership. Completion claims it once, decodes outside the mutex, then publishes or disposes. Replacement waits for result consumption plus all callbacks and losers to become quiet.',
    'Response bytes remain borrowed only during decoder execution. The owned descriptor moves on success; aliasing it elsewhere does not create another owner. Caller must preserve decoder/disposer contexts.',
    'The arbiter uses caller storage but each response may have a separately fallible allocation and variable byte count. Actual controller totals82 versus100 requested heap bytes expose the extra response owner.',
    'Winner transfer copies a descriptor, not the payload allocation. Losing descriptors are disposed once; decoding can inspect/copy bytes under its explicit contract without publishing a partial result.',
    'There are at most two attempts, but decoder/disposer latency can be unbounded. Actual paused loser disposal retains the gate/scope even after the winning result is taken.',
    'Object inspection shows mutex calls and indirect clock/decoder/disposer calls plus actual gate/scope operations. No compiler effect system proves that callbacks will terminate or avoid reentry.',
    'Post-decode acceptability and a fresh scope check linearize publication. Health is snapshotted there; abandonment during later cleanup does not retroactively neutralize the published outcome.',
    'Failed/empty decoding cannot win under the supplied decoder contract. Stale/duplicate/unstarted completion is rejected; expiry during decode disposes the response rather than publishing it.',
    'Request generation is a local stale-completion guard. Effect deduplication must occur at the resource. Independent-tail gains and correlated-loss cost are measured under the same explicit effect/cleanup contract.'],
8: [
    'The receiver retains bounded issuer/client/request/intent records and a128-record64-byte journal. Serialized writer authority and borrowed device callbacks are additional state outside the C struct.',
    'Open reconstructs only a valid durable prefix before publishing output. Apply persists the record before publishing logical state; an uncertain writer must be reopened/recovered before reuse.',
    'One serialized writer owns the journal/effect state; a follower may not independently admit requests. Device callbacks are nonreentrant and their contexts outlive every operation.',
    'The core uses fixed state and automatic record buffers. POSIX descriptors/kernel caches and fixture storage are external resources; zero injected allocator calls does not mean zero process memory.',
    'Each64-byte record is written as body56/flush/footer8/flush, possibly in partial chunks. Equal-intent replay returns the original response; checkpoints cannot erase still-required identity evidence.',
    'Recovery scans at most128 records; each callback can block. Closing a descriptor is separate from a successful durable flush, and a process exit is not a physical power-cut experiment.',
    'The journal object uses indirect read/write/flush callbacks. Native tests bind actual positional I/O/fdatasync/fork; model callbacks expose independent persistence cut points.',
    'The linearization argument assumes one writer at the effect resource. Fencing rejects stale epochs at that resource, not merely at an upstream queue; divergent writable replicas are a counterexample.',
    'Uncertain write progress poisons the instance. Failed recovery preserves the caller output; torn suffix is distinguishable from an invalid committed record. Duplicate altered intent conflicts.',
    'Versioned wire records use explicit byte encoding/checksums, not struct images. Checksums are not authentication; finite retained IDs and epoch rotation make exactly-once scope explicit, not consensus.'],
9: [
    'Composition retains two NetStacks, scope, timer wheel, exact bucket, two-response arbiter, journal/outbox/hub, plus borrowed distinct devices. Native service adds mutex, gate and active-call retirement.',
    'A request owner spans admission through effect/reply and callback cleanup. A notification owner spans commit through durable ACK; completed replies do not return those four event credits.',
    'The virtual controller is serialized; native service protects shared admission/effects. File descriptors are borrowed and not closed by the service. Destroy refuses while active I/O still owns a borrow.',
    'One scope plus up to two18-byte responses are injected: observed peaks64/82/100 according to actual outcomes. Native POSIX internals and caller devices are outside those requested-byte counts.',
    'Requests/replies serialize18 octets, transport retains request bytes, struct msg_hub copies event sequence, journal/ACK persist64/32-byte records. Parser copies only complete validated frames and retains partial progress.',
    'At most two response owners and four event owners bound bookkeeping. Partial send/read waits, disposers and flushes can block. Stop wakes and joins actual workers before final service destruction.',
    'Actual direct struct net_stack/struct msg_hub/hosted deadline calls coexist with device/allocator/decoder callbacks. Optimized controller imports memset/stack protection; service imports poll/send/clock_gettime. Hosted dependencies are explicit.',
    'Native two-worker duplicate race and six queued frames in two service orders are separate workloads. The serial9096-history oracle is not an all-interleaving proof of the native service.',
    'Reply cancellation/expiry can follow committed effect. Recovery may redeliver an unACKed event; durable ACK frees credit. Allocation/write cuts and queue capacity reject before the documented later boundaries.',
    'Local socketpairs and explicit framed bytes do not imply a deployed protocol. Logical restart reconstructs identities rather than pointers; native fork tests separately exercise process exit. Physical storage durability remains unmeasured.']
}

LAYOUT = {1:['scope','scope_align'],2:['owned_queue','pressure_pump'],3:['timers'],
          4:['bucket'],5:['retry','tcp'],6:['gate'],7:['arbiter'],
          8:['journal'],9:['scope','timers','retry','tcp','arbiter','journal','outbox','hub','service_io','result','fixture_device']}


def matrix(specs, report, report_path, timer_path, abi_path):
    rows=[]
    positions=[name for name in DIMENSIONS if name not in ['Representation','Compilation','Binary/stack']]
    for number,spec in specs.items():
        assert len(DETAILS[number])==len(positions)
        dimensions=dict(zip(positions,DETAILS[number]))
        measured=[{'configuration':p['configuration'],
                   'layout_bytes':{key:p['layout'][key] for key in LAYOUT[number]},
                   'layout_record':p['layout_log'],
                   'objects':{key:p['mechanism_objects'][key] for key in spec['parts']},
                   'builds':p['builds'], 'stack':p['stack'],
                   'map':p['pressure']['map'] if number==2 else p['map']}
                  for p in report['profiles']]
        dimensions['Representation']='Actual sizeof observations for '+', '.join(LAYOUT[number])+ ' are attached for all four configurations. Valid-state encoding and invariants are in '+spec['contract']+'.md; sizeof alone does not establish validity. Only scope alignment was directly printed, so other alignment is a C/compiler-layout premise, not an additional measurement.'
        dimensions['Compilation']='Attached clean/unchanged/single-controller-object-rebuild and pressure-build records observe the actual composed translation graph with GCC/Clang O0/O3. They do not isolate this mechanism\'s compile time. Strict diagnostics and source dependency closures bind the observation; they do not diagnose every semantic misuse.'
        dimensions['Binary/stack']='Attached mechanism object sizes/disassembly/undefined symbols and linked map distinguish this object from transitive dependencies. The recorded compiler stack file gives per-function frames, not a whole-thread bound; callback/library/OS frames remain additional. No target helper-free or physical WCET claim.'
        rows.append({'exercise':f'X-T75-{number:03d}',
                     'dimensions':[{ 'dimension':name,'analysis':dimensions[name]} for name in DIMENSIONS],
                     'enforcers':{'ISO_C':'Declared scalar/object semantics and representation access rules; ISO C does not enforce ownership or callback termination.',
                                  'runtime':'The specific identity, bounds, overflow, state and admission checks in the named implementation.',
                                  'diagnostics':'Actual strict C17 builds and sanitizer runs, with finite tested execution scope.',
                                  'external_model':'Named independent bounded oracle; not a proof over all executions.',
                                  'caller':'Valid storage/aliases, declared serialization, live callback context and environment premises in the exact contract.'},
                     'measured_evidence':measured,
                     'reports':[report_path]+([timer_path] if number==3 else []),
                     'changed_abi_probe':abi_path,
                     'limits':['No usable hosted32-bit environment was observed; compiler diversity on this host is not a changed-ABI validation.',
                               'No independent learner choice, non-author review or physical evidence is inferred.']})
    return {'schema_version':1,'status':'authored-analysis-bound-to-observed-reports','mechanisms':rows,
            'scope':'Nine mechanism-specific13-dimension dossiers, with original author interpretation separate from attached raw execution records.'}
