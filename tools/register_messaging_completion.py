"""Record exact T73 author-support readiness, preserving external evidence gaps."""
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,validate

CAMPAIGN='reviews/v5/expansion/modern_completion/messaging_current_campaign.json'
COMPOSITION='reviews/v5/expansion/modern_completion/modern_composition_campaign.json'
MODELS='research/v5/modern/messaging_models_validation.json'
COROUTINES='research/v5/modern/coroutines_followthrough_validation.json'
COST='research/v5/modern/messaging_cost_review.json'
READY='research/v5/modern/messaging_objective_readiness.json'
SUPPORT={
1:('NURSERY.md',['nursery.c'],['nursery_test.c'],[
 'Creation-order error aggregation, cancellation flag versus joined descendant, and context ownership are explained in NURSERY and its assessor analysis.',
 'MfnTree executes two actual MsgNursery groups, actual hs_pool_sum and hs_read_until; adapters preserve contexts through upward join.',
 'core_admission_edges covers0/1/8 children, ninth/late refusal, all eight injected failures, attempt exhaustion and barrier-coordinated sibling cancellation; separate TSan profiles run.',
 'Reversed completion orders preserve both errors and the creation-order primary; parent deadline/external cancellation propagate downward, drops occur after all admitted descendants finish.']),
2:('OWNED.md',['owned.c','registration.c'],['owned_test.c','registration_test.c','registration_oracle.c'],[
 'Actual hub predicates distinguish readiness, accepted rendezvous and close/drain; Go randomness is explicitly not promised.',
 'Common mutex or registered sticky recheck prevents lost wakeups; separately pinned ca_channel_deliver imports actual MsgHub with retained CATicket/CAFrame through join and ack.',
 'Owned-channel fixtures cover buffered close/cancel, both selection masks, spurious broadcasts and accepted/unaccepted rendezvous; real two-producer traces conserve exact values.',
 'Unique descriptor transfer is exercised by owning hub; separately locked channel registry forces the empty-scan/pre-sleep publication and rejects omitted notification and skipped-buffer controls.']),
3:('OPERATION.md',['operation.c','completion.c'],['operation_test.c','completion_test.c'],[
 'Sender recipe, connected storage and completion ownership remain distinct; dated proposal and exposed C fields have explicitly different enforcement.',
 'MfoOperation composes actual MsgOperation, while Linux port retains real backend borrows through joined cancellation acknowledgement; callbacks may free application owners.',
 'Invalid/repeated orders and receiver-freed heap contexts are executed; MfxCompletion arbitrates concurrent callers with one atomic winner and a retained token per loser.',
 'Inline predecessor receiver destroys its heap object and starts the successor after moving ownership; start and completion cache everything needed before invoking the callback.']),
4:('ACTOR_EXECUTOR.md',['owned.c','actor_executor.c'],['owned_test.c','actor_executor_test.c'],[
 'Mailbox ownership and incarnation are separated from pointer validity; a synthetic effect before failure demonstrates that restarting cannot roll it back.',
 'MfeExecutor imports actual MfcHub and MfaActor with eight stable generation-bearing envelopes and one executor; failed publication versus failed dispatch have distinct owner rules.',
 'Full/stale/reentrant/dead/restart/retirement cases and2,000 native messages are checked against a mutex-protected reference under the observed order.',
 'Self-enqueue never recursively enters the handler; active and abandoned owners drop once; queued old-generation replies survive restart unchanged and enter dead letters.']),
5:('SUPERVISOR.md',['supervisor.c','owned.c'],['supervisor_test.c'],[
 'Intensity bounds and restart topology are distinguished from successful resource initialization and from process-level isolation.',
 'MfsTree executes actual actors in dependency order, refuses while stepping, rolls back partial constructors and escalates against a separate parent budget.',
 'All12 strategy/child selections, all16 initialization cuts, maximum/backwards/exact-window time and permanent escalation are checked with independent action-order models.',
 'Two-level parent/child budgets and child1-before-child2 dependency are exercised during partial restart; rollback/escalation releases owned messages and resources in reverse order.']),
6:('WEAK.md',['weak.c','lifetime.c'],['weak_test.c','weak_deque_model.c','lifetime_test.c'],[
 'Last-item compare/exchange, nonwrapping indices and atomic stale-slot reads have separate arguments; actual lock-free queries do not establish starvation freedom.',
 'Original fixed circular slots and owner/thief operations are preserved; lower-order successor has explicit release/fence/acquire proof and separate pointer-growth lifetime contract.',
 'Independent1095 original traces,128 small native linearization histories, TSan and generated-code observations cover their declared bounds, including empty/full/reuse.',
 'Pinned RC11 executions and seven edge-removal controls precede measured costs; growing pointer deque retains actual arrays/nodes while a thief pauses, with failure and final-release tests.']),
7:('TRANSACTIONS.md',['transactions.c','lifetime.c'],['transactions_test.c'],[
 'Snapshot validation, payload race freedom and dynamic node lifetime are distinct; inconsistent reads must abort before exposing a committed result.',
 'Actual four-cell SC core validates staged read/write sets and restores partial locks, preserving payload on abort while permitting timestamp gaps.',
 'Seven abortable and nine postpublication observation cuts, read conflicts, repeated writes, empty/read-only operations and3360 independent bounded schedules are tested.',
 'Retry body stages synthetic log effects for commit; actual allocated-node privatization keeps reader leases alive after replacement and destroy, unlike the fixed-cell proof.']),
8:('VERSIONS.md',['versions.c','lifetime.c'],['versions_test.c'],[
 'Version visibility and pin lifetime are derived separately from stronger isolation and allocated payload ownership; forgotten pins retain history.',
 'Actual timestamped versions use exact pin generations, select immutable history, and collect within the four-pin/five-selected-version bound.',
 'Independent3910 histories, generation/stale/refusal cases and four simultaneous versions are checked; measured pin/copy costs are compared to explicit RCU lifetime assumptions, not an unimplemented RCU timing claim.',
 'Three clone-allocation cuts preserve existing readers; two-key independent and actual-thread write-skew cases distinguish write-only from observed-predicate validation.']),
9:('WEAK.md',['weak.c','lifetime.c'],['weak_test.c','weak_sequence_model.c','lifetime_test.c'],[
 'Atomic storage race freedom, odd/even validation, single writer, bounded attempts and finite counter lifetime each have their own obligation.',
 'SC and weak scalar representations publish no result before validation, preserve failure output and refuse nonwrapping counter exhaustion.',
 'Zero attempts, odd/interrupted writer, output canaries and native coherent-pair stress run under TSan; one-attempt mutex comparison includes refusals and actual generated instructions.',
 'RC11 fence/ordering controls reject missing required edges; separately retained pointer snapshot allows a paused reader to outlive replacement without claiming seqlock validation retains storage.']),
10:('STREAM.md',['stream.c','owned.c'],['stream_test.c'],[
 'Demand, terminal order and cancellation are explicitly compared to selected Reactive Streams rules, with unsigned C subset and reentrancy differences recorded.',
 'MfqStream imports actual MsgHub/MsgStream into one executor; request and delivery conserve credit while context ownership survives in-flight work.',
 'Independent19608 request/emit/end/cancel words and boundary tests cover saturation, zero demand, recursive signals, callback requests and terminal delivery without credit.',
 'Several producer threads serialize through the worker; paused callbacks retain context until join/last handle, and bounded WAIT/DROP/FAIL tests compare their distinct accepted/dropped contracts.']),
11:('CONTRACTS.md',['composition.c','linux_port.c'],['tests.c','recovery.c'],[
 'Composed graph orders arm, quiescent disarm, cleanup, receiver; generation mismatch is explicitly separate from retained lifetime.',
 'Actual COAsync, MsgOperation and cleanup-stack APIs compose serialized poll/cancel; local nonblocking socket pairs exercise complete and partial input without premature output.',
 'Pending/full/EOF/cancel and receiver-freed states run with generation/duplicate-call checks; disarm/cleanup mutants fail the named order assertions.',
 'Real worker poll/queued notice uses retained references; request_cancel returns before join/discard, finish_cancel establishes quiescence before receiver cleanup, including delayed undispatched notice.'])}


