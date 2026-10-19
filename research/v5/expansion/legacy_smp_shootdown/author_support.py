"""Attach exact T53.09 successor without rewriting original criteria or receipts."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-09';SUPPORT='research/v5/expansion/legacy_smp_shootdown/support.json';CONTRACT='labs/smp_depth/shootdown/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_shootdown/campaign.json'
SCOPES={
'T53.09.a':'PTE/invalidation/ACK/reclamation authority, generation9 versus ACK8, and actual concurrent membership entry with frozen snapshots and context flush before address-space use.',
'T53.09.b':'Bounded request publication, local invalidation before exact ACK, timeout frame retention, lock-free handler path, finite generations and actual selected GLOBAL-page invalidation with dependency audits.',
'T53.09.c':'Original synthetic stale-translation witness and repaired guest, exact two-address-space write-protection faults/target masks/shared-frame retention, and equal-page batching with qualified repeated virtual timing.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual bounded generation-qualified shootdown, retained frame authority, GLOBAL scope and two-address-space permissions.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/shootdown').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/smp_depth/locks/locks.h']
 for path in paths:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_shootdown_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-invalidation-and-reclamation-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-selected-named-changed-conditions','evidence':['assessor/legacy_smp/shootdown.c','assessor/legacy_smp/shootdown_guest.c','assessor/legacy_smp/shootdown_frames.S','labs/smp_depth/shootdown/oracle.py','labs/smp_depth/shootdown/native.c']},'author_validation':{'state':'passed-model-native-and-actual-isolated-guest-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-latency-or-errata-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, actual pthread participants with both TSan builds, independent finite protocol/cache model, and original two-/four-CPU CMK QEMU TCG guest with actual page tables and INVLPG.',
  'external_blockers':['Independent human review and seven-day unaided learner assessment remain unattempted.','Physical CPU/errata and calibrated latency claims require separately controlled physical observations.'],
  'evidence_limits':['Fixed online CPU set, 4KiB pages and PCIDE=0; the original GLOBAL-or-PCID variant selects GLOBAL, not an unimplemented PCID claim.','Handler executes on the declared CPU without the membership lock; callback contexts remain stable and the port supplies actual target-specific ordering.','Completed request permits do not eliminate other mappings or frame owners.','Native callbacks record protocol publication only; model arrays are not a real hardware translation cache.','The original isolated guest demonstrates synthetic stale data and controlled write-protection faults only, not arbitrary code execution or privilege escalation.','Virtual local TSC samples establish neither physical time nor a universal batching speedup; finite models and selected executions are not all-execution proofs.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Concurrent entry, selected GLOBAL-page scope, and exact two-address-space permission changes are implemented and author-executed; original delayed unaided learner evidence remains absent.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-09',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All original shootdown criteria, three named changed conditions and adjacent-page batching have bounded implementations, discriminating controls and source-bound author execution. The original screening interruption and prior pilots remain preserved; physical and independent evidence are not substituted.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_shootdown/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_shootdown_depth.py','assessor/legacy_smp/shootdown.md','research/v5/expansion/legacy_smp_shootdown/author_support.py',resource,'research/v5/expansion/legacy_smp_shootdown/source_access.json','research/v5/expansion/legacy_smp_shootdown/research_gate.json','research/v5/expansion/legacy_smp_shootdown/interruption_20261018.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_shootdown_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform/errata and calibrated timing evidence before expanded physical claims.'],'scope':'Exact T53.09 criteria, selected original changed conditions and batching transfer. No PCID, hotplug or arbitrary target support; later memory/scheduling mechanisms remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
