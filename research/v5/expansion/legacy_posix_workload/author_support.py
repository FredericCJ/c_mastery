"""Bind the original mapped-versus-explicit record and random-workload criterion."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_posix_workload/support.json';REPORT='reviews/v5/expansion/legacy_posix_workload/workload_validation.json';CONTRACT='labs/posix_lifecycle_depth/workload/CONTRACTS.md';ID='AX-PR-T23-03-T23.09.b'
def write(path,value):
 if path.name=='support.json':
  import runpy
  value=runpy.run_path(str(ROOT/'research/v5/expansion/legacy_posix_followthrough/variant_support.py'))['expand'](value,ROOT)
 path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!='PR-T23-03':return record
 next(c for c in record['criteria'] if c['id']==ID)['successor_support']={'path':SUPPORT,'id':ID}
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[r for r in additions if r.get('path')!=SUPPORT];additions.append({'path':SUPPORT,'project':record['project'],'criterion_ids':[ID],'scope':'Equivalent bounded record copies, atomic error output and delayed sparse random workload; prior mapping evidence retains its scope.'})
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for name in ('record.h','record_stub.c','tests.c','bench.c','run_workload.py'):
  path='labs/posix_lifecycle_depth/workload/'+name
  if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
 return record
def main():
 shared=[{'id':'R-LMAP-MMAP','path':'research/v5/expansion/legacy_posix_mapping/resources.json'},{'id':'R-LIO-PREAD','path':'research/v5/expansion/legacy_posix_io/resources.json'}]
 resource={'id':'R-LWORK-COURSE','title':'Original bounded mapping/borrow and record-workload course contracts','authors':['C Mastery authors'],'url':'labs/posix_lifecycle_depth/mapping/map.h','locators':['PMMap/PMBorrow and pm_copy interface in map.h','pm_map/pm_copy logical extent, generation and copying in assessor/posix_lifecycle_depth/mapping/map.c','Isolated protection/truncation observations in labs/posix_lifecycle_depth/mapping/fault_helper.c'],
  'domains':['T23'],'kind':'course','edition':'C17 original course contract','access':'Actual named header, functions and complete bounded fault helper inspected','role':'oracle','assignable':True,'reason':'Equivalent validation/ownership contract and separately reused actual process-fault evidence.','prerequisites':['Logical versus rounded extent, positioned reads and object lifetime'],
  'inspection':{'date':'2026-10-17','method':'Read actual source functions and exact original criterion; reuse the separately registered inspected primary Linux mmap/pread passages.','scope':'Bounded record copies, lifetime and error model; no cold-cache, physical-storage or general performance conclusion.','limits':'Original course implementation inspection is not external review or a new full standard inspection.','evidence_urls':['labs/posix_lifecycle_depth/mapping/map.h']}}
 source=BASE/'resources.json';write(source,{'schema_version':1,'resources':[resource],'canonical_external_sources':shared});pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [source.relative_to(ROOT).as_posix(),*[r['path'] for r in shared]]};write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T23'],'inspection_kind':'Actual course sources plus explicitly reused inspected primary passages','research_input_sha256':pins})
 report=json.loads((ROOT/REPORT).read_text(encoding='utf-8'))
 if not report.get('passed') or len(report['profiles'])!=6 or sum(len(p['outcomes']) for p in report['profiles'])!=42 or sum(len(p['samples']) for p in report['profiles'])!=192:raise ValueError('Workload campaign incomplete')
 if not all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in report['inspected_paths'].items()):raise ValueError('Stale workload evidence')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-03.json';record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record)
 update={'id':ID,'original_source':path.relative_to(ROOT).as_posix(),'original_objective':'T23.09.b','project':'PR-T23-03',
  'dimensions':{'teaching':{'state':'complete-for-declared-record-contract','evidence':[CONTRACT]},'implementation':{'state':'complete-for-reference-and-random-variant','evidence':['assessor/posix_lifecycle_depth/workload/record.c','labs/posix_lifecycle_depth/workload/tests.c','labs/posix_lifecycle_depth/workload/bench.c']},'author_validation':{'state':'passed-with-stated-scope','evidence':[REPORT,report['reused_fault_evidence']['path']]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-warm-hosted-comparison-no-disk-claim','evidence':[]}},
  'closure_rationale':'Same logical bounds and unchanged-output contract for8192 copied32-byte records; EOF-in-record and actual truncation errors, short/EINTR/error controls, rounded-mapping counterexample, and192 measured sequential/sparse-random samples across six profiles. Existing isolated process-fault evidence is source-validated reuse.',
  'variant_ids':[ID+'-DELAYED'],'variant_readiness':'Named sparse random access variant executed as author reference; original delayed learner gate remains unassessed.','required_environment':'Owned Linux regular files, libc/GCC/Clang and explicit completed mapping import; stable backing size during measured samples.','external_blockers':['External review and delayed independent learner attempt.'],'evidence_limits':['Small prevalidated warm workload; mapping setup excluded and reported, no fixed CPU-cache state or physical disk throughput guarantee.','Call counters observe pread/fortified pread dispatches, not kernel syscall tracing.','No universal speed ordering or mutable-file concurrency claim.']}
 pins={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [CONTRACT,REPORT,path.relative_to(ROOT).as_posix(),source.relative_to(ROOT).as_posix()]};write(BASE/'support.json',{'schema_version':1,'required':True,'source_sha256':pins,'updates':[update],'supersedes_pending_binding':['research/v5/expansion/legacy_posix_mapping/support.json:T23.09.b','research/v5/expansion/legacy_posix_io/support.json:T23.09.b'],'remaining_author_work':[],'remaining_external_work':update['external_blockers']})
 family_path=ROOT/'labs/posix_lifecycle_depth/family.json';family=json.loads(family_path.read_text(encoding='utf-8'));test='posix_lifecycle_depth.workload_record';row=next(r for r in family['projects'] if r['id']=='PR-T23-03')
 if test not in row['tests']:row['tests'].append(test)
 if test not in family['tests']:family['tests'].append(test)
 campaigns=family.setdefault('supplemental_campaigns',[]);new={'id':'workload-followthrough','producer':'tools/run_posix_workload.py','report':REPORT,'contract':CONTRACT,'scope':'Exact T23.09.b same-contract copied records and random-workload measurements; explicit mapping import and reused isolated faults.'}
 for i,row in enumerate(campaigns):
  if row.get('id')==new['id']:campaigns[i]=new;break
 else:campaigns.append(new)
 write(family_path,family);print(json.dumps({'criterion':ID,'outcomes':42,'samples':192}))
if __name__=='__main__':main()
