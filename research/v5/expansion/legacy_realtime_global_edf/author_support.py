"""Bind exact T52.10 criteria to global EDF, DAG/overlap and explicit overhead evidence."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_global_edf/support.json'
CONTRACT='labs/realtime_depth/global_edf/CONTRACTS.md';PROJECT='PR-T52-10'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_global_edf/campaign.json'
SCOPES={
 'T52.10.a':'Original long-job start1/finish11/deadline10 miss and four positive-cost scaling instances derive utilization approaching1 without substituting zero cost.',
 'T52.10.b':'Explicit task/generation retention, rejected overlapping releases, unique CPU ownership per sequential strand and acyclic ready-strand DAGs; full service/identity/tick conservation discriminates duplicate dispatch and overwrite controls.',
 'T52.10.c':'Equal-contract global/partitioned actual retained comparisons, anchored release jitter, serialized queue cost, migration charges and pinned-task histories distinguish finite tardiness, sufficient-bound failure and actual model throughput.'}

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
      'scope':'Exact explicit jobs/DAGs, actual retained comparison and finite overhead/affinity traces; no native SMP scheduler, physical overhead/WCET or universal schedulability guarantee.'})
    links.sort(key=lambda r:r['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/global_edf').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/src/advanced.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_global_edf_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
          'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-global-EDF-DAG-and-overhead-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_realtime/global_edf.c','labs/realtime_depth/global_edf/oracle.py','assessor/execution/advanced.c']},
            'author_validation':{'state':'passed-hosted-independent-traces-and-strict-M4-execution','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-WCET-and-migration-costs-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 finite externally serialized explicit-job model; six hosted profiles, independent Python node/event oracle and three strict M4 profiles with actual retained advanced.c simulator comparison and audited course helpers.',
          'external_blockers':['Independent review and delayed unaided learner attempts remain separate.','Physical queue/migration/WCET claims need actual selected-target observations; this finite model does not implement a native SMP scheduler.'],
          'evidence_limits':['Only the declared finite release/overlap/DAG/serialized-overhead policy is modeled; overhead values are explicit inputs.','The466 histories contain25,604 rows and intentional misses/rejections; successful model execution does not mean every input meets deadlines.','A failed sufficient global-EDF bound is inconclusive; its assumptions exclude affinities, DAG parallelism and modeled overhead.','Retained cmx_simulate comparisons exclude overlapping releases so its old overwrite policy cannot hide unfinished jobs.','Strict Cortex-M4 code/stack/helper observations are emulator model execution, not actual multicore or physical deadline validation.']}

        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
              original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact positive-small-cost scaling, acyclic ready-strand DAG or pinned-task restriction variant implemented and author-executed; the independent seven-day learner attempt remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-10',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original global-EDF criteria and exact changed conditions have bounded reference, independent full-trace oracle, actual retained comparison and strict-target evidence; physical/SMP claims and independent assessment remain explicit.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_global_edf/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_global_edf_depth.py',
      'assessor/legacy_realtime/global_edf.md','research/v5/expansion/legacy_realtime_global_edf/author_support.py',resource_path,
      'research/v5/expansion/legacy_realtime_global_edf/research_gate.json','research/v5/expansion/legacy_realtime_global_edf/source_notes.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_global_edf_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_external_work':['Independent review and delayed unaided learner assessment.','Physical queue/migration/WCET observations before asserting a physical scheduling guarantee.'],
      'scope':'Only T52.10 original criteria and exact changed conditions; separate finite explicit-job/actual CMX/strict M4 observations, with no closure of other realtime topics or physical/learner claims.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
