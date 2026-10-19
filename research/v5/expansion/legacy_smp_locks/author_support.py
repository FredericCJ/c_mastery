"""Attach exact T53.05 support without changing retained criteria or evidence."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-05';SUPPORT='research/v5/expansion/legacy_smp_locks/support.json';CONTRACT='labs/smp_depth/locks/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_locks/campaign.json'
SCOPES={
'T53.05.a':'Actual TAS/four-bit ticket/MCS representations,15-live admission, wrap publication, queued node retirement and same-node refusal; per-CPU task/interrupt node reuse with separate nodes for simultaneously nested locks.',
'T53.05.b':'Actual IF save/restore for initial0/1, two nested increasing-rank locks, exact LIFO release, remote progress while local IF0, real deferred self-IPI with owned nodes, and freestanding generated-code/helper audits.',
'T53.05.c':'Equal-work four-participant native/TSan observations, actual RC11 missing-release and wrap controls, finite lock-order cycle model, and same workload across two observed virtual NUMA nodes with explicit page-placement/physical-performance limits.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual finite lock/publication, IRQ ownership and equal-work changed-topology support.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/locks').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_lock_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-lock-ownership-and-placement-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-named-changed-conditions','evidence':['assessor/legacy_smp/locks.c','assessor/legacy_smp/locks_guest.c','assessor/legacy_smp/locks_frames.S','labs/smp_depth/locks/publication_model.c','labs/smp_depth/locks/wrap_model.c','labs/smp_depth/locks/oracle.py','labs/smp_depth/locks/native.c','labs/smp_depth/locks/numa_guest.py']},'author_validation':{'state':'passed-native-RC11-IRQ-and-virtual-NUMA-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-performance-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; native pthread plus both TSan; GenMC0.19 RC11 LLVM21.1.8; original CMK QEMU TCG IRQ guest; private Linux7.0.14 two-node NUMA guest with static GCC/Clang probes.',
  'external_blockers':['Independent human review and seven-day unaided learner assessments remain unattempted.','Physical NUMA/page placement/cache-traffic or timing claims require separately selected hardware and fresh evidence.'],
  'evidence_limits':['Exclusive noncopied guards/nodes, no cancellation/forced reclamation while queued or held; all participating ranked paths must share global order.','Real IF masks only local maskable interrupts, not NMI or other CPUs; scheduler migration/context switching while held is excluded.','RC11 evidence covers selected bounded programs and spin assumptions; native/TSan and finite histories do not prove all C executions or fairness.','Virtual NUMA CPU membership/affinity is executed; physical topology, page locality and universal performance ranking are not established.','Selected RMW counters omit admission/node-validation operations and are not bus-traffic counters.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Per-CPU node reuse, nested ranked IF restoration or actual virtual-NUMA placement transfer implemented and author-validated; delayed learner evidence remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-05',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named finite original and changed T53.05 mechanisms have concrete source-bound implementation, target/native/model evidence and discriminating controls; physical and independent assessment remain distinct.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_locks/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_lock_depth.py','assessor/legacy_smp/locks.md','research/v5/expansion/legacy_smp_locks/author_support.py',resource,'research/v5/expansion/legacy_smp_locks/source_access.json','research/v5/expansion/legacy_smp_locks/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_lock_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform evidence for any physical placement/cache/timing claim.'],'scope':'Only exact T53.05 finite locks, IRQ ownership and named changed conditions; source editions, model assumptions and virtual measurements retain explicit limits.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
