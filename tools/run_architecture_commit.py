"""Compose the frozen clock/ISA producers with new explicit pending ownership."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_commit';PUBLIC=ROOT/'labs/architecture_commit';PRIVATE=ROOT/'assessor/architecture_commit';BUILD=ROOT/'build'/('architecture-commit-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['baseline','clocked','input','status','load','speculation'];PACKETS=dict(baseline=12,clocked=12,input=1,status=767,load=8,speculation=36)
MUTANTS={'ALIAS':('baseline','m.reg[0]==6','SC_MUTATE_CLOCK'),'ATOMIC':('baseline','at_same(&m,&old) && equal(&control,&old_control','SC_MUTATE_CLOCK2'),'INPUT':('input','m.state.pc==4 && !trace.wires.branch_taken','AT_MUTANT_INPUT'),'ZERO':('status','m.zero==(a==b)','AT_MUTANT_ZERO'),'CANCEL':('load','(cancel?AT_CANCELLED:AT_OK)','AT_MUTANT_CANCEL'),'SQUASH':('speculation','expected && at_same(&m.state,&branch)','AT_MUTANT_SQUASH')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/systems_certificates/include/systems_certificates.h','labs/systems_certificates/CONTRACTS.md','labs/systems_certificates/src/systems_certificates.c','assessor/systems_certificates/systems_certificates.c','labs/architecture_alu/alu.h','labs/architecture_alu/learner_x86.S','labs/architecture_alu/learner_thumb.S','assessor/architecture_alu/x86.S','assessor/architecture_alu/thumb.S','labs/algorithms/include/algo.h','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T37-03.json','curriculum/v5/exercises/X-T37-002.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','evidence.oracle'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/systems_certificates/include','-I',ROOT/'labs/architecture_alu','-I',ROOT/'labs/algorithms/include']
 def sources(arch,stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'commit.c',ROOT/('labs/systems_certificates/src/systems_certificates.c' if stub else 'assessor/systems_certificates/systems_certificates.c'),ROOT/('labs/architecture_alu/learner_'+arch+'.S' if stub else 'assessor/architecture_alu/'+arch+'.S'),PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AT_THUMB_FAIL' if bad else 'AT_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version']);run('evidence.oracle',[sys.executable,PRIVATE/'oracle.py',PUBLIC/'evidence.json','evidence'],marker='"status": "pass"')
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2','-D'+definition,'-DAT_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources('x86'),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources('x86',True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AT_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-D'+definition],[*sources('x86'),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,independent_packets=PACKETS,actual_isa_zero_comparisons=65536,ordinary_commits=11,multicycle_ticks=12,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'commit.c','-o',obj]);harness.OUTPUTS.add(obj);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'sc_clock','memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact six original PR-T37-03 criteria and V1 tasks; original X-T37-002 V1/V2 mechanisms are implemented by this successor without rewriting its historical contract.','Original sc_clock/sc_run and native/Thumb compare providers are directly compiled unchanged, not replaced by matching signatures.','Toy multicycle and speculation are original deterministic mechanisms, not commercial microarchitecture, hardware timing or weak-memory models.','Documented zero flag relation is compared with actual native/emulated CMP; the toy extension updates it only on SUB by its explicit policy.','The contradictory board packet is explicitly synthetic/counterfactual, not invented physical evidence; its investigation preserves valid bounded-model results.','External review, seven-day independent learner performance, physical execution and exact final C17/C23 text remain unclaimed.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
