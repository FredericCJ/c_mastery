"""Run cache/shadow certificates, exact dimensional arithmetic and overlap."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_cache';PUBLIC=ROOT/'labs/architecture_cache';PRIVATE=ROOT/'assessor/architecture_cache';BUILD=ROOT/'build'/('architecture-cache-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['ordinary','layouts','geometry','costs','overlap'];PACKETS=dict(ordinary=5,layouts=4,geometry=60,costs=3,overlap=12)
MUTANTS={'VALID':('ordinary','r.steps[0].category==ACH_COMPULSORY','ACH_MUTANT_VALID'),'WAYS':('ordinary','r.hits==5 && !r.conflict','ACH_MUTANT_WAYS'),'SHADOW':('ordinary','r.steps[5].category==ACH_CAPACITY','ACH_MUTANT_SHADOW'),'FIFO':('ordinary','r.steps[6].category==ACH_CAPACITY','ACH_MUTANT_FIFO'),'OVERLAP':('overlap','r.makespan==41','ACH_MUTANT_OVERLAP'),'UNITS':('costs','r.time_ns.numerator==120','ACH_MUTANT_UNITS'),'ADDITIONAL':('costs','ach_cost(x,&r)==ACH_OK && r.amat_ns.numerator==21 && r.amat_ns.denominator==1','ACH_MUTANT_ADDITIONAL')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/algorithms/include/algo.h','labs/algorithms/SPEC.md','assessor/algorithms/algo.c','labs/containers_depth/byte_helpers.c','fixtures/algorithms/PR-T37-05.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','generate'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.resource',cc+'.core.sizes'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC,'-I',ROOT/'labs/algorithms/include']
 def sources(stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'cache.c',PUBLIC/'prerequisite_stub.c' if stub else generated/'prior.c',PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources(stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'ACH_THUMB_FAIL' if bad else 'ACH_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version']);run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS actual unchanged algo_cache');harness.OUTPUTS.update(generated.iterdir())
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2','-D'+definition,'-DACH_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources(),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources(True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'ACH_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-D'+definition],[*sources(),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,independent_packets=PACKETS,original_hits=3,two_way_hits=5,lru_last_category='conflict',fifo_last_category='capacity',independent_overlap_cycles=[80,41,40,23],chain_cycles=80,maximum_chain_cycles=4352,physical=False))
   obj=BUILD/(cc+'.core.o');stack=obj.with_suffix('.su');run(cc+'.core',[cc,*strict,'-O2','-fstack-usage','-ffreestanding','-fno-builtin','-c',PRIVATE/'cache.c','-o',obj]);harness.OUTPUTS.update([obj,stack]);u=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in u.splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'};run(cc+'.core.resource',[sys.executable,PRIVATE/'oracle.py',stack,'resource'],marker='"status": "pass"');run(cc+'.core.sizes',['nm','-S','--size-sort',obj])
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact three PR-T37-05 criterion/V1 mechanisms; subsequent T37 projects remain separately required.','Actual unchanged algo_cache selected body is reused with source/body provenance for its compatible direct-map/LRU aggregate contract; unrelated family mechanisms are not revalidated.','Target/set geometry, replacement and service timing are explicit bounded pedagogical models, not descriptions of a commercial cache or processor.','Fraction arithmetic can reject intermediate representation overflow conservatively; no floating-point accuracy or arbitrary-precision claim.','FIFO shadow changes the diagnostic reference model, not target cache behavior; summed miss service work is distinct from overlapped elapsed model ticks.','Dedicated M4 execution verifies the C model and final link closure, not physical cache/timing, independent review or delayed learner evidence.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
