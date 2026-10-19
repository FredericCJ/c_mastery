"""Attach source-bound unit-cohort continuation while retaining the per-dispatch predecessor."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T52-05';SUPPORT='research/v5/expansion/legacy_realtime_cohort/support.json'
PARENT='research/v5/expansion/legacy_realtime_server/support.json'
CONTRACT='labs/realtime_depth/cohort/CONTRACTS.md'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_cohort/campaign.json'
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project']!=PROJECT:return record
    # Keep the existing per-criterion original successor pointer: its ledger,
    # polling and overload obligations are prerequisites, not superseded code.
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Additional exact cohort-age/debt and117-versus99 victim-trace evidence; retains the prior per-dispatch, finite-ledger and overload contract.'})
    links.sort(key=lambda row:row['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    for p in sorted((ROOT/'labs/realtime_depth/cohort').iterdir()):
        if p.is_file() and p.relative_to(ROOT).as_posix() not in record['supplied_artifacts']:
            record['supplied_artifacts'].append(p.relative_to(ROOT).as_posix())
    return record
def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_cohort_depth import validate_campaign
    require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),
          'closure_rationale':'Retains exact predecessor criteria/changed conditions, and adds full finite unit-cohort state across late/preempted replenishment, actual historical-defect victim trace and explicit overrun debt. This is an original corrected model, not a claim of current POSIX or literal revised-paper implementation.',
          'dimensions':{
            'teaching':{'state':'complete-for-declared-original-plus-cohort-contracts','evidence':[CONTRACT,'labs/realtime_depth/server/CONTRACTS.md']},
            'implementation':{'state':'complete-for-declared-reference-and-changed-conditions','evidence':['assessor/legacy_realtime/cohort.c','labs/realtime_depth/cohort/specimens.c','labs/realtime_depth/cohort/oracle.py',PARENT]},
            'author_validation':{'state':'passed-hosted-history-witness-and-emulated-model','evidence':[CAMPAIGN,'reviews/v5/expansion/legacy_realtime_server/campaign.json']},
            'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-logical-timing-only','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 finite serialized unit rights, actual completed per-dispatch and CMX advanced imports; six host profiles, independently specified histories/timing witness, owned audited Cortex-M4 image.',
          'external_blockers':['Revised2011 paper full pseudocode access remains a separate reference gap.','Physical enforcement/timing observations before deployment timing claims.','Independent human review and unaided delayed learner evidence.'],
          'evidence_limits':['Q<=32 and bounded observed overrun; no universal scheduling proof or current POSIX conformance claim.','Borrowed service compensation does not undo the first overrun.','Parent delayed-merge/polling/overload obligations remain part of the contract; cohort evidence alone does not replace them.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],
              minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Exact predecessor changed condition plus new age/debt/preemption counterexample reference are implemented and author-executed; independent learner timing remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='LRS-CLASSICAL-LONG-ACTIVE',original_source=PARENT,
      closure_rationale='Original flawed active-interval/credit target reproduces premature return117 and capacity amplification5>4; corrected unit-cohort model retains distinct50/40 anchors, repayment rights and long-active behavior, producing victim completion99. Original derivation avoids misrepresenting unavailable revised pseudocode.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);updates.append(gap)
    shared='research/v5/expansion/legacy_realtime_server/resources.json';resources=json.loads((ROOT/shared).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_cohort_depth.py','assessor/legacy_realtime/cohort.md',
      'research/v5/expansion/legacy_realtime_cohort/author_support.py','research/v5/expansion/legacy_realtime_cohort/resources.json',
      'research/v5/expansion/legacy_realtime_cohort/research_gate.json',shared]
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'predecessor_support':PARENT,
      'source_records':[{'id':r['id'],'path':shared,'passages':r['locators']} for r in resources['resources'] if r['id'] in ('R-LRS-BOOK','R-LRS-SPRUNT','R-LRS-DEFECTS')],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_cohort_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[{'id':'LRS-RTOS-SERVER-PORT','required':True,'state':'unimplemented','task':'Compose actual scheduler/task/interrupt accounting and bounded enforcement; logical emulator execution alone is insufficient.'}],
      'remaining_external_work':['Exact revised2011 pseudocode source access, independent human/learner evidence and physical timing.'],
      'scope':'Exact unit-cohort/debt and historical-defect continuation with explicit predecessor composition; no whole scheduling-domain closure.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3,'continuation':1}))
if __name__=='__main__':main()
