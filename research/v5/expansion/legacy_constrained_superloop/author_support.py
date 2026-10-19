"""Attach the six exact cooperative-scheduling criteria without baseline rewrites."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T30-01';SUPPORT='research/v5/expansion/legacy_constrained_superloop/support.json';CONTRACT='labs/constrained_depth/superloop/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_constrained_superloop/campaign.json'
SCOPES={
'T30.01.a':'Checked2/7/1 and2/2/1 round/response derivation, actual100-byte sliced dispatch traces, unknown-bound refusal and separate bounded observation of an original nonyielding child.',
'T30.01.b':'Separate copied capacity3 FIFO for five independent commands and latest-state slot with explicit supersession accounting; exact accepted identifiers and unchanged refusal outputs.',
'T30.01.c':'Actual copied100-byte continuation produces identical output across quota2/100 with logical poll gaps5/103; equal-class rotation and continuously ready higher-priority starvation are separately demonstrated.',
'T30.02.a':'Three owned input/staged-output continuations, exact cursor/terminal/emission invariants, all51 cancellation boundaries including completion and no public partial output.',
'T30.02.b':'Constant-work release-count computation with bounded two-slot coalescing/stale rejection, finite64-bit boundary refusal and transactional no-loss admission rejection.',
'T30.02.c':'Actual ABC, HHH and replenished HABC traces, independent ready-mask model and derived four-tick lower-class bound under explicit unit-cost/no-interference premises.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact bounded superloop, copied continuations, event/release semantics and all six changed conditions. Original execution/status is historical; current scope is separately source-bound.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/constrained_depth/superloop').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_constrained_superloop import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-original-cooperative-contract-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'complete-for-six-original-criteria-and-six-named-variants','evidence':['assessor/legacy_constrained/superloop.c','labs/constrained_depth/superloop/tests.c','labs/constrained_depth/superloop/blocking.c','labs/constrained_depth/superloop/oracle.py']},'author_validation':{'state':'passed-six-host-profiles-independent-replay-and-explicit-progress-diagnosis','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-declared-logical-core-no-physical-timing-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, independent Python integer/list model, and explicitly isolated original nonyielding child; no RTOS or board needed for these exact logical mechanisms.',
  'external_blockers':['Independent human review and minimum-seven-day unaided learner transfer remain unattempted.'],
  'evidence_limits':['All control access is externally serialized; ISR/thread publication requires a separate adapter.','Unit-cost synthetic clocks and finite-state execution do not establish physical service times, WCET or device timing.','The expected timeout is confined to the diagnostic child and never counts as a passing scheduler or semantic mutant.','LOSSLESS refusal requires operational admission/service changes; a smaller historical model horizon after refusal is not a real-clock recovery procedure.','Host strong stack-protection and possible generated memory helpers are explicit dependencies, not a freestanding closure claim.','Finite enumeration and sanitizer execution do not prove every input or learner mastery.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='The exact original changed condition has concrete executable/reference analysis support; author execution is separate from the unattempted independent learner task.');updates.append(child)
 resource='research/v5/expansion/legacy_constrained_superloop/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_constrained_superloop.py','assessor/legacy_constrained/superloop.md','research/v5/expansion/legacy_constrained_superloop/author_support.py',resource,'research/v5/expansion/legacy_constrained_superloop/source_access.json','research/v5/expansion/legacy_constrained_superloop/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_constrained_superloop.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.'],'scope':'PR-T30-01 only; other T30 projects and physical/RTOS requirements are not closed by this package.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
