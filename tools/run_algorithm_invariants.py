"""Exact invariant, source-instrumentation and producer-reuse campaign."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_invariants';PUBLIC=ROOT/'labs/algorithm_invariants';PRIVATE=ROOT/'assessor/algorithm_invariants';BUILD=ROOT/'build'/('algorithm-invariants-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['euclid','growth','permutation','queues']
IMPORTS=['tools/run_thread_lifecycle.py','labs/containers_depth/containers.h','labs/containers_depth/CONTRACTS.md','labs/containers_depth/byte_helpers.c','assessor/containers_depth/sequence.c','assessor/containers_depth/transfer.c','fixtures/algorithms/PR-T36-01.json']
MUTANTS={'ZERO':('euclid','ai_step(state,0,&next)==AI_DONE'),'SET':('permutation','ai_permutation(tagged,corrupt,3,&out)==AI_OK && !out'),'GROWTH':('growth','d.copies==7 && d.writes==15')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','readelf']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','fixture','instrument'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','instrumented.compile','instrumented.queues','instrumented.oracle','stub.compile']+CASES};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
  out|={cc+'.strict.'+n for n in ['core','undefined','disassembly','probe','helpers','entry','link','final.undefined','headers','execute']}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 includes=['-I',PUBLIC,'-I',ROOT/'labs/containers_depth'];strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*includes];providers=[ROOT/'assessor/containers_depth/sequence.c',ROOT/'assessor/containers_depth/transfer.c'];core=[PRIVATE/'invariants.c',PRIVATE/'experiments.c'];instrumented=BUILD/'instrumented.c'
 def compile(name,cc,flags,sources,binary):run(name,[cc,*strict,*flags,*sources,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"');run('instrument',[sys.executable,PRIVATE/'instrument.py',instrumented],marker='OBS generated');harness.OUTPUTS.add(instrumented)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*core,*providers,PUBLIC/'tests.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    binary_i=BUILD/(tag+'.instrumented');compile(tag+'.instrumented.compile',cc,[*flags,'-DAI_INSTRUMENTED'],[*core,instrumented,PUBLIC/'tests.c'],binary_i);run(tag+'.instrumented.queues',[binary_i,'queues'],marker='OBS queues');raw=BUILD/(tag+'.instrumented.queues.log');run(tag+'.instrumented.oracle',[sys.executable,PRIVATE/'oracle.py',raw],marker='"status": "pass"')
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[PUBLIC/'learner.c',*providers,PUBLIC/'tests.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'AI_ASSERT:')
    for name,(case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,'-DAI_MUTANT_'+name],[*core,*providers,PUBLIC/'tests.c'],mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=tag,cases=CASES,euclid_cases=8190,modulo_steps=3,subtraction_steps=11,maximum_subtractions=4094,doubling_writes=15,additive_writes=36,growth_counts=list(range(33)),queue_events=7,ring_decisions=99,linked_decisions=28,transfers_each=6,node_probes=7,links=2,original_providers=True,source_decisions_are_instructions=False,physical=False))
   flags=['-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie'];objects=[]
   for name,source in [('core',PRIVATE/'invariants.c'),('probe',PUBLIC/'strict.c'),('helpers',ROOT/'labs/containers_depth/byte_helpers.c')]:
    obj=BUILD/(cc+'.'+name+'.o');run(cc+'.strict.'+name,[cc,*strict,*flags,'-c',source,'-o',obj]);objects.append(obj);harness.OUTPUTS.add(obj)
   unresolved=run(cc+'.strict.undefined',['nm','-u',objects[0]]);assert set(line.split()[-1] for line in unresolved.splitlines())<={'memset','memcpy'};run(cc+'.strict.disassembly',['objdump','-dr',objects[0]])
   entry=BUILD/(cc+'.entry.o');run(cc+'.strict.entry',[cc,'-c',PUBLIC/'strict_entry.S','-o',entry]);objects.append(entry);harness.OUTPUTS.add(entry)
   image=BUILD/(cc+'.strict.elf');mapping=BUILD/(cc+'.strict.map');run(cc+'.strict.link',[cc,'-nostdlib','-static','-no-pie','-Wl,--no-undefined,-Map,'+str(mapping),*objects,'-o',image]);harness.OUTPUTS.update([image,mapping]);assert not run(cc+'.strict.final.undefined',['nm','-u',image]).strip();headers=run(cc+'.strict.headers',['readelf','-lW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers;run(cc+'.strict.execute',[image])
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Exact three T36.01 ordinary/changed author tasks; all other T36/T37 work remains separately required.','Actual frozen container reuse and generated source-preserving instrumentation; source-defined decisions/transfers/probes are not total instructions, timing or hardware traffic.','ProfileC step/permutation object and native no-CRT image closure are distinct from hosted malloc/free growth adapter; no target/physical claim.','No independent learner delayed performance, external review or absolute mastery.','ODS C++ mechanism comparison and bounded Dijkstra induction passage; final published C17/C23 remains a source gap.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
