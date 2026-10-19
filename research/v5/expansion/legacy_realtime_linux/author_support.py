"""Bind original Linux comparison criteria to exact finite guest observations."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T52-12';SUPPORT='research/v5/expansion/legacy_realtime_linux/support.json';CONTRACT='labs/realtime_depth/linux/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_realtime_linux/campaign.json'
SCOPES={
'T52.12.a':'Original nonnegative-lag/minimum-virtual-deadline snapshot selector, independently enumerated2916 inputs per profile, checked0<Q<=D<=P and invalidQ3/D2 pre-syscall refusal; actual selected fair/deadline kernel source inspected with omitted mechanisms explicit.',
'T52.12.b':'Actual original disposable-VM workers with class/parameter/affinity readback, deliberate privilege/root-domain refusals, fixed workload and per-process limits; deliberate infinite workers killed/reaped by an ordinary supervisor under declared placement/reservation.',
'T52.12.c':'Actual OTHER/FIFO/RR/DEADLINE equivalent-work traces under none/batch/tmpfs interference, ordinary affinity changes and separate one-CPU deadline root domain; actual overrun signals and CPU/elapsed/miss observations interpreted against the inspected depletion path, without claiming per-throttle-interval tracing or physical bounds.'}
def write(path,data):path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for criterion in record['criteria']:criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
 links=record.setdefault('additional_successor_support',[]);links[:]=[r for r in links if r.get('path')!=SUPPORT]
 links.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Actual owned one/two-CPU Linux policy observations and independent bounded rule oracle; refused modes remain unavailable and physical timing remains unobserved.'});links.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for path in sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/linux').rglob('*') if p.is_file() and '__pycache__' not in p.parts):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_linux_policy_depth import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record);updates=[]
 for criterion in record['criteria']:
  row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
  'dimensions':{'teaching':{'state':'complete-for-explicit-policy-comparison-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_realtime/linux_policy.c','assessor/legacy_realtime/linux_probe.c','labs/realtime_depth/linux/guest.py','labs/realtime_depth/linux/oracle.py']},'author_validation':{'state':'passed-actual-disposable-Linux-guests-and-independent-host-oracle','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-timing-not-claimed','evidence':[]}},
  'variant_ids':[v['id'] for v in criterion['variants']],'required_environment':'Exact7.0.14-cmastery-b06 debug/KASAN source/configuration; static BusyBox/probes, owned networkless/diskless QEMU TCG one/two CPUs; six hosted profiles and three static guest compiler/optimization profiles.',
  'external_blockers':['Independent human review and delayed unaided learner attempts remain separate.','Physical WCET/energy/latency bounds require separate measured platform and proof assumptions.'],
  'evidence_limits':['A mathematically valid tuple is not sufficient Linux admission. Actual restricted-root-domain and privilege refusals remain unavailable under those conditions.','Debug/KASAN virtualization costs remain in every retained sample; no PREEMPT_RT, physical latency or universal schedulability claim.','Overrun signals and CPU/elapsed observations support bounded reservation interpretation against actual source; every individual throttle/replenishment interval was not traced.','The finite selector omits buddy/slice protection, group hierarchy, weighted-accounting updates and placement.']}
  updates.append(row)
  for variant in criterion['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Original invalid tuple, infinite worker, changed affinity/interference and one-CPU conditions implemented and author-executed; independent delayed attempt remains unassessed.');updates.append(child)
 gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-PR-T52-12',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',closure_rationale='All original bounded Linux policy comparison criteria and named changed assumptions have concrete executable/reference/oracle support; unavailable scheduling conditions and physical/independent claims remain distinct.',criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']]);updates.append(gap)
 resource='research/v5/expansion/legacy_realtime_linux/resources.json';resources=json.loads((ROOT/resource).read_text(encoding='utf-8'))
 pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_linux_policy_depth.py','assessor/legacy_realtime/linux_policy.md','research/v5/expansion/legacy_realtime_linux/author_support.py',resource,'research/v5/expansion/legacy_realtime_linux/source_access.json','research/v5/expansion/legacy_realtime_linux/research_gate.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource,'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_linux_policy_depth.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review/delayed unaided learner assessment.','Physical platform timing evidence for physical claims.'],'scope':'Only exact T52.12 original and changed comparison contracts; pure rule model, actual guest observations, physical and learner evidence remain distinct.'})
 print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))
if __name__=='__main__':main()
