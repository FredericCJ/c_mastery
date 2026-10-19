"""Scoped T25.02 mechanism, mutation, detector and whole-call cost experiments."""
import hashlib,json,pathlib,re
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_sharing';BUILD=ROOT/'build/thread-sharing'
PUBLIC=ROOT/'labs/thread_depth/sharing';PRIVATE=ROOT/'assessor/thread_depth/sharing'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'labs/thread_depth/lifecycle/race_control.c']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def main():
 BUILD.mkdir(parents=True,exist_ok=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();failure=None;observations=[]
 harness.BUILD=BUILD;harness.PUBLIC=PUBLIC;harness.PRIVATE=PRIVATE
 run=harness.run;compile=harness.compile
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
    tag=cc+'.'+profile;binary=BUILD/tag
    compile(cc,flags,tag+'.compile',[PRIVATE/'sharing.c',PUBLIC/'tests.c'],binary)
    for case in ['partition','ownership','histogram']:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    for i,case,marker in [(1,'partition','out==expected'),(2,'ownership','state==TS_CALLER'),(3,'histogram','out[b]==expected[b]')]:
     mutant=BUILD/(tag+'.mutant'+str(i));compile(cc,flags,tag+'.mutation'+str(i)+'.compile',[PRIVATE/'sharing.c',PUBLIC/'tests.c'],mutant,['-DTS_MUTANT'+str(i)])
     run(tag+'.mutation'+str(i),[mutant,case],41,marker)
    learner=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'tests.c'],learner)
    for case in ['partition','ownership','histogram']:run(tag+'.learner.'+case,[learner,case],41,'TS_ASSERT:')
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    if profile=='optimized':
     output=run(tag+'.measurement',[binary,'measurement'],marker='OBS measurement 48 raw samples')
     records=[dict(zip(['bins','distribution','mode','trial','ns','buffers','requested_stacks'],map(int,m))) for m in re.findall(r'SAMPLE bins=(\d+) distribution=(\d+) mode=(\d+) trial=(\d+) ns=(\d+) buffers=(\d+) requested_stacks=(\d+)',output)]
     cache=int(re.search(r'ENV L1D=(\d+)',output).group(1));assert len(records)==48 and 65536*8>cache
     observations.append(dict(compiler=cc,reported_l1d_bytes=cache,samples=records,scope='whole calls including allocation, initialization, create/join and four equivalent exact-count modes',physical_cache_validation=False,universal_speedup_claim=False))
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped sources changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Author histories only; independent learner and reviewer not assessed.','Cache boundary uses selected guest libc reported L1D size, not all cache levels or physical validation.','Common4MiB numeric-buffer budget excludes separately reported stacks, fixed frames, libc metadata/TLS/guards/kernel state and instrumentation.','Whole-call timing includes thread creation and allocation; no steady-state or universal-speedup inference.','Snapshot mutation occurs after capture during the call; simultaneous instruction overlap is not measured.','Explicit known local worker failure is not arbitrary process-death or external-effect exactly-once recovery.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),samples=sum(len(o['samples']) for o in observations),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
