"""Actual serialized epoch/version adapters and complete target dependency audit."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_grace';PUBLIC=ROOT/'labs/advanced_concurrency_depth/grace';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/grace';BUILD=ROOT/'build'/('advanced-grace-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['epoch','versions','pressure','callbacks','writers','departure'];PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]
MUTANTS={'nested':('NESTED','epoch','d.retained.active[0] && d.depth[0] == 1'),'epoch':('EPOCH','pressure','ag_collect(&s) == 0'),'refs':('REFS','callbacks','ag_collect(&s) == 0'),'capture':('CAPTURE','callbacks','s.objects[b.version.index].refs == 2'),'writer':('WRITER','writers','ag_writer_acquire(&s, 2) == AG_BUSY')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','labs/execution/targets/README.md','labs/execution/src/support.c','assessor/execution/concurrent.c','assessor/execution/advanced.c','assessor/execution/support.c','labs/api_depth/include/api_depth.h','labs/api_depth/CONTRACTS.md','assessor/api_depth/api_depth.c','labs/thread_depth/lifecycle/race_control.c']
SYMBOLS=['ag_init','ag_register','ag_enter','ag_leave','ag_depart','ag_advance','ag_reclaimable','ag_store_init','ag_writer_acquire','ag_writer_release','ag_publish','ag_read','ag_copy','ag_read_end','ag_collect','ag_capture','ag_dispatch','ag_remove','ag_destroy_callbacks']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def expected_commands():
 out={'environment.kernel','models'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   t=cc+'.'+p;out|={t+'.compile',t+'.learner.compile',*[t+'.'+c for c in CASES],*[t+'.learner.'+c for c in CASES]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
   if p=='tsan':out|={t+'.race.compile',t+'.race'}
 for cc in ['arm-none-eabi-gcc','clang']:
  out.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   t=cc+'.'+cpu;out|={t+'.bare.reject',t+'.composed.link',t+'.composed.closed',t+'.composed.symbols'}
   for part in ['grace','epoch','registry','support']:out|={t+'.'+part+'.compile',t+'.'+part+'.imports',t+'.'+part+'.disassembly'}
 return out
def main():
 BUILD.mkdir(parents=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};observations=[];targets=[]
 include=ROOT/'labs/execution/include';api_include=ROOT/'labs/api_depth/include';epoch=ROOT/'assessor/execution/concurrent.c';rcu=ROOT/'assessor/execution/advanced.c';api=ROOT/'assessor/api_depth/api_depth.c'
 def compile(cc,flags,name,sources,binary):run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*flags,'-I',PUBLIC,'-I',include,'-I',api_include,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    t=cc+'.'+p;binary=BUILD/t;sources=[PRIVATE/'grace.c',epoch,rcu,api,PUBLIC/'tests.c'];compile(cc,flags,t+'.compile',sources,binary)
    for case in CASES:run(t+'.'+case,[binary,case],marker='OBS '+case)
    stub=BUILD/(t+'.learner');compile(cc,flags,t+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'retained_learner.c',PUBLIC/'tests.c'],stub)
    for case in CASES:run(t+'.learner.'+case,[stub,case],41,'AG_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     mutant=BUILD/(t+'.mutation.'+name);compile(cc,flags+['-DAG_MUTANT_'+define],t+'.mutation.'+name+'.compile',sources,mutant);run(t+'.mutation.'+name,[mutant,case],41,marker)
    if p=='tsan':
     race=BUILD/(t+'.race');compile(cc,flags,t+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(t+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=t,cases=CASES,serialized_core=True,actual_mutex_writers=2,publications=64,retired_versions=16,payload_bytes=512,joined_departure=True,registry='actual struct api_registry with counted version captures',independent_or_physical=False))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'))
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    t=cc+'.'+cpu;objects={};imports={}
    for part,source in [('grace',PRIVATE/'grace.c'),('epoch',epoch),('registry',api),('support',ROOT/'assessor/execution/support.c')]:
     obj=BUILD/(t+'.'+part+'.o');flags=[cc,*(['--target=arm-none-eabi','-Wno-error=atomic-alignment'] if cc=='clang' else []),'-mcpu='+cpu,'-mthumb','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I',PUBLIC,'-I',include,'-I',api_include]
     run(t+'.'+part+'.compile',flags+['-c',source,'-o',obj]);harness.OUTPUTS.add(obj);objects[part]=obj;imports[part]=run(t+'.'+part+'.imports',['arm-none-eabi-nm','-u',obj]).splitlines();run(t+'.'+part+'.disassembly',['arm-none-eabi-objdump','-dr',obj])
    link=['arm-none-eabi-ld','--gc-sections','--entry=ag_store_init',*['--undefined='+n for n in SYMBOLS]]
    run(t+'.bare.reject',link+[*[objects[n] for n in ['grace','epoch','registry']],'-o',BUILD/(t+'.bare.elf')],1,'mem')
    image=BUILD/(t+'.elf');run(t+'.composed.link',link+list(objects.values())+['-Map='+str(BUILD/(t+'.map')),'-o',image]);harness.OUTPUTS.add(image);harness.OUTPUTS.add(BUILD/(t+'.map'));assert not run(t+'.composed.closed',['arm-none-eabi-nm','-u',image]).strip();symbols=run(t+'.composed.symbols',['arm-none-eabi-nm',image],marker='mem');targets.append(dict(compiler=cc,cpu=cpu,bare_closure=False,declared_byte_provider_closure=True,imports=imports,execution='not-run',client_callbacks_audited=False,provider='assessor/execution/support.c'))
  assert set(x['id'] for x in harness.COMMANDS)==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if after!=before:failure='Scoped source changed during native campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,models=models,target_audits=targets,limits=['Author evidence only; external review, independent learner and physical observation not performed.','AG ordinary metadata requires one external serialization domain; only the explicit hosted mutex adapter is used by two actual writer threads.','Native epoch/RCU model is not Linux RCU; the separately pinned guest campaign supplies context-specific observations.','The active-at1/current5 ledger is an injected conservative state, not a reachable retained-tracker history.','Normalized32-byte retention counts exclude metadata; actual AG storage size is reported separately.','Bare target linkage fails implicit byte helpers; explicit original byte provider closes the selected component symbols, not arbitrary client callbacks or ISR usage.','Bounded models, profiles and mutations do not prove all schedules or unbounded progress.','Original T26.09/.10 kernel obligations and final C17/C23 source comparison remain separate.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
