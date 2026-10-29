"""Bind mechanism-specific cost analysis to observed uninstrumented work."""
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
RUN='research/v5/modern/messaging_cost_validation.json'
BUILD='research/v5/modern/messaging_build_cost_validation.json'
FIELDS=('reachable_storage','lifetime','mutation_aliasing','allocation','movement_copying','destruction','dispatch','concurrency','failure','boundary')
# Each row is an explicit analysis of its mechanism, not a family-pass inference.
ROWS={
1:dict(types=['struct msg_nursery','struct mfn_tree'],modes=['thread_stack'],analysis=[
 'Eight inline child records plus actual pthread stacks; nested tree adds two groups and four retained task contexts. Live stack attributes are virtual reservations, not resident pages.',
 'Start admission precedes task entry; cancel records intent; every admitted child must join before its context can be released. A descendant still running retains every enclosing owner.',
 'One owner mutates nursery admission/join records. Children read atomic cancellation and publish context results before join synchronization; arbitrary parallel spawn is not admitted.',
 'Nursery metadata is caller-owned; pthread_create can allocate internally and fail. All eight injected admission positions preserve earlier children for joining.',
 'No whole nursery copy is a move: child owner pointers retain the original address. Task contexts remain borrowed at fixed addresses until the matching join.',
 'At most eight joins in a flat nursery, but no finite latency without cooperative child termination and scheduling assumptions. Nested teardown joins leaves before parents.',
 'Task entry and destructor callbacks are indirect; mfn_pool_task invokes the actual earlier hosted pool. The timing workload measures simple cooperative children separately.',
 'Atomic cancel publication does not establish child completion. Creation-order error choice remains deterministic under reversed completion schedules; both failures survive the bounded aggregate.',
 'Admission failure triggers cancellation and retains prior admissions. Ninth/late admission refuses; empty join succeeds; attempt exhaustion refuses before thread creation.',
 'POSIX thread creation/join and GNU live-stack query are host contracts. Neither ISO C nor a stack size query supplies a real-time or maximum-used-stack guarantee.']),
2:dict(types=['struct msg_hub','struct mfc_hub','struct mfr_hub'],modes=['select_shared','select_registered'],analysis=[
 'Two queues of up to four copied values; owning successor adds eight stable message records. Registered select adds one caller-stack node per live select and two channel locks plus registry.',
 'Enqueued ownership survives sender return; rendezvous acceptance differs from an outstanding offer. Registration nodes remain linked through predicate wait and are removed before return.',
 'Common-hub mutex protects the original predicates; split design uses channel-before-registry order. Concurrent selectors may consume a wakeup candidate before another selector resumes.',
 'Queue and ownership metadata are inline. The observer sample arrays and producer payload allocation are host-test costs; original unsigned timing has no payload allocation.',
 'Unsigned values are copied; owning descriptors transfer only on accepted send. Receiver extraction does not deep-copy payload storage. Failed publication retains the sender owner.',
 'Destruction drains accepted owners only after admission closes and active calls quiesce. Callback runtime is not bounded by the eight metadata slots.',
 'Common select scans two channels; registration scans two channels and notifies O(W) active waiter nodes. Mutex and condition operations cross the POSIX library boundary.',
 'Sticky notification plus registration-before-scan closes the empty-observation/sleep window. Signals require predicate recheck; pthread cancellation unwinding is explicitly not supported.',
 'FULL/EMPTY preserve output/owner; close drains accepted values; cancel distinguishes accepted from unaccepted rendezvous. BUSY destruction retains the object until calls finish.',
 'Deterministic rotating selection is not Go randomized selection. Coroutine ticket import retains frames through external worker join and explicit acknowledgement; tokens alone do not prove quiescence.']),
3:dict(types=['struct msg_operation','struct mfo_operation','struct mfx_completion'],modes=['operation_owned'],analysis=[
 'Serialized operation is inline; concurrent successor allocates one independent control with atomic flags/count and retains it for all contenders, separately from receiver-owned application context.',
 'Connect/start grants one terminal delivery. Inline receiver may free the application object; concurrent losers retain only their control and must never access the freed receiver context.',
 'Serialized original requires caller exclusion. Concurrent successor admits one atomic completion winner before ordinary operation/pending fields; each thread owns a separately retained token.',
 'Inline adapter allocates nothing. Concurrent control allocation can fail without publishing output; retain is allocation-free and refuses SIZE_MAX count.',
 'Completion clears the unique descriptor before receiver invocation. A descriptor move is not a payload clone; failed/losing completion preserves the contender owner.',
 'One terminal payload release and one final control free; receiver/destructor latency is externally supplied. Stop request neither destroys control nor joins backend work.',
 'Backend and receiver are indirect calls. Observed GCC O3 mfo_start caches backend/context before calling; msg_complete stores DONE before the callback and has no subsequent object load.',
 'Cancellation and completion claim share an atomic modification order in the concurrent successor. Earlier cancellation fixes the stopped result; late cancellation cannot revise the winner.',
 'Invalid status/descriptor rejects. Duplicate claim returns BUSY. Cancellation before completion produces no early receiver; receiver-free tests and retained losers distinguish lifetime from arbitration.',
 'Dated P2300 comparison does not confer C++ lifetime/type enforcement on an exposed C struct. Public token discipline and normal-return callbacks remain caller obligations.']),
4:dict(types=['struct msg_actor','struct mfa_actor','struct mfe_executor','struct mfe_envelope'],modes=['actor_owned'],analysis=[
 'Four queued messages plus one currently executing owned record; executor adds two actual actors and eight envelopes. Externally allocated payload storage is additional reachable memory.',
 'Executor owns dispatch; handler borrows one payload through return. Submitted generation is immutable in its envelope; restart never retags already accepted work.',
 'One executor mutates actor state. Producers publish through the actual owning hub; pool mutex protects stable envelopes. Blocking self-send can deadlock the sole consumer and is prohibited.',
 'Actor/executor metadata is inline; payload allocation belongs to producers. Pool exhaustion and channel refusal preserve caller ownership until acceptance.',
 'Channel moves one unique envelope; executor extracts payload and frees the slot before actor dispatch. Self-enqueue moves a separate owner without recursive handler entry.',
 'Failure drops active/queued owners once; stale messages go to a consuming dead-letter sink. Restart and destruction refuse while stepping; callback costs remain external.',
 'Handler and dead-letter dispatch are indirect. One pump delivers one accepted event. The measured actor cycle excludes allocating payload and cross-thread queue latency.',
 'The2,000-message threaded test compares the observed actor effect order to a mutex-protected reference, not all possible histories or performance superiority.',
 'FULL/CLOSED retain owners before admission; STALE consumes to dead letters. Generation exhaustion retires. Effects emitted by a handler before it fails remain observable.',
 'An incarnation token prevents stale protocol delivery, not forged authority or arbitrary pointer use. C actors provide no process isolation or automatic rollback.']),
5:dict(types=['struct msg_supervisor','struct mfs_tree'],modes=['supervisor_tree'],analysis=[
 'Five supervisor records and16 actual actors in a fixed two-level tree; bounded timestamps and event arrays coexist with externally constructed child resources.',
 'Stop affected children in reverse dependency order, initialize forward, and close staged resources before escalation. Parent and child budgets are distinct lifetimes.',
 'Supervisor calls are serialized and refuse while an actor steps. Constructor/destructor callbacks must not reenter the tree or retain borrowed control storage.',
 'Tree metadata is inline. Fallible resource constructors are explicit; all16 initial failures and partial restart cuts exercise reverse cleanup.',
 'Restart plans copy bounded action records. Resource identities are not duplicated by copying actor structs; restart creates a new incarnation after old owned messages are dropped.',
 'Timing replaces two children: two closes/two opens per iteration, plus16 initialization/final operations. Rollback may add callbacks, whose runtime is not bounded by policy.',
 'Plan generation scans a finite timestamp window and child range; execution dispatches open/close/handler callbacks. A plan is not itself successful recovery.',
 'A two-level tree is externally serialized; no concurrent supervisor mutation or scheduler independence is claimed. Unlimited recovery requests can amplify failure without intensity limits.',
 'Exact expiration boundary admits a fresh restart, backwards time refuses, maximum timestamps are nonwrapping; permanent escalation closes the tree if parent recovery fails.',
 'OTP restart strategy names are comparisons. The C callbacks define actual resource effects; this lab does not inherit Erlang process cleanup or language isolation.']),
6:dict(types=['struct msg_deque','struct mfw_deque','struct mfm_deque','struct mfg_deque'],modes=['deque_sc','deque_weak','deque_mutex','deque_sc4','deque_weak4','deque_mutex4','fairness'],analysis=[
 'Atomic fixed deque stores eight unsigned slots and nonwrapping indices. Growing pointer successor separately retains allocated arrays and payload controls until all paused thieves release them.',
 'Owner controls bottom; a successful top compare/exchange transfers one value to a thief. Losing thieves in the pointer variant retain array/payload before dereferencing or racing removal.',
 'Only one owner pushes/pops; thieves arbitrate top atomically. Pointer/growth successor uses a mutex for retention and is deliberately not the same lock-free algorithm.',
 'Fixed core allocates nothing. Growth first allocates new array and payload control; each failure preserves old representation. Paused leases can retain old arrays beyond replacement.',
 'Unsigned publication copies one value. Growth copies descriptors while retaining node ownership; no copied raw pointer creates an independent lifetime.',
 'Fixed core has no payload destruction. Growing deque releases array/node references, with destruction delayed by paused clients; BUSY teardown preserves active calls.',
 'Observed GCC O3 weak push uses ordinary mov instructions for release stores; SC push uses xchg. Weak steal retains a locked fence and locked compare/exchange.',
 'RC11 finite clients validate selected edges and reject omitted-edge controls. Native two/four-thief tests preserve fixed total work; global success does not imply per-thief fairness.',
 'EMPTY/FULL/CONFLICT and finite retirement are separate. Failed pop restores bottom. Repeated competing steals can deny one client success despite both taking steps.',
 'Atomic_is_lock_free and emitted x86-64 instructions are host observations. Dynamic reclamation proof does not follow from the fixed-array paper/model and is supplied separately under a mutex.']),
7:dict(types=['struct msg_store','struct msg_txn','struct mft_store','struct mft_txn'],modes=['transaction','transaction_mutex'],analysis=[
 'Four cells with versioned lock words plus bounded read/write sets; owning store adds shared nodes and a64-entry committed synthetic effect log.',
 'Transaction begins at a clock snapshot; commit locks writes, validates reads, publishes values then unlocks. Dynamic readers retain nodes across replacement/privatization.',
 'Payload cells are atomic even for failed transactions. Owning store outer mutex serializes reference acquisition/publication; callbacks cannot publish unlogged external effects safely on retry.',
 'Fixed-cell core allocates nothing. Owning writes stage a new shared node before commit; allocation or log-capacity failure preserves published owners and cleans staged nodes.',
 'Read-your-writes reads staged values. Ownership moves into the store only on commit; old nodes are detached and released after the publication lock.',
 'Abort releases staged references and restores acquired version locks. Committed effects publish once; individual release callbacks are outside the mutex and may have external cost.',
 'Commit visits four bounded cells; retry callback may execute several times. Effect log dispatch occurs only after successful commit, unlike an immediate synthetic side effect.',
 'SC core provides coherent validated transactions under its fixed-cell proof. Mutex alternative meets the paired-increment contract but differs in refusal/progress. Failed commits may consume clock values.',
 'Seven prepublication abort cuts preserve values; nine later observational phases cannot retroactively abort committed data. Invalid read admission preserves the transaction; admitted conflict aborts it.',
 'TL2 algorithm discussion is not a C lifetime proof. Atomic loads do not retain dynamic nodes, and a finite effect log is neither durable storage nor arbitrary effect rollback.']),
8:dict(types=['struct msg_versions','struct mfv_store','struct mfk_store','struct mfk_txn','struct mfm_pair'],modes=['snapshot_pins','snapshot_copy','snapshot_pins4','snapshot_copy4'],analysis=[
 'Eight version slots, four pin records; at most five distinct retained versions under four readers. Owned payload leases can outlive both pin and store and add allocated storage.',
 'Pins retain exact historical visibility; generations validate pin reuse. Shared leases retain payload lifetime separately, including after store destruction.',
 'Mutex serializes version selection/publication. Predicate adapter records all observed keys, including zero; write-only validation permits the supplied write-skew counterexample.',
 'Core versions are inline. Owned clone has payload-record, byte-storage and control allocation cuts; no new version publishes until all three succeed.',
 'Equal-value reference copies two fields at snapshot time; pin alternative reads its retained version later. Four-pin transfer compares four simultaneous history values, not just the latest pair.',
 'Dropping one pin may reclaim versions no longer selected; non-LIFO release must preserve others. Last shared lease release invokes payload destruction independently of the store.',
 'Finite scans select visibility and collect slots; read/copy alternatives include their mutex acquisition. Timing cannot extrapolate two-word copying to arbitrary object graphs.',
 'Pinned reads are repeatable; stronger predicate validation can reject disjoint writes that would violate a joint invariant. RCU would require a separate read-side/grace-period contract and extra retention for escaping snapshots.',
 'Fifth active pin refuses unchanged output; stale generations and clock exhaustion refuse. Clone failure leaves existing readers intact; overwritten reused slots must release their old payload.',
 'PostgreSQL isolation passages explain the counterexample but do not certify this finite C store as a database. Pin/copy timing is not a measurement of an RCU implementation.']),
9:dict(types=['struct msg_sequence','struct mfw_sequence','struct mfm_pair','struct mfp_pointer'],modes=['sequence_sc','sequence_weak','sequence_mutex'],analysis=[
 'Sequence counter and two atomic unsigned fields; pointer successor adds a mutex and retained shared payload because the sequence alone cannot keep a pointee live.',
 'Single writer publishes odd/data/even; a reader accepts only unchanged even sequence. A paused pointer reader retains its node until done, even after replacement and control destruction.',
 'All scalar payload accesses remain atomic, including rejected samples. Ordinary racy loads are not repaired by a later validation failure.',
 'Scalar protocol allocates nothing. Pointer replacement stages a shared control before publication and preserves caller ownership on refusal or counter exhaustion.',
 'Successful scalar snapshot publishes two copied fields after validation. Failure leaves canaries unchanged; pointer snapshot copies a retained token, not the pointee bytes.',
 'Scalar teardown is quiescent caller discipline. Pointer old-owner destruction waits for the last retained reader and occurs outside the protecting mutex.',
 'Observed GCC O3 weak reader is loads/comparisons with no hardware fence instruction for the acquire fence; this compiler result does not erase the C ordering obligation.',
 'One-attempt timing counts successful reads and refusals, comparing a one-try-lock mutex reference. RC11 clients establish only their finite explored executions and edge counterexamples.',
 'Zero retry, odd writer, changed sequence and nonwrapping counter retirement refuse without output writes. A writer paused while odd can deny every reader within its bounded attempt budget.',
 'The public C++ proposals are not adopted C17 features. Neither scalar sequence validation nor clean TSan output proves pointer reclamation or arbitrary weak-ISA behavior.']),
10:dict(types=['struct msg_stream','struct mfq_stream','struct mfo_value'],modes=['stream_item','stream_batch16'],analysis=[
 'Capacity4 channel plus one callback in flight, worker/control records and retained handle count. Context storage remains separately owned until worker join and final handle release.',
 'All signals pass through one executor; request/end/cancel state precedes callback dispatch. Cancellation does not release an in-flight callback context.',
 'Control mutex serializes demand and terminal state. Callback can request or cancel under the explicit reentrancy rules; recursive delivery is rejected.',
 'Stream control and channel slots are inline. Worker creation may fail without consuming the context; payload is copied unsigned in this adapter, not an implicit deep-cloned object.',
 'Accepted input copies one unsigned value. Per-item and batch16 requests deliver exactly the same N ordered values; batching changes request count, not delivery credit accounting.',
 'Close/cancel wakes blocked producers; worker must join and all handles release before context drop. A nonreturning callback prevents quiescence despite a finite buffer.',
 'Executor calls next/terminal indirectly and never overlaps signals. Per-item request count is N; batch16 count is ceil(N/16), checked independently of timing.',
 'Several producers use actual pthread exclusion and one executor. Slow-subscriber WAIT/DROP/FAIL policies have different accepted/dropped accounting and cannot be compared as identical delivery guarantees.',
 'Zero/maximum demand, terminal-without-credit, late emit and recursive signals are explicit boundaries. FULL/BUSY/CLOSED preserve owners/handles as specified; counts retire without wrap.',
 'Selected Reactive Streams rules guide demand/serialization comparison; unsigned C statuses and fixed capacities are not JVM interoperability or full protocol certification.'])}


