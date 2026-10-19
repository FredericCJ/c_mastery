"""Bind original history/progress tasks to exact author successor evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_history'
from research.v5.expansion.legacy_advanced.author_support import attach
PROJECT='PR-T26-01';RECEIPT='research/v5/expansion/legacy_advanced_history/validation.json';SUPPORT='research/v5/expansion/legacy_advanced_history/completion_dispositions.json'
ARTIFACTS=['labs/advanced_concurrency_depth/history/CONTRACTS.md','assessor/advanced_concurrency_depth/history/ANALYSIS.md','assessor/advanced_concurrency_depth/history/history.c','assessor/advanced_concurrency_depth/history/oracle.py','assessor/advanced_concurrency_depth/history/progress_models.py','assessor/execution/concurrent.c']
CASES={'T26.01.a':['history'],'T26.01.b':['separation','history'],'T26.01.c':['points','pressure','history'],'T26.02.a':['retry'],'T26.02.b':['pressure'],'T26.02.c':['pressure','retry']}
RATIONALES={'T26.01.a':'Original overlapping/nonoverlapping capacity2 examples, explicit optional pending completion and independently compared bounded checker with concrete witnesses.','T26.01.b':'Executed SC atomic LIFO rejected as FIFO but admitted by a separately specified SET; actual set/pending oracle and separate missing-HB relation model.','T26.01.c':'Actual retained consumer helps lagging Tail from a valid linked prefix; successful points distinguished from permissive bounded-scan admission failure and logical capacity.','T26.02.a':'Four explicitly declared algorithms, repeatable counterexample schedules and quantifier analysis; bounded retry return is not silently counted as successful service.','T26.02.b':'Actual retained allocation/collection path and native/target object audit, including failed strict malloc/free variant and explicit whole-path progress premises.','T26.02.c':'Separate loser/system counts and repeatable CAS-interference schedule, actual31-retired-node admission failure/recovery, and explicit stronger per-request proof premises.'}
REMAINING=['LRUN-18: T26.04–.08 construction, ABA, hazard, epoch and RCU obligations remain required; history/progress alone do not close the parent.','LRUN-19: kernel/LKMM/guest criteria need their separate evidence bindings.','Final published C17/C23 source, independent learner, external review and physical observations remain distinct.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
def selected(d):return [c for c in d['criteria'] if c['objective'] in CASES]
def checks_for(commands,objective):
 result=[x for x in commands if any(x.endswith('.'+c) for c in CASES[objective]) and '.learner.' not in x and '.mutation.' not in x]
 if objective.startswith('T26.01'):result+=['gcc.history.oracle','clang.history.oracle']
 if objective in ['T26.01.b','T26.02.a','T26.02.b','T26.02.c']:result+=['progress.models']
 if objective=='T26.02.b':result += [x for x in commands if '.allocator.' in x or any(x.startswith(c+'.cortex-') for c in ['arm-none-eabi-gcc','clang'])]
 return list(dict.fromkeys(result))
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/advanced_concurrency_depth/history').iterdir() if p.is_file())+['labs/execution/include/cmx.h','labs/execution/README.md'];d=read(canon());snapshot=HERE/(PROJECT+'.before-history.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 write(HERE/'attachment.json',dict(project=PROJECT,domain='T26',key='advanced_history_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/execution/README.md'],source_register='research/v5/expansion/legacy_advanced_history/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in selected(d)],scope='Only original T26.01/.02 ordinary and delayed history/progress obligations; author evidence, not independent learner or other construction completion.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive history/progress attachment; original tasks unchanged')
def package():
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={pathlib.Path(__file__),ROOT/'tools/check_advanced_history.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py',canon()};files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-history.json'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d;commands=[c['id'] for c in report['commands']];rows=[]
 for c in selected(d):
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=checks_for(commands,c['objective'])),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=report['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six exact original history/progress criterion IDs and six unchanged delayed variants; no parent closure.',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_advanced_history/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,parents=0,commands=len(report['commands']),export_commands=len(exports['commands']))));print('PASS twelve exact history/progress author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
