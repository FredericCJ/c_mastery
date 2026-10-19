"""Attach exact T52.03 reference/variant evidence, preserving original assessment bodies."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_response/support.json'
CONTRACT='labs/realtime_depth/response/CONTRACTS.md';PROJECT='PR-T52-03'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_response/campaign.json'
SCOPES={'T52.03.a':'Derive the explicit arrival-bound recurrence; exact3,4,4 and jittered3,4,5,5 traces; D4 changed condition returns first exceeded5.',
 'T52.03.b':'Transactional checked full-width ceiling/product/sum and monotone iteration; distinct converged/exceeded/limit/invalid/overflow statuses and real Cortex-M4 helper closure.',
 'T52.03.c':'Independent integer oracle and discriminating arithmetic/assumption controls; separate per-job overhead and explicit IRQ interference with3,5,7,8,8 trace; actual physical cost measurements remain unobserved.'}
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Actual original CMX comparison, full-width checked response model and exact changed conditions; model and emulator evidence are not physical measurements.'})
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/response').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record
def main():
    sys.path.insert(0,str(ROOT/'tools'));from expansion_evidence import require_batch_research
    from check_response_depth import validate_campaign
    require_batch_research(ROOT/'research/v5/expansion/legacy_realtime')
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),
          'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{'teaching':{'state':'complete-for-explicit-model-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-declared-reference-and-changed-conditions','evidence':['assessor/legacy_realtime/response.c','labs/realtime_depth/response/oracle.py','assessor/execution/cmx.c']},
            'author_validation':{'state':'passed-hosted-oracle-and-emulated-arithmetic','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-cost-measurements-remain','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 exact uint64, explicit completed CMX import; hosted GCC/Clang, independent Python big integers and owned QEMU Cortex-M4 with audited original byte helpers.',
          'external_blockers':['Actual physical release-jitter/dispatch/IRQ measurements and justified worst-case parameter derivation for physical claims.','Independent human review and actual unaided delayed learner attempt.'],
          'evidence_limits':['The arrival-count bound is an explicit premise of original course analysis, not an inspection or proof of an unseen general jitter theorem.','Finite oracle cases and emulator arithmetic do not establish WCET, arbitrary schedules, full T52 closure or mastery.','Cost table entries are assumptions/derivations unless actually measured; no missing physical values are fabricated.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact changed deadline/full-width arithmetic/interrupt model implemented and author-executed; delayed unaided learner evidence remains separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T52-03',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original response criteria and exact changed conditions have source-bound author reference and falsification evidence; physical cost measurement and independent assessment remain explicitly open.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime/resources.json';resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_response_depth.py','assessor/legacy_realtime/response.md',
      'research/v5/expansion/legacy_realtime_response/author_support.py',resource_path,'research/v5/expansion/legacy_realtime/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_response_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],'remaining_external_work':['Physical cost/jitter/IRQ measurements before corresponding physical bounds.','Independent review and delayed unaided learner assessment.'],
      'scope':'Only T52.03; remaining scheduler, overload, server, multicore and OS/RTOS comparisons stay required separate implementations.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
