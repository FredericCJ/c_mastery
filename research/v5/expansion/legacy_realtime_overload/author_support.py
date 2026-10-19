"""Bind T52.07 finite overload policy/transfer evidence without rewriting baseline bodies."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_overload/support.json'
CONTRACT='labs/realtime_depth/overload/CONTRACTS.md';PROJECT='PR-T52-07'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_overload/campaign.json'
SCOPES={
 'T52.07.a':'Capacity2 burst ledger distinguishes periodic CPU admission from application arrivals; reject-new/drop-oldest/latest preserve exact IDs, and actual CMX mutex/semaphore waits expose and repair the blocking-backpressure dependency cycle.',
 'T52.07.b':'Atomic candidate installation requires a complete empty-boundary cyclic proof; finite successful prefixes remain inconclusive. Explicit supervisor slots, per-request overrun budgets and criticality-mode shedding preserve high work and account for refused recovery.',
 'T52.07.c':'Ordinary/isolated-overrun/sustained and blocked-I/O histories retain every terminal/pending ID, exact bounded occupancy and heartbeat. Independent row expectations and seven semantic controls distinguish liveness from recovery and resource guarantees.'}


def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')

def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:
        criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Exact cyclic admission, visible queue/overrun/mode/IO policies and actual CMX wait-cycle repair; not current Linux scheduling, real I/O cancellation, physical timing or learner assessment.'})
    links.sort(key=lambda r:r['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/overload').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_overload_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
          'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-admission-and-overload-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-finite-reference-and-original-changed-conditions','evidence':['assessor/legacy_realtime/overload.c','labs/realtime_depth/overload/oracle.py','assessor/execution/cmx.c']},
            'author_validation':{'state':'passed-hosted-independent-oracle-and-emulated-model','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-timing-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 externally serialized finite model with explicit completed CMX import; six GCC/Clang hosted profiles, Python integer oracle and three strict QEMU Cortex-M4 profiles with audited course byte helpers.',
          'external_blockers':['Independent review, delayed unaided learner attempt and physical timing observations remain separate.'],
          'evidence_limits':['The complete cyclic proof requires synchronous implicit deadlines, fixed costs, tick preemption and the explicit supervisor calendar; no arbitrary sporadic, jitter or blocking theorem is claimed.','I/O eligibility is a finite copied-request model; no actual device, borrowed buffer cancellation or runtime port is claimed.','The dependency cycle uses actual completed CMX state operations, not native-thread timing or a physical scheduler.','Author execution and finite oracle agreement do not establish learner mastery, independent review or physical worst-case execution time.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
              original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact blocking-backpressure, criticality-mode and blocked-I/O changed conditions implemented and author-executed; independent seven-day learner attempt remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-07',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original overload criteria and three exact changed conditions have source-bound implementations/analyses and falsification evidence within their declared finite scope; physical timing and independent assessment remain separate.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_overload/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_overload_depth.py',
      'assessor/legacy_realtime/overload.md','research/v5/expansion/legacy_realtime_overload/author_support.py',resource_path,
      'research/v5/expansion/legacy_realtime_overload/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_overload_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_reference_work':[],
      'remaining_external_work':['Independent review and delayed unaided learner assessment.','Physical timing/accounting and actual I/O quiescence before applying physical response guarantees.'],
      'scope':'Only T52.07 three finite overload criteria and exact original variants; no closure of other realtime topics or unexecuted physical claims.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
