"""Attach exact T52.04 reference/variant evidence, preserving original assessment bodies."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_ceiling/support.json'
CONTRACT='labs/realtime_depth/ceiling/CONTRACTS.md';PROJECT='PR-T52-04'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_ceiling/campaign.json'
SCOPES={'T52.04.a':'Exact declared user/ceiling table and both-held/only-B restoration; actual PI/immediate core comparison and original PCP predicate distinction; undeclared priority1 refusal then fresh correctly recomputed table.',
 'T52.04.b':'Actual CMX-backed nested acquisition, cancellation and handoff/poll restoration; frozen base-priority policy rejects dynamic changes; timeout and sequence-exhaustion cleanup.',
 'T52.04.c':'Equivalent no-protocol/PI/immediate workloads with direct blocking, boosted execution and unrelated delay; explicit stable-current tie premise, conditional single-CPU argument and two-CPU counterexample to transplanted bound.'}

def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Declared users/nesting over actual CMX priority/ownership, equivalent workloads and exact changed conditions; emulator/model observations are not physical timing or an SMP port.'})
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/ceiling').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record
def main():
    sys.path.insert(0,str(ROOT/'tools'));from expansion_evidence import require_batch_research
    from check_ceiling_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),
          'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{'teaching':{'state':'complete-for-declared-user-and-proof-premise-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-declared-reference-and-changed-conditions','evidence':['assessor/legacy_realtime/ceiling.c','labs/realtime_depth/ceiling/oracle.py','assessor/execution/cmx.c']},
            'author_validation':{'state':'passed-hosted-ownership-oracle-and-emulated-model','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-latency-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 externally serialized finite model, actual completed CMX import; hosted GCC/Clang, independent Python resource/ownership oracle, owned QEMU Cortex-M4 and audited byte helpers.',
          'external_blockers':['Actual physical lock/scheduler latency and justified duration bounds before applying physical timing claims.','Independent human review and actual unaided delayed learner attempt.'],
          'evidence_limits':['The HLP bound requires the stated single-CPU, proper nesting, stable-current tie and no-self-suspension premises.','The original PCP predicate and SMP counterexample are bounded analyses, not complete implementations of those protocols.','Logical model ticks are not physical latency/WCET; no actual task port or learner mastery is inferred.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact new-user/frozen-base and two-CPU proof-premise conditions implemented or separately derived and author-executed as declared; delayed unaided learner evidence remains separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T52-04',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original ceiling criteria and exact changed conditions have source-bound author implementation/analysis and falsification evidence; physical latency and independent assessment remain separate.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_ceiling/resources.json';resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_ceiling_depth.py','assessor/legacy_realtime/ceiling.md',
      'research/v5/expansion/legacy_realtime_ceiling/author_support.py',resource_path,'research/v5/expansion/legacy_realtime_ceiling/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_ceiling_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],'remaining_external_work':['Physical latency/duration measurements before applying physical blocking bounds.','Independent review and delayed unaided learner assessment.'],
      'scope':'Only T52.04 declared-user immediate-ceiling criteria and exact analysis variants; other T52 server/overload/multicore/OS mechanisms remain required.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
