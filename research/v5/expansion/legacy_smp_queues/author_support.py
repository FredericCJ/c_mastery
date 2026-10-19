"""Attach exact T53.06 successor without changing accepted criteria/evidence."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-06';SUPPORT='research/v5/expansion/legacy_smp_queues/support.json';CONTRACT='labs/smp_depth/queues/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_queues/campaign.json'
SCOPES={
'T53.06.a':'Exclusive queue/execution ownership, fixed task generations and revisions, explicit two-sided TRANSFER reservation/rollback, and actual saved-stack migration CPU0 to CPU1.',
'T53.06.b':'Bounded per-CPU FIFO dispatch/finish/wake with full-destination unchanged ownership, running return reservations, duplicate wake/stale generation refusals and equivalent coarse/per-queue policies.',
'T53.06.c':'Actual1/2/4-CPU switch/clock observations, independent actor/task causal trace validation, exhaustive recorded revision checks, independent state-model comparisons and sampled-history inconclusiveness.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual per-CPU queue ownership, saved-context transfer and complete/sampled observation contracts.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/queues').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/smp_depth/locks/locks.h']
 for path in paths:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_queue_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-bounded-queue-and-saved-context-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-conditions','evidence':['assessor/legacy_smp/queues.c','assessor/legacy_smp/queues_guest.c','assessor/legacy_smp/queues_frames.S','labs/smp_depth/queues/oracle.py','labs/smp_depth/queues/native.c','labs/smp_depth/queues/model_driver.c']},'author_validation':{'state':'passed-model-native-and-actual-saved-context-guest-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-timing-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, native pthread with both TSan builds, Python independent queue/trace oracle and original CMK QEMU TCG general-register saved-context guest.',
  'external_blockers':['Independent human review and seven-day unaided learner assessments remain unattempted.','Physical target timing/clock claims require separately selected hardware and evidence.'],
  'evidence_limits':['One actor caller per CPU, no interrupt queue operations or concurrent generation creation/reuse; world snapshots require quiescence.','Actual saved-stack migration is general-register-only in one address space, without TLS/preemption or held task locks.','Polling idle is not an idle-notification or lost-wakeup proof; T53.07 remains a separate required mechanism.','Complete trace consistency assumes faithful instrumentation; sampled logs explicitly remain inconclusive.','Sequential state-model and selected native/guest runs do not prove every C execution or fairness; virtual TSC fractions are not physical utilization/WCET.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Explicit TRANSFER with actual saved-stack continuation, equivalent coarse-lock workload or sampled-log inconclusive validator is implemented and author-executed; delayed unaided learner evidence remains absent.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-06',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named original queue/state/trace criteria and changed conditions have concrete source-bound implementation and discriminating validation. Idle notification, physical timing, external review and learner evidence are not inferred.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_queues/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_queue_depth.py','assessor/legacy_smp/queues.md','research/v5/expansion/legacy_smp_queues/author_support.py',resource,'research/v5/expansion/legacy_smp_queues/source_access.json','research/v5/expansion/legacy_smp_queues/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_queue_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform evidence for any physical timing/clock claim.'],'scope':'Exact T53.06 finite per-CPU queues, saved-context ownership and three named changed conditions; later notification and global scheduling obligations remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
