"""Attach exact reduction successor without changing original contracts/history."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T53-10';SUPPORT='research/v5/expansion/legacy_smp_reduction/support.json';CONTRACT='labs/smp_depth/reduction/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_smp_reduction/campaign.json'
SCOPES={
'T53.10.a':'Overflow-safe quotient/remainder ranges, empty and singleton ownership, exact per-index ledger, original55 fixture and noncommutative ordered polynomial composition independent of worker completion order.',
'T53.10.b':'Checked unsigned/signed local and fixed-order combination, release/acquire partial publication, output preservation under worker/launch failures, actual retained dynamic257-job comparison and separate binary64 cancellation/compensation results.',
'T53.10.c':'Mathematical integer/bin/hash expectations, actual1/2/4CPU guest identities, explicit single-CPU negative control, equal-work native wall/process/thread clocks including launch/join/combine, and packed/padded histogram implementations. Physical cache attribution remains unobserved.'}
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual owned static/ordered reductions, checked failure, floating variant, packed/padded histogram experiment and retained dynamic guest comparison; physical cache evidence remains separate.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/smp_depth/reduction').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_smp_reduction_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
  'dimensions':{'teaching':{'state':'complete-for-declared-partition-arithmetic-order-and-cost-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-criteria-and-named-changed-condition-mechanisms','evidence':['assessor/legacy_smp/reduction.c','assessor/legacy_smp/reduction_float.c','assessor/legacy_smp/reduction_guest.c','labs/smp_depth/reduction/native.c','labs/smp_depth/reduction/oracle.py']},'author_validation':{'state':'passed-host-native-and-actual-guest-conditions-physical-attribution-pending','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-cache-layout-and-calibrated-scaling-remain-required','evidence':[]}},
  'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan, actual pthread participants on four eligible CPUs with both TSan builds, binary64 no-fast-math, independent wide integer/ordered-byte oracle and original CMK QEMU TCG1/2/4CPU guest.',
  'external_blockers':['Independent human review and seven-day unaided learner assessment remain unattempted.','Original .c-DELAYED physical false-sharing evidence requires physical topology/cache-line validation and controlled observations; WSL timing is not substituted.'],
  'evidence_limits':['Inputs/world/ledger remain live and disjoint until every caller is joined; world reuse requires externally established quiescence.','The explicit ownership ledger has real O(n) verification traffic included consistently; comparison is not a minimal bare reduction benchmark.','Checked local and fixed-order intermediates can refuse even if a rearranged mathematical sum fits.','Compensation yields the exact answer for the supplied cancellation fixture, not every binary64 input/order/rounding mode.','The unchanged dynamic producer is a fixed257-job comparison, not an assertion that its arbitrary partial-launch failure paths are repaired.','Native process clocks and virtual TSC observations do not establish physical cache coherence, WCET, scaling or a universal speedup.']}
  updates.append(row)
  for v in c['variants']:
   child=deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Ordered noncommutative composition, binary64 cancellation/compensation and packed/padded histogram implementations are author-executed. Physical false-sharing observations and delayed unaided learner assessment remain separate and unattempted.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T53-10',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']],closure_rationale='All original partition/reduction mechanisms and named changed-condition implementations have exact bounded reference support and source-bound host/native/guest execution. The original physical false-sharing observation remains an explicit external obligation, not inferred from virtual/native timings.');updates.append(gap)
 resource='research/v5/expansion/legacy_smp_reduction/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_smp_reduction_depth.py','assessor/legacy_smp/reduction.md','research/v5/expansion/legacy_smp_reduction/author_support.py',resource,'research/v5/expansion/legacy_smp_reduction/source_access.json','research/v5/expansion/legacy_smp_reduction/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_smp_reduction_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent human review and delayed unaided learner assessment.','Required physical false-sharing/layout/topology and calibrated clock evidence before expanded physical performance claims.'],'scope':'Exact T53.10 mechanisms and original changed conditions, with physical cache validation retained as incomplete. Later worker-pool and DAG mechanisms remain separate.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
