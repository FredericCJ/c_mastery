"""Actual finite identity pools, ABA models and atomic-width dependency audit."""
import datetime,hashlib,json,pathlib,re,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_identity';PUBLIC=ROOT/'labs/advanced_concurrency_depth/identity';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/identity';BUILD=ROOT/'build'/('advanced-identity-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['pool','exhaust','compact','retained','aba'];PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS={'generation':('GENERATION','pool','!ai_valid(&p, handles[0])'),'increment':('INCREMENT','pool','out != handles[0]'),'wrap':('WRAP','exhaust','(h & 255u) == slot + 1'),'tag':('TAG','aba','!ai_word_cas(&root, &paused, 514)')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','assessor/execution/advanced.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def expected_commands():
 result={'environment.kernel','models'}
 for cc in ['gcc','clang']:
  result|={cc+'.version',cc+'.width.compile',cc+'.width',cc+'.width.imports',cc+'.width.ldd'}
  for profile,_ in PROFILES:
   tag=cc+'.'+profile;result|={tag+'.compile',tag+'.learner.compile',*[tag+'.'+c for c in CASES],*[tag+'.learner.'+c for c in CASES]}
   for name in MUTANTS:result|={tag+'.mutation.'+name+'.compile',tag+'.mutation.'+name}
 for cc in ['arm-none-eabi-gcc','clang']:
  result.add(cc+'.target-version')
  for cpu in ['cortex-m4','cortex-m0']:
   tag=cc+'.'+cpu;result|={tag+'.core.compile',tag+'.core.imports',tag+'.core.link',tag+'.pair.compile',tag+'.pair.imports',tag+'.pair.reject'}
   if cpu=='cortex-m4':result.add(tag+'.core.closed')
 return result
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};targets=[];observations=[];widths={};external={};provider=ROOT/'assessor/execution/advanced.c';include=ROOT/'labs/execution/include'
 def compile(cc,flags,name,sources,binary):run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*flags,'-I',PUBLIC,'-I',include,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;sources=[PRIVATE/'identity.c',provider,PUBLIC/'tests.c'];compile(cc,flags,tag+'.compile',sources,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    stub=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'retained_learner.c',PUBLIC/'tests.c'],stub)
    for case in CASES:run(tag+'.learner.'+case,[stub,case],41,'AI_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(cc,flags+['-DAI_MUTANT_'+define],tag+'.mutation.'+name+'.compile',sources,mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,reduced_pool_incarnations=4080,retained_slot_incarnations=65535,compact_variant='saturation-boundary-only',concurrent_pool=False,physical=False))
   binary=BUILD/(cc+'.width');compile(cc,['-O2',*(['-Wno-error=atomic-alignment'] if cc=='clang' else [])],cc+'.width.compile',[PRIVATE/'identity.c',PUBLIC/'atomic_width.c',PUBLIC/'width_driver.c','-latomic'],binary);out=run(cc+'.width',[binary],marker='OBS width');widths[cc]={k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',out)};run(cc+'.width.imports',['nm','-u',binary],marker='__atomic_compare_exchange');listing=run(cc+'.width.ldd',['ldd',binary],marker='libatomic.so');match=re.search(r'libatomic\.so\.\d+\s+=>\s+(\S+)',listing);assert match,'actual loaded libatomic path missing';library=pathlib.Path(match.group(1)).resolve();external[str(library)]=sha(library)
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'))
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    tag=cc+'.'+cpu;obj=BUILD/(tag+'.o');pair=BUILD/(tag+'.pair.o');image=BUILD/(tag+'.elf');flags=[cc,*(['--target=arm-none-eabi','-Wno-error=atomic-alignment'] if cc=='clang' else []),'-mcpu='+cpu,'-mthumb','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-I',PUBLIC]
    run(tag+'.core.compile',flags+['-c',PRIVATE/'identity.c','-o',obj]);harness.OUTPUTS.add(obj);imports=run(tag+'.core.imports',['arm-none-eabi-nm','-u',obj]);supported=cpu=='cortex-m4';run(tag+'.core.link',['arm-none-eabi-ld','--gc-sections','--entry=ai_take','--undefined=ai_init','--undefined=ai_release','--undefined=ai_valid','--undefined=ai_word_cas',obj,'-o',image],0 if supported else 1,None if supported else '__atomic_compare_exchange_4')
    if supported:harness.OUTPUTS.add(image);assert not run(tag+'.core.closed',['arm-none-eabi-nm','-u',image]).strip()
    run(tag+'.pair.compile',flags+['-c',PUBLIC/'atomic_width.c','-o',pair]);harness.OUTPUTS.add(pair);pair_imports=run(tag+'.pair.imports',['arm-none-eabi-nm','-u',pair],marker='__atomic_compare_exchange');run(tag+'.pair.reject',['arm-none-eabi-ld','--entry=ai_pair_cas',pair,'-o',BUILD/(tag+'.rejected.elf')],1,'__atomic_compare_exchange');targets.append(dict(compiler=cc,cpu=cpu,strict_core_closure=supported,strict_pair_closure=False,core_imports=imports.splitlines(),pair_imports=pair_imports.splitlines(),execution='not-run'))
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,external_input_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,widths=widths,models=models,target_audits=targets,limits=['Author execution only; no independent learner/reviewer or physical observations.','Pool accesses are externally serialized; separate atomic root probes do not make the pool a concurrent algorithm.','All4080 reduced incarnations and65535 retained-slot incarnations ran/profile;24-bit saturation is a boundary projection, not268435440 executed allocations.','Native selected no-padding ABI pair atomics use the explicitly pinned hosted libatomic; non-lock-free result does not identify its hidden locking algorithm.','Strict target core+32-bit CAS closes only on the selected M4 profiles; both double-width pairs and M0 CAS require helpers and are rejected. No target or ISR execution.','Generation validation is not borrow protection, and hazard/deferred reuse does not eliminate all logical same-node ABA.','Final C17/C23 and other T26 reclamation/kernel obligations remain distinct.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
