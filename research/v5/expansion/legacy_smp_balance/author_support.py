"""Attach exact T53.08 successor without rewriting original criteria or receipts."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-08';SUPPORT='research/v5/expansion/legacy_smp_balance/support.json';CONTRACT='labs/smp_depth/balance/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_balance/campaign.json'
SCOPES={
'T53.08.a':'Effective versus pending affinity, exclusion of a running CPU only after actual saved-context finish, disallowed READY dispatch refusal and explicit generation/reservation ownership.',
'T53.08.b':'Actual ordered-lock queue reservation and commit/rollback, injected pre/post-reservation failure, finite wake/transfer schedules and generation/serial timer lease rerouting across an old CPU reference.',
'T53.08.c':'Actual pinned/movable saved-context workloads, complete actor/task causal traces, equivalent native work, repeated clock-scoped observations and a concrete balancing-cost counterexample.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual bounded affinity, saved-context movement, timer authority and pinned/movable workload comparisons.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/balance').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/smp_depth/locks/locks.h','labs/smp_depth/queues/queues.h','labs/smp_depth/wakeup/wakeup.h']
 for path in paths:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_balance_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-affinity-migration-and-timer-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-conditions','evidence':['assessor/legacy_smp/balance.c','assessor/legacy_smp/balance_guest.c','assessor/legacy_smp/balance_frames.S','labs/smp_depth/balance/oracle.py','labs/smp_depth/balance/native.c']},'author_validation':{'state':'passed-model-native-and-actual-saved-context-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-cache-or-WCET-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, two allowed native CPUs with actual pthread affinity and both TSan builds, independent transition/cost/trace models and original CMK QEMU TCG general-register saved-context guest.',
  'external_blockers':['Independent human review and seven-day unaided learner assessment remain unattempted.','Physical cache attribution/PMU or WCET claims require separately controlled physical hardware observations.'],
  'evidence_limits':['One caller per actor and all underlying operations through the wrapper; creation/reuse requires whole-world quiescence, stable port contexts and no live timer lease.','Pending affinity is cooperative: no forced preemption, CPU hotplug, TLS/FP/SIMD or arbitrary asynchronous callback cancellation.','The timer entry is a generation/serial-checked synchronous callback protocol, not a hardware timer ISR.','This policy comparison polls; sleeping destination/IPI evidence remains the separate frozen T53.07 adapter.','Native first/repeated sweep durations have multiple causes and do not isolate cache misses; virtual TSC intervals do not establish physical time or WCET.','Finite schedule/cost models and complete recorded trace consistency do not prove all C executions, fairness or a universal performance advantage.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Actual deferred running-mask boundary, timer migration/rerouting and pinned-task restricted placement are implemented and author-executed; delayed unaided learner evidence remains absent.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-08',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All original affinity/movement criteria and three named changed conditions have actual queue/wakeup composition, discriminating controls and source-bound author execution. Unsupported cache attribution and physical/independent evidence remain explicit, not substituted by timing averages.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_balance/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_balance_depth.py','assessor/legacy_smp/balance.md','research/v5/expansion/legacy_smp_balance/author_support.py',resource,'research/v5/expansion/legacy_smp_balance/source_access.json','research/v5/expansion/legacy_smp_balance/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_balance_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform/cache/PMU and calibrated timing evidence before expanded physical claims.'],'scope':'Exact T53.08 affinity, saved-context balancing and three original changed conditions; TLB and later memory/scheduling mechanisms remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
