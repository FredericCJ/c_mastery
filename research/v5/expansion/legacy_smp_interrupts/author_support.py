"""Attach exact interrupt support without rewriting accepted baseline evidence."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-04';SUPPORT='research/v5/expansion/legacy_smp_interrupts/support.json';CONTRACT='labs/smp_depth/interrupts/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_interrupts/campaign.json'
SCOPES={
'T53.04.a':'Separate ICR send, arrival, EOI and matching CPU/generation completion; actual required-mask7 trace rejects stale/duplicate/wrong CPU events; four copied work values coalesced into one publication with duplicate notification and exactly-once callback scope.',
'T53.04.b':'Actual per-CPU bounded handlers, one-shot/periodic/initial-count0 timer operations, release/acquire mailbox and target link audit; changed condition genuinely executes x2APIC MSR mode after feature checks and separately rejects unavailable feature before access.',
'T53.04.c':'Actual sparse1/2/4-CPU distinct timer/IPI/EOI/work/ack observations; early-ack and target-owned ISR/IRR missing-EOI controls; delayed CPU timeout retains outstanding batch while peers complete, then late completion; fenced C-handler/work TSC intervals reported separately.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual per-CPU timer/IPI/EOI modes, published work/ACK lifetime, coalesced batch and retained delayed completion.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/interrupts').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_interrupt_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-APIC-and-mailbox-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-named-changed-conditions','evidence':['assessor/legacy_smp/interrupts.c','assessor/legacy_smp/interrupts_guest.c','assessor/legacy_smp/interrupts_frames.S','labs/smp_depth/interrupts/oracle.py','labs/smp_depth/interrupts/native.c']},'author_validation':{'state':'passed-actual-APIC-modes-timers-mailbox-controls-and-native-threads','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-target-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'Six hosted C17 GCC/Clang debug/release/ASan+UBSan profiles; native pthread consumer matrix additionally GCC/Clang TSan; original CMK startup/QEMU10.2.1 TCG qemu64 xAPIC/max x2APIC, GCCO0/O3 and ClangO3 adapted mechanism with retained GCC prerequisites and GNU as.',
  'external_blockers':['Independent human review and seven-day unaided learner assessments remain unattempted.','Any physical timing or current hardware errata claim requires a separately selected physical environment and source review.'],
  'evidence_limits':['Single serialized producer and exclusive consumer per copied fixed-capacity batch; no MPMC, consumer migration, callback cancellation or arbitrary remote ACK authentication.','Native stress/TSan and finite transition histories do not establish every C weak-memory execution.','Actual x2APIC transfer chosen; TSC-deadline comparison inspected but not claimed executed.','TSC interval differences cover instrumented C handler/work under emulation, excluding assembly entry/exit and physical WCET.','Timer count/frequency is not calibrated elapsed time; pending event after stop is accounted separately.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Named coalesced work, actual x2APIC port or delayed destination retention implemented and author-validated; delayed learner evidence remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-04',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All named finite original and changed T53.04 mechanisms have concrete reference support, actual target/native evidence and discriminating controls; physical and independent assessment remain distinct.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_interrupts/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_interrupt_depth.py','assessor/legacy_smp/interrupts.md','research/v5/expansion/legacy_smp_interrupts/author_support.py',resource,'research/v5/expansion/legacy_smp_interrupts/source_access.json','research/v5/expansion/legacy_smp_interrupts/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_interrupt_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical platform evidence for any physical timing/hardware claim.'],'scope':'Only exact T53.04 finite mailbox/controller contracts and named variants; source editions, ordering, observations and cost intervals retain their explicit boundaries.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