def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def dump(path,value):(ROOT/path).write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')
def pins_of(value):
    pins=value.get('inspected_paths',value.get('input_sha256'))
    assert isinstance(pins,dict) and pins
    for p,h in pins.items():assert sha(p)==h,p
    return pins


def dimensions(teaching,implementation,evidence):
    return {'teaching':{'state':'complete','evidence':teaching},
            'implementation':{'state':'complete','evidence':implementation},
            'author_validation':{'state':'passed','evidence':evidence},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-applicable-host-contract','evidence':['labs/messaging/CONTRACTS.md']}}


def main():
    reports=[CAMPAIGN,COMPOSITION,MODELS,COROUTINES,'research/v5/modern/messaging_cost_validation.json','research/v5/modern/messaging_build_cost_validation.json',
             'research/v5/modern/ownership_publication_validation.json']
    inputs={};criteria=[];roots=[];updates=[]
    for path in reports:
        d=read(path);assert d['passed'];inputs.update(pins_of(d));inputs[path]=sha(path)
    costs=read(COST);inputs.update(pins_of(costs));inputs[COST]=sha(COST)
    for n,(contract,implementations,tests,demonstrations) in SUPPORT.items():
        ident=f'X-T73-{n:03d}';brief='curriculum/v5/exercises/'+ident+'.md';meta,body=parse_brief(ROOT/brief);validate(meta,body)
        family='modern_composition' if n==11 else 'messaging'
        teaching=[brief,f'assessor/{family}/analyses/{ident}.md']
        teaching += ['labs/modern_composition/CONTRACTS.md'] if n==11 else ['labs/messaging/CONTRACTS.md','labs/messaging/followthrough/'+contract]
        impl=[f'assessor/{family}/'+('' if n==11 else 'followthrough/')+p for p in implementations]
        impl += [f'labs/{family}/'+('tests/' if n==11 else 'followthrough/')+p for p in tests]
        if n!=11:impl+=['assessor/messaging/messaging.c','labs/messaging/tests/tests.c','tools/verify_messaging_completion.py','tools/verify_messaging_fixtures.py']
        evidence=[COMPOSITION] if n==11 else [CAMPAIGN,COST]
        if n in (6,9):evidence+=[MODELS]
        if n==1:impl+=['assessor/hosted_systems/pool.c','assessor/hosted_systems/deadline.c']
        if n==2:
            teaching+=['labs/messaging/followthrough/REGISTRATION.md','labs/coroutines/followthrough/CONTRACTS.md']
            impl+=['assessor/coroutines/followthrough/channel_follow.c','assessor/coroutines/followthrough/async_follow.c','labs/coroutines/followthrough/channel_tests.c'];evidence+=[COROUTINES]
        if n==3:teaching+=['labs/messaging/followthrough/COMPLETION.md'];evidence+=[COMPOSITION]
        if n in (6,7,8,9):teaching+=['labs/messaging/followthrough/LIFETIME.md']
        for index,criterion in enumerate(meta['rubric']):
            paths=sorted(set(teaching+impl+evidence))
            row=dict(**criterion,author_readiness='ready-bounded',demonstration=demonstrations[index],
                     teaching=teaching,implementation_and_observation=impl+evidence,
                     limits=['Exact supplied contract, finite clients and host/tool profiles only; no all-schedule or arbitrary-target proof.',
                             'Independent learner attempts, delayed performance and non-author review are not established by author implementations.'],
                     external_review='not-run',independent_learner='not-assessed',physical_evidence='not-claimed',
                     pins={p:sha(p) for p in paths})
            criteria.append(row)
            updates.append(dict(id=row['id'],dimensions=dimensions(teaching,impl,evidence+[READY]),closure_rationale=demonstrations[index]))
        root=dict(id=ident,criteria=[r['id'] for r in meta['rubric']],author_readiness='ready-bounded',
                  delayed_variant=meta['delayed_transfer']['variant_ids'],independent_learner='not-assessed')
        roots.append(root)
        for target in [ident,*root['delayed_variant']]:
            updates.append(dict(id=target,dimensions=dimensions(teaching,impl,evidence+[READY]),
                                closure_rationale='All exact rubric rows have concrete author support in '+READY+'; the original delayed model is implemented, but no independent learner performance is inferred.'))
    readiness=dict(schema_version=1,domain='T73',criteria=criteria,roots=roots,current_reports=reports,
                   scope='Per-criterion author support with explicit real imports; independent learner and exact inaccessible-source gaps remain separate.')
    readiness['import_dispositions']={
        'work.pool':{'producer':'LAB-HS-10 / PR-T25-01','status':'actual-source-composition','consumer':'mfn_pool_task calls hs_pool_sum; pool.c is an explicit hosted_systems dependency'},
        'io.deadline':{'producer':'LAB-HS-03 / PR-T24-05','status':'actual-source-composition','consumer':'mfn_wait calls hs_read_until; deadline.c is an explicit hosted_systems dependency'},
        'coro.frame':{'producer':'T72 coroutine family','status':'actual-source-composition','consumer':'ca_channel_deliver uses actual MsgHub with retained asynchronous tickets; McRead imports actual COAsync'},
        'sync.condition_queue':{'producer':'LAB-HS-11 / PR-T25-02','status':'prerequisite-contract-and-proof-transfer','consumer':'The new predicate/registration protocol is implemented from primitives; it does not claim to call the earlier copied-value queue'},
        'sync.spsc':{'producer':'U-T55-01-03 / PR-T55-01','status':'prerequisite-contract-not-invented-api','consumer':'Catalog publication/storage contract informs ordering prerequisites; the owner/multiple-thief deque is a separately implemented protocol, not an SPSC call-through'},
        'io.async':{'producer':'U-T21-02-04 / PR-T21-02','status':'contract-only-producer-explicit-adapter','consumer':'McLinuxPort implements real readiness/request/acknowledgement against the ownership contract; no undocumented legacy API is claimed'},
        'testing.faults':{'producer':'U-T32-02-02 / PR-T32-02','status':'contract-only-producer-explicit-fixtures','consumer':'Admission, allocation and commit-phase controls implement declared fault schedules and separate oracles; there is no invented linked fault library'}}
    dump(READY,readiness);inputs[READY]=sha(READY)
    for row in criteria:inputs.update(row['pins'])
    for gap in read('research/v5/modern/messaging_gap_ledger.json')['gaps']:
        ident=gap['id']
        if ident=='T73-reference':
            update=dict(id=ident,dimensions={
                'teaching':{'state':'partial-exact-reference-unavailable','evidence':['research/v5/modern/messaging_sources.md']},
                'implementation':{'state':'not-applicable-source-access','evidence':[]},
                'author_validation':{'state':'not-run-unavailable-passages','evidence':['research/v5/modern/messaging_sources.md']},
                'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-applicable-source-access','evidence':[]}},
                closure_rationale='Still open: exact authorized published C17 atomic text and Boehm2012 full text were not inspected. Bibliographic pages and inaccessible URLs do not close this obligation.')
        else:
            evidence=[READY,CAMPAIGN]
            if ident=='T73-composition':evidence += [COMPOSITION,COROUTINES]
            if ident=='T73-weak-order':evidence += [MODELS]
            if ident in ('T73-cost','T73-fairness'):evidence += [COST,'research/v5/modern/messaging_cost_validation.json','research/v5/modern/messaging_build_cost_validation.json']
            update=dict(id=ident,dimensions=dimensions(['labs/messaging/followthrough/MEASUREMENTS.md',READY],
                         sorted({p for r in criteria for p in r['implementation_and_observation'] if p.endswith(('.c','.py'))}),evidence),
                        closure_rationale='Required author implementation, experiments and bounded analysis are supplied per exact rubric. Transfer still requires independent learner attempts; fairness analysis explicitly refutes unsupported per-client/latency claims rather than asserting a universal scheduler guarantee.')
        updates.append(update)
    gap_path='research/v5/modern/messaging_gap_ledger.json';ledger=read(gap_path)
    ledger['status']='author support validated for stated contracts; exact-source access and independent evidence remain open'
    for gap in ledger['gaps']:
        gap['current_author_status']='exact-source-access-gap' if gap['id']=='T73-reference' else 'ready-bounded'
        gap['current_evidence']=['research/v5/modern/messaging_sources.md'] if gap['id']=='T73-reference' else [READY]
        gap['remaining_evidence']='Exact authorized published C17 atomic passages and Boehm2012 full text remain uninspected.' if gap['id']=='T73-reference' else 'Independent learner attempts and non-author review remain separate; finite host/model results do not establish all schedules or changed targets.'
    dump(gap_path,ledger)
    # Refresh earlier composition obligations with current transitive source evidence.
    for update in read('research/v5/modern/completion_dispositions.json')['updates']:
        updates.append(update)
    updates.append(dict(id='MC-INDEPENDENT',dimensions=dimensions(
        ['labs/modern_composition/CONTRACTS.md','curriculum/v5/exercises/X-T73-011.md','curriculum/v5/exercises/X-T66-011.md'],
        ['assessor/modern_composition/composition.c','assessor/modern_composition/linux_port.c','assessor/modern_composition/recovery.c'],[COMPOSITION,READY]),
        closure_rationale='Required author reference adapters and delayed-model support are implemented and executed. The named independent learner and non-author review obligations remain not-assessed/not-run.'))
    for path in ['tools/register_messaging_completion.py','tools/check_messaging_followthrough.py',
                 'research/v5/modern/messaging_gap_ledger.json','research/v5/modern/composition_gap_ledger.json',
                 'research/v5/modern/messaging_sources.md','research/v5/modern/messaging_resources.json','research/v5/modern/messaging_research_gate.json']:
        inputs[path]=sha(path)
    result=dict(schema_version=1,domain='T73',updates=updates,current_reports=reports,input_sha256=inputs,
                scope='Exact original-ID author-support dispositions; external learner/reviewer evidence and inaccessible references remain explicit.',
                summary=dict(criteria=len(criteria),roots=len(roots),updates=len(updates)))
    dump('research/v5/modern/messaging_completion_dispositions.json',result)
    print(json.dumps(result['summary']))


if __name__=='__main__':main()
