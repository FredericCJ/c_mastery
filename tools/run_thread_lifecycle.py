"""Actual bounded T25.01 protocols and compile-only interface comparisons."""
import hashlib,json,os,pathlib,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/legacy_thread_lifecycle';BUILD=ROOT/'build/thread-lifecycle'
PUBLIC=ROOT/'labs/thread_depth/lifecycle';PRIVATE=ROOT/'assessor/thread_depth/lifecycle'
OLD=['assessor/posix_lifecycle_depth/posix_lifecycle.c','labs/posix_lifecycle_depth/include/posix_lifecycle.h','labs/posix_lifecycle_depth/tests/tests.c']
COMMANDS=[];OUTPUTS=set()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/p for p in OLD]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def run(ident,args,expected=0,marker=None):
 start=time.monotonic();env=os.environ.copy()
 for key in ['CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS']:env.pop(key,None)
 env.update(ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',TSAN_OPTIONS='halt_on_error=1:exitcode=66')
 try:p=subprocess.run([str(x) for x in args],cwd=ROOT,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=30);output=p.stdout;code=p.returncode
 except subprocess.TimeoutExpired as ex:output=str(ex);code=124
 path=BUILD/(ident+'.log');path.write_text(output,encoding='utf8');OUTPUTS.add(path)
 passed=code==expected and (marker is None or marker in output)
 COMMANDS.append(dict(id=ident,command=[str(x) for x in args],exit_code=code,expected_exit=expected,required_marker=marker,passed=passed,elapsed_seconds=time.monotonic()-start,output=path.relative_to(ROOT).as_posix(),output_sha256=sha(path)))
 print(ident,'PASS' if passed else 'FAIL',flush=True)
 if not passed:raise RuntimeError(ident+'\n'+output[-3000:])
 return output
def compile(cc,flags,name,sources,binary,extra=()):
 run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,'-I',PUBLIC,'-I',ROOT/'labs/posix_lifecycle_depth/include',*extra,*sources,'-o',binary]);OUTPUTS.add(binary)
def main():
 BUILD.mkdir(parents=True,exist_ok=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();failure=None
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
    tag=cc+'.'+profile;binary=BUILD/tag
    compile(cc,flags,tag+'.compile',[PRIVATE/'lifecycle.c',PUBLIC/'tests.c'],binary)
    for case in ['pool','detached']:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    old=BUILD/(tag+'.baseline');compile(cc,flags,tag+'.baseline.compile',[ROOT/OLD[0],PUBLIC/'baseline.c'],old);run(tag+'.baseline',[old],marker='PASS posix_lifecycle_depth.pool')
    for number,case,assertion in [(1,'pool','out.created==count'),(2,'pool','preserved_join_output'),(3,'detached','worker_pending_after_creator')]:
     mutant=BUILD/(tag+'.mutant'+str(number));compile(cc,flags,tag+'.mutation'+str(number)+'.compile',[PRIVATE/'lifecycle.c',PUBLIC/'tests.c'],mutant,['-DTL_MUTANT'+str(number)])
     run(tag+'.mutation'+str(number),[mutant,case],41,assertion)
    learner=BUILD/(tag+'.learner');compile(cc,flags,tag+'.learner.compile',[PUBLIC/'learner.c',PUBLIC/'tests.c'],learner)
    for case in ['pool','detached']:run(tag+'.learner.'+case,[learner,case],41,'TL_ASSERT:')
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[PUBLIC/'race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
   for api,marker in [('c','C_REJECT_POINTER_RESULT'),('posix','POSIX_REJECT_INTEGER_RESULT')]:
    for edition in ['c17','c23']:
     tag=cc+'.'+edition+'.'+api;binary=BUILD/tag
     run(tag+'.compile',[cc,'-std='+edition,'-Wall','-Wextra','-Werror','-Wpedantic','-pthread',PUBLIC/('probe_'+api+'.c'),'-o',binary]);OUTPUTS.add(binary)
     run(tag,[binary],marker='THREADS')
     run(tag+'.wrong',[cc,'-std='+edition,'-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-DTL_BAD_SIGNATURE','-fsyntax-only',PUBLIC/('probe_'+api+'.c')],1,marker)
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped sources changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(OUTPUTS)},commands=COMMANDS,limits=['Author execution only; external review and independent learner tasks not attempted.','Original intentionally racy sensitivity target has no value oracle and is confined to a separate local process.','Exact published C17/C23 source comparison remains open; compiler modes and inspected drafts are distinct evidence.','Failure injection before join preserves the handle; arbitrary actual corrupted/consumed handles are not recoverable by this policy.','No asynchronous cancellation, scheduling bound or proof of all histories.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
