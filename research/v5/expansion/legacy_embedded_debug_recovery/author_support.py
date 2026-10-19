"""Attach original recovery criteria to exact current successor evidence."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T31-05';SUPPORT='research/v5/expansion/legacy_embedded_debug_recovery/support.json';CONTRACT='labs/embedded_debug_depth/recovery/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_embedded_debug_recovery/campaign.json'
SCOPES={
'T31.09.a':'Checked image/compatibility/state admission and actual frozen M4 handoff with VTOR/MSP/CONTROL/PRIMASK/MPU checks; changed clock and vector reject, no physical board clock/reset claim.',
'T31.09.b':'Two-slot commit-last model with561 bit-prefix and256 byte-subset cuts/profile; old slot retained and shared erase domains rejected before mutation, physical durability premise explicit.',
'T31.09.c':'Harmless synthetic recomputed-CRC acceptance plus actual OpenSSL/lce signature admission, signed-old replay rejected by current trusted floor, revoked/wrong-key/fail-open controls.',
'T31.10.a':'Count/status admission before pointer use, first-fault latch and actual lce nested-log fallback with bounded nonrecursive callback behavior; failure never replaces primary cause.',
'T31.10.b':'Actual lce two-record persistence composition consumes retry before permission,50 cut/restart/lost-delivery cases/profile, five failures grant exactly2 retries, corrupt state conservatively stops.',
'T31.10.c':'Synthetic irreversible output history survives detection and failed/successful shutdown; separate detection10/containment13/current output0/history1, future effects refused and no physical safety claim.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Six original update/recovery criteria and six exact changed conditions with executable model, actual crypto-policy boundary and M4 handoff; physical/review/learner dimensions remain distinct.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 sys.path.insert(0,str(ROOT/'tools'));from run_embedded_debug_recovery import headers,INCLUDE
 paths=['fixtures/embedded/PR-T31-05.json',*[p.relative_to(ROOT).as_posix() for p in headers()],*[(INCLUDE/n).relative_to(ROOT).as_posix() for n in ('boot_guest.S','boot_guest.ld','EXTENSIONS.md','SIGNATURE.md','BOOT_PAIR.md')],*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/embedded_debug_depth/recovery').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for p in paths:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_embedded_debug_recovery import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-criteria-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'implemented-original-recovery-and-actual-signature-handoff-composition','evidence':['assessor/embedded_debug_depth/recovery.c','assessor/embedded_debug_depth/recovery_target.c','assessor/embedded_debug_depth/recovery.md','labs/embedded_debug_depth/recovery/signature.c','labs/embedded_debug_depth/recovery/tests.c','labs/embedded_debug_depth/recovery/oracle.py']},'author_validation':{'state':'passed-six-host-and-three-M4-profiles-independent-cuts-and-semantic-controls','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-Flash-power-loss-reset-and-actuator-containment-unobserved','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, explicit frozen lifecycle/signature providers and private OpenSSL host dependency; original M4 AN386 guest with exact handoff source/helper closure. Physical transfer requires a separately authorized dedicated board.',
   'external_blockers':['Independent human review and original minimum-seven-day unaided learner assessment remain unattempted.','Physical Flash programming/erase/durability, power cuts and actual actuator shutdown are not observed.'],
   'evidence_limits':['Digital erase is atomic and sectors independent by declared model premise; arbitrary analog faults are not enumerated.','Actual handoff is selected Cortex-M4 architecture evidence, not a board reset, F439 Flash execution or clock-transition proof.','C image/state records require truthful trusted ownership; they are not unforgeable capabilities.','OpenSSL is explicitly permitted hosted cryptography; signer/verifier share a provider, not independent cryptographic correctness or secure-boot certification.','Trusted current rollback floor/key policy is separate from signed bytes and is not a new tamper-resistant storage implementation.','Retry persistence is a byte-backed model with actual lifecycle record code; physical reset/durability must satisfy its premises.','Detection/containment steps are logical and retain prior effects; no physical safety claim.','Finite tests do not prove every execution or learner mastery; exact published C17 remains inaccessible.']}
  updates.append(row)
  for variant in c['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=c['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Implemented exact clock/vector, shared-erase, signed-old replay, nested-fault, persisted retry and irreversible-effect changed conditions; independent delayed learner and physical evidence remain separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));source_records=[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']]
 for shared in resources['canonical_external_sources']:
  r=next(r for r in json.loads((ROOT/shared['path']).read_text(encoding='utf-8'))['resources'] if r['id']==shared['id']);source_records.append({**shared,'passages':r['locators']})
 pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_embedded_debug_recovery.py','assessor/embedded_debug_depth/recovery.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')],*{r['path'] for r in resources['canonical_external_sources']}]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':source_records,'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_embedded_debug_recovery.py','report':CAMPAIGN}],'prior_attempts':['reviews/v5/expansion/legacy_embedded_debug_recovery/campaign_attempt1.json','reviews/v5/expansion/legacy_embedded_debug_recovery/campaign_attempt2.json'],'remaining_author_work':[],'remaining_external_work':['Physical Flash/reset/power-cut/actuator observations.','Independent review and delayed learner assessment.'],'scope':'All six original criterion and changed-condition contracts under exact model/host/architecture boundaries, never production boot or physical safety certification.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
