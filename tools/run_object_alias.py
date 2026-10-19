"""Execute defined alias contracts, source-separated compiler comparisons and strict closure."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_object_alias';PUBLIC=ROOT/'labs/object_followthrough/alias';PRIVATE=ROOT/'assessor/object_followthrough/alias';BUILD=ROOT/'build'/('object-alias-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
FD=ROOT/'assessor/foundation_depth/depth.c';FDINC=ROOT/'labs/foundation_depth/include';FDSTUB=ROOT/'labs/foundation_depth/src/depth.c'
PROFILES=[('debug',['-O0','-g']),('strict',['-O3','-fstrict-aliasing']),('relaxed',['-O3','-fno-strict-aliasing']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['lifecycle','wide','representations','restricted','identity']
MUTANTS={'LIFECYCLE':(0,'FD_MUTATE_LIFECYCLE','fd_slot_initialize(&s,initialize,destroy,&t)==FD_INIT_FAILED'), 'DESTRUCTOR':(1,'OS_MUTANT_DESTRUCTOR','os_wide_reset(&s)==-1 && s.state==OS_CLEANUP'),'WIRE':(2,'OS_MUTANT_WIRE','os_wire(wire,8,&out)==0'),'PARTITION':(3,'OS_MUTANT_PARTITION','os_partition(inplace,16,2,0,4,0x55,1)==-1'),'OVERLAP':(3,'FD_MUTATE_OVERLAP','inplace[2+i]==i'),'HANDLES':(4,'FD_MUTATE_HANDLES','fd_pool_acquire(&p,(FDValue){4,11},&h)==FD_OK')}
SYSROOT=ROOT/'build/posix-io-toolchain/sysroot';LIBS=SYSROOT/'usr/lib32';GCCDIR=SYSROOT/'usr/lib/gcc/x86_64-linux-gnu/15/32';LOADER=LIBS/'ld-linux.so.2';LLD=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
IMPORTS=['assessor/foundation_depth/depth.c','labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/foundation_depth/CONTRACTS.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),LOADER,LIBS/'libc.so.6',LLD.resolve()})}
def expected_commands():
    s={'kernel','libc','i386.compile','i386.probe.compile','i386.oracle','i386.elf','i386.dependencies'}|{'i386.'+c for c in CASES}
    for cc in ['gcc','clang']:
        s|={cc+'.version'}
        for p,_ in PROFILES:
            t=cc+'.'+p;s|={t+'.'+q for q in ['compile','probe.compile','oracle','stub.compile',*CASES,*['stub.'+c for c in CASES],'extension.compile','extension','wrap.safe.compile','wrap.safe','wrap.unsafe.compile','wrap.unsafe']}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
            if p in ['strict','relaxed']:s|={t+'.'+q for q in ['bench.core','bench.entry','bench.link','measure','assembly','sizes']}
        for opt in ['O0','O3']:s|={cc+'.arm.'+opt+'.'+q for q in ['core','foundation','entry','objects','link','undefined','symbols','assembly','bad.entry','bad.link']}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None;deps={}
    flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(FDINC)]
    def compile(tag,cc,opt,source=None,test='tests.c',foundation=FD):
        b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,source or PRIVATE/'alias.c',foundation,PUBLIC/test,'-o',b]);return b
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for p,opt in PROFILES:
                t=cc+'.'+p;b=compile(t,cc,opt)
                for i,c in enumerate(CASES):run(t+'.'+c,[b,i],marker='OS_'+c.upper()+'_PASS')
                probe=compile(t+'.probe',cc,opt,test='probe.c');run(t+'.oracle',[sys.executable,PRIVATE/'oracle.py',probe,BUILD/(t+'.packets.json')],marker='"range_cases": 6048')
                stub=compile(t+'.stub',cc,opt,PUBLIC/'learner.c',foundation=FDSTUB)
                for i,c in enumerate(CASES):run(t+'.stub.'+c,[stub,i],41,'OS_ASSERT:')
                for m,(i,define,marker) in MUTANTS.items():
                    b=compile(t+'.mutant.'+m,cc,[*opt,'-D'+define]);run(t+'.mutant.'+m,[b,i],41,'OS_ASSERT:'+marker)
                b=BUILD/(t+'.extension');run(t+'.extension.compile',[cc,*flags,*opt,PUBLIC/'extension.c','-o',b]);run(t+'.extension',[b],marker='OS_EXTENSION_PASS')
                for variant,define in [('safe',[]),('unsafe',['-DFD_MUTATE_HANDLES'])]:
                    b=BUILD/(t+'.wrap.'+variant);run(t+'.wrap.'+variant+'.compile',[cc,*flags,*opt,*define,FD,PUBLIC/'wrap.c','-o',b]);run(t+'.wrap.'+variant,[b],3 if variant=='safe' else 0,'OS_WRAP_REFUSED' if variant=='safe' else 'OS_WRAP_COLLISION')
                if p in ['strict','relaxed']:
                    core=BUILD/(t+'.bench.o');entry=BUILD/(t+'.bench.entry.o');b=BUILD/(t+'.bench');vector=['-fopt-info-vec-optimized'] if cc=='gcc' else ['-Rpass=loop-vectorize','-Rpass-missed=loop-vectorize']
                    run(t+'.bench.core',[cc,*flags,*opt,*vector,'-c',PRIVATE/'alias.c','-o',core]);run(t+'.bench.entry',[cc,*flags,*opt,'-c',PUBLIC/'bench.c','-o',entry]);run(t+'.bench.link',[cc,*opt,core,FD,'-I'+str(FDINC),entry,'-o',b])
                    run(t+'.measure',[sys.executable,PRIVATE/'measure.py',b,BUILD/(t+'.timing.json')],marker='"samples": 240');assembly=run(t+'.assembly',['objdump','-dr',b]);assert '<os_transform>' in assembly and '<os_ordinary>' in assembly and ('call' in assembly);run(t+'.sizes',['nm','-S','--size-sort',b],marker='os_transform')
            for opt in ['O0','O3']:
                t=cc+'.arm.'+opt;compiler='arm-none-eabi-gcc' if cc=='gcc' else 'clang';target=[] if cc=='gcc' else ['--target=arm-none-eabi'];opts=[*target,'-mcpu=cortex-m4','-mthumb','-'+opt,'-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections'];objects=[]
                for name,source in [('core',PRIVATE/'alias.c'),('foundation',FD),('entry',PUBLIC/'target.c')]:
                    obj=BUILD/(t+'.'+name+'.o');objects.append(obj);run(t+'.'+name,[compiler,*flags,*opts,'-c',source,'-o',obj])
                run(t+'.objects',['arm-none-eabi-nm','-u',*objects]);image=BUILD/(t+'.elf');link=[compiler,*opts,'-nostdlib',*(['--ld-path='+str(LLD)] if cc=='clang' else []),'-Wl,--gc-sections,-e,_start,-Ttext=0x10000,-Map='+str(BUILD/(t+'.map'))]
                run(t+'.link',[*link,*objects,'-o',image]);assert not run(t+'.undefined',['arm-none-eabi-nm','-u',image]).strip()
                symbols=run(t+'.symbols',['arm-none-eabi-nm',image]);assert all(n in symbols for n in ['fd_slot_initialize','fd_move','fd_pool_release','os_wide_reset','os_clone','os_transform','os_target_result']);assert not any(n in symbols for n in ['__aeabi_','malloc','memcpy','memmove','memset','__atomic_'])
                run(t+'.assembly',['arm-none-eabi-objdump','-dr',image],marker='<os_clone>');bad=BUILD/(t+'.bad.o');run(t+'.bad.entry',[compiler,*flags,*opts,'-DOS_TARGET_HELPER','-c',PUBLIC/'target.c','-o',bad]);run(t+'.bad.link',[*link,*objects[:2],bad,'-o',BUILD/(t+'.bad.elf')],1,'malloc')
        narrow=['-m32','--sysroot='+str(SYSROOT),'-isystem',str(SYSROOT/'usr/include/x86_64-linux-gnu'),'-B'+str(GCCDIR)+'/','-L'+str(LIBS),'-Wl,--dynamic-linker='+str(LOADER)]
        output=run('i386.dependencies',['gcc',*flags,*narrow,'-M',PRIVATE/'alias.c',FD,PUBLIC/'tests.c'])
        for token in output.replace('\\\n',' ').split():
            p=pathlib.Path(token)
            if p.is_absolute() and p.is_file():deps[str(p)]=sha(p)
        b=compile('i386','gcc',[*narrow,'-O2']);run('i386.elf',['objdump','-f',b],marker='elf32-i386');prefix=[LOADER,'--library-path',LIBS]
        for i,c in enumerate(CASES):run('i386.'+c,[*prefix,b,i],marker='OS_'+c.upper()+'_PASS')
        probe=compile('i386.probe','gcc',[*narrow,'-O2'],test='probe.c');run('i386.oracle',[sys.executable,PRIVATE/'oracle.py',probe,BUILD/'i386.packets.json',json.dumps(list(map(str,prefix)))],marker='"range_cases": 6048')
        assert set(q['id'] for q in h.COMMANDS)==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext or any(sha(pathlib.Path(n))!=v for n,v in deps.items()):failure='Scoped source/tool/header identity changed'
    limits=['Strict profile C core imports actual FDSlot/fd_move/FDPool; hosted allocation, clocks, libc and sanitizers are verification dependencies only.','Eight native compiler/optimization profiles, one actual i386 ABI and four CortexM4 final-link audits; target images are not executed.','Invalid effective-type/restrict uses remain analysis-only. The may_alias experiment is a named GCC/Clang extension, not ISO C.','Truthful allocator size/alignment and actual allocated storage are preconditions; no portable validation of arbitrary pointers or malicious callbacks.','Fallible destructor failure retains resources; callbacks must preserve retryability and cannot reenter or retain freed storage.','Generation collision is a bounded logical token demonstration, not evaluation of a pointer after C lifetime ends.','Four timing experiments retain240 raw samples each and equal disjoint correctness contracts; no universal speedup, hardware timing stability or mastery claim.','N1570 C11 draft and N2219/N2263 nonnormative proposals are distinct; published C17/C23 remain source gaps.','External review, independent delayed learner attempts and physical observation are unperformed.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,dependency_sha256=deps,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
