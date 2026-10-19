"""Attach exact T30.05 support without changing historical tasks or receipts."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T30-05';SUPPORT='research/v5/expansion/legacy_constrained_supervision/support.json';CONTRACT='labs/constrained_depth/supervision/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_constrained_supervision/campaign.json'
SCOPES={
'T30.08.a':'Exact last-successful-feed fixture1,0,1,1; before-commit false-health counterexample and corrected boundary,8192 independent histories and actual M4 core execution.',
'T30.08.b':'SC coherent capture with forced intervening publisher, finite nonwrapping identities, ISR BUSY ownership, bitmap exchange/lost-update counterexample,20000 native commits under eight profiles including both TSan builds.',
'T30.08.c':'Exactly three retry permissions for hung/overload/failed boot then terminal safe state; bounded reason/mirror record and256 one-bit corruptions defaulting safe, without physical persistence claim.',
'T30.09.a':'Actual original M4 before/between/after work, pending retention and mask restoration plus spurious rechecks; separate32-case architectural WFE model because QEMU does not implement event-latch semantics.',
'T30.09.b':'Separate Sleep/Stop/Standby ledger, exact F439 HSI16 Stop/IWDG register adapter with errata admission and clock/epoch gating, six-profile register model and three-profile retained target compile/link; external wake without retained clock publishes unknown elapsed time. Physical F439 Stop remains unobserved.',
'T30.09.c':'Exact synthetic92.4µJ and wake-inclusive125.4µJ/108.9µJ-per-batched-transaction comparisons; checked64-bit arithmetic and concrete physical instrumentation/uncertainty design, no invented current measurement.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'All six original supervision/sleep criteria and named changed conditions; selected host/QEMU/architectural model and F439 compile-only scopes are explicit. Historical execution/status unchanged.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=['labs/concurrency_lifecycle_depth/targets/m4.ld',*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/constrained_depth/supervision').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for p in paths:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_constrained_supervision import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-criteria-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'implemented-selected-host-M4-and-HSI16-register-contracts','evidence':['assessor/legacy_constrained/supervision.c','assessor/legacy_constrained/power.c','assessor/legacy_constrained/power_f439.c','assessor/legacy_constrained/sleep_arm.c','labs/constrained_depth/supervision/tests.c','labs/constrained_depth/supervision/power_tests.c','labs/constrained_depth/supervision/oracle.py']},'author_validation':{'state':'passed-six-host-eight-native-three-target-profiles-F439-model-and-compile-only','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-F439-Stop-watchdog-LSI-WFE-latch-and-energy-unobserved','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; pthreads with both TSan builds; original QEMU mps2-an386 M4 and strict explicit helpers; selected F439 MMIO register model/compile binding; independent integer/wake model.',
   'external_blockers':['Independent human review and minimum-seven-day unaided learner assessment remain unattempted.','Actual selected F439 Stop/IWDG reset, LSI tolerance, external wake wiring, physical WFE latch and power instrumentation require dedicated equipment and new observations.'],
   'evidence_limits':['Progress reflects the chosen useful-commit contract, not arbitrary application health; premature publication can lie without a data race.','The SC protocol is sufficient and bounded per attempt, not a weakest-order or general wait-free proof. ISR contention must not spin on a preempted publisher.','Mirrored logical recovery state is not authenticated or crash-atomically persisted; unauthorized fresh initialization can defeat the retry policy.','QEMU10.2.1 ignores SEVONPEND and deep sleep; its WFE runs do not validate event-latch or physical low-power behavior.','F439 profile requires HSI16/noPLL/normal regulator/noDebug, caller-certified quiescence and enabled wake source. PLL restoration and Standby reset-entry are separate profiles, not implemented by the returning Stop function.','Retained timestamps are caller-authorized; external wake without a clock produces unknown elapsed time, not an invented delta.','Synthetic energy arithmetic and finite test maxima are not physical observations or future timing bounds.','Exact published C17 remains unavailable; current errata Rev24 was actually read while its local download timed out.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Exact changed condition has executed reference/model support under the stated scope; independent delayed learner work and hardware observations remain separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_constrained_supervision.py','assessor/legacy_constrained/supervision.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_constrained_supervision.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review/delayed learner assessment.','Physical selected F439 Stop/IWDG/LSI/external wake/event-latch and energy observations.'],'scope':'Exact original six criteria plus six variants under selected contracts; no arbitrary PLL/Standby/physical closure.','exploratory_failure':{'status':'diagnosed-before-final-campaign','observation':'An early owned QEMU pilot timed out after a test wrote SLEEPDEEP, assumed it was retained, then entered ordinary WFI with no remaining wake source. Primary QEMU source inspection showed RAZ/WI. Corrected harness explicitly tests this limitation and uses SLEEPONEXIT for admission refusal.','evidence_limit':'Observed in tool-session output before campaign tooling; no reconstructed old binary hash or fabricated retained execution log.'}})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
