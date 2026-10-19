"""Source-bound progress, cost, history and actual queue composition campaign."""
import datetime,hashlib,json,pathlib,re,statistics,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_progress';BUILD=ROOT/'build'/('thread-progress-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PUBLIC=ROOT/'labs/thread_depth/progress';PRIVATE=ROOT/'assessor/thread_depth/progress'
WAIT=ROOT/'labs/thread_depth/waiting';WAIT_PRIVATE=ROOT/'assessor/thread_depth/waiting'
GROUPS={'model':['donation','bound','effects','history'],'metrics':['layout','aggregation','freshness','cost'],'queue':['shutdown','stress']}
MUTANTS={'donation':('TP_MUTANT_DONATION','out[0] == 7'), 'bound':('TP_MUTANT_BOUND','out == 12'), 'effects':('TP_MUTANT_EFFECT','tp_nonsleepable(1u << i)'), 'history':('TP_MUTANT_HISTORY','TP_NONLINEAR'), 'layout':('TP_MUTANT_LAYOUT','tp_separated(0, 4, 2, 4, 64) == 0'), 'aggregation':('TP_MUTANT_AGGREGATION','sum == expected && check == checksum'), 'freshness':('TP_MUTANT_FRESHNESS','value == items'), 'cost':('TP_MUTANT_COST','sum == (mode == 0 ? 0 : expected)'), 'shutdown':('TP_MUTANT_SHUTDOWN','q.discarded == 3'), 'stress':('TW_MUTANT4','probe == 7')}
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]
IMPORTS=['tools/run_thread_lifecycle.py','labs/thread_depth/lifecycle/race_control.c',*[f'labs/thread_depth/waiting/{p}' for p in ['waiting.h','CONTRACTS.md','learner.c','tests.c','test_support.h','queue_tests.c','transaction_tests.c','primitive_tests.c']],*[f'assessor/thread_depth/waiting/{p}.c' for p in ['queue','transactions','primitives']],'assessor/thread_depth/atomics/handoff_model.py','assessor/thread_depth/atomics/models.c','assessor/thread_depth/atomics/publication.c','labs/thread_depth/atomics/atomics.h','labs/thread_depth/atomics/operation_tests.c']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def group_sources(group,learner=False):
 if group=='model':return ([PUBLIC/'learner.c'] if learner else [PRIVATE/'progress.c',PRIVATE/'history.c'])+[PUBLIC/'tests.c']
 if group=='metrics':return [PUBLIC/'metrics_learner.c' if learner else PRIVATE/'metrics.c',PUBLIC/'metrics_tests.c']
 return ([PUBLIC/'queue_learner.c',PUBLIC/'learner.c',WAIT/'learner.c'] if learner else [PRIVATE/'queue.c',PRIVATE/'progress.c',PRIVATE/'history.c',WAIT_PRIVATE/'queue.c'])+[PUBLIC/'queue_tests.c']
def summarize(text):
 groups={};samples=0
 for line in text.splitlines():
  if not line.startswith('SAMPLE '):continue
  fields=dict(re.findall(r'(\w+)=(\d+)',line));samples+=1
  if fields['warmup']=='1':continue
  key=line.split()[1]+':'+','.join(k+'='+fields[k] for k in ['separated','mode','intensity'] if k in fields)
  groups.setdefault(key,[]).append(int(fields['nanoseconds']))
 return dict(raw_samples=samples,groups={k:dict(n=len(v),minimum=min(v),median=statistics.median(v),quartiles=statistics.quantiles(v,n=4,method='inclusive'),maximum=max(v),raw_nanoseconds=v) for k,v in groups.items()})
