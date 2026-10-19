"""Attach exact fixed-priority/EDF job-ledger evidence without rewriting original criteria."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_jobs/support.json'
CONTRACT='labs/realtime_depth/jobs/CONTRACTS.md';PROJECTS=('PR-T52-01','PR-T52-02')
CAMPAIGN='reviews/v5/expansion/legacy_realtime_jobs/campaign.json'
SCOPES={
 'T52.01.a':'Actual stable RM/DM ordering, harmonic counterexample and exact rational sufficient-bound comparison; explicit jitter generation and changed RTA arrival-bound premise.',
 'T52.01.b':'Transactional wrapper of actual CMX priority producer, two releases/task, rejected-input preservation; D>T simultaneous job identities and explicit bounded backlog dispositions.',
 'T52.01.c':'Chosen feasible/infeasible costs, actual RTA and RM/DM event traces, per-preemption accounting; nonpreemptible section and derived two-unit blocking counterexample.',
 'T52.02.a':'Exact A/B/C absolute-deadline trace, stable tie and every remaining-service state; simultaneous same-task backlog retained or explicitly dropped.',
 'T52.02.b':'Bounded active set and retained identity ledger; deadline-before-release and horizon2/3 demand counterexample; arbitrary-deadline overload with protected nonpreemptible victim exclusion.',
 'T52.02.c':'Independent hand trace, event oracle and enumerated demand counts; key/miss/tie/endpoint falsification; one preemption-overhead tick changes formerly on-time completion to a miss.'}
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project'] not in PROJECTS:return record
    for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Actual CMX priority/demand and RTA imports; explicit job identities, bounded overload, jitter/blocking/overhead changed conditions. Model/emulator observations are not physical timing.'})
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/jobs').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/src/cmx.c','labs/execution/src/advanced.c','labs/execution/src/support.c',
      'labs/realtime_depth/response/response.h','labs/realtime_depth/response/response.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record
def main():
    sys.path.insert(0,str(ROOT/'tools'));from expansion_evidence import require_batch_research
    from check_jobs_depth import validate_campaign
    require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    updates=[];canonical_paths=[]
    for project in PROJECTS:
        canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');canonical_paths.append(canonical.relative_to(ROOT).as_posix())
        record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);project_updates=[]
        for criterion in record['criteria']:
            row={'id':criterion['id'],'project':project,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),
              'closure_rationale':SCOPES[criterion['objective']],
              'dimensions':{'teaching':{'state':'complete-for-explicit-model-contract','evidence':[CONTRACT]},
                'implementation':{'state':'complete-for-declared-reference-and-changed-conditions','evidence':['assessor/legacy_realtime/jobs.c','labs/realtime_depth/jobs/oracle.py','assessor/execution/advanced.c','assessor/execution/cmx.c','assessor/legacy_realtime/response.c']},
                'author_validation':{'state':'passed-hosted-ledger-oracle-and-emulated-model','evidence':[CAMPAIGN]},
                'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-physical-timing-not-claimed','evidence':[]}},
              'variant_ids':[v['id'] for v in criterion['variants']],
              'required_environment':'C17 fixed arrays/exact uint32/uint64, explicit completed CMX/RTA imports; hosted GCC/Clang, independent Python integer event oracle and owned QEMU Cortex-M4 with audited byte helpers.',
              'external_blockers':['Actual physical cost/jitter/blocking measurements before applying corresponding physical schedulability claims.','Independent human review and actual unaided delayed learner attempt.'],
              'evidence_limits':['Finite one-CPU traces and demand horizons do not prove universal feasibility or arbitrary-release optimality.','QEMU executes the model; this is not an actual RTOS scheduler port or measured WCET.','The RTA arrival bound is an explicit premise, not an uninspected general jitter theorem.','Separate T52 server/multicore/timer/OS obligations remain required.']}
            project_updates.append(row)
            for variant in criterion['variants']:
                child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                  variant_readiness='Exact changed condition implemented and author-executed; original seven-day unaided learner assessment remains unattempted.')
                project_updates.append(child)
        gap=deepcopy(project_updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+project,original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
          closure_rationale='All three original criteria and their exact changed model conditions have source-bound implementation/oracle/control evidence; no global schedulability, physical or learner completion claim.',
          criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);project_updates.append(gap);updates.extend(project_updates)
    resource_path='research/v5/expansion/legacy_realtime_jobs/resources.json';resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,*canonical_paths,'tools/check_jobs_depth.py','assessor/legacy_realtime/jobs.md','research/v5/expansion/legacy_realtime_jobs/author_support.py',
      resource_path,'research/v5/expansion/legacy_realtime_jobs/research_gate.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_jobs_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],'remaining_external_work':['Physical timing/arrival/blocking measurement before applying physical bounds.','Independent review and delayed unaided learner assessment.'],
      'scope':'Only T52.01/.02 model criteria and exact variants; other original T52 obligations remain required.'})
    print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
