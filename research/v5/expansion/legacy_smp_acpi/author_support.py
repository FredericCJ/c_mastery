"""Attach exact discovery successor evidence without changing original criteria."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-01';SUPPORT='research/v5/expansion/legacy_smp_acpi/support.json';CONTRACT='labs/smp_depth/acpi/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_acpi/campaign.json'
SCOPES={
'T53.01.a':'Sparse UID/APIC/index identities, disabled/online states, explicit xAPIC refusal and full-width type9 record transfer implemented against actually inspected layout; no claim of enabling x2APIC or complete normative ACPI compliance.',
'T53.01.b':'Transactional byte/root-region parsing, all76 protected truncations, structural/checksum/duplicate/capacity failures,1957 independent wire cases, and actual big-endian PPC64 ELFv2 execution including captured root tables.',
'T53.01.c':'Six actual original-CMK discovery captures for i440fx10.2/q3510.2 with1/2/4 CPUs, independently decoded full table regions; six host and three big-endian profiles reject four distinct semantic defects and unsolved stubs.'}
def write(p,value):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Checked discovery, actual changed-machine firmware capture, independent wire oracle and actual big-endian transfer; normative-source and physical limits explicit.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/acpi').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_acpi_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-bounded-discovery-contract-with-source-gap-explicit','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_smp/acpi.c','labs/smp_depth/acpi/guest.py','labs/smp_depth/acpi/oracle.py','labs/smp_depth/acpi/big_endian.S']},'author_validation':{'state':'passed-host-bounds-independent-oracle-actual-QEMU-and-big-endian','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'C17 six hosted GCC/Clang debug/release/ASan+UBSan profiles; original CMK/GRUB/QEMU TCG i440fx10.2/q3510.2 with1/2/4 CPUs; Clang PPC64 ELFv2 O0/O2/O3 and qemu-user10.2.1.',
  'external_blockers':['G-LCON-ACPI: exact normative ACPI6.6 RSDP/root/type9 passages remain inaccessible; ACPICA layout is not replacement normative conformance.','Independent human review, delayed unaided learner attempts and physical observations remain separate.'],
  'evidence_limits':['Type9 record decoding does not enable x2APIC or establish every32-bit ID is usable as a hardware interrupt destination.','Retained CMK boot traversal retains its platform physical mapping assumptions; checked host regions do not prove those assumptions.','Guest serial capture stops at MADT before AP startup; target ELFv2 is Linux-process execution, not physical Power or bare-board validation.','Finite1957-row oracle and bounded guard pages falsify defects; no absolute mastery or universal parser proof.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Named wide-record, actual big-endian and changed-machine tasks implemented and author-executed; seven-day independent learner attempt remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-01',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named bounded T53.01 ordinary and changed experiments have source-bound executable/reference support; normative ACPI gap and independent/physical claims remain explicit.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_acpi/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_acpi_depth.py','assessor/legacy_smp/acpi.md','research/v5/expansion/legacy_smp_acpi/author_support.py',resource,'research/v5/expansion/legacy_smp_acpi/source_access.json','research/v5/expansion/legacy_smp_acpi/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_acpi_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['G-LCON-ACPI normative root/x2 source-access gap remains partial, not closed by inspected ACPICA declarations.','Independent human review and delayed unaided learner assessment.','Physical platform evidence for any physical claim.'],'scope':'Only exact T53.01 original/changed discovery contracts; firmware capture, checked host regions, ISA capability and normative conformance remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