def expected_commands():
 result={'environment.kernel','environment.libc','hazards.model','ordering.minimize','handoff.model'}
 for cc in ['gcc','clang']:
  result|={cc+'.version',cc+'.history.shared.compile',cc+'.history.oracle',cc+'.effect.compile',cc+'.effect.imports',cc+'.weak.compile',cc+'.weak.model',cc+'.metrics.disassembly'}
  for profile,_ in PROFILES:
   tag=cc+'.'+profile
   for group,cases in GROUPS.items():
    result|={tag+'.'+group+'.compile',tag+'.'+group+'.learner.compile'}
    for case in cases:result|={tag+'.'+case,tag+'.learner.'+case,tag+'.mutation.'+case,tag+'.mutation.'+case+'.compile'}
   result|={tag+'.waiting.compile',*[tag+'.waiting.'+c for c in ['order','queue','wake','deadline']]}
   if profile=='tsan':result|={tag+'.race.compile',tag+'.race'}
 return result
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();failure=None;observations=[];measurements={};models={}
 harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run
 def compile(cc,flags,name,sources,binary):
  run(name,[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread',*flags,'-I',PUBLIC,'-I',WAIT,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile
    for group,cases in GROUPS.items():
     binary=BUILD/(tag+'.'+group);compile(cc,flags,tag+'.'+group+'.compile',group_sources(group),binary)
     for case in cases:
      output=run(tag+'.'+case,[binary,case],marker='OBS '+case)
      if profile=='optimized' and case in ['layout','aggregation','cost']:measurements[tag+'.'+case]=summarize(output)
     stub=BUILD/(tag+'.'+group+'.learner');compile(cc,flags,tag+'.'+group+'.learner.compile',group_sources(group,True),stub)
     for case in cases:run(tag+'.learner.'+case,[stub,case],41,'TP_ASSERT:')
     for case in cases:
      define,marker=MUTANTS[case];mutant=BUILD/(tag+'.mutation.'+case);compile(cc,flags+['-D'+define],tag+'.mutation.'+case+'.compile',group_sources(group),mutant);run(tag+'.mutation.'+case,[mutant,case],41,marker)
    waiting=BUILD/(tag+'.waiting');sources=[WAIT_PRIVATE/(n+'.c') for n in ['queue','transactions','primitives']]+[WAIT/p for p in ['tests.c','queue_tests.c','transaction_tests.c','primitive_tests.c']]
    compile(cc,flags,tag+'.waiting.compile',sources,waiting)
    for case in ['order','queue','wake','deadline']:run(tag+'.waiting.'+case,[waiting,case],marker='OBS '+case)
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=tag,cases=[c for cs in GROUPS.values() for c in cs],actual_queue_histories=20,performance_comparison=profile=='optimized',independent_or_physical=False))
   shared=BUILD/(cc+'.history.so');compile(cc,['-O2','-fPIC','-shared'],cc+'.history.shared.compile',[PRIVATE/'progress.c',PRIVATE/'history.c'],shared);models[cc+'.history']=json.loads(run(cc+'.history.oracle',[sys.executable,PRIVATE/'history_oracle.py',shared],marker='"status": "pass"'))
   obj=BUILD/(cc+'.effects.o');run(cc+'.effect.compile',[cc,'-std=c17','-Wall','-Wextra','-Werror','-O2','-c',PUBLIC/'effects_probe.c','-o',obj]);harness.OUTPUTS.add(obj);symbols=run(cc+'.effect.imports',['nm','-u',obj]);assert all(s in symbols for s in ['malloc','free','pthread_mutex_lock','pthread_mutex_unlock','write'])
   weak=BUILD/(cc+'.weak');ap=ROOT/'labs/thread_depth/atomics';av=ROOT/'assessor/thread_depth/atomics';run(cc+'.weak.compile',[cc,'-std=c17','-Wall','-Wextra','-Werror','-pthread',*(['-Wno-atomic-alignment'] if cc=='clang' else []),'-O2','-I',ap,av/'models.c',av/'publication.c',ap/'operation_tests.c','-latomic','-o',weak]);harness.OUTPUTS.add(weak);run(cc+'.weak.model',[weak,'model'],marker='OBS model')
   asm=run(cc+'.metrics.disassembly',['objdump','-dr',BUILD/(cc+'.optimized.metrics')]);assert all(s in asm for s in ['<tp_mix>','<tp_accumulate>','pthread_mutex_lock','pthread_mutex_unlock','clock_gettime']),cc
  for ident,path in [('hazards.model',PRIVATE/'hazards.py'),('ordering.minimize',PRIVATE/'ordering_model.py'),('handoff.model',ROOT/'assessor/thread_depth/atomics/handoff_model.py')]:models[ident]=json.loads(run(ident,[sys.executable,path],marker='"status": "pass"'))
  assert {c['id'] for c in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 limits=['Author implementation/execution only; no independent review, delayed learner assessment or physical observation.','Finite schedules/search/models do not prove general fairness, liveness, all histories or absolute mastery.','Performance comparisons use ordinary optimized profiles only; instrumented runs validate behavior. Host affinity is inspected, never changed; no scheduling privileges or host policy changes.','Cache addresses and elapsed time are not hardware coherence-traffic measurements; reported line size and timing remain host-specific.','Weak-order changed packet uses explicit restricted models, not execution on a weakly ordered physical target.','Current mutex totals change the intermediate freshness contract explicitly; private/batched and per-item schemes share only the final result contract.','Changed queue shutdown settles admitted calls before discarding residue; cancellation may be delivered after commit and does not imply rollback.','Exact final published C17/C23 and all externally assessed/physical obligations remain separately open.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,measurements=measurements,models=models,limits=limits)
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
