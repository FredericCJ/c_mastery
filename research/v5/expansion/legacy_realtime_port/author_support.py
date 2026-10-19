"""Attach the actual bounded task-port continuation without rewriting predecessor evidence."""
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T52-05';SUPPORT='research/v5/expansion/legacy_realtime_port/support.json'
CONTRACT='labs/realtime_depth/port/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_realtime_port/campaign.json'
PARENTS=['research/v5/expansion/legacy_realtime_server/support.json','research/v5/expansion/legacy_realtime_cohort/support.json']
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project']!=PROJECT:return record
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Additional actual three-PSP bounded per-dispatch port and pending-tick-collapse counterexample; all predecessor criterion scopes remain required.'})
    links.sort(key=lambda row:row['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    artifacts=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/realtime_depth/port').iterdir()) if p.is_file()]
    artifacts+=['labs/execution/include/cmx_port.h','labs/concurrency_lifecycle_depth/cmx/task.ld']
    for p in artifacts:
        if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
    return record
def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_server_port import validate_campaign
    require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    original=next(row for row in json.loads((ROOT/PARENTS[0]).read_text(encoding='utf-8'))['remaining_author_work'] if row['id']=='LRS-RTOS-SERVER-PORT')
    row={'id':'LRS-RTOS-SERVER-PORT','project':PROJECT,'original_source':PARENTS[0],'original_record':original,
      'criterion_ids':[r['id'] for r in record['criteria']],
      'closure_rationale':'Actual retained CMX SVC/PendSV/SysTick dispatch now composes the completed budget ledger with generation-bound task sleeps, conservative controlled-quantum charge, stronger preemption and close/exit/detach/reap. Actual five-wrap masking witness falsifies accounting outside its stated environment. This closes the exact bounded runtime-adapter author obligation, not arbitrary elapsed-service or physical timing guarantees.',
      'dimensions':{'teaching':{'state':'complete-for-declared-bounded-runtime-contract','evidence':[CONTRACT]},
        'implementation':{'state':'complete-for-declared-bounded-runtime-contract','evidence':['assessor/legacy_realtime/port.c','labs/realtime_depth/port/workload.c','labs/realtime_depth/port/oracle.py',*PARENTS]},
        'author_validation':{'state':'passed-hosted-oracle-and-actual-emulated-task-dispatch','evidence':[CAMPAIGN]},
        'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
        'physical_observation':{'state':'not-run-emulator-only','evidence':[]}},
      'required_environment':'Owned Cortex-M4 QEMU mps2-an386 guest, instruction-count mode, three real PSP contexts, fixed-priority saturated server with short masking and no voluntary payload block/yield. Hosted independent oracle under six GCC/Clang profiles.',
      'external_blockers':['Independent human review and unaided learner assessment.','Actual physical timing/stack/masking observations before any hardware timing claim.'],
      'evidence_limits':['The long-mask scenario intentionally violates the premise; observing five wraps and one charge is a falsification witness, not successful enforcement.','No arbitrary-blocking accounting clock, current POSIX implementation, SMP port, literal classical-server theorem or physical WCET is claimed.','Reference observations do not establish universal mastery; predecessor polling/cohort/ledger/overload scopes remain distinct.']}
    resources='research/v5/expansion/legacy_realtime_port/resources.json';source=json.loads((ROOT/resources).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_server_port.py',
      'research/v5/expansion/legacy_realtime_port/author_support.py',resources,'research/v5/expansion/legacy_realtime_port/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':[row],'predecessor_supports':PARENTS,
      'source_records':[{'id':r['id'],'path':resources,'passages':r['locators']} for r in source['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_server_port.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_external_work':['Physical timing/masking validation, independent human review and independent learner attempt.'],
      'scope':'Only exact LRS-RTOS-SERVER-PORT author obligation under its explicit finite single-core contract. No rewrite or replacement of earlier task evidence.'})
    print(json.dumps({'updates':1,'runtime_continuation':'LRS-RTOS-SERVER-PORT'}))
if __name__=='__main__':main()
