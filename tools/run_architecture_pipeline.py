"""Execute bounded in-order certificates and renamed out-of-order issue."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_pipeline';PUBLIC=ROOT/'labs/architecture_pipeline';PRIVATE=ROOT/'assessor/architecture_pipeline';BUILD=ROOT/'build'/('architecture-pipeline-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['schedule','matrix','boundaries','ooo'];PACKETS=dict(schedule=6,matrix=33,boundaries=1,ooo=16)
MUTANTS={'LOAD':('schedule','s.cycles==13','AP_MUTANT_LOAD'),'SQUASH':('schedule','ap_schedule(program,7,c,&s)==AP_OK','AP_MUTANT_SQUASH'),'READ_ORDER':('schedule','s.cycles==17','AP_MUTANT_READ_ORDER'),'PORT':('matrix','s.port_stalls==1 && s.cycles==9','AP_MUTANT_PORT'),'TAG':('ooo','equal(r.registers,sequential,sizeof(sequential))','AP_MUTANT_TAG'),'PRIOR':('schedule','cycles==s.cycles','S_MUTATE_PIPELINE')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/b045_systems/include/systems.h','labs/b045_systems/CONTRACTS.md','assessor/b045_systems/systems.c','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T37-04.json','curriculum/v5/exercises/X-T37-001.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','distinctions.oracle','generate'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.cost',cc+'.core.sizes',cc+'.race.compile'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/b045_systems/include']
 def sources(stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'pipeline.c',PUBLIC/'prerequisite_stub.c' if stub else generated/'prior.c',PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources(stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AP_THUMB_FAIL' if bad else 'AP_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version']);run('distinctions.oracle',[sys.executable,PRIVATE/'oracle.py',PUBLIC/'distinctions.json','distinctions'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS actual unchanged s_pipeline');harness.OUTPUTS.update(generated.iterdir())
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2','-D'+definition,'-DAP_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version']);obj=BUILD/(cc+'.race.o');run(cc+'.race.compile',[cc,*strict,'-O2','-c',PUBLIC/'race_examples.c','-o',obj]);harness.OUTPUTS.add(obj)
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources(),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources(True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AP_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-D'+definition],[*sources(),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,independent_packets=PACKETS,default_cycles=13,default_retired=6,default_flushed=2,two_cycle_memory_cycles=14,id_branch_cycles=14,read_before_write_four_cycles=17,physical=False))
   obj=BUILD/(cc+'.core.o');stack=obj.with_suffix('.su');run(cc+'.core',[cc,*strict,'-O2','-fstack-usage','-ffreestanding','-fno-builtin','-c',PRIVATE/'pipeline.c','-o',obj]);harness.OUTPUTS.update([obj,stack]);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'};run(cc+'.core.cost',[sys.executable,PRIVATE/'oracle.py',stack,'cost'],marker='"status": "pass"');run(cc+'.core.sizes',['nm','-S','--size-sort',obj])
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact three PR-T37-04 criteria/V1 tasks plus original X-T37-001 write/read-order and structural-port variants; next T37 projects remain required.','The original s_pipeline body is extracted byte-identically with provenance and used only for compatible retired-stream cycle comparison; it cannot represent fetched-instance squash.','In-order model has forward-only declared branches, fixed stage rules, one/two-cycle load stage and bounded128-cycle/64-instance trace; out-of-order variant is separate straight-line renamed issue with in-order retirement.','Executing the model on host or M4 emulator does not measure either real processor pipeline.','Data-race source is compile/analysis-only, never run; atomic counter example is a bounded semantic repair, not a new concurrent execution claim.','External review, independent learner delay, physical observation and exact final C17/C23 text remain unclaimed.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
