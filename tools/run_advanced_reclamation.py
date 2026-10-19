"""Source-bound complete bounded MPMC, cancellation and protection campaign."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_reclamation';PUBLIC=ROOT/'labs/advanced_concurrency_depth/reclamation';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/reclamation';BUILD=ROOT/'build'/('advanced-reclamation-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['validation','protection','publication','collector','registration','generation','retention','departure','stress']
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]
MUTANTS={'validation':('AP_MUTANT_VALIDATION','validation','g.first_validated == 1'),'protection':('AP_MUTANT_PROTECTION','protection','collected == 0 && state == 2'),'collector':('AP_MUTANT_COLLECTOR','collector','atomic_load(&q.node[0].state) == 6'),'registration':('AP_MUTANT_REGISTRATION','registration','later.generation == 2'),'generation':('AP_MUTANT_GENERATION','generation','count < expected'),'cancel':('AR_MUTANT_CANCEL','reservation','waiting == AR_PENDING'),'pending':('AR_MUTANT_PENDING','reservation','ar_pop(&q, &v) == AR_PENDING')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','assessor/execution/advanced.c','assessor/execution/concurrent.c','labs/advanced_concurrency_depth/history/history.h','labs/advanced_concurrency_depth/history/learner.c','labs/advanced_concurrency_depth/history/CONTRACTS.md','labs/advanced_concurrency_depth/history/tests.c','assessor/advanced_concurrency_depth/history/history.c','labs/advanced_concurrency_depth/spsc/CONTRACTS.md','assessor/advanced_concurrency_depth/spsc/models.py','labs/thread_depth/lifecycle/race_control.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def expected_commands():
 result={'environment.kernel','models','spsc.models'}
 for cc in ['gcc','clang']:
  result.add(cc+'.version')
  for profile,_ in PROFILES:
   tag=cc+'.'+profile
   for variant in ['sc','relaxed']:
    t=tag+'.'+variant;result|={t+'.compile',t+'.histories.compile',t+'.histories',t+'.history.oracle',*[t+'.'+c for c in CASES]}
   result|={tag+'.reservation.compile',tag+'.reservation',tag+'.retained.compile',tag+'.retained.points',tag+'.retained.pressure',tag+'.learner.compile',tag+'.learner.reservation.compile',tag+'.learner.reservation',tag+'.learner.histories.compile',tag+'.learner.histories',*[tag+'.learner.'+c for c in CASES]}
   for name in MUTANTS:result|={tag+'.mutation.'+name+'.compile',tag+'.mutation.'+name}
   if profile=='tsan':result|={tag+'.race.compile',tag+'.race'}
 for cc in ['arm-none-eabi-gcc','clang']:
  result.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   for core in ['reclamation','reservation']:
    t=cc+'.'+cpu+'.'+core;result|={t+'.compile',t+'.imports',t+'.disassembly',t+'.link'}
    if cpu=='cortex-m4':result.add(t+'.closed')
 return result
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};spsc={};targets=[];observations=[];oracles={}
 history=ROOT/'assessor/advanced_concurrency_depth/history/history.c';advanced=ROOT/'assessor/execution/advanced.c';concurrent=ROOT/'assessor/execution/concurrent.c'
 def compile(cc,flags,name,sources,binary):
  run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*flags,'-I',PUBLIC,'-I',ROOT/'labs/execution/include','-I',ROOT/'labs/advanced_concurrency_depth/history',*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile
    for variant in ['sc','relaxed']:
     t=tag+'.'+variant;vf=flags+(['-DAP_RELAX_ALLOC'] if variant=='relaxed' else []);binary=BUILD/t
     compile(cc,vf,t+'.compile',[PRIVATE/'reclamation.c',PUBLIC/'tests.c'],binary)
     for case in CASES:run(t+'.'+case,[binary,case],marker='OBS '+case)
     binary=BUILD/(t+'.histories');compile(cc,vf,t+'.histories.compile',[PRIVATE/'reclamation.c',history,PUBLIC/'history_tests.c'],binary);run(t+'.histories',[binary],marker='OBS histories actual_calls=256')
     oracles[t]=json.loads(run(t+'.history.oracle',[sys.executable,PRIVATE/'history_oracle.py',BUILD/(t+'.histories.log')],marker='"status": "pass"'))
    binary=BUILD/(tag+'.reservation');compile(cc,flags,tag+'.reservation.compile',[PRIVATE/'reservation.c',advanced,PUBLIC/'reservation_tests.c'],binary);run(tag+'.reservation',[binary],marker='OBS reservation')
    binary=BUILD/(tag+'.retained');compile(cc,flags,tag+'.retained.compile',[history,concurrent,ROOT/'labs/advanced_concurrency_depth/history/tests.c'],binary)
    for case in ['points','pressure']:run(tag+'.retained.'+case,[binary,case],marker='OBS '+case)
    binary=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'tests.c'],binary)
    for case in CASES:run(tag+'.learner.'+case,[binary,case],41,'AP_ASSERT:')
    binary=BUILD/(tag+'.learner.reservation');compile(cc,flags,tag+'.learner.reservation.compile',[PUBLIC/'reservation_learner.c',advanced,PUBLIC/'reservation_tests.c'],binary);run(tag+'.learner.reservation',[binary],41,'ar_pop(&q, &v) == AR_EMPTY')
    binary=BUILD/(tag+'.learner.histories');compile(cc,flags,tag+'.learner.histories.compile',[PUBLIC/'learner.c',history,PUBLIC/'history_tests.c'],binary);run(tag+'.learner.histories',[binary],41,'AP_HISTORY_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     binary=BUILD/(tag+'.mutation.'+name);sources=[PRIVATE/'reservation.c',advanced,PUBLIC/'reservation_tests.c'] if case=='reservation' else [PRIVATE/'reclamation.c',PUBLIC/'tests.c'];compile(cc,flags+['-D'+define],tag+'.mutation.'+name+'.compile',sources,binary);run(tag+'.mutation.'+name,[binary,*([] if case=='reservation' else [case])],41,marker)
    if profile=='tsan':
     binary=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],binary);run(tag+'.race',[binary],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=tag,variants=['sc','relaxed'],cases=CASES,histories_per_variant=64,calls_per_variant=256,stress_values_per_variant=[1024,2048],generation_admissions_per_variant=[63,127],actual_paused_publisher=True,joined_departure=True,cancellation_requires_owner_ack=True,independent_or_physical=False))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'));spsc=json.loads(run('spsc.models',[sys.executable,ROOT/'assessor/advanced_concurrency_depth/spsc/models.py'],marker='"status": "pass"'))
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    for core,names in [('reclamation',['ap_init','ap_register','ap_leave','ap_push','ap_pop','ap_collect']),('reservation',['ar_init','ar_reserve','ar_finish','ar_cancel','ar_pop'])]:
     t=cc+'.'+cpu+'.'+core;obj=BUILD/(t+'.o');image=BUILD/(t+'.elf');flags=[cc,*(['--target=arm-none-eabi','-Wno-error=atomic-alignment'] if cc=='clang' else []),'-mcpu='+cpu,'-mthumb','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I',PUBLIC]
     run(t+'.compile',flags+['-c',PRIVATE/(core+'.c'),'-o',obj]);harness.OUTPUTS.add(obj);imports=run(t+'.imports',['arm-none-eabi-nm','-u',obj]);run(t+'.disassembly',['arm-none-eabi-objdump','-dr',obj],marker='<'+names[0]+'>');supported=cpu=='cortex-m4';run(t+'.link',['arm-none-eabi-ld','--gc-sections','--entry='+names[0],*['--undefined='+n for n in names[1:]],obj,'-o',image],0 if supported else 1,None if supported else '__atomic_')
     if supported:harness.OUTPUTS.add(image);assert not run(t+'.closed',['arm-none-eabi-nm','-u',image]).strip()
     targets.append(dict(compiler=cc,cpu=cpu,core=core,strict_closure=supported,imports=imports.splitlines(),execution='not-run',isr_authorized=False))
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped sources changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,oracles=oracles,models=models,spsc_models=spsc,target_audits=targets,limits=['Author implementation/execution only; no external review, independent learner, physical board or ISR observations.','Recorded actual histories, bounded models and sanitizer paths do not establish every schedule, universal linearizability or unbounded progress.','AP_FULL is a failed bounded allocation scan, not an exact concurrent logical-capacity or global-full observation.','Only the allocation-candidate state load is weakened; all hazard publication/validation/scan operations remain SC.','Reservation cancellation requires producer acknowledgement; a stalled producer or consumer may block the FIFO prefix.','Four/eight participants keep32 nodes; hazard-slot bounds exclude unscanned and terminal-generation retirements.','Actual source inspection covers the listed paper figures and C11 draft clauses; final published C17/C23 comparison remains distinct.','Target audits inspect generated dependencies and reject unsupported closure; no target execution or runtime lock-free guarantee is inferred.','Other T26.07–.10 obligations remain required.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
