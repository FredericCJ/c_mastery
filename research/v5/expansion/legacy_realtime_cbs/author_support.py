"""Bind T52.06 finite CBS policy/transfer evidence without rewriting baseline bodies."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_cbs/support.json'
CONTRACT='labs/realtime_depth/cbs/CONTRACTS.md';PROJECT='PR-T52-06'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_cbs/campaign.json'
SCOPES={
 'T52.06.a':'Exact soft service state pairs and finish8 versus periodic throttle22; actual competing Q2/P5 EDF stream changes completion to14, with separate application deadlines and every priority transition.',
 'T52.06.b':'Actual retained CMX soft-CBS operations with explicit strict/book non-strict equality adapter, transactional oversized charge and modular/full-width boundaries; independently sourced finite Q2/D4/P10 hard-policy comparison preserves explicit late-refill refusal.',
 'T52.06.c':'Two-client overrun histories retain every request/tick and discriminating throttle/no-charge controls; actual CMX shared-mutex model charges the running owner and demonstrates deadline3 versus8 before applying any isolation theorem.'}

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
      'scope':'Exact finite soft/book/hard policy distinctions, independent workloads and actual CMX mutex model; not current Linux execution, arbitrary isolation proof, physical timing or learner assessment.'})
    links.sort(key=lambda r:r['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/cbs').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_cbs_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
          'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-policy-and-proof-premise-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-finite-reference-and-original-changed-conditions','evidence':['assessor/legacy_realtime/cbs.c','labs/realtime_depth/cbs/oracle.py','assessor/execution/cmx.c']},
            'author_validation':{'state':'passed-hosted-independent-oracle-and-emulated-model','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-timing-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 externally serialized finite model with explicit completed CMX import; six GCC/Clang hosted profiles, Python integer oracle and three strict QEMU Cortex-M4 profiles with audited course byte helpers.',
          'external_blockers':['RETIS TR98-01 full isolation-proof report remains unavailable; inspected theorem statements are not inspection of that proof.','Independent review, delayed unaided learner attempt and physical timing observations remain separate.'],
          'evidence_limits':['The 600 finite workloads include595 represented traces and5 explicit late-hard-model refusals; they do not prove arbitrary histories or the full isolation theorem.','Constrained D comparison is an original finite implementation of inspected documentation, not an actual Linux SCHED_DEADLINE observation.','The shared mutex uses actual CMX logical task transitions, not bandwidth inheritance or a native/physical task port.','Emulated model ticks, code/stack observations and actual compiler checks do not establish physical worst-case execution time.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
              original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact competing EDF, sourced constrained deadline or actual CMX mutex changed condition implemented and author-executed; independent seven-day learner attempt remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-06',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original CBS criteria and three changed conditions have explicit source-bound implementations/analyses and falsification evidence within their declared finite scope; unavailable full proof, physical timing and independent assessment remain separate.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_cbs/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_cbs_depth.py',
      'assessor/legacy_realtime/cbs.md','research/v5/expansion/legacy_realtime_cbs/author_support.py',resource_path,
      'research/v5/expansion/legacy_realtime_cbs/research_gate.json','research/v5/expansion/legacy_realtime_cbs/source_access.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_cbs_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_reference_work':['LRCBS-REF-TR98: obtain and inspect the separately cited full isolation proof; preserve current access failure record.'],
      'remaining_external_work':['Independent review and delayed unaided learner assessment.','Physical timing/accounting bounds before applying a physical isolation guarantee.'],
      'scope':'Only T52.06 three finite CBS criteria and exact original variants; no closure of other realtime topics, unavailable proof inspection or unexecuted physical claims.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
