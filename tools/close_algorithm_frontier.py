"""Package the three exact T36.09 criteria and their unchanged V1 tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_frontier'
from research.v5.expansion.legacy_algorithm_frontier.author_support import attach
PROJECT='PR-T36-07';RECEIPT='research/v5/expansion/legacy_algorithm_frontier/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_frontier/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_frontier/CONTRACTS.md','assessor/algorithm_frontier/ANALYSIS.md','assessor/algorithm_frontier/frontier.c','assessor/algorithm_frontier/generate.py','assessor/algorithm_frontier/oracle.py','labs/algorithm_frontier/tests.c']
RATIONALES={'T36.09.a':'Source-derived actual T15 BFS/Dijkstra refined through frozen queue/heap, actual separate frontier packets and exact negative-edge settlement counterexample.','T36.09.b':'Actual sparse8/24 CSR and finite65534 staging, caller-owned typed queue/heap handles, deterministic vertex ties and parallel-edge one-item/decrease-key costs.','T36.09.c':'Independent simple paths and replay of2193 actual packets/profile, parent-consistent nonoptimal counterexample, executed signed witness inequalities/rootedness/arithmetic and negative-cycle rejection.'}
REMAINING=['PR-T36-08 original and changed tasks remain independently required; no whole-domain closure.','All T37 original/changed tasks remain separate.','External review, independent seven-day learner, physical observation and final published C17/C23 remain unclaimed.']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.09.a':['.fixture','.exhaustive','.mutation.TIE','thumb.'],'T36.09.b':['.boundaries','.parallel','.mutation.RANGE','.mutation.UPDATE','.disassembly','.core','thumb.'],'T36.09.c':['.certificates','.exhaustive','.oracle','.mutation.INEQUALITY','thumb.']}
 return dict(report=RECEIPT,checks=['fixture','generate',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-frontier.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_frontier').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_frontier/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed T36.09 mechanisms: source-derived queue/heap refinement, finite public staging, actual frontier certificates and signed witness transfer.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive frontier attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_frontier.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-frontier.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact three original T36.09 criteria and three original V1 identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_frontier/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact frontier dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
