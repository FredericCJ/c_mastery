"""Actual assembly sink integration, complete-swap observers and changed heap contracts."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_heap';PUBLIC=ROOT/'labs/algorithm_heap';PRIVATE=ROOT/'assessor/algorithm_heap';BUILD=ROOT/'build'/('algorithm-heap-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['premise','scalar','observed','prior','minimum','ternary','binary']
MUTANTS={'PREMISE':('premise','ah_trace(a,5,5,1,2,0,&s,&trace)==1','-DAH_MUTANT_PREMISE'),'INDEX':('prior','equivalent(&x,&y)&&valid(&y)','-DAH_MUTANT_INDEX'),'DUPLICATE':('ternary','certificate(before,a,n,p,arity,minimum,&s,&trace)','-DAH_MUTANT_DUPLICATE'),'COUNTS':('observed','ah_stats.comparisons==counts[f]','-DAH_MUTANT_COUNTS')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','assessor/algorithms/x86_64.S','assessor/algorithms/thumb.S','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','labs/algorithms/src/x86_64.S','labs/algorithms/src/thumb.S','labs/algorithms/adapters/checked_asm.c','labs/containers_depth/byte_helpers.c','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','assessor/containers_depth/ordered.c','fixtures/algorithms/PR-T36-04.json']
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
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','trace.oracle','disassembly','stub.compile']+CASES};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.'+n for n in ['core','core.undefined','kernel','kernel.undefined']}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in [*MUTANTS,'stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*sum((['-I',p] for p in [PUBLIC,ROOT/'labs/algorithms/include',ROOT/'labs/containers_depth']),[])];provider=ROOT/'labs/algorithms'
 def sources(arch,stub):
  original='x86_64' if arch=='x86' else 'thumb';common=[PUBLIC/'suite.c',PUBLIC/'instrument.c',PUBLIC/('probe_'+arch+'.S'),provider/'adapters/checked_asm.c',provider/('tests/abi_'+original+'.S')]
  return common+([PUBLIC/'learner.c',PUBLIC/('learner_'+arch+'.S'),provider/('src/'+original+'.S')] if stub else [PRIVATE/'trace.c',PRIVATE/(arch+'.S'),generated/'heap_adapter.c',generated/('instrumented_'+original+'.S'),ROOT/('assessor/algorithms/'+original+'.S'),ROOT/'assessor/containers_depth/ordered.c'])
 def native(stub=False):return [*sources('x86',stub),PUBLIC/'native.c',PUBLIC/'trace_x86.S']
 def compile(name,cc,flags,src,binary):run(name,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');src=[*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c']
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*flags,*src,'-T',PUBLIC/'link.ld','-Wl,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AH_THUMB_FAIL' if bad else 'AH_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS generated');harness.OUTPUTS.update(generated.iterdir())
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,native(),binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    run(tag+'.trace.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.observed.log')],marker='"status": "pass"');run(tag+'.disassembly',['objdump','-d',binary])
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,native(True),stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AH_ASSERT:')
    for name,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,definition],native(),mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,fixture_comparisons=[3,4,2],complete_swap_snapshots=3,prior_build_cells=136,prior_updates_per_nonempty_cell=3,retirement_rounds_per_mode=3,exhaustive_alphabet=3,exhaustive_max_length=6,maximum_scalar_length=1024,metric_is_machine_instructions=False,physical=False))
   for name,source in [('core',PRIVATE/'trace.c'),('kernel',PRIVATE/'x86.S')]:
    obj=BUILD/(cc+'.'+name+'.o');run(cc+'.'+name,[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',source,'-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.'+name+'.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in unresolved.splitlines())<=({'memcpy','memset'} if name=='core' else set())
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for name,(_,_,definition) in MUTANTS.items():
   tag='thumb.'+name;image=thumb(tag,['-O2',definition]);execute(tag,image,True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact three T36.06 original and changed author tasks; other T36/T37 remain required.','The actual frozen struct cd_heap API and algorithms are reused; only sink is replaced. Its signed priority/sequence order is distinct from unsigned scalar child-left ties.','Native SysV and MPS2 Cortex-M4 functional execution, no physical timing or board claim.','Observed key comparisons/swaps exclude admission scans, snapshot copies, handle maintenance and observer ABI overhead; they are not total instructions or speedup.','C sanitizer execution does not automatically instrument handwritten assembly; finite state, ABI and mutant evidence is bounded.','No external review or independent seven-day learner execution; no absolute mastery.','Pinned source passages only; exact final C17/C23 remains a gap.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
