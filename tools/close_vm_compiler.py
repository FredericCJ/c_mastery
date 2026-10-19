"""Bind six original ordinary and six changed compiler tasks to concrete evidence."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_vm_compiler';PROJECTS=['PR-T46-05','PR-T46-06'];REPORT='research/v5/expansion/legacy_vm_compiler/validation.json';SUPPORT='research/v5/expansion/legacy_vm_compiler/completion_dispositions.json'
from research.v5.expansion.legacy_vm_representation.author_support import attach
CASES={'T46.05.a':'names','T46.05.b':'scopes','T46.05.c':'bounds','T46.06.a':'branches','T46.06.b':'controls','T46.06.c':'fuel'}
MUTANTS={'names':['INITIALIZER','CANARY'],'scopes':['SHADOW'],'bounds':['CANARY'],'branches':['TARGET','EAGER'],'controls':['BREAK'],'fuel':['FUEL']}
ARTIFACTS=['labs/runtime_followthrough/compiler/CONTRACTS.md','labs/runtime_followthrough/compiler/compiler.h','labs/runtime_followthrough/compiler/tests.c','assessor/runtime_followthrough/compiler/compiler.c','assessor/runtime_followthrough/compiler/ANALYSIS.md','assessor/runtime_followthrough/compiler/oracle.py','assessor/runtime_followthrough/compiler/stack.py']
WHY={'names':'Counted ASCII scanning/signatures, outer-scope initialization and block/slot reuse with copied escaped values.','scopes':'Innermost lexical loads and assignments, independent tree/bytecode models and retained baseline comparison.','bounds':'Source/name/code/local bounds, byte-preserved failure, explicit parser/emitter continuation stacks and actual own-function nonrecursion audit.','branches':'Absolute valid instruction-boundary targets, independent joins and Boolean-only short-circuit RHS behavior.','controls':'Exact guest type/effect sequencing, nearest active-loop-body break value and exited-local/temporary cleanup.','fuel':'Independent exact dispatch budgets, balanced infinite loops and explicit fuel-incompatible safe literal folding.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def project(n):return 'curriculum/v5/expansion/legacy_assessments/'+n+'.json'
def original(n):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(n)],cwd=ROOT,text=True,encoding='utf8'))
def rows(r):
 out=[]
 for name in PROJECTS:
  d=read(ROOT/project(name))
  for c in d['criteria']:
   case=CASES[c['objective']];v=c['variants'][0];checks=[cc+'.'+p+'.'+case for cc in ['gcc','clang'] for p in ['debug','optimized','sanitized']]+['gcc.debug.oracle','gcc.lto.oracle','audit.acyclic','audit.recursive.refusal']+['gcc.debug.mutation.'+m for m in MUTANTS[case]]
   for delayed in [False,True]:
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:3]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_vm_compiler/exports.json']),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    out.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,case=case,analysis='assessor/runtime_followthrough/compiler/ANALYSIS.md#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[case],remaining_author_work=[],evidence_limits=r['limits']))
 return out
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/runtime_followthrough/compiler').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/runtime/include/cvm.h','labs/runtime/SPEC.md','labs/runtime/src/cvm.c','labs/runtime/src/compiler.c','labs/runtime_followthrough/verifier/verifier.h','labs/runtime_followthrough/verifier/learner.c','labs/runtime_followthrough/verifier/CONTRACTS.md']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  before=HERE/(name+'.before-compiler.json')
  if not before.exists():before.write_bytes((ROOT/project(name)).read_bytes())
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T46',key='vm_compiler_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/runtime/SPEC.md'],source_register='research/v5/expansion/legacy_vm_compiler/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original ordinary and unchanged delayed compiler/control tasks; actual iterative implementation, verifier/VM, independent oracles and explicit resource/equivalence limits.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/project(name),after)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],case=CASES[c['objective']],teaching=ARTIFACTS[0],implementation=ARTIFACTS[3],assessment=ARTIFACTS[4]+'#'+c['objective'].lower().replace('.','-')) for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,limitations=['Original learner delayed attempts remain unperformed.','No claim that C types enforce the guest invariants or that a finite oracle corpus proves equivalence.']))
 print('PASS six original criteria and six delayed tasks preserved; actual public compiler attached')
def package():
 import run_vm_compiler as run
 r=read(HERE/'validation.json');e=read(HERE/'exports.json');g=read(HERE/'generator_validation.json');assert r['source_sha256']==run.pins()
 files={pathlib.Path(__file__),ROOT/'tools/check_vm_compiler.py',ROOT/'research/v5/expansion/legacy_vm_representation/author_support.py',*[ROOT/project(n) for n in PROJECTS]}
 for report in [r,e,g]:
  assert report['status']=='pass' and report['commands'] and all(q['passed'] for q in report['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in report[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in report['source_sha256']}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for name in PROJECTS:assert read(ROOT/project(name))['criteria']==original(name)['criteria']
 put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T46-05/06 six ordinary and six unchanged delayed criterion implementations',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_vm_compiler/exports.json','research/v5/expansion/legacy_vm_compiler/generator_validation.json'],updates=rows(r),remaining_parents=['Other runtime components retain their own exact source/evidence obligations.','External review, independent learner/delayed assessment and physical observations remain separate.']))
 print('PASS12 exact compiler/control dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
