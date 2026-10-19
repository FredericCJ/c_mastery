"""Attach exact DAG successor without rewriting original criteria or receipts."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-12';SUPPORT='research/v5/expansion/legacy_smp_dag/support.json';CONTRACT='labs/smp_depth/dag/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_dag/campaign.json'
SCOPES={
'T53.12.a':'Independent work11/span8 derivation and ideal schedules; adding B→C changes span to11. An actual disjoint-range split preserves exact work/result and changes span to6, with qualified measured overhead and per-node costs at1/2/4 workers.',
'T53.12.b':'Copied arbitrary bounded DAG validation, endpoint/duplicate/cycle refusal, checked work/span, exact last-predecessor publication, actual simultaneous predecessor completion and runtime append before/after completion under an explicit older-parent-only acyclicity rule.',
'T53.12.c':'Independent exhaustive four-node graph oracle, exact owner/result/readiness ledgers, failed and cancelled dependency propagation, running-claim retention, explicit pool lifecycle composition and original fixed16-node helper comparison in disposable1/2/4CPU guests.'}

def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Arbitrary/runtime DAGs, dependency publication, equivalent split work and failure/cancellation with actual pool/CMK composition. Original status is historical; exact current evidence is in this successor.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 paths=[p for p in (ROOT/'labs/smp_depth/dag').rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[ROOT/'labs/smp_depth/locks/locks.h',ROOT/'labs/smp_depth/locks/CONTRACTS.md']+[ROOT/'labs/smp_depth/pool'/p for p in ('pool.h','guest.py','CONTRACTS.md')]
 for path in sorted(p.relative_to(ROOT).as_posix() for p in paths):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record

def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_dag_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-DAG-publication-and-runtime-admission-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-condition-mechanisms','evidence':['assessor/legacy_smp/dag.c','assessor/legacy_smp/dag_guest.c','labs/smp_depth/dag/native.c','labs/smp_depth/dag/oracle.py']},'author_validation':{'state':'passed-host-native-and-actual-guest-conditions','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-timing-or-WCET-claim','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, actual pthread schedules on four eligible CPUs with both TSan builds, independent graph and integer models, and original CMK QEMU TCG1/2/4CPU guests.',
  'external_blockers':['Independent human review and seven-day unaided learner assessment remain unattempted.','Physical topology and calibrated timing remain required before physical-performance claims.'],
  'evidence_limits':['Runtime append only adds new nodes with existing parents; it does not support arbitrary back-edge mutation.','Fixed16-node/32-edge limits, trusted completed lock/launcher and externally unique world identities are explicit preconditions.','Graph drained is not worker quiescence; graph and caller stack storage remain live until all actual callers and threads have left.','The imported fixed16-node helper is actually executed separately and is not credited as arbitrary graph validation.','Ready times are intervals around predecessor publication, not invented exact linearization clocks.','The split task is a particular independently checked disjoint-range program, not a claim that arbitrary sequential tasks can be split safely.','Explicit memcpy/memset target byte helpers close the observed Clang lowering imports; the initial failed link remains recorded.','CMK has four process threads including the coordinator; joined stacks return to an arena, not a nonexistent free syscall.','Finite graph enumeration, stress, sanitizers and virtual guests do not prove all C executions, physical scaling or learner mastery.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Added-dependency work/span, runtime creation in both parent-completion orders, failure/cancellation propagation and valid long-node decomposition have concrete implementations and author execution. Independent learner performance remains unattempted.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-12',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='Original DAG work/span, arbitrary dependency validation, result publication, failure and all named changed conditions now have exact reference support and host/native/guest evidence. External human, delayed learner and physical evidence remain separate.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_dag/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_dag_depth.py','assessor/legacy_smp/dag.md','research/v5/expansion/legacy_smp_dag/author_support.py',resource,'research/v5/expansion/legacy_smp_dag/source_access.json','research/v5/expansion/legacy_smp_dag/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_dag_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Physical/calibrated observations before physical scaling or WCET claims.'],'scope':'Exact T53.12 criteria and original changed conditions, with explicit bounded runtime-creation and equivalent-task assumptions.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
