"""Three exact original invariant/cost/permutation and delayed identities."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_invariants'
from research.v5.expansion.legacy_algorithm_invariants.author_support import attach
PROJECT='PR-T36-01';RECEIPT='research/v5/expansion/legacy_algorithm_invariants/validation.json';SUPPORT='research/v5/expansion/legacy_algorithm_invariants/completion_dispositions.json'
ARTIFACTS=['labs/algorithm_invariants/CONTRACTS.md','assessor/algorithm_invariants/ANALYSIS.md','assessor/algorithm_invariants/invariants.c','assessor/algorithm_invariants/experiments.c','assessor/algorithm_invariants/instrument.py','assessor/algorithm_invariants/oracle.py']
RATIONALES={'T36.01.a':'Actual checked modulo/subtraction transitions and independent divisor enumeration8190cases, original3-versus11 trace and4094-step boundary, preservation/progress arguments and explicit zero-divisor admission.','T36.01.b':'Actual frozen vector doubling/additive workloads0..32; exact15/36 writes and worst5/8, paired original ring/node states and source-preserving instrumentation count every declared primitive including failed put55.','T36.01.c':'First lost-value transition and strengthened full-record multiset relation, equal-key same-set/different-tag-multiplicity counterexample and intended set-only mutant; sortedness/stability remain separate predicates.'}
REMAINING=['T36.02 through T36.10 paired ABI, sorting/sift/hash, graph and wrapper original/changed tasks remain required separate packages.','All T37 original/changed obligations remain separately required; retained implementations must be inspected before credit.','External review, independent delayed learner, physical observations and exact final published C17/C23 remain open.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T36.01.a':['.euclid','.mutation.ZERO','.strict.'],'T36.01.b':['.growth','.queues','.instrumented.','.mutation.GROWTH','instrument'],'T36.01.c':['.permutation','.mutation.SET','.strict.']}
 return dict(report=RECEIPT,checks=['fixture',*[n for n in commands if any(p in n for p in patterns[objective])]])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-invariants.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_invariants').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T36',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_algorithm_invariants/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact T36.01 ordinary/changed tasks with actual frozen container reuse and explicit source-work metric; no independent/physical inference.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive invariants attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_algorithm_invariants.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-invariants.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three exact original T36.01 criteria and three delayed identities; bounded author implementation/execution',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_algorithm_invariants/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact invariant dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
