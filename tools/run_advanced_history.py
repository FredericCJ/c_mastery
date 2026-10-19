"""Source-bound history/progress, imported queue and dependency experiments."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_history';PUBLIC=ROOT/'labs/advanced_concurrency_depth/history';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/history';BUILD=ROOT/'build'/('advanced-history-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['history','separation','points','pressure','retry']
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]
MUTANTS={'realtime':('REALTIME','history','== AH_NONLINEAR'),'order':('ORDER','history','== AH_NONLINEAR'),'pending':('PENDING','history','w.completed_pending == 1'),'retry':('RETRY','retry','n == budget ? AH_INCOMPLETE : AH_NONLINEAR'),'tail':('TAIL','points','cmx_hp_pop(&q, 1, &value) == CMX_OK'),'pressure':('PRESSURE','pressure','cmx_hp_push(&q, 0, 77) == CMX_FULL')}
MUTANTS['witness']=('WITNESS','history','w.order[0] < 3')
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/execution/README.md','assessor/execution/concurrent.c','labs/thread_depth/lifecycle/race_control.c']
PROVIDER_MUTATIONS={'TAIL':('if (next == NO_NODE) {\n      clear(q, p);\n      return CMX_RETRY;\n    }','if (head == tail) {\n      clear(q, p);\n      return CMX_RETRY;\n    }'),'PRESSURE':('if (n == NO_NODE)\n    return CMX_FULL;','if (n == NO_NODE)\n    return CMX_OK;')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def expected_commands():
 result={'environment.kernel','progress.models'}
 for cc in ['gcc','clang']:
  result|={cc+'.version',cc+'.history.oracle',cc+'.allocator.compile',cc+'.allocator.imports',cc+'.allocator.reject'}
  for profile,_ in PROFILES:
   tag=cc+'.'+profile;result|={tag+'.compile',tag+'.learner.compile',*[tag+'.'+c for c in CASES],*[tag+'.learner.'+c for c in CASES]}
   for name in MUTANTS:result|={tag+'.mutation.'+name+'.compile',tag+'.mutation.'+name}
   if profile=='tsan':result|={tag+'.race.compile',tag+'.race'}
 for cc in ['arm-none-eabi-gcc','clang']:
  result.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   tag=cc+'.'+cpu;result|={tag+'.compile',tag+'.imports',tag+'.disassembly',tag+'.link'}
   if cpu=='cortex-m4':result.add(tag+'.closed')
 return result
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;oracles={};models={};targets=[];observations=[];provider=ROOT/'assessor/execution/concurrent.c';include=ROOT/'labs/execution/include';copies={}
 def compile(cc,flags,name,sources,binary):run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,'-I',PUBLIC,'-I',include,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  source=provider.read_text(encoding='utf8')
  for name,(old,new) in PROVIDER_MUTATIONS.items():
   assert source.count(old)==1,(name,'mutation must match exactly once');p=BUILD/('provider_'+name+'.c');p.write_text(source.replace(old,new),encoding='utf8');copies[name]=p;harness.OUTPUTS.add(p)
  run('environment.kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;sources=[PRIVATE/'history.c',provider,PUBLIC/'tests.c'];compile(cc,flags,tag+'.compile',sources,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    stub=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'retained_learner.c',PUBLIC/'tests.c'],stub)
    for case in CASES:run(tag+'.learner.'+case,[stub,case],41,'AH_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(cc,flags+['-DAH_MUTANT_'+define],tag+'.mutation.'+name+'.compile',[PRIVATE/'history.c',copies.get(define,provider),PUBLIC/'tests.c'],mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=tag,cases=CASES,constructed_prefix_not_paused_thread=True,retired_nodes=31,logical_occupancy_at_failed_admission=0,independent_or_physical=False))
   oracles[cc]=json.loads(run(cc+'.history.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(cc+'.optimized')],marker='"status": "pass"'))
   obj=BUILD/(cc+'.allocator.o');run(cc+'.allocator.compile',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-c',PUBLIC/'allocator_probe.c','-o',obj]);harness.OUTPUTS.add(obj);output=run(cc+'.allocator.imports',['nm','-u',obj],marker='malloc');assert 'free' in output;run(cc+'.allocator.reject',[cc,'-nostdlib','-static','-no-pie','-Wl,--entry=ah_allocate_node',obj,'-o',BUILD/(cc+'.rejected.elf')],1,'malloc')
  models=json.loads(run('progress.models',[sys.executable,PRIVATE/'progress_models.py'],marker='"status": "pass"'))
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    tag=cc+'.'+cpu;obj=BUILD/(tag+'.o');image=BUILD/(tag+'.elf');flags=[cc,*(['--target=arm-none-eabi','-Wno-atomic-alignment'] if cc=='clang' else []),'-mcpu='+cpu,'-mthumb','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I',include]
    run(tag+'.compile',flags+['-c',provider,'-o',obj]);harness.OUTPUTS.add(obj);imports=run(tag+'.imports',['arm-none-eabi-nm','-u',obj]);run(tag+'.disassembly',['arm-none-eabi-objdump','-dr',obj],marker='<cmx_hp_push>');supported=cpu=='cortex-m4';run(tag+'.link',['arm-none-eabi-ld','--gc-sections','--entry=cmx_hp_push','--undefined=cmx_hp_init','--undefined=cmx_hp_pop','--undefined=cmx_hp_collect',obj,'-o',image],0 if supported else 1,None if supported else '__atomic_compare_exchange_4')
    if supported:harness.OUTPUTS.add(image);assert not run(tag+'.closed',['arm-none-eabi-nm','-u',image]).strip()
    targets.append(dict(compiler=cc,cpu=cpu,strict_selected_closure=supported,object_imports=imports.splitlines(),execution='not-run',isr_authorized=False))
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,oracles=oracles,models=models,target_audits=targets,limits=['Author execution only, no external review or independent delayed learner assessment.','Histories are bounded and totalized by the new contract; the source paper and C memory-model obligations remain distinct.','Tail helping is actual consumer execution from a constructed valid prefix, not a falsely claimed paused producer thread or full MPMC stress.','CMX_FULL reports failed bounded allocation scan, not an exact concurrent global pool-full or logical-capacity observation.','Finite schedules supply explicit repeating-state counterexamples; no universal liveness or per-request time proof is inferred.','Both selected M4 images close; M0 images require runtime atomic helpers and are rejected. No target execution or ISR authorization.','No physical observations or complete hazard/epoch/RCU correctness claim; other LRUN-18/19 obligations remain required.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
