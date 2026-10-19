"""Attach exact pool successor without changing original contracts/history."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-11';SUPPORT='research/v5/expansion/legacy_smp_pool/support.json';CONTRACT='labs/smp_depth/pool/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_pool/campaign.json'
SCOPES={
'T53.11.a':'Capacity3 queue and separate accepted/queued/running/terminal ledger, exact original trace, queued versus claimed cancellation, finite world/generation cleanup, exhaustive bounded differential histories and actual three-parent child-submission refusal.',
'T53.11.b':'All0..4 allocation/create prefixes, actual custom pthread stack ownership, degraded failed-join retention with real retry, actual CMK four-thread admission failure and joined-stack arena recycling; no nonexistent target free syscall claim.',
'T53.11.c':'Independent exact job ledgers and equivalent static/dynamic modular work, actual1/2/4CPU guest placement, long-interval chunking preserving the mathematical result, fixed owner/thief deque with forced native stealing and explicit coarse-lock lifetime proof.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual bounded jobs, partial worker startup, retained failed joins, nested refusal, owned stack arena and fixed-array stealing. Original status remains historical; current exact evidence is in this successor.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=[p for p in (ROOT/'labs/smp_depth/pool').rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[ROOT/'labs/smp_depth/locks/locks.h',ROOT/'labs/smp_depth/locks/CONTRACTS.md']
 for path in sorted(p.relative_to(ROOT).as_posix() for p in paths):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_pool_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-job-startup-ownership-and-progress-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-condition-mechanisms','evidence':['assessor/legacy_smp/pool.c','assessor/legacy_smp/pool_guest.c','labs/smp_depth/pool/native.c','labs/smp_depth/pool/oracle.py']},'author_validation':{'state':'passed-host-native-and-actual-guest-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-timing-or-WCET-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, actual pthread custom stacks on four eligible CPUs with both TSan builds, independent state/unsigned-integer models and original CMK QEMU TCG1/2/4CPU guests.',
  'external_blockers':['Independent human review and seven-day unaided learner assessment remain unattempted.','Physical topology, calibrated timing and hardware observations remain required before making physical-performance claims.'],
  'evidence_limits':['Pool/launch/port/context and all worker-borrowed storage remain live until successful joins; same-world reset with surviving handles is forbidden.','Failed join preserves stack authority indefinitely if the port never succeeds; this is containment, not guaranteed recovery.','Fixed-array stealing is serialized by a coarse lock, with no resizing, speculative readers or lock-free reclamation claim.','The unchanged retained helper is inspected for its partial-startup early returns; its historical receipts are not rewritten or promoted to the new cleanup contract.','CMK has four process threads including the controller and no free syscall; the target releases joined stack leases to its arena, not kernel frames.','Actual timings include documented costs; sampled occupancy and admission-attempt timestamps do not equal exact high-water or enqueue-linearization measurements.','Mock premature-release controls record a violation without freeing actual live stacks. Native/guest evidence does not prove all C executions or physical WCET.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Concrete child-submission refusal/helping, failed-join retention/retry and fixed owner/thief deque implementations have author execution. Independent changed-condition learner performance remains unattempted.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-11',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='Original bounded-pool state, startup-failure, job-cost and named changed-condition mechanisms now have exact reference support and source-bound host/native/guest execution. Delayed learner, external human and physical evidence remain separate.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_pool/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_pool_depth.py','assessor/legacy_smp/pool.md','research/v5/expansion/legacy_smp_pool/author_support.py',resource,'research/v5/expansion/legacy_smp_pool/source_access.json','research/v5/expansion/legacy_smp_pool/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_pool_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical/calibrated performance observations before physical scaling or WCET claims.'],'scope':'Exact T53.11 mechanisms and original changed conditions; arbitrary runtime DAG mechanisms remain separate in T53.12.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
