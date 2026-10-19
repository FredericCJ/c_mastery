"""Attach exact T53.07 successor without changing accepted criteria/evidence."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-07';SUPPORT='research/v5/expansion/legacy_smp_wakeup/support.json';CONTRACT='labs/smp_depth/wakeup/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_wakeup/campaign.json'
SCOPES={
'T53.07.a':'Durable queue state versus coalesced doorbell, four-work/one-notification fixture, acknowledgement and recheck, actual hosted level-predicate wait plus independent late-clear witness.',
'T53.07.b':'Actual imported generation/home/TRANSFER queue rules, notification failure after committed work, and actual IF0 STI/HLT sleeping destination with saved-RIP proof and controlled rescue of a missed wake.',
'T53.07.c':'Source-bound before-check/check-to-sleep/after-halt/busy-task experiments, one/four-item batching at fixed capacity, interrupt counts and separate sender-local/receiver-local timestamp intervals.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual durable remote wake, level predicate, IF0 idle entry, failure retry and bounded clock-scoped observations.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/wakeup').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/smp_depth/locks/locks.h','labs/smp_depth/queues/queues.h']
 for path in paths:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_wakeup_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-durable-wake-and-idle-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-conditions','evidence':['assessor/legacy_smp/wakeup.c','assessor/legacy_smp/wakeup_guest.c','assessor/legacy_smp/wakeup_frames.S','labs/smp_depth/wakeup/oracle.py','labs/smp_depth/wakeup/native.c']},'author_validation':{'state':'passed-model-native-and-actual-idle-guest-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-timing-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, real pthread level-predicate destination with both TSan builds, independent finite platform schedules and original CMK QEMU TCG xAPIC STI/HLT guest.',
  'external_blockers':['Independent human review and seven-day unaided learner assessments remain unattempted.','Physical target/errata and calibrated timing claims require separately selected hardware and evidence.'],
  'evidence_limits':['One caller per actor CPU, all queue operations through the wrapper, nonreentrant stable port callbacks and no ISR queue operations.','A level-triggered hosted predicate/model and an actual fixed-IPI guest are separate implementations; no physical level-triggered I/O APIC device was exercised.','Early notification violates publication order but the retained control lock may still prevent lost work; actual missed-wakeup evidence is the separately rescued split-enable/HLT control.','Finite platform schedules and selected native/guest runs do not prove all C/ISA executions or fairness.','Virtual sender-local/receiver-local TSC intervals are not cross-CPU subtraction, calibrated physical time or WCET.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Actual level-predicate acknowledgement/recheck, sleeping destination with rescued missed-wakeup control, and fixed-capacity one/four-item batching are implemented and author-executed; delayed unaided learner evidence remains absent.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-07',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All original remote-wakeup criteria and three named changed conditions have actual queue composition, distinct falsification controls and source-bound author execution. Physical timing/other ports, external review and learner evidence are not inferred.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_wakeup/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_wakeup_depth.py','assessor/legacy_smp/wakeup.md','research/v5/expansion/legacy_smp_wakeup/author_support.py',resource,'research/v5/expansion/legacy_smp_wakeup/source_access.json','research/v5/expansion/legacy_smp_wakeup/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_wakeup_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Selected physical platform/errata and timing evidence before physical claims.'],'scope':'Exact T53.07 durable remote wake and three original changed conditions; later memory-management and global scheduling obligations remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
