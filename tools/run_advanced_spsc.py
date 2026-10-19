"""Actual SPSC ownership variants, independent finite models and target closure."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_spsc';PUBLIC=ROOT/'labs/advanced_concurrency_depth/spsc';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/spsc';BUILD=ROOT/'build'/('advanced-spsc-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['interface','edges','wrap','circular','retained'];MUTANTS={'interface':('INTERFACE','interface','q.capacity == n'),'publication':('PUBLISH','edges','as_pop(&q, &out, NULL, NULL) == CMX_RETRY'),'reuse':('REUSE','edges','as_push(&q, value(43), NULL, NULL) == CMX_FULL'),'wrap':('WRAP','wrap','as_push(&q, value(99), NULL, NULL)'),'circular':('CIRCULAR','circular','as_circular_push(&q, value(i)) == CMX_OK')}
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/execution/README.md','assessor/execution/concurrent.c','labs/thread_depth/lifecycle/race_control.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def expected_commands():
 result={'environment.kernel','models'}
 for cc in ['gcc','clang']:
  result.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;result|={tag+'.compile',tag+'.learner.compile',*[tag+'.'+c for c in CASES],*[tag+'.learner.'+c for c in CASES[:-1]]}
   for m in MUTANTS:result|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
   if p=='tsan':result|={tag+'.race.compile',tag+'.race'}
 for cc in ['arm-none-eabi-gcc','clang']:
  result.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   tag=cc+'.'+cpu;result|={tag+'.core.compile',tag+'.provider.compile',tag+'.object.imports',tag+'.provider.imports',tag+'.object.disassembly',tag+'.core.link',tag+'.selected.link'}
   if not (cc=='clang' and cpu=='cortex-m0'):result.add(tag+'.core.link.imports')
   if cpu=='cortex-m4':result.add(tag+'.selected.imports')
 result|={'implicit-copy.compile','implicit-copy.imports','implicit-copy.reject'}
 return result
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};targets=[];observations=[]
 include=ROOT/'labs/execution/include';provider=ROOT/'assessor/execution/concurrent.c'
 def compile(cc,flags,name,sources,binary):run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,'-I',PUBLIC,'-I',include,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;sources=[PRIVATE/'spsc.c',provider,PUBLIC/'tests.c'];binary=BUILD/tag;compile(cc,flags,tag+'.compile',sources,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    stub=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',provider,PUBLIC/'tests.c'],stub)
    for case in CASES[:-1]:run(tag+'.learner.'+case,[stub,case],41,'AS_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(cc,flags+['-DAS_MUTANT_'+define],tag+'.mutation.'+name+'.compile',sources,mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=tag,cases=CASES,multiword_transfers=16000,wrap_states=1728,wrap_edges=2944,physical=False))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'))
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    tag=cc+'.'+cpu;flags=[cc,*(['--target=arm-none-eabi','-Wno-atomic-alignment'] if cc=='clang' else []),'-mcpu='+cpu,'-mthumb','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I',PUBLIC,'-I',include]
    obj=BUILD/(tag+'.o');old=BUILD/(tag+'.provider.o');run(tag+'.core.compile',flags+['-c',PRIVATE/'spsc.c','-o',obj]);run(tag+'.provider.compile',flags+['-c',provider,'-o',old]);harness.OUTPUTS.update([obj,old]);imports=run(tag+'.object.imports',['arm-none-eabi-nm','-u',obj]);old_imports=run(tag+'.provider.imports',['arm-none-eabi-nm','-u',old]);run(tag+'.object.disassembly',['arm-none-eabi-objdump','-dr',obj],marker='<as_push>')
    symbols=['as_init','as_pop','as_circular_init','as_circular_push','as_circular_pop'];core_supported=not(cc=='clang' and cpu=='cortex-m0');core=BUILD/(tag+'.core.elf');run(tag+'.core.link',['arm-none-eabi-ld','--gc-sections','--entry=as_push',*['--undefined='+s for s in symbols],obj,'-o',core],0 if core_supported else 1,None if core_supported else '__atomic_load_4')
    if core_supported:harness.OUTPUTS.add(core);assert not run(tag+'.core.link.imports',['arm-none-eabi-nm','-u',core]).strip()
    image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');supported=cpu=='cortex-m4';symbols+=['cmx_spsc_init','cmx_spsc_push','cmx_spsc_pop']
    run(tag+'.selected.link',['arm-none-eabi-ld','--gc-sections','--entry=as_push',*['--undefined='+s for s in symbols],'-Map='+str(mapping),obj,old,'-o',image],0 if supported else 1,None if supported else '__atomic_is_lock_free' if cc=='arm-none-eabi-gcc' else '__atomic_load_4');harness.OUTPUTS.add(mapping)
    if supported:harness.OUTPUTS.add(image);assert not run(tag+'.selected.imports',['arm-none-eabi-nm','-u',image]).strip()
    targets.append(dict(compiler=cc,cpu=cpu,strict_core_closure=core_supported,strict_selected_closure=supported,object_imports=imports.splitlines(),provider_object_imports=old_imports.splitlines(),execution='not-run',isr_authorized=False))
  obj=BUILD/'implicit-copy.o';run('implicit-copy.compile',['arm-none-eabi-gcc','-mcpu=cortex-m0','-mthumb','-std=c17','-O2','-ffreestanding','-fno-builtin','-I',PUBLIC,'-I',include,'-c',PUBLIC/'helper_probe.c','-o',obj]);harness.OUTPUTS.add(obj);run('implicit-copy.imports',['arm-none-eabi-nm','-u',obj],marker='memcpy');run('implicit-copy.reject',['arm-none-eabi-ld','--entry=as_implicit_copy',obj,'-o',BUILD/'implicit-rejected.elf'],1,'memcpy')
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,models=models,target_audits=targets,limits=['Author implementation/execution only; no external review, independent delayed learner or physical observations.','Core uses C17 with explicit two-way acquire/release ownership; N1570 source is a C11 draft, not final C17.','CMX boolean initialization and new AS enum admission remain distinct; original API and accepted source are unchanged.','Each producer/consumer domain is externally exclusive; owner-local reentry flags do not implement MPMC synchronization.','Finite reduced states and native schedules do not prove all ISO executions or scheduler progress.','Selected target link closure is not target execution or ISR authorization. Clang M0 core requires atomic helpers; both composed M0 images are rejected, including retained CMX runtime lock-free-query helper on GCC.','An original aggregate-copy diagnostic is rejected for implicit memcpy; explicit field-copy AS core alone passes selected GCC M0 closure.','LRUN-18 advanced MPMC/reclamation and LRUN-19 kernel tasks are separate required obligations.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
