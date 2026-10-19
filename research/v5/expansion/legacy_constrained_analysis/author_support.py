"""Attach exact T30.06/T30.10 criteria and delayed variants without historical edits."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T30-04';SUPPORT='research/v5/expansion/legacy_constrained_analysis/support.json';CONTRACT='labs/constrained_depth/analysis/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_constrained_analysis/campaign.json'
SCOPES={
'T30.06.a':'Actual retained RTA import, exact1/3 response and13/20 utilization, blocking1→4, overflow/premise refusal and1152 independently reconstructed cases with18 actual logical release/finish records.',
'T30.06.b':'Actual CMX/ceiling same-work protocol comparison and proposed opposite-order cycle; distinguish priority boost from explicit CMX_DEADLOCK admission rejection and complete cleanup.',
'T30.06.c':'Eight independently rejected missing premises, explicit no-jitter constrained-deadline periodic/sporadic minimum-separation derivation, and an actual owned endless-driver child with bounded noncompletion observation and reaping.',
'T30.10.a':'Distinct requirement/model-bound/observed-mean/maximum tags, unchanged-output refusal of category substitution, exact image/clock/IRQ/cache/probe/resolution configuration gates and explicitly different measurement units/workloads.',
'T30.10.b':'Actual strict Cortex-M4 scalar/wide division, queue-copy/fault-record paths and T11 runtime imports; source/map/symbol/assembly/local-stack evidence plus deliberate missing-helper link rejection under three target profiles.',
'T30.10.c':'Actual dedicated M4/QEMU timer/debugger probe, two reload configurations, image/configuration/raw-clock/read-overhead/transcript binding, resumed IRQ progress versus stopped peripheral state, disabled-timer/stub controls; no physical freeze/WCET inference.'}
PUBLIC_IMPORTS=['labs/realtime_depth/response/response.h','labs/realtime_depth/response/CONTRACTS.md','labs/execution/include/cmx.h','labs/realtime_depth/ceiling/ceiling.h','labs/realtime_depth/ceiling/CONTRACTS.md','labs/realtime_depth/ceiling/tests.c','labs/machine_image/freestanding_depth/free.h','labs/concurrency_lifecycle_depth/targets/m4.ld']
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual conditional response/CMX/runtime/debugger support for all six original criteria and six named changed conditions; historical execution/status unchanged.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=[*PUBLIC_IMPORTS,*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/constrained_depth/analysis').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for path in paths:
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_constrained_analysis import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-response-and-evidence-contracts-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'complete-for-selected-host-and-dedicated-QEMU-contracts','evidence':['assessor/legacy_constrained/analysis.c','assessor/legacy_constrained/analysis_target.c','labs/constrained_depth/analysis/tests.c','labs/constrained_depth/analysis/oracle.py','labs/constrained_depth/analysis/guest.py','labs/constrained_depth/analysis/cycle.c']},'author_validation':{'state':'passed-six-host-profiles-three-target-profiles-actual-imports-and-debugger-observations','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-QEMU-timer-and-host-observation-only','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 host GCC/Clang debug/release/ASan+UBSan; exact completed RTA/CMX/ceiling imports; Cortex-M4 GCC/Clang strict original runtime, QEMU mps2-an386 and private UNIX GDB socket; independent Python integer/trace oracle.',
   'external_blockers':['Independent human review and minimum-seven-day unaided learner transfer remain unattempted.','Physical target WCET, cache effects and debugger/watchdog freeze require separately selected equipment and observations.'],
   'evidence_limits':['Premise bits are externally justified claims, not a verifier of all future arrivals or execution costs.','The wrapper excludes jitter, self-suspension, multiprocessors and deadlines greater than periods; no stronger theorem is inferred from arithmetic.','CMX/ceiling traces have explicit logical work and zero transition overhead; they are not physical execution measurements.','Owned driver timeout establishes only bounded noncompletion, not an infinite-duration proof.','Configuration identity is not authentication; raw maxima and probe overhead are not WCET bounds.','QEMU timer/MMIO/GDB behavior is selected-model evidence, not physical board debug-freeze or memory-access-width evidence.','Exact published C17 and external general RTA sources were not newly inspected.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='The exact named changed condition has executed reference or explicit-model support in the declared environment; independent delayed learner work remains unattempted.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_constrained_analysis.py','assessor/legacy_constrained/analysis.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',resource,BASE/'source_access.json',BASE/'research_gate.json')]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_constrained_analysis.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical target timing/cache/debugger behavior remain separate.'],'scope':'PR-T30-04 exact selected response/evidence tasks; no arbitrary scheduler/WCET/physical closure claim.','preserved_attempts':[{'path':'reviews/v5/expansion/legacy_constrained_analysis/campaign_attempt1.json','status':'failed','reason':'Unsolved target section GC removed unused result/fault symbols; harness lookup failed, not an intended semantic rejection. Public startup now explicitly initializes these observation outputs.'},{'path':'reviews/v5/expansion/legacy_constrained_analysis/campaign_attempt2.json','status':'passed','reason':'Full current source campaign after explicit observation-symbol initialization.'}]})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
