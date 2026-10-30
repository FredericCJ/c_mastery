"""Scoped actual T23 experiments; no host clock/affinity/security changes."""
import hashlib,json,os,pathlib,re,statistics,subprocess,tempfile,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/legacy_posix_context';BUILD=ROOT/'build/posix-context-followthrough'
PUBLIC=ROOT/'labs/posix_context_depth';PRIVATE=ROOT/'assessor/posix_context_depth'
COMMANDS=[];OUTPUTS=set();OBS=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
    files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files+=[pathlib.Path(__file__),ROOT/'fixtures/systems/PR-T23-05.json',ROOT/'fixtures/systems/PR-T23-06.json']
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def run(ident,args,expected=0,marker=None,env=None):
    start=time.monotonic();e=os.environ.copy()
    for k in ['CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS']:e.pop(k,None)
    e.update(ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    if env:e.update(env)
    try:
        p=subprocess.run([str(x) for x in args],cwd=ROOT,env=e,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=40)
        output=p.stdout;code=p.returncode
    except subprocess.TimeoutExpired as ex:output=str(ex);code=124
    path=BUILD/(ident+'.log');path.write_text(output,encoding='utf8');OUTPUTS.add(path)
    passed=(code in expected if isinstance(expected,list) else code==expected) and (marker is None or marker in output)
    COMMANDS.append(dict(id=ident,command=[str(x) for x in args],exit_code=code,expected_exit=expected,required_marker=marker,passed=passed,elapsed_seconds=time.monotonic()-start,output=path.relative_to(ROOT).as_posix(),output_sha256=sha(path)))
    print(ident,'PASS' if passed else 'FAIL',flush=True)
    if not passed:raise RuntimeError(ident+' '+output[-3000:])
    return output
def compile_case(cc,flags,name,source,target,extra=()):
    run(name,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,'-I',PUBLIC,'-I',PUBLIC/'tests',*extra,*source,'-o',target]);OUTPUTS.add(target)
def profiles(storage):
    for cc in ['gcc','clang']:
        run(cc+'.version',[cc,'--version'])
        for mode,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            prefix=cc+'.'+mode;helper=BUILD/(prefix+'.helper');compile_case(cc,flags,prefix+'.helper.compile',[PUBLIC/'tests/helper.c'],helper)
            for case in ['process','storage','timing','capability','reap']:
                binary=BUILD/(prefix+'.'+case);compile_case(cc,flags,prefix+'.'+case+'.compile',[PRIVATE/'context.c',PUBLIC/'tests'/f'{case}.c'],binary)
                output=run(prefix+'.'+case,[binary,helper],marker='PC_'+case.upper()+'_PASS',env={'PC_STORAGE_ROOT':storage})
                if case=='timing':
                    rows=[dict(zip(['planned','wake','dispatch','done','late'],map(int,m))) for m in re.findall(r'PC_SAMPLE planned=(\d+) wake=(\d+) dispatch=(\d+) done=(\d+) late=(\d+)',output)]
                    assert len(rows)==12 and all(x['planned']<=x['wake']<=x['dispatch']<=x['done'] for x in rows)
                    values=sorted(x['late'] for x in rows);OBS.append(dict(profile=prefix,kind='timing',samples=rows,median_lateness_ns=statistics.median(values),maximum_observed_lateness_ns=max(values),upper_bound_claim=False))
                elif case=='storage':OBS.append(dict(profile=prefix,kind='storage',filesystem=re.search(r'PC_FILESYSTEM type=(\w+)',output).group(1),physical_persistence=False,process_cuts=4))
            for mutant,case in [(1,'process'),(2,'storage'),(3,'timing'),(4,'capability'),(5,'reap')]:
                binary=BUILD/(prefix+'.mutant'+str(mutant));compile_case(cc,flags,prefix+'.mutation.'+case+'.compile',[PRIVATE/'context.c',PUBLIC/'tests'/f'{case}.c'],binary,['-DPC_MUTANT='+str(mutant)])
                run(prefix+'.mutation.'+case,[binary,helper],expected=41,marker='PC_ASSERT:',env={'PC_STORAGE_ROOT':storage})
            for case in ['process','storage','timing','capability','reap']:
                binary=BUILD/(prefix+'.learner.'+case);compile_case(cc,flags,prefix+'.learner.'+case+'.compile',[PUBLIC/'learner.c',PUBLIC/'tests'/f'{case}.c'],binary)
                run(prefix+'.learner.'+case,[binary,helper],expected=41,marker='PC_ASSERT:',env={'PC_STORAGE_ROOT':storage})
def features():
    probes={'pipe2':'#include <fcntl.h>\n#include <unistd.h>\nint main(void){int p[2];return pipe2(p,O_CLOEXEC);}\n','openat':'#include <fcntl.h>\nint main(void){return openat(0,"synthetic",O_RDONLY);}\n','clock':'#include <time.h>\nint main(void){struct timespec t;return clock_gettime(CLOCK_MONOTONIC,&t);}\n'}
    for cc in ['gcc','clang']:
        for kind,text in probes.items():
            source=BUILD/(kind+'.c');source.write_text(text);OUTPUTS.add(source)
            for mode,macro,expected in [('hidden',[],1),('visible',['-D_GNU_SOURCE' if kind=='pipe2' else '-D_POSIX_C_SOURCE=200809L'],0)]:
                run('feature.'+cc+'.'+kind+'.'+mode,[cc,'-std=c17','-Wall','-Werror','-Werror=implicit-function-declaration',*macro,'-fsyntax-only',source],expected=expected,marker=kind if mode=='hidden' else None)
def main():
    BUILD.mkdir(parents=True,exist_ok=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();failure=None
    try:
        run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version']);run('environment.mount',['findmnt','-T','/home/frede','-n','-o','FSTYPE,OPTIONS'])
        with tempfile.TemporaryDirectory(prefix='cmastery-posix-context-',dir='/home/frede') as storage:profiles(storage)
        features();run('models',['python3',PRIVATE/'models.py'],marker='PC_ORACLE_PASS')
    except Exception as ex:failure=str(ex)
    after=pins()
    if before!=after:failure='Source changed during campaign'
    external=pathlib.Path('/home/frede/cmastery-startup-tools/source/glibc-2.43/sysdeps/unix/sysv/linux/spawni.c')
    report=dict(schema_version=1,status='pass' if failure is None else 'fail',commands=COMMANDS,observations=OBS,failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(OUTPUTS)},external_source_sha256={str(external):sha(external)},limits=['Author executions only; no independent review or learner assessment.','Storage runs use a private directory on the reported filesystem; process termination is not power/cache loss.','GNU glibc spawn file-action and observed direct ENOENT behavior are not universal POSIX guarantees.','Timing distributions have no hard upper-bound claim; CPU/permission changes are modeled except child-only lowered resource limit.','No non-Linux runtime or older kernel actually executed; source-level port and explicit capability models are separate.'])
    (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
