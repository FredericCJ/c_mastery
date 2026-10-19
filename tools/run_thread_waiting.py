"""Actual nine original predicate/primitive mechanisms and changed conditions."""
import hashlib,json,pathlib,re
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_waiting';BUILD=ROOT/'build/thread-waiting'
PUBLIC=ROOT/'labs/thread_depth/waiting';PRIVATE=ROOT/'assessor/thread_depth/waiting'
CASES=['bank','order','lifecycle','queue','wake','deadline','barrier','fairness','permit']
MARKERS=['e == EBUSY','tw_order(11, 12)','refs == 2','out == 1','q.timed_waits >= 2','ETIMEDOUT && n == 77','p[0].result == ECANCELED','reader_entries == 0','tw_select(i) == (int)i']
OLD=['assessor/hosted_systems/queue.c','assessor/hosted_systems/common.h','labs/hosted_systems/include/hosted.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'labs/thread_depth/lifecycle/race_control.c',*[ROOT/p for p in OLD]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def main():
 BUILD.mkdir(parents=True,exist_ok=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();failure=None;observations=[]
 harness.BUILD=BUILD;harness.PUBLIC=PUBLIC;harness.PRIVATE=PRIVATE;run=harness.run;compile=harness.compile
 tests=sorted(PUBLIC.glob('*tests.c'));sources=[PRIVATE/(s+'.c') for s in ['transactions','queue','primitives']]
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
    tag=cc+'.'+profile;binary=BUILD/tag
    compile(cc,flags,tag+'.compile',sources+tests,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    baseline=BUILD/(tag+'.baseline');compile(cc,flags,tag+'.baseline.compile',[PRIVATE/'baseline.c',ROOT/OLD[0]],baseline,['-I',ROOT/'labs/hosted_systems/include']);run(tag+'.baseline',[baseline],marker='OBS baseline capacity4 close-drain')
    for number,(case,marker) in enumerate(zip(CASES,MARKERS),1):
     mutant=BUILD/(tag+'.mutant'+str(number));compile(cc,flags,tag+'.mutation'+str(number)+'.compile',sources+tests,mutant,['-DTW_MUTANT'+str(number)]);run(tag+'.mutation'+str(number),[mutant,case],41,marker)
    learner=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c']+tests,learner)
    for case in CASES:run(tag+'.learner.'+case,[learner,case],41,'TW_ASSERT:')
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    assembly=run(tag+'.async-disassembly',['objdump','-dr','--disassemble=async_region',binary],marker='<async_region>:')
    calls=re.findall(r'\bcall\w*\s+[^\n]*<([^>]+)>',assembly)
    expected_call='__interceptor_trampoline_pthread_setcancelstate' if tag in ['clang.sanitized','clang.tsan'] else 'pthread_setcancelstate@plt'
    assert calls==[expected_call],('Unexpected async-region calls',tag,calls)
    observations.append(dict(profile=tag,async_region_calls=calls,instrumentation_exclusion='Only pure compute region; surrounding ownership code retains selected instrumentation',ordinary_cases=CASES,changed_cases=CASES,physical_or_fairness_claim=False))
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped sources changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Author execution; no independent review, learner delay or mastery awarded.','POSIX.1-2017 reprint plus selected Linux/GNU backend; exact published ISO C17/C23 and POSIX.1-2024 comparison not claimed.','No general asynchronous cancellation of library operations: only audited compute region, with explicit local sanitizer exclusion.','Starvation models establish repeatable policy cycles, not infinite native measurements or host scheduler preference.','Wake theft uses a controlled additional unlock/relock gap, not control of the libc scheduler return instruction.','Timing is a waiting deadline, not a hard whole-call latency guarantee.','Other T25 memory-ordering, CAS, reclamation and performance work remains separate.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
