"""Actual typed assembly lookup and prior-prefix composition, both selected ABIs."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_lookup';PUBLIC=ROOT/'labs/algorithm_lookup';PRIVATE=ROOT/'assessor/algorithm_lookup';BUILD=ROOT/'build'/('algorithm-lookup-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['lookup','prior','matching','callbacks','stream']
MUTANTS={'TOMBSTONE':('lookup','p.used==6','-DAL_MUTANT_TOMBSTONE'),'OVERLAP':('matching','w.required==2','-DAL_MUTANT_OVERLAP'),'COUNT':('matching','count==2','-DAL_MUTANT_COUNT'),'CANCEL':('callbacks','status==(stop<6?','-DAL_MUTANT_CANCEL'),'PREFIX':('stream','a+b==used','-DAD_MUTANT=4')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','assessor/containers_depth/indexed.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md','assessor/algorithm_depth/matching.c','fixtures/algorithms/PR-T36-06.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','fixture','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,'lookup.oracle','matching.oracle','stream.oracle']};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.core',cc+'.core.undefined'}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in [*MUTANTS,'stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*sum((['-I',p] for p in [PUBLIC,ROOT/'labs/containers_depth',ROOT/'labs/algorithm_depth']),[])]
 def sources(arch,stub=False):return ([PUBLIC/'learner.c',PUBLIC/('learner_'+arch+'.S')] if stub else [PRIVATE/'lookup.c',PRIVATE/(arch+'.S')])+[ROOT/'assessor/containers_depth/indexed.c',ROOT/'assessor/algorithm_depth/matching.c',PUBLIC/('probe_'+arch+'.S'),PUBLIC/'tests.c']
 def compile(name,cc,flags,src,binary):run(name,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');src=[*sources('thumb',stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c']
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*src,'-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AL_THUMB_FAIL' if bad else 'AL_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"')
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for name,(_,_,definition) in MUTANTS.items():
   tag='thumb.'+name;execute(tag,thumb(tag,['-O2',definition]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources('x86'),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in ['lookup','matching','stream']:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources('x86',True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AL_ASSERT:')
    for name,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,definition],[*sources('x86'),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,whole_span_pairs=945,stream_splits=4815,typed_capacities=16,hash_modes=2,callback_stop_cases=15,empty_boundary_maximum=65,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'lookup.c','-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in unresolved.splitlines())<={'ad_match_init','ad_match_feed','al_lookup_kernel','memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact three original T36.08 tasks and their three changed tasks; remaining T36/T37 separate.','Completed struct cd_hash typed12-byte buckets and struct ad_matcher nonempty/unchanged-count contracts are actually linked; adapters do not silently change providers.','All callback publication is synchronous; accepted effects cannot be rolled back; failure callbacks must reject before publishing.','Binary finite enumeration, compiler checks and emulator runs do not establish all inputs, absolute mastery or external review.','Cortex-M4 QEMU functional evidence is not physical timing or board observation.','Assembly is not instrumented by C sanitizers; explicit trace/canary/register tests and disassembly remain necessary.','Final published C17/C23 source text remains uninspected; selected C17 implementation baseline retained.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
