"""Actual array/cursor models, local loader boundary and strict target closure."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_object_cursor';PUBLIC=ROOT/'labs/object_followthrough/cursor';PRIVATE=ROOT/'assessor/object_followthrough/cursor';BUILD=ROOT/'build'/('object-cursor-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
FD=ROOT/'assessor/foundation_depth/depth.c';FDINC=ROOT/'labs/foundation_depth/include'
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['matrix','allocated','cursors','order','conversions']
MUTANTS={'ROW':(0,'oc_read(&m,1,2,&out)==0 && out==6'),'REVERSE':(2,'oc_previous(&c,3,&p)==FD_RANGE && c.position==2 && p==a+2'),'FRAGMENT':(3,'oc_order(lengths,3,(OcPosition){0,2},(OcPosition){2,0},&order)==0 && order==0'),'NULL':(4,'oc_decode_index(values,3,wire,&out)==0 && out==NULL'),'FD_CURSOR':(2,'oc_take(&c,4,&p)==FD_RANGE && p==a && c.position==1')}
SYSROOT=ROOT/'build/posix-io-toolchain/sysroot';LIBS=SYSROOT/'usr/lib32';GCCDIR=SYSROOT/'usr/lib/gcc/x86_64-linux-gnu/15/32';LOADER=LIBS/'ld-linux.so.2';LLD=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
IMPORTS=['assessor/foundation_depth/depth.c','labs/foundation_depth/include/depth.h','labs/foundation_depth/src/depth.c','labs/foundation_depth/CONTRACTS.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),LOADER,LIBS/'libc.so.6',LLD.resolve()})}
def expected_commands():
    s={'kernel','libc','i386.compile','i386.probe.compile','i386.oracle','i386.elf','i386.dependencies'}|{'i386.'+c for c in CASES}
    for cc in ['gcc','clang']:
        s|={cc+'.version',cc+'.negative.table',cc+'.negative.zero',cc+'.negative.function'}
        for p,_ in PROFILES:
            t=cc+'.'+p;s|={t+'.'+q for q in ['compile','probe.compile','oracle','stub.compile',*CASES,*['stub.'+c for c in CASES]]}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
            s|={t+'.plugin.'+v for v in ['valid','abi','size','missing']}
            for v in ['reference','failure','CLOSE','RETURN','stub']:s|={t+'.loader.'+v+'.compile',t+'.loader.'+v}
        for opt in ['O0','O3']:
            t=cc+'.arm.'+opt;s|={t+'.'+q for q in ['core','foundation','entry','objects','link','undefined','symbols','assembly','bad.entry','bad.link']}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None;deps={}
    flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(FDINC)]
    def compile(tag,cc,opt,source=None,test='tests.c'):
        b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,source or PRIVATE/'cursor.c',FD,PUBLIC/test,'-o',b]);return b
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for p,opt in PROFILES:
                t=cc+'.'+p;b=compile(t,cc,opt)
                for i,c in enumerate(CASES):run(t+'.'+c,[b,i],marker='OC_'+c.upper()+'_PASS')
                probe=compile(t+'.probe',cc,opt,test='probe.c');run(t+'.oracle',[sys.executable,PRIVATE/'oracle.py',probe,BUILD/(t+'.packets.json')],marker='"interval_cases": 6435')
                stub=compile(t+'.stub',cc,opt,PUBLIC/'learner.c')
                for i,c in enumerate(CASES):run(t+'.stub.'+c,[stub,i],41,'OC_ALLOCATED_ASSERT:' if c=='allocated' else 'OC_ASSERT:')
                for m,(i,marker) in MUTANTS.items():
                    define='FD_MUTATE_CURSOR' if m=='FD_CURSOR' else 'OC_MUTANT_'+m;b=compile(t+'.mutant.'+m,cc,[*opt,'-D'+define]);run(t+'.mutant.'+m,[b,i],41,'OC_ASSERT:'+marker)
                plugins=[]
                for v,macro in [('valid',None),('abi','OC_PLUGIN_BAD_ABI'),('size','OC_PLUGIN_BAD_SIZE'),('missing','OC_PLUGIN_MISSING')]:
                    so=BUILD/(t+'.'+v+'.so');plugins.append(so);run(t+'.plugin.'+v,[cc,*flags,*opt,'-fPIC','-shared',*(['-D'+macro] if macro else []),PUBLIC/'plugin.c','-o',so])
                for v in ['reference','failure','CLOSE','RETURN','stub']:
                    b=BUILD/(t+'.loader.'+v);sources=[PUBLIC/'loader_learner.c'] if v=='stub' else [PRIVATE/'loader.c',PRIVATE/'function_lookup.c'];opts=[x for x in flags if x!='-Wpedantic'];define=[] if v in ['reference','stub'] else ['-DOC_CLOSE_FAILURE','-Wl,--wrap=dlclose'] if v=='failure' else ['-DOC_MUTANT_'+v]
                    run(t+'.loader.'+v+'.compile',[cc,*opts,*opt,*define,*sources,PUBLIC/'loader_tests.c','-ldl','-o',b]);run(t+'.loader.'+v,[b,*plugins],0 if v in ['reference','failure'] else 41,'OC_LOADER_PASS' if v in ['reference','failure'] else 'OC_LOADER_ASSERT:')
            for name in ['table','zero']:run(cc+'.negative.'+name,[cc,*flags,'-fsyntax-only',PUBLIC/('negative_'+name+'.c')],1,'error:')
            run(cc+'.negative.function',[cc,*flags,'-pedantic-errors','-fsyntax-only',PRIVATE/'function_lookup.c'],1 if cc=='gcc' else 0,'error:' if cc=='gcc' else None)
            for opt in ['O0','O3']:
                t=cc+'.arm.'+opt;compiler='arm-none-eabi-gcc' if cc=='gcc' else 'clang';target=[] if cc=='gcc' else ['--target=arm-none-eabi'];opts=[*target,'-mcpu=cortex-m4','-mthumb','-'+opt,'-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections'];objects=[]
                for name,source in [('core',PRIVATE/'cursor.c'),('foundation',FD),('entry',PUBLIC/'target.c')]:
                    obj=BUILD/(t+'.'+name+'.o');objects.append(obj);run(t+'.'+name,[compiler,*flags,*opts,'-c',source,'-o',obj])
                run(t+'.objects',['arm-none-eabi-nm','-u',*objects]);image=BUILD/(t+'.elf');link=[compiler,*opts,'-nostdlib',*(['--ld-path='+str(LLD)] if cc=='clang' else []),'-Wl,--gc-sections,-e,_start,-Ttext=0x10000,-Map='+str(BUILD/(t+'.map'))]
                run(t+'.link',[*link,*objects,'-o',image]);assert not run(t+'.undefined',['arm-none-eabi-nm','-u',image]).strip()
                symbols=run(t+'.symbols',['arm-none-eabi-nm',image]);assert all(n in symbols for n in ['fd_take','fd_subview','oc_previous','oc_difference','oc_encode_index','oc_target_result'])
                assert not any(n in symbols for n in ['__aeabi_','malloc','memcpy','memmove','memset','dlopen','dlsym','__atomic_'])
                run(t+'.assembly',['arm-none-eabi-objdump','-dr',image],marker='<oc_previous>');bad=BUILD/(t+'.bad.o');run(t+'.bad.entry',[compiler,*flags,*opts,'-DOC_TARGET_HELPER','-c',PUBLIC/'target.c','-o',bad]);run(t+'.bad.link',[*link,*objects[:2],bad,'-o',BUILD/(t+'.bad.elf')],1,'malloc')
        narrow=['-m32','--sysroot='+str(SYSROOT),'-isystem',str(SYSROOT/'usr/include/x86_64-linux-gnu'),'-B'+str(GCCDIR)+'/','-L'+str(LIBS),'-Wl,--dynamic-linker='+str(LOADER)]
        text=run('i386.dependencies',['gcc',*flags,*narrow,'-M',PRIVATE/'cursor.c',FD,PUBLIC/'tests.c'])
        for token in text.replace('\\\n',' ').split():
            p=pathlib.Path(token)
            if p.is_absolute() and p.is_file():deps[str(p)]=sha(p)
        b=compile('i386','gcc',[*narrow,'-O2']);run('i386.elf',['objdump','-f',b],marker='elf32-i386');prefix=[LOADER,'--library-path',LIBS]
        for i,c in enumerate(CASES):run('i386.'+c,[*prefix,b,i],marker='OC_'+c.upper()+'_PASS')
        probe=compile('i386.probe','gcc',[*narrow,'-O2'],test='probe.c');run('i386.oracle',[sys.executable,PRIVATE/'oracle.py',probe,BUILD/'i386.packets.json',json.dumps(list(map(str,prefix)))],marker='"interval_cases": 6435')
        assert set(q['id'] for q in h.COMMANDS)==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext or any(sha(pathlib.Path(n))!=v for n,v in deps.items()):failure='Scoped source/tool/header identity changed'
    limits=['Strict profileC core reuses actual fd_take/fd_subview; hosted allocation, sanitizers and dynamic loader belong only to explicit verification/comparison executables.','Actual GCC/Clang native and i386 executions plus four CortexM4 object/final-link audits; target images are not executed and are not bootable firmware claims.','No arbitrary-pointer admission, outside-row walk, invalid conversion execution or expired-pointer evaluation. Size metadata requires truthful live ownership.','6435 interval cases per oracle profile are finite evidence, not absolute correctness or mastery.','The original local plugin is trusted executable code with a truthful complete table object and matching compiler ABI; version/size checks are not hostile-module validation or sandboxing.','ISO permits object/void and compatible restored function roundtrips; actual Linux function lookup relies on a named platform contract, not ISO C.','Final published C17/C23 and direct OpenGroup pages remain access gaps; Linux man-pages6.19 is the inspected loader authority.','External review, independent delayed learner attempts and physical observations remain unperformed.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,dependency_sha256=deps,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
