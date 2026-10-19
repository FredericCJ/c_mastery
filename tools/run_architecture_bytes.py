"""Actual register, selected decoder, typed storage and dependency experiments."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_bytes';PUBLIC=ROOT/'labs/architecture_bytes';PRIVATE=ROOT/'assessor/architecture_bytes';BUILD=ROOT/'build'/('architecture-bytes-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['registers','bytes','storage','decode','branches']
MUTANTS={'PARTIAL':('registers','v==want[i]'),'ENDIAN':('bytes','word==0x12345678'),'EXTENT':('bytes','==AB_RANGE && start==0x1014'),'PC':('branches','out.target==pc+2+delta'),'EXTENSION':('decode','ab_x86(ext,2,0,0,&out)==AB_UNSUPPORTED')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithm_wrappers/wrappers.h','labs/containers_depth/containers.h','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T37-02.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','objcopy','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-objcopy','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','encoding.oracle','placement.compile','placement.symbols','placement.oracle'}
 for arch in ['att','intel','wrong','thumb']:out|={arch+'.encoding.'+n for n in ['compile','binary','disassembly']}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.bad_cast.compile',cc+'.core',cc+'.core.undefined'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile','allocated','timing','timing.oracle',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in [*CASES,'allocated']}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/algorithm_wrappers','-I',ROOT/'labs/containers_depth']
 def sources(arch,stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'bytes.c',PUBLIC/('learner_'+arch+'.S') if stub else PRIVATE/(arch+'.S'),PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-DAB_ARM','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AB_THUMB_FAIL' if bad else 'AB_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for arch in ['att','intel','wrong','thumb']:
   prefix='arm-none-eabi-' if arch=='thumb' else '';tag=arch+'.encoding';obj=BUILD/(arch+'.o');raw=BUILD/(arch+'.bin');run(tag+'.compile',[prefix+'gcc',*(['-mcpu=cortex-m4','-mthumb'] if prefix else []),'-c',PUBLIC/('encoding_'+arch+'.S'),'-o',obj]);run(tag+'.binary',[prefix+'objcopy','-O','binary','-j','.text',obj,raw]);run(tag+'.disassembly',[prefix+'objdump','-d',obj]);harness.OUTPUTS.update([obj,raw])
  run('encoding.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD,'encoding'],marker='"status": "pass"')
  image=BUILD/'placement.elf';mapping=BUILD/'placement.map';run('placement.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-nostdlib',PUBLIC/'placement.c','-T',PUBLIC/'placement.ld','-Wl,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);run('placement.symbols',['arm-none-eabi-nm','-S','--defined-only',image]);run('placement.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/'placement.symbols.log','placement'],marker='"status": "pass"')
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2','-DAB_MUTANT_'+m,'-DAB_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version']);obj=BUILD/(cc+'.bad_cast.o');run(cc+'.bad_cast.compile',[cc,*strict,'-O2','-c',PUBLIC/'bad_cast.c','-o',obj]);harness.OUTPUTS.add(obj)
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources('x86'),PUBLIC/'native.c'],binary)
    for case in [*CASES,'allocated','timing']:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in [*CASES,'timing']:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources('x86',True),PUBLIC/'native.c'],stub)
    for case in [*CASES,'allocated']:run(tag+'.stub.'+case,[stub,case],41,'AB_ASSERT:')
    for m,(case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-DAB_MUTANT_'+m],[*sources('x86'),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,address_packets=5472,storage_packets=50,thumb_halfwords=65536,thumb_cmp_packets=2048,branch_packets=7936,timing_samples=124,actual_allocated_backing=True,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'bytes.c','-o',obj]);harness.OUTPUTS.add(obj);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Six original T37.03/.04 criteria and unchanged V1 tasks; later T37 projects remain required.','Native x86-64 and emulated M4 selected instructions execute; no physical M4 or F439 DMA observation.','Original -1 accumulator bytes use explicit assembly directives because GNU mnemonic assembly selects an imm8 form; syntax-equivalent mnemonic snippets and wrong-width control are checked separately.','Bounded offline decoder rejects all other encodings; admitted PC arithmetic rejects address wrap rather than modeling all ISA modes.','Partial/full register timing retains all124 samples/profile, CPU migration and code-size/scheduling confounds; no universal latency or causation claim.','Compile-only bad_cast is never executed; compiler acceptance and aligned hardware access do not establish effective-type permission.','CCM/SRAM actual linked typed objects establish placement only; DMA configuration/ownership and physical transfer remain outside this variant.','Exact final C17/C23 source text, external review and seven-day independent learner evidence remain unclaimed.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
