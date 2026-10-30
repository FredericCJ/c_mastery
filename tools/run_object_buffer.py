"""Execute partial-construction, flexible storage, VLA and relocation contracts with bounded diagnostics."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_object_buffer';PUBLIC=ROOT/'labs/object_followthrough/buffer';PRIVATE=ROOT/'assessor/object_followthrough/buffer';BUILD=ROOT/'build'/('object-buffer-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['initialization','sizing','workspace','representations','relocation','handles']
MUTANTS={'CLEANUP':(0,'ob_cleanup(&c,a)==0 && !c.initialized && !t.releases'),'SIZE':(1,'ob_size(16,4,3,255,&out)==0 && out==28'),'SHARED':(3,'ob_shared_retain(first,&second)==0 && first->references==2'),'RESIZE':(4,'ob_resize(&table,8)==-1 && table.value==owner'),'GENERATION':(4,'ob_resolve(&table,h,&out)==-1 && out==3'),'ERASE':(5,'ob_erase(&a,0)==0 && a.generation==2')}
SYSROOT=ROOT/'build/posix-io-toolchain/sysroot';LIBS=SYSROOT/'usr/lib32';GCCDIR=SYSROOT/'usr/lib/gcc/x86_64-linux-gnu/15/32';LOADER=LIBS/'ld-linux.so.2'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'labs/support/cm_parse.c',ROOT/'labs/support/cm_parse.h'])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','valgrind','objdump']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),LOADER,LIBS/'libc.so.6',*[pathlib.Path('/usr/libexec/valgrind')/n for n in ['memcheck-amd64-linux','vgpreload_core-amd64-linux.so','vgpreload_memcheck-amd64-linux.so']]})}
def expected_commands():
    s={'kernel','libc','valgrind.version','i386.dependencies','i386.compile','i386.elf'}|{'i386.'+c for c in CASES}|{'i386.'+k+q for k in ['size','history'] for q in ['.compile','.oracle']}
    for cc in ['gcc','clang']:
        s|={cc+'.version',cc+'.vla.forbidden',cc+'.fallback.compile',cc+'.fallback.workspace',cc+'.diagnostics.observe'}|{cc+'.diagnostic.'+p+'.compile' for p in ['debug','optimized','sanitized']}|{cc+'.diagnostic.'+p+'.assembly' for p in ['debug','optimized']}|{cc+'.warning.'+p for p in ['O0','O3']}
        for edition in ['c99','c11','c17','c23']:s|={cc+'.'+edition+'.vla.compile',cc+'.'+edition+'.vla'}
        for p,_ in PROFILES:
            t=cc+'.'+p;s|={t+'.'+q for q in ['compile','stub.compile',*CASES,*['stub.'+c for c in CASES],'size.compile','size.oracle','history.compile','history.oracle']}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
        s|={cc+'.debug.memcheck.'+c for c in ['initialization','relocation','handles']}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None;deps={}
    flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/support')]
    def compile(tag,cc,opt,stub=False,test='tests.c'):
        b=BUILD/tag;sources=[PUBLIC/'learner.c'] if stub else [PRIVATE/'buffer.c',PRIVATE/'workspace.c'];run(tag+'.compile',[cc,*flags,*opt,*sources,PUBLIC/test,ROOT/'labs/support/cm_parse.c','-o',b]);return b
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('valgrind.version',['valgrind','--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for p,opt in PROFILES:
                t=cc+'.'+p;b=compile(t,cc,opt)
                for i,c in enumerate(CASES):run(t+'.'+c,[b,i],marker='OB_'+c.upper()+'_PASS')
                for k,n in [('size',320),('history',2500)]:
                    probe=compile(t+'.'+k,cc,opt,test='size_probe.c' if k=='size' else 'history.c');run(t+'.'+k+'.oracle',[sys.executable,PRIVATE/'oracle.py',k,probe,BUILD/(t+'.'+k+'.packets.json')],marker='"observations": '+str(n))
                stub=compile(t+'.stub',cc,opt,stub=True)
                for i,c in enumerate(CASES):run(t+'.stub.'+c,[stub,i],41,'OB_ASSERT:')
                for m,(i,marker) in MUTANTS.items():
                    mutant=compile(t+'.mutant.'+m,cc,[*opt,'-DOB_MUTANT_'+m]);run(t+'.mutant.'+m,[mutant,i],41,'OB_ASSERT:'+marker)
                if p=='debug':
                    for i,c in [(0,'initialization'),(4,'relocation'),(5,'handles')]:run(t+'.memcheck.'+c,['valgrind','--error-exitcode=99','--track-origins=yes','--leak-check=full',b,i],marker='ERROR SUMMARY: 0 errors')
            b=compile(cc+'.fallback',cc,['-O2','-DOB_NO_VLA','-Wvla']);run(cc+'.fallback.workspace',[b,2],marker='OB_WORKSPACE_PASS');run(cc+'.vla.forbidden',[cc,*flags,'-Wvla','-c',PRIVATE/'workspace.c','-o',BUILD/(cc+'.forbidden.o')],1,'error:')
            for edition in ['c99','c11','c17','c23']:
                tag=cc+'.'+edition+'.vla';b=BUILD/tag;run(tag+'.compile',[cc,'-std='+edition,*flags[1:],'-O2',PRIVATE/'workspace.c',PUBLIC/'vla_probe.c','-o',b]);run(tag,[b],marker='OB_EDITION:')
            binaries=[]
            for p,opt in [('debug',['-O0','-g','-gdwarf-4']),('optimized',['-O3','-g','-gdwarf-4']),('sanitized',['-O1','-g','-fsanitize=address,undefined'])]:
                b=BUILD/(cc+'.diagnostic.'+p);binaries.append(b);run(cc+'.diagnostic.'+p+'.compile',[cc,'-std=c17',*opt,PUBLIC/'diagnostic.c','-o',b])
                if p!='sanitized':run(cc+'.diagnostic.'+p+'.assembly',['objdump','-d',b],marker='<main>:')
            for opt in ['O0','O3']:run(cc+'.warning.'+opt,[cc,'-std=c17','-Wall','-Wextra','-Wuninitialized',*(['-Wmaybe-uninitialized'] if cc=='gcc' else []),'-'+opt,'-c',PUBLIC/'warning.c','-o',BUILD/(cc+'.warning.'+opt+'.o')])
            run(cc+'.diagnostics.observe',[sys.executable,PRIVATE/'diagnostics.py',*binaries,BUILD/(cc+'.diagnostics.json')],marker='"observations": 6')
        narrow=['-m32','--sysroot='+str(SYSROOT),'-isystem',str(SYSROOT/'usr/include/x86_64-linux-gnu'),'-B'+str(GCCDIR)+'/','-L'+str(LIBS),'-Wl,--dynamic-linker='+str(LOADER)]
        output=run('i386.dependencies',['gcc',*flags,*narrow,'-M',PRIVATE/'buffer.c',PRIVATE/'workspace.c',PUBLIC/'tests.c',ROOT/'labs/support/cm_parse.c'])
        for token in output.replace('\\\n',' ').split():
            p=pathlib.Path(token)
            if p.is_absolute() and p.is_file():deps[str(p)]=sha(p)
        b=compile('i386','gcc',[*narrow,'-O2']);run('i386.elf',['objdump','-f',b],marker='elf32-i386');prefix=[LOADER,'--library-path',LIBS]
        for i,c in enumerate(CASES):run('i386.'+c,[*prefix,b,i],marker='OB_'+c.upper()+'_PASS')
        for k,n in [('size',320),('history',2500)]:
            probe=compile('i386.'+k,'gcc',[*narrow,'-O2'],test='size_probe.c' if k=='size' else 'history.c');run('i386.'+k+'.oracle',[sys.executable,PRIVATE/'oracle.py',k,probe,BUILD/('i386.'+k+'.packets.json'),json.dumps(list(map(str,prefix)))],marker='"observations": '+str(n))
        assert {q['id'] for q in h.COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext or any(sha(pathlib.Path(n))!=v for n,v in deps.items()):failure='Scoped source/tool/header identity changed'
    limits=['Profile A component uses explicit injected allocation; native harness uses actual malloc/realloc/free, forced-move copy/free and failure injection.','Six native profiles and actual i386 execute the full contract; no target or physical observation is claimed.','Uninitialized-use diagnostic targets are original local processes with synthetic state. Only initialized branches are positive language cases; diagnostic sensitivity is never a golden undefined result.','320 arithmetic boundaries and625 four-event histories per profile are finite independent models, not proof for all allocations/histories.','Raw views must be withdrawn before successful resize; integer pre-release address snapshots do not confer access authority.','Registry identities must be unique for the entire external token lifetime; a moved table uses an explicit current locator, not a stale control pointer.','N1570, C17 issue0400/0451 and N3096 later draft are separate inspected authorities; published C17/C23 remain unavailable.','External review, independent delayed learner assessment and physical validation are unperformed.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,dependency_sha256=deps,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
