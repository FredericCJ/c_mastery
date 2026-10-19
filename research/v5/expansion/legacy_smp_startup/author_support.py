"""Attach actual startup evidence while preserving original criterion bodies."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-02';SUPPORT='research/v5/expansion/legacy_smp_startup/support.json';CONTRACT='labs/smp_depth/startup/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_startup/campaign.json'
SCOPES={
'T53.02.a':'Actual original16→32→64 startup and SysV scalar entry, runtime CR0/CR3/CR4/EFER/stack checks, three separately generated per-page images and mode-specific disassembly for base0x8000 and relocated0x9000.',
'T53.02.b':'Finite generation/identity/resource admission with timeout retention, delayed AP1 and actual bootstrap→permanent-stack quiescence witness, plus parallel per-AP page/mailbox ownership;15,544 independent bounded histories per hosted profile.',
'T53.02.c':'Actual1/2/4-CPU QEMU startup with sparse APIC IDs0,1,2,4, generations/stacks/control-state observations, intended missing/stale-ack images, changed max CPU model, NX-unavailable preflight and zero-undefined/no-dynamic freestanding closure.'}
def write(p,value):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual regenerated serial/parallel/late AP startup, sparse identities and changed CPU capability checks; physical timing and independent assessment separate.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/startup').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_startup_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-startup-resource-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_smp/startup.c','assessor/legacy_smp/startup_guest.c','assessor/legacy_smp/startup_trampoline.S','labs/smp_depth/startup/oracle.py','labs/smp_depth/startup/audit.py']},'author_validation':{'state':'passed-host-model-and-actual-serial-parallel-late-relocated-guests','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'C17 six hosted GCC/Clang debug/release/ASan+UBSan profiles; actual original CMK/GRUB/GNU as/QEMU10.2.1 TCG i440fx10.2, GCCO0/O3 and ClangO3, qemu64/max CPU models.',
  'external_blockers':['Independent human review, delayed unaided learner attempts and physical timing/platform observations remain separate.','Current physical CPU errata and target-specific low-memory reservation must be investigated before hardware use.'],
  'evidence_limits':['Selected one-SIPI teaching policy has no retry; Intel typical two-SIPI sequence was inspected and its difference explicitly retained.','Host controller attestation is a port premise; the actual guest supplies only its specifically implemented permanent-stack handoff witness.','Finite spin budgets do not establish wall-clock deadlines or physical INIT/SIPI timing.','Only tested1/2/4 CPUs, three-slot state and two models; no arbitrary x2APIC/AP-hotplug/restart or universal ISA conformance claim.','Native controller is externally serialized; C ownership, quiescence and correct use remain explicit programmer/port obligations.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Exact relocation, parallel-mailbox and changed-model tasks implemented and author-executed; seven-day independent learner attempt remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-02',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named finite T53.02 ordinary and changed tasks have real startup/reference support and discriminating controls; physical and independent evidence remain unclaimed.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_startup/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_startup_depth.py','assessor/legacy_smp/startup.md','research/v5/expansion/legacy_smp_startup/author_support.py',resource,'research/v5/expansion/legacy_smp_startup/source_access.json','research/v5/expansion/legacy_smp_startup/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_startup_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform/errata/timing evidence for any hardware claim.'],'scope':'Only exact T53.02 original/changed startup contracts; selected Intel historical ISA passages, scalar ABI, controller premise and actual guest handoff remain distinct.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
