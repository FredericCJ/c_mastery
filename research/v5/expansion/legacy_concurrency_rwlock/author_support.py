"""Bind original T51.12 criteria and changed-policy variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_rwlock/support.json'
CONTRACT='labs/execution/sync_depth/RWLOCK.md';PROJECT='PR-T51-12'
HOST='reviews/v5/expansion/legacy_concurrency_sync/rwlock_campaign.json'
NATIVE='reviews/v5/expansion/legacy_concurrency_sync/rwlock_native.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/rwlock_target.json'
SCOPES={
 'T51.12.a':'Actual retained A/B/C writer-preference trace and rolling-reader starvation prefix; changed FIFO-writer/reader-cohort phase policy with explicit scheduler/progress assumptions.',
 'T51.12.b':'Bounded identity-tracked read reservations, nonowner/counter refusal, two-writer cancellation/timeout gate recomputation; explicit single-upgrader conversion, prior-writer deadlock refusal and retained-read cancellation.',
 'T51.12.c':'Actual three-reader/one-writer native workload on two effective allowed CPUs, recorded interval exclusion, independent set-based audit and15 equal-work timing samples against actual CMX mutex; separate sequential Cortex-M4 ABI/helper audit.'}

def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')

def attach(record):
    if record['project']!=PROJECT:return record
    ids=[]
    for criterion in record['criteria']:
        if criterion['objective'] in SCOPES:
            criterion['successor_support']={'path':SUPPORT,'id':criterion['id']};ids.append(criterion['id'])
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX writer preference, changed phase/single-upgrader policy, native exclusion/publication and equal-work observations; no physical cache counters or scheduler-time bound.'})
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=['labs/execution/sync_depth/'+name for name in ('rwlock.h','rwlock.c','rwlock_tests.c','rwphase.h','rwphase.c','rwphase_tests.c',
        'rw_native_tests.c','rw_m4.c','RWLOCK.md','run_rwlock.py','verify_rw_intervals.py')]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_rwlock_depth import validate_host,validate_native,validate_target
    require_batch_research(BASE)
    for path,validator in ((HOST,validate_host),(NATIVE,validate_native),(TARGET,validate_target)):
        validator(json.loads((ROOT/path).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
            'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
            'dimensions':{
                'teaching':{'state':'complete-for-declared-contract','evidence':[CONTRACT]},
                'implementation':{'state':'complete-for-declared-reference-and-changed-rw-policy','evidence':['assessor/legacy_concurrency_sync/rwlock.c','assessor/legacy_concurrency_sync/rwphase.c','assessor/execution/cmx.c','labs/execution/sync_depth/rw_native_tests.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,NATIVE,TARGET]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'C17 actual completed CMX imports; externally serialized core, native Linux pthread mutex/condition and two already-allowed CPUs; owned QEMU Cortex-M4 with explicit memory helpers.',
            'external_blockers':['Independent review and actual unaided delayed learner attempts.','Physical topology/cache-event/timing observations only for those physical claims.'],
            'evidence_limits':['Finite schedules and recorded intervals do not prove arbitrary weak-memory behavior, infinite starvation, numerical fairness bounds or absolute mastery.',
                'Native port uses a mutex/condition; it is not lock-free. QEMU image executes logical tasks sequentially, without IRQ/SMP-exclusion or context-switch claims.',
                'Elapsed samples include port/clock/trace/join costs and sanitizer/virtualization effects; no speed ordering, physical cache count or real-time bound is asserted.',
                'Task/control reuse requires quiescence; asynchronous owner death and arbitrary pthread failures are not implemented recovery paths.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Changed policy/native assumptions implemented and author-executed with stated bounds; delayed unaided learner evidence remains separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original T51.12 criteria and named phase/upgrade/native changed conditions have explicit current author support; no external/learner/physical closure is inferred.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,NATIVE,TARGET,CONTRACT,canonical.relative_to(ROOT).as_posix(),'tools/check_rwlock_depth.py',
        *['research/v5/expansion/legacy_concurrency_rwlock/'+n for n in ('author_support.py','resources.json','source_access.json','research_gate.json')]]
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_rwlock_depth.py','report':HOST},{'producer':'tools/run_rwlock_native.py','report':NATIVE},{'producer':'tools/run_rwlock_target.py','report':TARGET}],
        'scope':'Only original T51.12 criteria and exact delayed variants; no whole-domain, physical or learner completion.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
