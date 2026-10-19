"""Actual typed interfaces, original assembly and retained container composition."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_wrappers';PUBLIC=ROOT/'labs/algorithm_wrappers';PRIVATE=ROOT/'assessor/algorithm_wrappers';BUILD=ROOT/'build'/('algorithm-wrappers-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['layout','queues','fixture','ownership','backing','contexts','kernel'];ORACLES=['queues','fixture','backing','kernel']
MUTANTS={'PRECEDENCE':('fixture','aw_put(&o,r)==AW_DUPLICATE','-DAW_MUTANT_PRECEDENCE'),'VIEW':('ownership','aw_read(v,&out)==AW_STALE','-DAW_MUTANT_VIEW'),'COPY':('ownership','out.id==7 && out.rank==2','-DAW_MUTANT_COPY'),'ROLLBACK':('backing','s==AW_BACKING && same(before,&o,sizeof o)','-DAW_MUTANT_ROLLBACK')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','assessor/containers_depth/ordered.c','assessor/containers_depth/sequence.c','assessor/containers_depth/transfer.c','labs/api_depth/contracts/OWNERSHIP.md','fixtures/algorithms/PR-T36-08.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[x+'.oracle' for x in ORACLES]]};out|={tag+'.stub.'+n for n in CASES if n!='queues'}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.core',cc+'.core.undefined',cc+'.core.costs',cc+'.core.disassembly'}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/containers_depth']
 def sources(target,stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'wrappers.c',PUBLIC/('learner_'+target+'.S') if stub else PRIVATE/(target+'.S'),PUBLIC/('probe_'+target+'.S'),*[ROOT/'assessor/containers_depth'/n for n in ['ordered.c','sequence.c','transfer.c']],PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AW_THUMB_FAIL' if bad else 'AW_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2',definition,'-DAW_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources('x86'),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in ORACLES:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources('x86',True),PUBLIC/'native.c'],stub)
    for case in CASES:
     if case!='queues':run(tag+'.stub.'+case,[stub,case],41,'AW_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,definition],[*sources('x86'),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,kernel_queries=7560,queue_states=7,free_slot_inspections=7,backing_failure_paths=10,callback_failure_positions=9,fixture_combinations=4,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-fno-inline','-ffreestanding','-fno-builtin','-c',PRIVATE/'wrappers.c','-o',obj]);harness.OUTPUTS.add(obj);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'aw_lower','cd_tree_init','cd_tree_find','cd_tree_insert','cd_tree_delete','memcpy','memset','_GLOBAL_OFFSET_TABLE_'};sizes=run(cc+'.core.costs',['nm','-S',obj]);assert 'aw_finite_compare' in sizes and 'aw_callback_compare' in sizes;run(cc+'.core.disassembly',['objdump','-d',obj])
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Six exact original T36.02/T36.10 criteria and unchanged V1 tasks; no whole T37 closure.','Actual T14 tree/ring/slot queue are imported unchanged; ID membership index and rank-ordered view are distinct layers.','All caller/control/context and borrowed object lifetimes remain valid; mutation diagnostics do not detect dangling pointers or concurrent races.','Callbacks lease preallocated typed banks; arbitrary malicious storage or hidden callback effects are outside the admission guarantee.','Finite dispatch cost records describe these selected objects, flags and equivalent bounded semantics, not a universal zero-cost guarantee.','Cortex-M4 QEMU functionality is not physical timing or board evidence; external review and delayed learner assessment remain unperformed.','Exact final C17/C23 text remains uninspected; C17 implementation baseline retained.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
