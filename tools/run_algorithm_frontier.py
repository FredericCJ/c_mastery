"""Execute source-derived queue/heap refinement with actual frozen providers."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_frontier';PUBLIC=ROOT/'labs/algorithm_frontier';PRIVATE=ROOT/'assessor/algorithm_frontier';BUILD=ROOT/'build'/('algorithm-frontier-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['fixture','boundaries','parallel','certificates','exhaustive']
MUTANTS={'RANGE':('boundaries','==AG_RANGE','-DAG_MUTANT_RANGE'),'TIE':('boundaries','s->priority[i]==(int)','-DAG_MUTANT_TIE'),'UPDATE':('fixture','s->priority[i]==(int)','-DAG_MUTANT_UPDATE'),'INEQUALITY':('certificates','ag_certificate(&g,0,&c)==AG_BAD_CERTIFICATE','-DAG_MUTANT_INEQUALITY')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','assessor/containers_depth/indexed.c','assessor/containers_depth/ordered.c','assessor/containers_depth/sequence.c','labs/algorithm_depth/algorithms.h','labs/algorithm_depth/CONTRACTS.md','assessor/algorithm_depth/graph_dp.c','fixtures/algorithms/PR-T36-07.json']
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
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[case+'.oracle' for case in ['fixture','boundaries','parallel','exhaustive']]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.core',cc+'.core.undefined'}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for variant in [*MUTANTS,'stub']:out|={'thumb.'+variant+'.compile','thumb.'+variant+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*sum((['-I',p] for p in [PUBLIC,ROOT/'labs/containers_depth',ROOT/'labs/algorithm_depth']),[])]
 def sources(stub=False):return ([PUBLIC/'learner.c'] if stub else [PRIVATE/'frontier.c',generated/'refined.c'])+[ROOT/'assessor/containers_depth'/n for n in ['indexed.c','ordered.c','sequence.c']]+[ROOT/'assessor/algorithm_depth/graph_dp.c',PUBLIC/'tests.c']
 def compile(name,cc,flags,src,binary):run(name,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');src=[*sources(stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c']
  run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*src,'-T',PUBLIC/'link.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'AG_THUMB_FAIL' if bad else 'AG_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS generated');harness.OUTPUTS.update(generated.iterdir())
  run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for profile,optimization in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+profile;image=thumb(tag,[optimization]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();headers=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for name,(_,_,definition) in MUTANTS.items():
   tag='thumb.'+name;execute(tag,thumb(tag,['-O2',definition]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources(),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for case in ['fixture','boundaries','parallel','exhaustive']:run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources(True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AG_ASSERT:')
    for name,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,definition],[*sources(),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,enumerated_graphs=729,sources_per_graph=3,actual_packets=2193,fixture_heap_updates=2,fixture_relaxations=5,parallel_heap_updates=2,finite_limit=65534,negative_witness_distance=-1,physical=False))
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-c',PRIVATE/'frontier.c','-o',obj]);harness.OUTPUTS.add(obj);unresolved=run(cc+'.core.undefined',['nm','-u',obj]);assert set(x.split()[-1] for x in unresolved.splitlines())<={'ag_refined_bfs','ag_refined_dijkstra','cd_csr_build','cd_ring_read','memcpy','memset','_GLOBAL_OFFSET_TABLE_'}
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact three original T36.09 tasks and changed V1 tasks; no whole T36/T37 closure.','Actual T15 BFS/Dijkstra bodies are source-derived at frontier boundaries and compose frozen T14 ring/heap/CSR; original scan/array implementations remain separately linked comparisons.','Signed negative-edge transfer is an executed certificate-policy and independent simple-path investigation, not a new general negative-edge target solver.','All simple-path enumeration is restricted to tiny synthetic graphs; it is not the target algorithm or proof for all graphs.','Finite edge arithmetic is checked before publishing results/traces; caller workspace is scratch and may change after admission.','Cortex-M4 QEMU functionality is not physical timing, board evidence, external review or delayed learner assessment.','Exact final C17/C23 source text remains uninspected; selected C17 implementation baseline retained.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
