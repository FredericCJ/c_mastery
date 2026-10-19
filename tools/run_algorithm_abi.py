"""Original paired assembly, changed ABI/callback/stride and real target execution."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_abi';PUBLIC=ROOT/'labs/algorithm_abi';PRIVATE=ROOT/'assessor/algorithm_abi';BUILD=ROOT/'build'/('algorithm-abi-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['search','abi','callback','windows','records','guard','scale','signedness'];STUB_CASES=CASES[:-1]
MUTANTS={'SIGNED':('signedness','algo_asm_lower(single,1,0)==0'),'ABI':('abi','algo_abi_probe(NULL,0,9,&out)==1 && out==0'),'CALLBACK':('callback','ab_callback_lower(a,5,a[i],ab_poison,&context)==trace.index'),'SCALE':('scale','algo_asm_lower(words,2,8)==2')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','assessor/algorithms/algo.c','assessor/algorithms/x86_64.S','assessor/algorithms/thumb.S','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','labs/algorithms/src/x86_64.S','labs/algorithms/src/thumb.S','labs/algorithms/thumb/link.ld','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T36-02.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','readelf','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','fixture','generate','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','oracle','disassembly','stub.compile']+CASES};out|={tag+'.stub.'+n for n in STUB_CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.'+n for n in ['changed.compile','changed.abi','overread.compile','overread.guard','core','core.undefined','kernel','kernel.undefined']}
 for p in ['debug','optimized']:
  out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in ['changed','SIGNED','ABI','CALLBACK','stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 includes=['-I',PUBLIC,'-I',ROOT/'labs/algorithms/include'];strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*includes];generated=BUILD/'generated'
 def compile(name,cc,flags,sources,binary):run(name,[cc,*strict,*flags,*sources,'-lm','-o',binary]);harness.OUTPUTS.add(binary)
 def native(core=None,extension=None,baseline=None,probe=None,oracle=False):return [core or PRIVATE/'bound.c',extension or PRIVATE/'x86.S',baseline or ROOT/'assessor/algorithms/x86_64.S',probe or ROOT/'labs/algorithms/tests/abi_x86_64.S',PUBLIC/'callback_probe_x86.S',PUBLIC/'callback_entry.c',PUBLIC/'poison_x86.S',PUBLIC/'native.c',*([ROOT/'assessor/algorithms/algo.c'] if oracle else [])]
 def thumb(tag,flags=(),stub=False,changed=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');sources=[PUBLIC/'learner.c' if stub else PRIVATE/'bound.c',PUBLIC/'learner_thumb.S' if stub else PRIVATE/'thumb.S',ROOT/('labs/algorithms/src/thumb.S' if stub else 'assessor/algorithms/thumb.S'),generated/'probe_thumb.S' if changed else ROOT/'labs/algorithms/tests/abi_thumb.S',PUBLIC/'callback_probe_thumb.S',PUBLIC/'callback_entry.c',PUBLIC/'poison_thumb.S',PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c']
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*flags,*sources,'-T',ROOT/'labs/algorithms/thumb/link.ld','-Wl,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AB_THUMB_FAIL' if bad else 'AB_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS generated');harness.OUTPUTS.update(generated.iterdir())
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,[*flags,'-DAB_WITH_C_ORACLE'],native(oracle=True),binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.search.log')],marker='"status": "pass"');run(tag+'.disassembly',['objdump','-d',binary])
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,native(core=PUBLIC/'learner.c',extension=PUBLIC/'learner_x86.S',baseline=ROOT/'labs/algorithms/src/x86_64.S'),stub)
    for case in STUB_CASES:run(tag+'.stub.'+case,[stub,case],41,'AB_ASSERT:')
    for name,(case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);definitions=['-DAB_MUTANT_CALLBACK'] if name=='CALLBACK' else ['-DALGO_ASM_'+name] if name in ['SIGNED','ABI'] else []
     compile(tag+'.mutation.'+name+'.compile',cc,[*flags,*definitions],native(baseline=generated/'scale.S' if name=='SCALE' else None),mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,search_cases=5125,certificate_steps=15,validation_comparisons=4,search_comparisons=3,guard_layouts=2,guard_queries_per_layout=2,callback_poisons=2,windows_execution='native-ms_abi-not-Windows-OS',physical=False))
   changed=BUILD/(cc+'.changed');compile(cc+'.changed.compile',cc,['-O2'],native(probe=generated/'probe_x86_64.S'),changed);run(cc+'.changed.abi',[changed,'abi'],marker='OBS abi')
   overread=BUILD/(cc+'.overread');compile(cc+'.overread.compile',cc,['-O2','-DAB_EXPECT_OVERREAD'],native(baseline=generated/'overread.S'),overread);run(cc+'.overread.guard',[overread,'guard'],marker='OBS guard')
   for name,source in [('core',PRIVATE/'bound.c'),('kernel',PRIVATE/'x86.S')]:
    obj=BUILD/(cc+'.'+name+'.o');run(cc+'.'+name,[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',source,'-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.'+name+'.undefined',['nm','-u',obj]);allowed={'algo_asm_lower','ab_records_lower','memcpy','memset'} if name=='core' else set();assert set(x.split()[-1] for x in unresolved.splitlines())<=allowed
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  image=thumb('thumb.changed',['-O2'],changed=True);execute('thumb.changed',image)
  for name in ['SIGNED','ABI','CALLBACK']:
   tag='thumb.'+name;image=thumb(tag,['-O2','-DAB_MUTANT_CALLBACK' if name=='CALLBACK' else '-DALGO_ASM_'+name]);execute(tag,image,True)
  image=thumb('thumb.stub',['-O2'],stub=True);execute('thumb.stub',image,True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact six T36.03/.04 original and changed author tasks; other T36/T37 remain required.','Linux native SysV and Microsoft ms_abi leaf boundary; no Windows loader, unwind, SEH or Windows OS execution.','Actual MPS2 Cortex-M4 functional emulation, two compiler optimizations and generated-code/link closure; no physical timing or board evidence.','Assembly loads are not ASan-instrumented; isolated Linux page guards discriminate two original boundary defects, not all memory errors or arbitrary code execution.','ProfileC kernels/certificates plus audited byte helpers are separate from hosted oracle/guard-page harness and semihost target startup.','Declared source decisions and sortedness checks are separate counts, not measured speedup; trusted-input admission requires immutable independently validated data.','No external review or independent delayed learner execution; no finite tests prove absolute mastery.','Actual pinned ABI passages only; final published C17/C23 comparison remains a source gap.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
