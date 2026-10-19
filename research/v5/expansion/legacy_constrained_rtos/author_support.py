"""Attach exact original RTOS integration/resource criteria without rewriting history."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T30-02';SUPPORT='research/v5/expansion/legacy_constrained_rtos/support.json';CONTRACT='labs/constrained_depth/rtos/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_constrained_rtos/campaign.json'
SCOPES={
'T30.03.a':'Actual static FreeRTOS producer/consumer with OFFERED-before-queue authority, bounded blocked-send cancellation, notification plus observed suspension, and priority-admitted real FromISR producer/handoff.',
'T30.03.b':'Actual tagged CM4F PendSV/PSP/BASEPRI/EXC_RETURN assembly, integer and full s16–s31 task preservation variants, selected ISA frame accounting and actual nested FP restoration.',
'T30.03.c':'Explicit selected-kernel create/block/wake/notification/quiescence/delete boundary; actual imported PR-T30-01 cooperative scheduler executes102 stop cuts and refuses preemption/ISR feature requests.',
'T30.07.a':'Original800-byte arithmetic preserved separately from actual target map/sizeof reservations, copied pointer-record queue, variable-length bounded pointee pool and added monitor/control/stack costs.',
'T30.07.b':'Actual compiler local stack frames, generated direct/indirect calls, manual hardware/port/probe frames and nested FP handler evidence; unknown transitive bounds remain unknown and budget admission refuses them.',
'T30.07.c':'Capacity3 queue versus two ordinary payload owners, pre-publication third-owner refusal, finite blocked-producer cleanup, terminal-generation cleanup and separately reserved fault capacity.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact static RTOS integration/resource work and six changed conditions; prior execution/status remains historical, current source-bound application evidence is separate from first-principles T50.'});rows.sort(key=lambda r:r['path'])
 for path in (CONTRACT,'labs/constrained_depth/superloop/CONTRACTS.md'):
  if path not in record['shared_contracts']:record['shared_contracts'].append(path)
 supplied=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/constrained_depth/rtos').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/constrained_depth/superloop/superloop.h','labs/constrained_depth/superloop/superloop.c']
 for path in supplied:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_constrained_rtos import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-original-static-RTOS-integration-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'complete-for-six-original-criteria-and-six-named-variants','evidence':['assessor/legacy_constrained/ownership.c','assessor/legacy_constrained/ownership_guest.c','assessor/legacy_constrained/cooperative.c','labs/constrained_depth/rtos/oracle.py','labs/constrained_depth/rtos/audit.py']},'author_validation':{'state':'passed-six-host-profiles-and-three-actual-CM4F-compiler-profiles','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-emulated-context-only','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; exact clean FreeRTOS11.2.0; GCC/Clang Cortex-M4F and QEMU mps2-an386; independent Python histories/arithmetic; explicit freestanding helper/link dependencies.',
   'external_blockers':['Independent human review, minimum-seven-day unaided learner transfer, and physical timing/stack observations remain unattempted.'],
   'evidence_limits':['Static selected RTOS application integration is not the first-principles T50 scheduler implementation or a proof of all kernel executions.','QEMU context/IRQ results do not establish physical interrupt latency, energy or worst-case execution time.','Local compiler frames, hardware/port frame arithmetic and fill observations do not close an arbitrary whole-program stack bound; unknown call/nesting premises remain explicit and refuse budget admission.','The pool is externally serialized, fixed-capacity and world-qualified; no lock-free or generic asynchronous cancellation guarantee.','Finite independent histories and sanitizer runs do not prove every input or learner mastery.','Exact published C17 remains unavailable; primary Arm and tagged kernel source have only their registered inspected scopes.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Exact named changed condition has executed reference/analysis support; author execution does not perform the delayed independent learner assessment.');updates.append(child)
 resource='research/v5/expansion/legacy_constrained_rtos/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_constrained_rtos.py','assessor/legacy_constrained/ownership.md','research/v5/expansion/legacy_constrained_rtos/author_support.py',resource,'research/v5/expansion/legacy_constrained_rtos/source_access.json','research/v5/expansion/legacy_constrained_rtos/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_constrained_rtos.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical timing/stack measurements remain unperformed; no required physical result is inferred from QEMU.'],'scope':'PR-T30-02 exact original static integration/resource work only; unknown whole-program stack claims are explicitly not made.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
