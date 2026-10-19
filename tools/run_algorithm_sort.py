"""Actual original assembly sorting, source observers, prior T15 reuse and variants."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_sort';PUBLIC=ROOT/'labs/algorithm_sort';PRIVATE=ROOT/'assessor/algorithm_sort';BUILD=ROOT/'build'/('algorithm-sort-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['insertion','counts','descending','wide','failure','prior']
MUTANTS={'UNSTABLE':('insertion','algo_checked_insert(a,n,n)==ALGO_OK && stable(before,a,n,0)','-DALGO_ASM_UNSTABLE'),'WIDE':('wide','a[rank].key==before[i].key && a[rank].tag==before[i].tag','-DAS_MUTANT_WIDE'),'FAILURE':('failure','as_fallible(a,5,5,compare,&c,&work)==2 && same(a,fixture,5)','-DAS_MUTANT_FAILURE'),'COUNTS':('counts','as_stats.comparisons==comparisons','-DAS_MUTANT_COUNTS')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','assessor/algorithms/x86_64.S','assessor/algorithms/thumb.S','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','labs/algorithms/src/x86_64.S','labs/algorithms/src/thumb.S','labs/algorithms/thumb/link.ld','labs/algorithms/adapters/checked_asm.c','labs/containers_depth/byte_helpers.c','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','assessor/containers_depth/ordered.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md','assessor/algorithm_depth/sorting.c','fixtures/algorithms/PR-T36-03.json']
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
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','prefix.oracle','metric.oracle','disassembly','stub.compile']+CASES};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.'+n for n in ['core','core.undefined','kernel','kernel.undefined']}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in [*MUTANTS,'stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*sum((['-I',p] for p in [PUBLIC,ROOT/'labs/algorithms/include',ROOT/'labs/algorithm_depth',ROOT/'labs/containers_depth']),[])];provider=ROOT/'labs/algorithms'
 def native(stub=False):return [PUBLIC/'suite.c',PUBLIC/'native.c',PUBLIC/'instrument.c',PUBLIC/'trace_x86.S',PUBLIC/'wide_probe_x86.S',provider/'adapters/checked_asm.c',provider/'tests/abi_x86_64.S',*([PUBLIC/'learner.c',PUBLIC/'learner_x86.S',provider/'src/x86_64.S'] if stub else [PRIVATE/'fallible.c',PRIVATE/'adapter.c',PRIVATE/'wide_x86.S',generated/'instrumented_x86_64.S',generated/'descending_x86_64.S',ROOT/'assessor/algorithms/x86_64.S',ROOT/'assessor/algorithm_depth/sorting.c',ROOT/'assessor/containers_depth/ordered.c'])]
 def compile(name,cc,flags,sources,binary):run(name,[cc,*strict,*flags,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');sources=[PUBLIC/'suite.c',PUBLIC/'thumb_start.c',PUBLIC/'instrument.c',PUBLIC/'wide_probe_thumb.S',provider/'adapters/checked_asm.c',provider/'tests/abi_thumb.S',ROOT/'labs/containers_depth/byte_helpers.c',*([PUBLIC/'learner.c',PUBLIC/'learner_thumb.S',provider/'src/thumb.S'] if stub else [PRIVATE/'fallible.c',PRIVATE/'wide_thumb.S',generated/'instrumented_thumb.S',generated/'descending_thumb.S',ROOT/'assessor/algorithms/thumb.S'])]
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-DAS_NO_PRIOR','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*flags,*sources,'-T',PUBLIC/'link.ld','-Wl,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AS_THUMB_FAIL' if bad else 'AS_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS generated');harness.OUTPUTS.update(generated.iterdir())
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,native(),binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    run(tag+'.prefix.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.insertion.log'),'prefix'],marker='"status": "pass"');run(tag+'.metric.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.counts.log'),'metric'],marker='"status": "pass"');run(tag+'.disassembly',['objdump','-d',binary])
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,native(True),stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AS_ASSERT:')
    for name,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,definition],native(),mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,insertion_inputs=132,metric_rows=132,descending_inputs=132,wide_lengths=33,prior_sort_cells=204,prior_merge_checkpoint=1,callback_failure_positions=18,fixture_comparisons=9,fixture_shifts=6,fixture_placements=4,prefix_snapshots=4,metric_is_machine_instructions=False,physical=False))
   for name,source in [('core',PRIVATE/'fallible.c'),('kernel',PRIVATE/'wide_x86.S')]:
    obj=BUILD/(cc+'.'+name+'.o');run(cc+'.'+name,[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',source,'-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.'+name+'.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in unresolved.splitlines())<=({'memcpy','memset'} if name=='core' else set())
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for name,(_,_,definition) in MUTANTS.items():
   tag='thumb.'+name;image=thumb(tag,['-O2',definition]);execute(tag,image,True)
  image=thumb('thumb.stub',['-O2'],True);execute('thumb.stub',image,True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Three exact T36.05 original and changed author tasks; other T36/T37 remain required.','Actual unchanged prior T15 insertion/merge/heap providers through explicit signed-order mapping on their common n<=16 domain; new assembly admits n<=32 separately.','Actual SysV and MPS2 Cortex-M4 functional execution, no physical timing or board claim.','Assembly-derived observer snapshots/counters measure declared key comparisons/shift/placement events; observer copies and callback ABI overhead are separate, not total instructions or speedup.','Failure rollback covers staged array output; comparator-owned context effects cannot be undone and comparator order consistency remains a caller obligation.','Assembly accesses are not automatically C-sanitizer-instrumented; finite ABI/mutation evidence does not prove all inputs.','No external review or independent seven-day learner execution; no absolute mastery.','Pinned source passages only; exact final C17/C23 remains a gap.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
