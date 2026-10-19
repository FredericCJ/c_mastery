"""Attach exact T52.05 policy/ledger/overload evidence without rewriting history."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_server/support.json'
CONTRACT='labs/realtime_depth/server/CONTRACTS.md';PROJECT='PR-T52-05'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_server/campaign.json'
SCOPES={
 'T52.05.a':'Actual retained per-dispatch credit adapter plus background/polling models and explicit preempted active-interval grouped comparison; comparison is limited to complete intervals shorter than P, not a complete classical server.',
 'T52.05.b':'Fixed16 rejection/coalescing, modular wrap and half-range refusal, actual retained due-progress observation versus transactional wrapper; smaller-ledger conservative delayed merge.',
 'T52.05.c':'Complete equivalent-input three-policy request/tick traces, early/duplicate replenishment controls and finite-window credit argument; sustained overload explicitly conserves admitted work and preserves supervisor service.'}

def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')

def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Exact per-dispatch storage/failure and overload contracts; bounded grouping comparison does not establish a complete classical server or actual RTOS timing.'})
    links.sort(key=lambda row:row['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/server').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/src/advanced.c',
             'labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_server_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),
          'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-per-dispatch-and-bounded-group-comparison','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-declared-reference-and-exact-changed-conditions','evidence':['assessor/legacy_realtime/server.c','labs/realtime_depth/server/oracle.py','assessor/execution/advanced.c']},
            'author_validation':{'state':'passed-hosted-credit-job-oracle-and-emulated-model','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-server-timing-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 serialized finite models, explicitly completed CMX advanced producer; GCC/Clang host and Python token/job oracle; owned QEMU Cortex-M4 with audited course byte helpers.',
          'external_blockers':['Physical timing observations before physical server-interference claims; actual runtime adapter implementation remains author work below.','Independent human review and actual delayed unaided learner attempt.'],
          'evidence_limits':['Full classical active intervals spanning replenishment boundaries remain a separate required author obligation.','The strict sliding-window unit-credit argument applies to the per-dispatch policy, not the grouped comparison or polling.','Overload latency comparisons must disclose policy-dependent admitted sets.','No actual RTOS server port, physical WCET or absolute learner mastery is inferred.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],
                         minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                         variant_readiness='Exact preempted grouping/smaller-ledger delayed merge/sustained overload condition implemented and author-executed in its stated finite contract; independent delayed learner attempt remains separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-05',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original criteria and exact changed conditions have bounded reference/analysis and falsification evidence; complete classical server continuation is explicitly retained as additional author work.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_server/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_server_depth.py','assessor/legacy_realtime/server.md',
          'research/v5/expansion/legacy_realtime_server/author_support.py',resource_path,'research/v5/expansion/legacy_realtime_server/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_server_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[{'id':'LRS-CLASSICAL-LONG-ACTIVE','required':True,'state':'unimplemented',
          'task':'Reproduce the historical active-interval/cohort and amplification defects across replenishment boundaries; implement explicitly corrected cohort/debt accounting and test it against a separately derived model. Do not promote the disputed book equivalence claim to a guarantee.'},
        {'id':'LRS-RTOS-SERVER-PORT','required':True,'state':'unimplemented',
          'task':'Compose a server with an actual scheduler/task/interrupt adapter and retain policy-specific accounting and preemption evidence; logical tick execution alone is insufficient.'}],
      'remaining_external_work':['Physical timing observations before physical interference claims.','Independent review and delayed unaided learner assessment.'],
      'scope':'T52.05 exact original per-dispatch/finite-ledger/overload criteria and changed-condition references; full classical continuation and real runtime port remain required.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3,'remaining_author':2}))

if __name__=='__main__':main()
