"""Actual retained AVL/provider certificates, free-list transfer and concurrent admission."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_tree';PUBLIC=ROOT/'labs/algorithm_tree';PRIVATE=ROOT/'assessor/algorithm_tree';BUILD=ROOT/'build'/('algorithm-tree-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])];CASES=['rotations','prior','reject','policies','costs']
MUTANTS={'HEIGHT':('reject','at_check(&bad,1,&h)==ALGO_BAD&&h==77','-DAT_MUTANT_HEIGHT'),'DUPLICATE':('reject','at_insert(&p,2,0,&w)==AT_DUPLICATE','-DAT_MUTANT_DUPLICATE'),'FREE':('costs','at_delete(&free,(int)(2*k),1,&w)==ALGO_OK&&freelist(&free)','-DAT_MUTANT_FREE'),'REPAIR':('rotations','at_insert(&p,cases[f][i],0,&w)==ALGO_OK&&certificate(&p,&c)','-DAT_MUTANT_REPAIR')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','assessor/algorithms/algo.c','labs/containers_depth/byte_helpers.c','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','assessor/containers_depth/ordered.c','fixtures/algorithms/PR-T36-05.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','fixture','generate','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','rotations.oracle','prior.oracle','threads.compile','threads','disassembly','stub.compile']+CASES};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.core',cc+'.core.undefined'}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in [*MUTANTS,'stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*sum((['-I',p] for p in [PUBLIC,ROOT/'labs/algorithms/include',ROOT/'labs/containers_depth']),[])]
 def sources(stub=False):return [PUBLIC/'learner.c'] if stub else [PRIVATE/'certificate.c',generated/'observed.c',generated/'free.c',generated/'baseline.c',ROOT/'assessor/containers_depth/ordered.c']
 def compile(name,cc,flags,src,binary):run(name,[cc,*strict,*flags,*src,'-lm','-pthread','-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');src=[*sources(stub),PUBLIC/'tests.c',PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c']
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*src,'-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AT_THUMB_FAIL' if bad else 'AT_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS generated');harness.OUTPUTS.update(generated.iterdir())
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources(),PUBLIC/'tests.c',PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in ['rotations','prior']:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    thread=BUILD/(tag+'.threads');compile(tag+'.threads.compile',cc,flags,[*sources(),PUBLIC/'threads.c'],thread);out=run(tag+'.threads',[thread],marker='OBS threads mode=serialized admitted=1 count=1');assert 'OBS threads mode=split-check admitted=2 count=1' in out;run(tag+'.disassembly',['objdump','-d',binary])
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources(True),PUBLIC/'tests.c',PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AT_ASSERT:')
    for name,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,definition],[*sources(),PUBLIC/'tests.c',PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,actual_certificates=15,deletion_distinct_repairs=2,scan_slot_checks=2080,free_slot_checks=64,reuse_pairs=32,search_nodes=[2,3],key_predicates=[3,5],serialized_admissions=1,split_admissions=2,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'certificate.c','-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in unresolved.splitlines())<={'algo_avl_init','at_free_contains','at_free_delete','at_free_insert','at_observed_contains','at_observed_delete','at_observed_insert','memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for name,(_,_,definition) in MUTANTS.items():
   tag='thumb.'+name;execute(tag,thumb(tag,['-O2',definition]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True);assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact T36.07 original and changed author tasks; remaining T36/T37 separate.','Actual64-slot baseline and32-slot completed prior struct cd_tree are distinct frozen providers; no capacity/policy identity is silently equated.','Red-black work is an executed certificate-policy comparison, not a new red-black update algorithm.','Thread experiment proves the bounded split-admission counterexample and selected mutex repair; no fairness/progress/all-schedules proof.','Instrumentation context is global and requires one common serialization lock; raw component is not independently thread safe.','Actual Cortex-M4 emulator functional execution is not physical timing.','Source-bound author implementation/execution is not external review, independent seven-day learner evidence or absolute mastery.','Final published C17/C23 text remains a source gap.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
