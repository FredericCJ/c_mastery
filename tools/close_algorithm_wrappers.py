"""Package the six exact T36.02/T36.10 criteria and their unchanged V1 tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_wrappers'
from research.v5.expansion.legacy_algorithm_wrappers.author_support import attach
PROJECT='PR-T36-08';RECEIPT='research/v5/expansion/legacy_algorithm_wrappers/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_wrappers/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_wrappers/CONTRACTS.md','assessor/algorithm_wrappers/ANALYSIS.md','assessor/algorithm_wrappers/wrappers.c','assessor/algorithm_wrappers/x86.S','assessor/algorithm_wrappers/thumb.S','assessor/algorithm_wrappers/oracle.py','labs/algorithm_wrappers/tests.c']
RATIONALES={'T36.02.a':'Actual eight-byte Record layout and separate typed node controls, finite-generation index replacement with terminal rejection before reuse.','T36.02.b':'Actual retained ring and typed slot queue seven-state replay, distinct traversal/slot-scan costs and changed movable-bank success/failure.','T36.02.c':'Readonly owner/generation/index view admission before payload access; failed growth retains, successful growth invalidates; real control lifetime remains a precondition.','T36.10.a':'Actual completed AVL ID index with real value and pointer arrays, new x86/Thumb rank-ID lower-bound kernels, ABI probes and finite/callback same-contract code observations.','T36.10.b':'Actual copy-versus-borrow live mutation counterexample, context mismatch/inconsistent comparator/reentry diagnosis, empty-epoch direction switch with view invalidation.','T36.10.c':'Duplicate before full, no caller frees or retained payload pointers after destroy; five failed update boundaries in both wrappers and nine comparator failure positions preserve state.'}
REMAINING=['All T37 original/changed tasks remain independently required.','External review, independent seven-day learner, physical observation and final published C17/C23 remain unclaimed.']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.02.a':['.layout','thumb.'],'T36.02.b':['.queues','.ownership','.backing','thumb.'],'T36.02.c':['.ownership','.mutation.VIEW','thumb.'],'T36.10.a':['.fixture','.kernel','.disassembly','.core','thumb.'],'T36.10.b':['.ownership','.contexts','.mutation.COPY','thumb.'],'T36.10.c':['.fixture','.backing','.contexts','.mutation.PRECEDENCE','.mutation.ROLLBACK','thumb.']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-wrappers.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_wrappers').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','labs/api_depth/contracts/OWNERSHIP.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_wrappers/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six exact ordinary/changed typed layout, queue, readonly-view, real wrapper, context and transactional backing mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive wrappers attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_wrappers.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-wrappers.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact six original T36.02/T36.10 criteria and six original V1 identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_wrappers/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact wrappers dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
