"""Reject omitted atomic tasks, stale execution and overstated target/source claims."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_thread_atomics'
from research.v5.expansion.legacy_threads.author_support import attach
import run_thread_atomics as runner
from close_thread_atomics import CASES,ARTIFACTS,RECEIPT,REMAINING,checks_for,dimensions
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def expected_commands():
 result={'environment.kernel','environment.libc','handoff.model','baseline.abstract-closure','i386.compile','host.libatomic.path','host.libatomic.symbols'}|{'i386.'+c for c in runner.GROUPS['operation']}
 for cc in ['gcc','clang']:
  result|={cc+'.version',cc+'.c23.compile',cc+'.c23.dependency'}
  for profile in ['debug','optimized','sanitized','tsan']:
   tag=cc+'.'+profile
   for group,cases in runner.GROUPS.items():
    result|={tag+'.'+group+'.compile',tag+'.'+group+'.learner.compile'}
    for case in cases:result|={tag+'.'+case,tag+'.learner.'+case,tag+'.mutation.'+case,tag+'.mutation.'+case+'.compile'}
   if profile=='tsan':result|={tag+'.race.compile',tag+'.race'}
 for cc in ['clang','arm-none-eabi-gcc']:
  result.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   tag=cc+'.'+cpu;names=['probe.compile','undefined','disassembly','target_consume','target_acquire','target_control','target_release_fence','target_signal_fence','native-link','reject.target_wide','reject.target_pair']
   if cpu=='cortex-m4':names+=['native-undefined','core.cas','core.publication','core.models','core.link','core.undefined']
   result|={tag+'.'+n for n in names}
 return result
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-03.json');before=read(HERE/'PR-T25-03.before-atomics.json');need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original tasks/baseline');need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook')
 expected={}
 for c in canonical['criteria']:
  v=c['variants'][0];expected[c['id']]=(c,v,False);expected[v['id']]=(c,v,True)
 rows=d.get('updates',[]);need(len(rows)==25 and {q.get('id') for q in rows}==set(expected)|{'LRUN-16'},'exact24 criterion/variant identities and existing parent')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'nonempty '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 need(bool(r.get('external_sha256')),'actual foreign ABI/provider identities')
 for p,h in r.get('external_sha256',{}).items():need(pathlib.Path(p).is_file() and sha(pathlib.Path(p))==h,'external '+p)
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact complete passing command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id'])
 for cc in ['gcc','clang']:
  for profile in ['debug','optimized','sanitized','tsan']:
   tag=cc+'.'+profile
   for case,(_,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+case,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker,'intended semantic mutation '+tag+case);need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
   if profile=='tsan':
    need('-fsanitize=thread' in commands.get(tag+'.publication.compile',{}).get('command',[]),'actual TSan instrumentation');x=commands.get(tag+'.race',{});need(x.get('exit_code')==66 and x.get('required_marker')=='WARNING: ThreadSanitizer: data race','actual race sensitivity')
 observations=r.get('observations',[]);need(len(observations)==8 and {q.get('profile') for q in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p in ['debug','optimized','sanitized','tsan']},'eight observed native profiles')
 for q in observations:
  tsan=q.get('profile','').endswith('.tsan');need(q.get('cases')==[c for cs in runner.GROUPS.values() for c in cs] and q.get('independent_or_physical') is False,'complete bounded native observations');need(q.get('fence_execution')==(not tsan) and q.get('fence_exclusion')==('TSan does not model thread fences' if tsan else None),'honest fence instrumentation scope')
 models=r.get('finite_model',{}).get('models',[]);need(len(models)==4,'four finite models')
 for q,expected_model in zip(models,[(1,False,13,12),(1,True,24,30),(2,False,37,54),(2,True,134,256)]):
  need((q.get('readers'),q.get('early_clear'),q.get('states'),q.get('edges'))==expected_model and q.get('terminal_states')==1 and bool(q.get('bad_read_witness'))==q.get('early_clear'),'actual finite state counts/witness')
 targets=r.get('target_audits',[]);need(len(targets)==4 and {(q.get('compiler'),q.get('cpu')) for q in targets}=={(cc,cpu) for cc in ['clang','arm-none-eabi-gcc'] for cpu in ['cortex-m4','cortex-m0']},'four target compiler/CPU audits')
 for q in targets:
  need(q.get('execution')=='not-run' and q.get('isr_authorization') is False and q.get('native_closure')==(q.get('cpu')=='cortex-m4'),'no target/ISR overclaim');tag=q.get('compiler','')+'.'+q.get('cpu','')
  for entry,marker in [('target_wide','__atomic_fetch_add_8'),('target_pair','__atomic_load')]:
   x=commands.get(tag+'.reject.'+entry,{});need(x.get('exit_code')==1 and x.get('required_marker')==marker,'specific helper rejection '+tag+entry)
  x=commands.get(tag+'.native-link',{});need(x.get('exit_code')==(0 if q.get('cpu')=='cortex-m4' else 1),'selected native closure '+tag)
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==15 and all(x.get('passed') is True for x in exports['commands']),'actual15-command learner export')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=read(HERE/'research_gate.json')
 for p,h in gate['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  partial=row.get('objective')=='T25.08.c' or row.get('id')=='LRUN-16';need(row.get('dimensions')==dimensions(partial),'six exact evidence dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'actual artifacts/limits');need(row.get('remaining_author_work')==([REMAINING] if partial else []),'exact remaining source obligation')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];case=CASES[c['objective']];need(row.get('parent_project')==canonical['project'] and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'unchanged task/delayed interval');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),case)),'exact criterion execution binding')
  elif row.get('id')=='LRUN-16':
   parent=next(x for x in read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json')['gaps'] if x['id']=='LRUN-16');need(row.get('original_gap')==parent,'unchanged parent');need(row.get('author_validation')==dict(report=RECEIPT,checks=list(dict.fromkeys(x for case in CASES.values() for x in checks_for(list(commands),case)))),'parent evidence completeness')
 need(d.get('remaining_parents')==['LRUN-17: T25.10–.12 progress, performance and broader validation remain separate.','LRUN-02: exact final C17/C23 source obligations remain open.'],'uncovered parents retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','stale-external','missing-mutant','missing-tsan','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','fence-overclaim','target-execution','isr-overclaim','model-overclaim','source-gap-erased','altered-parent','parent-overclaim']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-external':'external_sha256'}[kind];y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:y['commands']=[q for q in y['commands'] if q['id']!=('gcc.debug.mutation.model' if kind=='missing-mutant' else 'clang.tsan.race')]
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='fence-overclaim':y['observations'][3]['fence_execution']=True
   elif kind=='target-execution':y['target_audits'][0]['execution']='passed'
   elif kind=='isr-overclaim':y['target_audits'][0]['isr_authorization']=True
   elif kind=='model-overclaim':y['finite_model']['models'][0]['states']=999999
   elif kind=='source-gap-erased':x['updates'][-1]['remaining_author_work']=[]
   elif kind=='altered-parent':x['updates'][-1]['original_gap']['work']='Only execute a counter'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
