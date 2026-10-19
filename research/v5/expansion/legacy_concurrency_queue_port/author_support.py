"""Bind actual CMX queue/IRQ/PSP completion without rewriting the earlier scoped observations."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_queue_port/support.json'
PRIOR='research/v5/expansion/legacy_concurrency_lifecycle/support.json'
CONTRACT='labs/concurrency_lifecycle_depth/cmx/CONTRACTS.md'
SCOPES={
 'T51.06.a':'Actual CL copied-byte acceptance integrated with CMX latch waits; original owned-buffer changed-condition implementation remains separately bound by prior support.',
 'T51.06.b':'Actual two-slot byte/full/short-output/zero-length contract, blocked sender and close/drain; original fragmentation variant remains separately bound.',
 'T51.06.c':'Actual CMX scheduler and six PSP stacks carry three producers/two consumers with60 unique messages; timed-out send and full abort; original acknowledgement model remains separately bound.',
 'T51.14.a':'Actual queue admission/registration/terminal-result lifecycle, old object/task/ticket identity refusal, software IRQ implicit lease joined before reclaim; original timer-join variant remains separately bound.',
 'T51.14.b':'Retained CMX planned-exit refusal remains distinct from the original executed robust owner/repairer-death record protocol; this adapter does not invent asynchronous task-kill recovery.',
 'T51.14.c':'Actual condition timeout waits through mutex reacquisition across close, all references/results collected before destroy, task exit/reap before stack reuse and generation retirement; original repaired/unrecoverable record and supervisor variants remain separately bound.'}
def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
    if record['project'] not in ('PR-T51-06','PR-T51-14'):return record
    ids=[]
    for criterion in record['criteria']:
        if criterion['objective'] not in SCOPES:continue
        ids.append(criterion['id']);links=criterion.setdefault('additional_successor_support',[])
        links[:]=[r for r in links if r.get('path')!=SUPPORT]
        links.append({'path':SUPPORT,'id':criterion['id'],'scope':SCOPES[criterion['objective']]})
    links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual copied-byte CMX wait/IRQ/PSP runtime adapter; prior owned-buffer/fragmentation/acknowledgement/robust-record scopes preserved separately.'})
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/concurrency_lifecycle_depth/cmx').iterdir() if p.is_file()]
    public+=['labs/execution/include/'+n for n in ('cmx.h','cmx_port.h')]
    public+=['labs/execution/src/'+n for n in ('startup.S','context.S','cortex_port.c','board.c','support.c','cmx.c')]
    public+=['labs/concurrency_lifecycle_depth/include/cl.h','labs/concurrency_lifecycle_depth/src/core.c',
      'labs/concurrency_lifecycle_depth/targets/m4.ld','labs/concurrency_lifecycle_depth/wait/irq.h','labs/concurrency_lifecycle_depth/wait/wait.h']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record
def main():
    sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_queue_port import validate_host,validate_target,validate_tasks
    from research.v5.expansion.legacy_concurrency_lifecycle import author_support as old
    require_batch_research(BASE)
    reports=['reviews/v5/expansion/legacy_concurrency_queue_port/'+name+'.json' for name in ('host','target','tasks')]
    for path,validator in zip(reports,(validate_host,validate_target,validate_tasks)):validator(json.loads((ROOT/path).read_text(encoding='utf-8')))
    canonical=[]
    for project in ('PR-T51-06','PR-T51-14'):
        path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');write(path,attach(json.loads(path.read_text(encoding='utf-8'))));canonical.append(path)
    # Rebind only the earlier additive author support to the additive metadata.
    # Its original experiment scopes and accepted/historical receipts stay intact.
    old.main()
    prior=json.loads((ROOT/PRIOR).read_text(encoding='utf-8'));updates=[]
    for previous in prior['updates']:
        row=deepcopy(previous);row['prior_support']={'path':PRIOR,'id':row['id']}
        objective=row.get('original_objective') or row.get('objective')
        if not objective and row['id'].startswith('AX-'):objective=row['id'].split('-')[-1].removesuffix('-DELAYED')
        if row['id'].endswith('-DELAYED'):objective=row['id'].split('-')[-2]
        row['closure_rationale']=SCOPES.get(objective,'The previously missing CMX task/blocking/IRQ adapter now has actual source imports and separately executed core, IRQ and PSP runtime observations; prior changed-condition mechanisms remain required and separately bound.')
        dimensions=row['dimensions']
        dimensions['teaching']={'state':'complete-for-composed-contract','evidence':[CONTRACT,'labs/concurrency_lifecycle_depth/CONTRACTS.md']}
        dimensions['implementation']={'state':'complete-for-declared-reference-and-runtime-composition','evidence':['assessor/concurrency_lifecycle_depth/cl_cmx.c','assessor/concurrency_lifecycle_depth/cl_cmx_irq.c','assessor/concurrency_lifecycle_depth/cl_cmx_task.c',PRIOR]}
        dimensions['author_validation']={'state':'passed-hosted-IRQ-and-actual-PSP-with-stated-scope','evidence':reports+[PRIOR]}
        dimensions['external_review']={'state':'not-independent-human-reviewed','evidence':[]}
        dimensions['independent_learner']={'state':'not-assessed','evidence':[]}
        dimensions['physical_observation']={'state':'not-run-no-physical-claim','evidence':[]}
        row['required_environment']='C17 explicit completed CL and CMX imports; serialized hosted controls; owned Cortex-M4 QEMU IRQ0 and six actual PSP stacks; no target libraries beyond audited original byte helpers.'
        row['evidence_limits']=['Prior robust/owned/fragmentation/acknowledgement/supervisor mechanisms retain their original hosted or model scope.','IRQ0 quiescence assumes the dedicated software-only source cannot be repended after join; no DMA/peripheral join claim.','Actual PSP task evidence does not establish arbitrary schedules, SMP, WCET, physical behavior, asynchronous kill recovery or learner mastery.','No external human review or delayed learner attempt is manufactured.']
        updates.append(row)
    pins=[CONTRACT,PRIOR,*reports,*[p.relative_to(ROOT).as_posix() for p in canonical],
      'assessor/concurrency_lifecycle_depth/cl_cmx.md','tools/check_queue_port.py',
      *['research/v5/expansion/legacy_concurrency_queue_port/'+n for n in ('author_support.py','resources.json','research_gate.json')]]
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'prior_support':PRIOR,
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_queue_port_host.py','report':reports[0]},{'producer':'tools/run_queue_port_target.py','report':reports[1]},{'producer':'tools/run_queue_tasks.py','report':reports[2]}],
      'remaining_author_work':[],'remaining_external_work':['Independent human review and actual unaided delayed learner attempts.','Physical observations and device-specific quiescence proof before corresponding physical/peripheral claims.'],
      'scope':'Six original criteria/six named changed-condition children and two original task-adapter gap rows; original implementation evidence retained through explicit prior support, not relabeled as target execution.'})
    print(json.dumps({'updates':len(updates),'criteria':6,'variants':6,'gaps':2}))
if __name__=='__main__':main()
