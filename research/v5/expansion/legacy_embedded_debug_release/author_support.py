"""Attach exact trace/release successors without rewriting original criteria."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T31-04';SUPPORT='research/v5/expansion/legacy_embedded_debug_release/support.json';CONTRACT='labs/embedded_debug_depth/release/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_embedded_debug_release/campaign.json'
SCOPES={
'T31.07.a':'Checked one-wrap piecewise conversion with outward rational rounding and1024 independent cases/profile; actual DWT capability/progression observation separated from selected CMSDK virtual timer and physical calibration.',
'T31.07.b':'Three-record drop-new/attempt-ID/saturation/cleanup implementation,4096 independent histories/profile, and actual nested maskable IRQ producers with saved-PRIMASK restoration; no SMP/NMI generalization.',
'T31.07.c':'Actual identical300-item volatile workload across disabled/record-drop/record-drain modes with21 raw virtual-timer samples/profile and expected297 drops; independent nonadditive-interference counterexample and ready physical paired-run plan.',
'T31.08.a':'Checked vector ranges and actual original/shifted GNU Arm ELF load segments, RAM VMA/load distinction, FF-filled holes and BSS; actual implicit-header stripping counterexample rejects changed loaded bytes.',
'T31.08.b':'Exact ELF/deployment/debug/map/source/flags/target consistency manifest and PC attribution; actual separate stripped/debug artifacts and eight substitutions rejected, no authenticated-hardware-provenance claim.',
'T31.08.c':'Poisoned typed RAM and actual dedicated M4 data/BSS startup plus SYSRESETREQ retained-record version/magic/image/check policy; explicit cold-power/silicon persistence limitations.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Six original criteria plus six named changed conditions with actual host/IRQ/reset/ELF evidence and explicit physical acquisition plans. Historical status unchanged.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for p in ['fixtures/embedded/PR-T31-04.json',*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/embedded_debug_depth/release').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_embedded_debug_release import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-criteria-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'implemented-host-core-and-actual-IRQ-reset-release-adapters-physical-plan-ready','evidence':['assessor/embedded_debug_depth/release.c','assessor/embedded_debug_depth/release_target.c','assessor/embedded_debug_depth/release_manifest.py','assessor/embedded_debug_depth/release.md','labs/embedded_debug_depth/release/tests.c','labs/embedded_debug_depth/release/oracle.py']},'author_validation':{'state':'passed-six-host-and-three-emulator-toolchain-profiles-exact-packaging-controls','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-calibration-Flash-programming-power-cycle-and-silicon-persistence-unobserved','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; explicit Cortex-M4 freestanding toolchain/helpers and dedicated QEMU AN386; GNU Arm readelf/objcopy; independent Python oracle. Physical follow-up requires a dedicated board, calibrated capture and authorized reset/programming procedure.',
   'external_blockers':['Independent human review and original minimum-seven-day unaided learner assessment remain unattempted.','Physical calibrated counter/trace observations, board reset persistence and cold power cycle require actual dedicated hardware.'],
   'evidence_limits':['CMSDK measurements are selected emulator virtual timer ticks, not CPU CYCCNT or physical WCET. DWT progression failure is recorded, not silently substituted.','Actual software reset is distinct from physical power loss; image token42 is distinct from host SHA-bound release identity.','The IRQ adapter is single-core and requires every caller to use the same reliable mask port; NMI/HardFault/SMP are excluded.','The ELF parser is a bounded declared GNU Arm subset, not a general loader or authority to flash hardware.','Manifest hashes establish trusted-input consistency, not authentication or proof of installed code.','Contention numbers are an original discrete model; raw target timing samples remain separate.','Finite tests/oracles do not prove all executions or learner mastery; exact published C17 remains unavailable.']}
  updates.append(row)
  for variant in c['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=c['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Executable piecewise/IRQ/contention/segmented-ELF/stripped-symbol/warm-reset controls ready under exact named scope; physical and delayed independent learner evidence remain separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));source_records=[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']]
 for shared in resources['canonical_external_sources']:
  r=next(r for r in json.loads((ROOT/shared['path']).read_text(encoding='utf-8'))['resources'] if r['id']==shared['id']);source_records.append({**shared,'passages':r['locators']})
 pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_embedded_debug_release.py','assessor/embedded_debug_depth/release.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')],*{r['path'] for r in resources['canonical_external_sources']}]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':source_records,'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_embedded_debug_release.py','report':CAMPAIGN}],'prior_attempts':['reviews/v5/expansion/legacy_embedded_debug_release/campaign_attempt1.json'],'remaining_author_work':[],'remaining_external_work':['Physical calibrated trace/counter measurement and board cold-power/reset observations.','Independent review and delayed learner assessment.'],'scope':'All six original criteria and named changes under selected host/IRQ/reset/ELF contracts. Actual programming, physical timing and independent mastery are not claimed.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
