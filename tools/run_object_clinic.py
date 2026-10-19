"""Execute repaired lifetime contracts separately from original invalid diagnostic children."""
import datetime,hashlib,json,pathlib,re,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_object_clinic';PUBLIC=ROOT/'labs/object_followthrough/clinic';PRIVATE=ROOT/'assessor/object_followthrough/clinic';BUILD=ROOT/'build'/('object-clinic-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CORE=['unique','containers'];SHARED=['parallel','serial','late']
MUTANTS={'ESCAPE':('unique','MC_MUTANT_ESCAPE','mc_snapshot(&owner,&copy)==0 && copy==7'),'RELEASE':('unique','MC_MUTANT_RELEASE','mc_release(&owner)==-1 && trace.releases==1'),'MEMBER':('containers','MC_MUTANT_MEMBER','mc_member(&owner,&other.member,&out)==-1 && out==17'),'OVERLAP':('unique','FD_MUTATE_OVERLAP','mc_move(bytes,12,2,0,8)==FD_OK && !memcmp(bytes,expected,12)')}
PRODUCER=['labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/foundation_depth/CONTRACTS.md','assessor/foundation_depth/depth.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in PRODUCER]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','valgrind','objdump','addr2line']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
    s={'kernel','libc','valgrind.version'}
    for cc in ['gcc','clang']:
        s|={cc+'.version'}
        for p,_ in PROFILES:
            t=cc+'.'+p;s|={t+'.'+q for q in ['compile',*CORE,'stub.compile',*['stub.'+c for c in CORE],'history.compile','history.oracle','shared.compile',*['shared.'+c for c in SHARED],'shared.stub.compile',*['shared.stub.'+c for c in SHARED],'shared.mutant.RETAIN.compile','shared.mutant.RETAIN']}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
        for p in ['O0','O3']:s|={cc+'.defects.'+p+'.compile',cc+'.defects.'+p+'.observe',cc+'.defects.'+p+'.assembly'}
        s|={cc+'.debug.memcheck.'+c for c in CORE}
        s|={cc+'.tsan.'+q for q in ['shared.compile',*['shared.'+c for c in SHARED],'race.compile','race.serial','race.parallel']}
    return s|{'clang.tsan.race.symbols'}
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None
    flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/foundation_depth/include')]
    def compile(tag,cc,opt,shared=False,stub=False,test=None):
        b=BUILD/tag
        if shared:sources=[PUBLIC/'shared_learner.c' if stub else PRIVATE/'shared.c',PUBLIC/(test or 'shared_tests.c')];extra=['-pthread']
        else:sources=[PUBLIC/'learner.c' if stub else PRIVATE/'clinic.c',ROOT/('labs/foundation_depth/src/depth.c' if stub else 'assessor/foundation_depth/depth.c'),PUBLIC/(test or 'tests.c')];extra=[]
        run(tag+'.compile',[cc,*flags,*opt,*extra,*sources,'-o',b]);return b
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('valgrind.version',['valgrind','--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for p,opt in PROFILES:
                t=cc+'.'+p;b=compile(t,cc,opt)
                for case in CORE:run(t+'.'+case,[b,case],marker='MC_'+case.upper()+'_PASS')
                probe=compile(t+'.history',cc,opt,test='history.c');run(t+'.history.oracle',[sys.executable,PRIVATE/'oracle.py',probe,BUILD/(t+'.history.json')],marker='"observations": 324')
                stub=compile(t+'.stub',cc,opt,stub=True)
                for case in CORE:run(t+'.stub.'+case,[stub,case],41,'MC_ASSERT:')
                for m,(case,macro,marker) in MUTANTS.items():
                    mutant=compile(t+'.mutant.'+m,cc,[*opt,'-D'+macro]);run(t+'.mutant.'+m,[mutant,case],41,'MC_ASSERT:'+marker)
                b=compile(t+'.shared',cc,opt,shared=True)
                for case in SHARED:run(t+'.shared.'+case,[b,case],marker='MC_SHARED_'+case.upper()+'_PASS')
                stub=compile(t+'.shared.stub',cc,opt,shared=True,stub=True)
                for case in SHARED:run(t+'.shared.stub.'+case,[stub,case],41,'MC_SHARED_ASSERT:')
                mutant=compile(t+'.shared.mutant.RETAIN',cc,[*opt,'-DMC_MUTANT_RETAIN'],shared=True);run(t+'.shared.mutant.RETAIN',[mutant,'parallel'],41,'MC_SHARED_ASSERT:mc_shared_retain(&state,&extra)==0 && state.references==2')
                if p=='debug':
                    for case in CORE:run(t+'.memcheck.'+case,['valgrind','--error-exitcode=99','--leak-check=full',BUILD/t,case],marker='ERROR SUMMARY: 0 errors')
            for p in ['O0','O3']:
                t=cc+'.defects.'+p;b=BUILD/t;run(t+'.compile',[cc,'-std=c17','-'+p,'-g','-fsanitize=address,undefined','-fsanitize-address-use-after-scope','-fno-omit-frame-pointer','-fno-builtin-memcpy',PUBLIC/'defects.c','-o',b]);run(t+'.assembly',['objdump','-d',b],marker='<main>:');run(t+'.observe',[sys.executable,PRIVATE/'observe.py',b,BUILD/(t+'.json'),*(['expect-direct-overlap-miss'] if cc=='gcc' and p=='O3' else [])],marker='"diagnostic_cases": 5')
            opt=['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'];t=cc+'.tsan';b=compile(t+'.shared',cc,opt,shared=True)
            for case in SHARED:run(t+'.shared.'+case,[b,case],marker='MC_SHARED_'+case.upper()+'_PASS')
            race=compile(t+'.race',cc,[*opt,'-DMC_MUTANT_RACE'],shared=True);run(t+'.race.serial',[race,'serial'],marker='MC_SHARED_SERIAL_PASS');race_log=run(t+'.race.parallel',[race,'parallel'],66,'WARNING: ThreadSanitizer: data race')
            if cc=='clang':
                offsets=list(dict.fromkeys(re.findall(r'clang\.tsan\.race\+(0x[0-9a-f]+)',race_log)));assert len(offsets)>=2
                run(t+'.race.symbols',['addr2line','-f','-e',race,*offsets],marker='mc_shared_add')
        assert {q['id'] for q in h.COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext:failure='Scoped source/tool identity changed'
    limits=['C17/Profile A owner and overlap mechanisms use actual injected allocation and unchanged fd_move; pthreads is an explicitly hosted changed-condition adapter.','Six GCC/Clang profiles plus separate GCC/Clang TSan execute original positive and negative controls; all instrumentation applies only to selected executions.','Original UB diagnostic programs and intentional data race run only in separate synthetic local processes; no UB return value is an oracle or exploitation claim.','Quarantine retains a real C allocation after logical release; clean instrumented access does not grant application ownership.','81 four-event histories per profile and three pthread schedules are finite, not proof of all histories or fairness.','Control storage outlives joined borrowers; callbacks are nonreentrant and default nonrobust mutex errors after initialization are terminal protocol failures.','Exact published C17/C23 source text, independent review, independent delayed learner and physical evidence remain unavailable or unperformed.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
