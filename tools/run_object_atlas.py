"""Source-bound lifetime, representation and alignment observations."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_object_atlas';PUBLIC=ROOT/'labs/object_followthrough/atlas';PRIVATE=ROOT/'assessor/object_followthrough/atlas';BUILD=ROOT/'build'/('object-atlas-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['lifetime','escape','representation','equality','alignment']
MUTANTS={'FAIL':(0,'oa_resize(&o,8,&a)==-1 && o.data==still_live && o.length==4 && o.generation==1 && o.data[3]==11'),'BORROW':(1,'oa_deep_copy(v,copy,sizeof copy,&deep)==0 && deep.data==copy && deep.length==3'),'BYTES':(2,'oa_equal(&a,&b)'),'ZERO':(3,'oa_encode(&a,x,13)==0 && oa_encode(&b,y,13)==0 && !memcmp(x,y,13)'),'SLICE':(4,'oa_place(32,offsets[i],16,8,8,&at)==0 && at==answers[i]'),'WIDE':(4,'!oa_wide_admit(_Alignof(max_align_t))')}
SYSROOT=ROOT/'build/posix-io-toolchain/sysroot';LIBS=SYSROOT/'usr/lib32';GCCDIR=SYSROOT/'usr/lib/gcc/x86_64-linux-gnu/15/32';LOADER=LIBS/'ld-linux.so.2'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py'])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),LOADER,LIBS/'libc.so.6',GCCDIR/'libgcc.a'})}
def expected_commands():
    s={'kernel','libc','i386.compile','i386.oracle','i386.elf','i386.dependencies','gcc.lto.compile','gcc.lto.oracle','decode.assembly'}|{'i386.'+c for c in CASES}
    for cc in ['gcc','clang']:
        s.add(cc+'.version')
        for profile,_ in PROFILES:
            t=cc+'.'+profile;s|={t+'.'+n for n in ['compile','oracle','stub.compile',*CASES,*['stub.'+c for c in CASES]]}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
    for profile in ['debug','optimized','sanitized','tsan']:
        t='gcc.tls.'+profile;s|={t+'.compile',t+'.run'}
        for m in ['GLOBAL','NOWRITE']:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;dependencies={}
    flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC)]
    def compile(tag,cc,opt,stub=False):
        binary=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'atlas.c',PUBLIC/'tests.c','-o',binary]);return binary
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for profile,opt in PROFILES:
                t=cc+'.'+profile;b=compile(t,cc,opt)
                for i,c in enumerate(CASES):run(t+'.'+c,[b,i],marker='OA_'+c.upper()+'_PASS')
                run(t+'.oracle',[sys.executable,PRIVATE/'oracle.py',b,BUILD/(t+'.packets.json')],marker='"placement_cases": 3690')
                b=compile(t+'.stub',cc,opt,True)
                for i,c in enumerate(CASES):run(t+'.stub.'+c,[b,i],41,'OA_ASSERT:')
                for m,(i,marker) in MUTANTS.items():b=compile(t+'.mutant.'+m,cc,[*opt,'-DOA_MUTANT_'+m]);run(t+'.mutant.'+m,[b,i],41,'OA_ASSERT:'+marker)
        b=compile('gcc.lto','gcc',['-O3','-flto','-DNDEBUG']);run('gcc.lto.oracle',[sys.executable,PRIVATE/'oracle.py',b,BUILD/'gcc.lto.packets.json'],marker='"placement_cases": 3690')
        narrow=['-m32','--sysroot='+str(SYSROOT),'-isystem',str(SYSROOT/'usr/include/x86_64-linux-gnu'),'-B'+str(GCCDIR)+'/','-L'+str(LIBS),'-Wl,--dynamic-linker='+str(LOADER)]
        text=run('i386.dependencies',['gcc',*flags,*narrow,'-M',PRIVATE/'atlas.c',PUBLIC/'tests.c'])
        for token in text.replace('\\\n',' ').split():
            p=pathlib.Path(token)
            if p.is_absolute() and p.is_file():dependencies[str(p)]=sha(p)
        b=compile('i386','gcc',[*narrow,'-O2']);prefix=[str(LOADER),'--library-path',str(LIBS)];run('i386.elf',['objdump','-f',b],marker='elf32-i386')
        for i,c in enumerate(CASES):run('i386.'+c,[*prefix,b,i],marker='OA_'+c.upper()+'_PASS')
        run('i386.oracle',[sys.executable,PRIVATE/'oracle.py',b,BUILD/'i386.packets.json',json.dumps(prefix)],marker='"placement_cases": 3690')
        run('decode.assembly',['objdump','-d','--disassemble=oa_decode_u32',BUILD/'gcc.optimized'],marker='<oa_decode_u32>')
        for profile,opt in [*PROFILES,('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
            t='gcc.tls.'+profile;b=BUILD/t;run(t+'.compile',['gcc','-std=gnu17','-Wall','-Wextra','-Werror','-pthread',*opt,PUBLIC/'tls.c','-o',b]);run(t+'.run',[b],marker='OA_TLS_PASS:')
            for m in ['GLOBAL','NOWRITE']:
                b=BUILD/(t+'.mutant.'+m);run(t+'.mutant.'+m+'.compile',['gcc','-std=gnu17','-Wall','-Wextra','-Werror','-pthread',*opt,'-DOA_TLS_'+m,PUBLIC/'tls.c','-o',b]);run(t+'.mutant.'+m,[b],41,'OA_TLS_ASSERT:')
        assert set(q['id'] for q in harness.COMMANDS)==expected_commands()
    except Exception as e:failure=str(e)
    if before!=pins() or ext!=external() or any(sha(pathlib.Path(n))!=h for n,h in dependencies.items()):failure='Scoped source/tool/header identity changed'
    limits=['C17/profileA core; exact GCC GNU __thread contract and pthread synchronization are labeled implementation extensions.','Native LP64 and actual i386 ILP32 execution share little-endian binary64 on these targets; no different-endian or hardware alignment-fault experiment claimed.','Character-byte padding observations and canonical normalization are separate from raw equality; wire floating codec admits only stated binary64 object/uint64_t correspondence.','Replacement uses explicit allocate-copy-free, not a claim about a particular realloc address policy. Numeric address history never legitimizes an expired pointer.','The typed caller owner supplies actual struct oa_record objects; numeric alignment arithmetic does not create typed objects or validate arbitrary pointers.','No invalid cast, expired-pointer evaluation, trapping representation, signaling NaN, or analysis-only defect is executed.','Independent finite placement oracle3690cases/profile is not a universal correctness or mastery proof.','Exact final C17/C23 text, independent learner delayed attempts, external review and physical observations remain open.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,dependency_sha256=dependencies,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(report['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
