"""Execute original bit-slice arithmetic and selected native/Thumb branch evidence."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_alu';PUBLIC=ROOT/'labs/architecture_alu';PRIVATE=ROOT/'assessor/architecture_alu';BUILD=ROOT/'build'/('architecture-alu-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['truth','exhaustive','admission','branches'];ORACLES=['truth','exhaustive','branches']
MUTANTS={'WIRES':('exhaustive','r.c_arm==carm && r.cf_x86==cf && r.v==overflow','-DAA_MUTANT_WIRES'),'BORROW':('exhaustive','r.value==value && r.z==(value==0)','-DAA_MUTANT_BORROW'),'ADMISSION':('admission','==AA_INVALID && equal(&r,&old)','-DAA_MUTANT_ADMISSION'),'SIGNED':('branches','aa_relations(&r)==expected','-DAA_MUTANT_SIGNED')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','assessor/algorithms/algo.c','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T37-01.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','objcopy','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-objcopy','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','generate','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[x+'.oracle' for x in ORACLES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.core',cc+'.core.undefined'}
 for arch in ['x86','thumb']:out|={arch+'.encoding.'+n for n in ['compile','binary','disassembly','oracle']}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/algorithms/include']
 def sources(target,stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'alu.c',PUBLIC/('learner_'+target+'.S') if stub else PRIVATE/(target+'.S'),generated/'prior.c',PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-DAA_ARM','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AA_THUMB_FAIL' if bad else 'AA_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version']);run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS actual unchanged prior ALU');harness.OUTPUTS.update(generated.iterdir())
  for arch in ['x86','thumb']:
   prefix='arm-none-eabi-' if arch=='thumb' else '';tag=arch+'.encoding';obj=BUILD/(tag+'.o');raw=BUILD/(tag+'.bin');run(tag+'.compile',[prefix+'gcc',*(['-mcpu=cortex-m4','-mthumb'] if prefix else []),'-c',PRIVATE/(arch+'.S'),'-o',obj]);run(tag+'.binary',[prefix+'objcopy','-O','binary','-j','.text',obj,raw]);run(tag+'.disassembly',[prefix+'objdump','-d',obj]);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',raw,arch],marker='"status": "pass"');harness.OUTPUTS.update([obj,raw])
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2',definition,'-DAA_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources('x86'),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in ORACLES:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources('x86',True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AA_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,definition],[*sources('x86'),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,truth_rows=8,alu_packets=5120,isa_branch_packets=1280,changed_width=5,carry_borrow_inputs=[0,1],native_flags='x86-64',physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'alu.c','-o',obj]);harness.OUTPUTS.add(obj);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact three original T37.02 criteria and unchanged V1 tasks; later T37 projects remain separately required.','Four/five-bit Boolean model is not a physical datapath or commercial processor timing model.','Native x86-64 and emulated Cortex-M4 execute32-bit comparisons on high-aligned finite operands; their carry/borrow conventions are explicitly distinct.','Only five inspected short conditional branch encodings are checked; no general instruction decoder or arbitrary-byte execution.','DUI0553B table3-4 LE entry prints AND; authoritative DDI0403E.e ConditionPassed gives the OR condition used here.','Finite campaign and selected disassembly do not establish external review, delayed learner assessment or physical board observations.','Exact final C17/C23 source text remains uninspected; implementation baseline stays C17.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