def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()


def main():
    run=read(RUN);build=read(BUILD);pins={RUN:sha(RUN),BUILD:sha(BUILD),'tools/register_messaging_cost_review.py':sha('tools/register_messaging_cost_review.py')}
    for doc in (run,build):
        assert doc['passed'] and doc['source_stable']
        for p,h in doc['inspected_paths'].items():assert sha(p)==h,p;pins[p]=h
    layouts={r['profile']:r['types'] for r in run['observations'] if r['mode']=='representation'}
    assert set(layouts)=={'gccO0','gccO3','clangO0','clangO3'}
    rows=[]
    for index,row in ROWS.items():
        dimensions=dict(zip(FIELDS,row['analysis']));assert len(dimensions)==10
        dimensions['representation']={'types':row['types'],'observed':{p:{t:v[t] for t in row['types']} for p,v in layouts.items()},'limit':'Exact observed ABI sizes/alignment, not portable object layout.'}
        dimensions['compilation']={'evidence':BUILD,'profiles':build['profiles'],'limit':'Observed clean/no-op/two-object header rebuild for the minimal shared driver. No whole-curriculum or statistical compiler-speed claim.'}
        dimensions['binary_stack']={'evidence':RUN,'artifacts':[a for a in run['artifacts'] if a['kind'] in ('stack-usage','link-map')],'limit':'Per-function stack usage and whole shared-driver binary/map; not sum-of-frames stack bounds or per-abstraction linker attribution.'}
        selected=[r for r in run['observations'] if r['mode'] in row['modes']]
        rows.append(dict(objective=f'T73.{index:02d}.d',exercise=f'X-T73-{index:03d}',dimensions=dimensions,
                         measured_modes=row['modes'],observed_rows=len(selected),evidence=RUN,
                         enforcers={'language':'Atomic object accesses and object-lifetime requirements are C semantics; exact published C17 text remains an access gap.',
                                    'runtime':'Status, capacity, generation, owner/refcount, predicate and invariant checks enforce only their stated admitted states.',
                                    'external':'RC11 exploration, independent Python models, sanitizers and disassembly have separately bounded evidence.',
                                    'discipline':'Valid borrowed storage, external admission gates, callback lifetime and no forged/copied owners remain caller obligations.'}))
    observations=[dict(scope='GCC -O3 x86-64 seven inspected functions',
       functions=['mfw_push','mfw_steal','mfw_read','msg_push','msg_steal','mfo_start','msg_complete'],
       findings=['Weak push emits mov stores while SC push emits xchg stores.',
                 'Weak steal emits a locked stack fence and locked cmpxchg; SC steal uses locked cmpxchg without that separate fence.',
                 'Weak sequence reader has no emitted hardware fence instruction; acquire ordering still constrains the compiler.',
                 'mfo_start caches backend/context before indirect invocation; msg_complete has no object read following the receiver call.'],
       limit='Author inspection of these actual functions only; not every function, target or compiler and not a weak-memory proof.')]
    out=dict(schema_version=1,domain='T73',status='author-analysis-with-executed-cost-evidence',rows=rows,assembly_inspection=observations,
             input_sha256=pins,current_reports=[RUN,BUILD],independent_learner='not-assessed',external_review='not-run')
    (ROOT/'research/v5/modern/messaging_cost_review.json').write_text(json.dumps(out,indent=2)+'\n',encoding='utf8')
    print('Ten mechanism rows with thirteen dimensions bound to current observations')


if __name__=='__main__':main()
