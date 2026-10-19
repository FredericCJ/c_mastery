"""Bind only the original uncertain-effect criterion to actual reusable evidence."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_posix_retry/support.json';REPORT='reviews/v5/expansion/legacy_posix_retry/retry_validation.json';CONTRACT='labs/posix_lifecycle_depth/retry/CONTRACTS.md';CRITERION='AX-PR-T23-02-T23.04.c'
def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!='PR-T23-02':return record
 next(c for c in record['criteria'] if c['id']==CRITERION)['successor_support']={'path':SUPPORT,'id':CRITERION}
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[r for r in additions if r.get('path')!=SUPPORT];additions.append({'path':SUPPORT,'project':record['project'],'criterion_ids':[CRITERION],'scope':'Lost reply, bounded cache expiry and effect-before-receipt crash variant only; no whole storage/journal closure.'})
 for path in (CONTRACT,'labs/service_depth/adapters/CONTRACTS.md'):
  if path not in record['shared_contracts']:record['shared_contracts'].append(path)
 for path in ('labs/posix_lifecycle_depth/retry/tests.c','labs/posix_lifecycle_depth/retry/run_retry.py','labs/service_depth/adapters/receipt.h','labs/service_depth/adapters/receipt.c','labs/storage/include/store.h'):
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 resource={'id':'R-LRETRY-TRANSACTION','title':'Original C Mastery durable effect/receipt teaching transaction','authors':['C Mastery authors'],'url':'labs/service_depth/adapters/CONTRACTS.md','locators':['Durable effect and receipt transaction: two clients, three receipts, high-water expiry and single-store commit','Actual sdr_apply/sdr_initialize/sdr_value in assessor/service_depth/kernel/receipt.c','Actual receipts and sector_cases in assessor/service_depth/kernel/receipt_faults.c'],
  'domains':['T23'],'kind':'course','edition':'C17 original course contract','access':'Actual named contract and complete bounded implementations inspected','role':'oracle','assignable':True,'reason':'Bind the exact original lost-reply and split-publication question to an existing implementation and newly supplied public counterexample harness.','prerequisites':['Serialized store transactions, unsigned wire encoding and partial effects'],
  'inspection':{'date':'2026-10-17','method':'Read actual public contract/header, complete receipt implementation and both fault-model functions; inspect current nested source-bound host/VM reports.','scope':'Bounded effect+receipt recovery and named semantic controls, not all store internals or all physical devices.','limits':'Course implementation evidence is not a published-standard inspection, external review or learner assessment.','evidence_urls':['labs/service_depth/adapters/CONTRACTS.md']}}
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':[resource]});write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Actual course-source and receipt inspection; no independent external review','research_input_sha256':{source.relative_to(ROOT).as_posix():hashlib.sha256(source.read_bytes()).hexdigest()}})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report['profiles'])!=6 or sum(len(p['outcomes']) for p in report['profiles'])!=30:raise ValueError('Retry campaign incomplete')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale retry evidence')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record)
 update={'id':CRITERION,'original_source':path.relative_to(ROOT).as_posix(),'original_objective':'T23.04.c','project':'PR-T23-02',
  'dimensions':{'teaching':{'state':'complete-for-declared-bounded-contract','evidence':[CONTRACT,'labs/service_depth/adapters/CONTRACTS.md']},'implementation':{'state':'complete-for-declared-reference-and-delayed-variant','evidence':['labs/posix_lifecycle_depth/retry/tests.c','assessor/service_depth/kernel/receipt.c','assessor/storage/store.c']},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT,*[r['path'] for r in report['reused_evidence']['receipts']]]},'external_review':{'state':'not-run-for-this-binding','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-durability-claim','evidence':[]}},
  'closure_rationale':'Original blind counter14, explicit three-entry expiry and full payload-bound identity; same existing receipt/store core passes74 cuts of two public device models per profile. Split-effect/receipt, reapply and payload controls compile then fail criterion-specific assertions. Existing185-cut/sector/152-VM evidence is source-validated reuse, not rerun or whole-storage completion.',
  'variant_ids':[CRITERION+'-DELAYED'],'variant_readiness':'Effect-before-dedup split transaction is an actual executed semantic control; learner delay/independence remains unassessed.','required_environment':'Owned hosted synthetic store/device; explicit completed learner receipt/store sources. Reused VM campaign has its own kernel/QEMU environment.','external_blockers':['Independent delayed learner attempt and external review.','Physical media behavior if claiming actual power-loss durability.'],
  'evidence_limits':['Finite device models and declared guest crash cuts; no arbitrary malicious corruption, distributed concurrent writers, epoch migration or unlimited result retention.','This binds only T23.04.c and does not close every storage/journal criterion.']}
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,REPORT,path.relative_to(ROOT).as_posix(),source.relative_to(ROOT).as_posix()]}
 write(BASE/'support.json',{'schema_version':1,'required':True,'source_sha256':pins,'updates':[update],'scope':'Exact successor of the earlier PR-T23-02 .04c binding gap; other namespace/byte-I/O scope unchanged.','supersedes_pending_binding':['research/v5/expansion/legacy_posix_io/support.json:remaining_author_work T23.04.c'],'remaining_author_work':[],'remaining_external_work':update['external_blockers']})
 family_path=ROOT/'labs/posix_lifecycle_depth/family.json';family=json.loads(family_path.read_text(encoding='utf-8'));campaigns=family.setdefault('supplemental_campaigns',[]);new={'id':'retry-followthrough','producer':'tools/run_posix_retry.py','report':REPORT,'contract':CONTRACT,'scope':'Exact T23.04.c; explicit receipt/store imports, public74-cut model and source-validated reuse of existing service evidence.'}
 for i,row in enumerate(campaigns):
  if row.get('id')==new['id']:campaigns[i]=new;break
 else:campaigns.append(new)
 write(family_path,family);print(json.dumps({'criterion':CRITERION,'outcomes':30,'reused_receipts':len(report['reused_evidence']['receipts'])}))
if __name__=='__main__':main()
