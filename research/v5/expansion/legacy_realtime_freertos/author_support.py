"""Additive original T52.11 binding to separate actual CMX/FreeRTOS adapters."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T52-11';SUPPORT='research/v5/expansion/legacy_realtime_freertos/support.json';CONTRACT='labs/realtime_depth/freertos/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_realtime_freertos/campaign.json'
SCOPES={
'T52.11.a':'Actual eight-slot copied queue count256/sum32640, full/empty timed outcomes and explicitly distinct priority/tick/allocation/ISR/shutdown contracts; actual pinned POSIX and ARM_CM4F vendor ports versus actual CMX transition/scheduler code.',
'T52.11.b':'Separate source/link closures, actual static FreeRTOS object allocation and allocator-free strict M4 image, bounded inversion and quiescent cleanup, plus checked ceiling mapping at100Hz/1000Hz without claiming a wall-time not-before promise.',
'T52.11.c':'Controlled owner-release/high-acquire/middle ordering, actual nested-mutex disinheritance counterexample and discriminating priority/floor/binary-semaphore/ISR-yield mutants; two compilers/optimization profiles and actual emulated port execution retain environment limits.'}
def write(path,data):path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
 links.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Separate actual CMX transition/scheduler and FreeRTOS POSIX/ARM_CM4F comparison; no physical timing or new CMX PSP-port claim.'});links.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/freertos').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 paths+=['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
 for path in sorted(paths):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_freertos_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-comparison-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_realtime/freertos_mapping.c','assessor/legacy_realtime/freertos_original.c','assessor/legacy_realtime/freertos_vendor.c','labs/realtime_depth/freertos/oracle.py']},'author_validation':{'state':'passed-separate-hosted-and-actual-emulated-static-MCU-port','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-timing-not-claimed','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'Pinned clean FreeRTOS V11.2.0; six hosted GCC/Clang profiles, independent integer oracle; GCC-O0/GCC-O3/Clang-O3 strict Cortex-M4 and owned QEMU; rates100/1000.',
  'external_blockers':['Independent human review and delayed unaided learner attempts remain separate.','Physical MCU interrupt/clock/stack/timing claims need selected board observations.'],
  'evidence_limits':['Actual CMX transition/scheduler calls are driven serially; no new original native-thread or PSP context port is asserted.','FreeRTOS POSIX kernel objects are static, while the actual port and pthread runtime allocate host support objects.','The ARM_CM4F image has no linked allocator or undeclared helper, but only the selected finite workload/configuration and software-pended emulator IRQ were executed.','No arbitrary multi-mutex semantic equivalence, FPU-task context, physical latency or worst-case timing guarantee.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Actual static ARM_CM4F port, changed conversion/rates and compiler/port repeats implemented and author-executed; independent delayed learner attempt remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T52-11',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',closure_rationale='All original bounded comparison criteria and named changed conditions have explicit separate adapters and discriminating evidence; physical guarantees and independent assessment remain separate.',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);updates.append(gap)
 resources_path='research/v5/expansion/legacy_realtime_freertos/resources.json';resources=json.loads((ROOT/resources_path).read_text(encoding='utf-8'))
 pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_freertos_depth.py','assessor/legacy_realtime/freertos.md','research/v5/expansion/legacy_realtime_freertos/author_support.py',resources_path,'research/v5/expansion/legacy_realtime_freertos/source_notes.json','research/v5/expansion/legacy_realtime_freertos/research_gate.json','research/v5/expansion/legacy_realtime_freertos/abi_correction.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resources_path,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_freertos_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review/delayed unaided learner assessment.','Physical MCU timing/stack/IRQ evidence for physical claims.'],'scope':'Only exact T52.11 original and changed comparison contracts; separate model/host/emulator/physical/learner evidence.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
