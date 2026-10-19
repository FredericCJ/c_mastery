"""Attach the six exact rollover/latency criteria, preserving historical records."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T30-03';SUPPORT='research/v5/expansion/legacy_constrained_timing/support.json';CONTRACT='labs/constrained_depth/timing/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_constrained_timing/campaign.json'
SCOPES={
'T30.04.a':'Checked ceil duration conversion under explicit half-range limits,8-bit1ms and16-bit250us variants, and forced defined torn versus coherent atomic acquisition.',
'T30.04.b':'All65536 eight-bit timestamp pairs against independently lifted chronology plus49 sixteen-bit boundaries; actual sequence-qualified epoch snapshots with no-wrap publication, native readers and3432 finite SC schedules.',
'T30.04.c':'Distinct elapsed/order premises with10/266 indistinguishability, checked long-epoch duration, canonical40-byte persistence in two actual processes and explicit cross-reset boot-identity rejection.',
'T30.05.a':'Exact release/start/finish and signed-magnitude jitter metrics; preemption partitions wall execution from CPU service, with complete trace coverage and transactional invalid-order rejection.',
'T30.05.b':'Disjoint CPU/ISR/preemption/DMA/blocking trace partitions, CPU3+ISR2+queue2 response7 and separate DMA1 response8; no double counting or physical DMA claim.',
'T30.05.c':'Exact synthetic statistics and same-histogram correlation counterexample; actual48-sample dispersed/clustered host workload per profile with raw monotonic/thread-CPU clocks, resolution, checksum and independent summaries, never a future/WCET bound.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact bounded chronology/coherent epochs/latency metrics and six changed conditions. Historical execution/status remains unchanged; current author evidence is separately source-bound.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/constrained_depth/timing').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_constrained_timing import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-original-chronology-and-latency-contract-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'complete-for-six-original-criteria-and-six-named-variants','evidence':['assessor/legacy_constrained/timing.c','labs/constrained_depth/timing/tests.c','labs/constrained_depth/timing/oracle.py','labs/constrained_depth/timing/native.c','labs/constrained_depth/timing/measure.c','labs/constrained_depth/timing/persist.c']},'author_validation':{'state':'passed-six-core-profiles-eight-native-profiles-independent-models-and-actual-host-clocks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-actual-host-clocks-not-physical-peripheral-timing','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; native pthreads with GCC/Clang TSan; POSIX monotonic/thread-CPU clocks and exclusive temporary files; independent Python integers/event schedules/raw observations.',
   'external_blockers':['Independent human review and minimum-seven-day unaided learner transfer remain unattempted.','Physical peripheral timing and reset identity provisioning are not established by the declared host experiment.'],
   'evidence_limits':['Pairwise modular ordering requires an external true-separation premise; an out-of-contract missed wrap cannot generally be detected from two samples.','Epoch identity authority is supplied, not made crash-safe or authenticated by the40-byte serialization.','The3432-schedule model covers one SC publication/read attempt, not all C executions or weakest memory orders.','Host sleeps and synthetic DMA slices are explicitly different observations; no physical DMA/ISR latency is inferred.','Raw clock maxima and correlated samples do not prove WCET, independence or a future tail guarantee.','Atomic operations and host drivers have implementation-specific dependencies; no lock-free, signal-safe or freestanding guarantee.','Exact published C17 remains unavailable; N1570 is explicitly the C11 draft.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='The exact original changed condition has executed reference or declared-model support. Independent delayed learner work remains unattempted.');updates.append(child)
 resource='research/v5/expansion/legacy_constrained_timing/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_constrained_timing.py','assessor/legacy_constrained/timing.md','research/v5/expansion/legacy_constrained_timing/author_support.py',resource,'research/v5/expansion/legacy_constrained_timing/source_access.json','research/v5/expansion/legacy_constrained_timing/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_constrained_timing.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical target timing/reset identity observations remain separate.'],'scope':'PR-T30-03 exact bounded chronology/latency tasks only; no physical timing, arbitrary boot-authority or future-latency closure claim.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
