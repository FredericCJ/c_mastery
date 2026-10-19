"""Attach exact per-CPU reference/transfer support without baseline rewrites."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-03';SUPPORT='research/v5/expansion/legacy_smp_percpu/support.json';CONTRACT='labs/smp_depth/percpu/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_percpu/campaign.json'
SCOPES={
'T53.03.a':'Checked transactional nine-span-per-CPU layout and actual object extent validation; numeric independent oracle; changed2048-byte/8-alignment layout and actual identical finite-call-tree compilation under SysV AMD64 and AAPCS32 with regenerated frame calculations.',
'T53.03.b':'Actual private GDT/TSS/IDT and permanent BSP/AP/task stacks, five live nested C frames, register-preserving interrupt/cooperative switches and selected x87/SSE FXSAVE ownership variant with actual clobber/restore controls; canary and finite budget evidence distinguished.',
'T53.03.c':'Actual sparse1/2/4-CPU known vector244→UD→GP nesting through distinct IST stacks; CPU-specific frames and peer progress retained; TSS-alias and suppressed-fault-accounting controls; known-site recovery is not arbitrary kernel fault isolation.'}
def write(p,value):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual private storage, known nested faults, register/SIMD preservation, changed-ABI finite frame calculation and equivalent-contract padding measurements.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/percpu').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_percpu_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-per-CPU-frame-and-state-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_smp/percpu.c','assessor/legacy_smp/percpu_guest.c','assessor/legacy_smp/percpu_frames.S','labs/smp_depth/percpu/oracle.py','labs/smp_depth/percpu/abi_probe.c','labs/smp_depth/percpu/cost.c']},'author_validation':{'state':'passed-actual-private-context-fault-SIMD-and-ABI-measurements','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-target-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'Six hosted C17 GCC/Clang debug/release/ASan+UBSan profiles; original CMK startup/QEMU10.2.1 TCG, GCCO0/O3 and ClangO3 adapted mechanism with retained GCC prerequisites; GNU as; actual Arm GCC14.2 AAPCS32 compile; two allowed native logical CPUs for cost samples.',
  'external_blockers':['Independent human review and delayed unaided learner assessments remain unattempted.','Physical target timing/errata observations require a separately selected physical environment.'],
  'evidence_limits':['Recovery handles only exact original UD2 and invalid-selector sites with declared finite IST nesting; arbitrary faults, double faults and same-IST recursive reentry are not recovered.','Selected SIMD profile saves x87/SSE state only; no AVX/YMM/other extended state or asynchronous scheduler claim.','Frame estimate is for the explicit finite successful call/nesting tree and separately generated ABI probe; intact canaries do not prove universal bounds.','AAPCS32 evidence is actual compilation/disassembly/calculation, not Arm exception or board execution; x86 table layout is not an Arm port.','Padding samples compare identical counts on logical CPUs without speedup threshold, physical-core topology or universal performance conclusion.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Named changed-ABI calculation, selected SIMD state and nested emergency-stack work implemented and author-validated; delayed learner evidence remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-03',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named finite original/changed T53.03 investigations now have concrete reference support, actual target evidence and discriminating controls; parent padding transfer is also executed under equivalent counts. Broader/physical/independent claims remain excluded.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_percpu/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_percpu_depth.py','assessor/legacy_smp/percpu.md','research/v5/expansion/legacy_smp_percpu/author_support.py',resource,'research/v5/expansion/legacy_smp_percpu/source_access.json','research/v5/expansion/legacy_smp_percpu/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_percpu_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform evidence for any physical timing or hardware claim.'],'scope':'Only exact T53.03 finite contracts and named variants; recovery, frame estimates, compiler comparisons and host measurements retain their explicit boundaries.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
