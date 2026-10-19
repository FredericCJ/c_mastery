"""Actual PR-T02-04 translation, linkage, diagnostic and runtime experiments."""
import datetime,hashlib,json,os,pathlib,shutil,subprocess,sys,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/legacy_language_headers'
PUBLIC=ROOT/'labs/language_followthrough/headers';PRIVATE=ROOT/'assessor/language_followthrough/headers';PROBES=PUBLIC/'probes'
BUILD=ROOT/'build'/('header-definitions-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
DIALECTS=[('c17',['-std=c17']),('gnu17',['-std=gnu17']),('gnu89-inline',['-std=gnu17','-fgnu89-inline'])]
MUTANTS={'HELPER':(0,'hh_a_address()(7) == 22 && hh_b_call(7) == 22'),'SHARED':(1,'a != b && *a == 0 && *b == 0'),'OUTPUT':(0,'hh_read(a, 4, 4, &out) == -1 && out == 40')}
COMMANDS=[];OBSERVATIONS=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__)])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','ar']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def run(ident,args,expected=0,marker=None,locale='C',cwd=ROOT):
    env=os.environ.copy()
    for k in ['CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS']:env.pop(k,None)
    env.update(LC_ALL=locale,ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:abort_on_error=0:exitcode=77',UBSAN_OPTIONS='halt_on_error=1:exitcode=77')
    start=time.monotonic();p=subprocess.run(list(map(str,args)),cwd=cwd,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=40)
    out=BUILD/(ident+'.log');out.write_text(p.stdout,encoding='utf8');ok=(expected is None or p.returncode==expected) and (marker is None or marker in p.stdout)
    COMMANDS.append(dict(id=ident,command=list(map(str,args)),cwd=str(cwd),locale=locale,exit_code=p.returncode,expected_exit=expected,required_marker=marker,passed=ok,output=out.relative_to(ROOT).as_posix(),output_sha256=sha(out),elapsed_seconds=time.monotonic()-start));print(ident,'PASS' if ok else 'FAIL',flush=True)
    if not ok:raise RuntimeError(ident+'\n'+p.stdout[-3000:])
    return p.stdout,p.returncode
def expected_commands():
    s={'kernel','libc','locales','analyzer.capability','analyzer.unknown','analyzer.known','analyzer.repaired'}
    for cc in ['gcc','clang']:
        s.add(cc+'.version')
        for profile,_ in PROFILES:
            t=cc+'.'+profile;s|={t+'.'+n for n in ['compile','definitions','state','stub.compile','stub.definitions','stub.state']}
            for m in MUTANTS:s|={t+'.mutant.'+m+'.compile',t+'.mutant.'+m}
        for opt in ['O0','O3']:
            t=cc+'.static.'+opt;s|={t+'.'+n for n in ['a','b','external','a.nm','b.nm','external.nm','symbols','missing','duplicate']}
            for dialect,_ in DIALECTS:
                for form in ['inline','extern']:
                    t=cc+'.'+dialect+'.'+opt+'.'+form;s|={t+'.'+n for n in ['a','b','a.nm','b.nm','symbols','link']}
                    emits=(form=='extern')!=(dialect=='gnu89-inline')
                    if not emits:s|={t+'.definition',t+'.repaired.link',t+'.repaired.run'}
                t=cc+'.'+dialect+'.'+opt+'.callonly';s|={t+'.'+n for n in ['a','b','link']}
        s|={cc+'.header.'+n for n in ['first','reverse','masked','missing','object','archive','installed.compile','installed.run']}
        for locale in ['C','C.utf8']:
            for n in ['syntax','redeclaration','arity','type','repair']:s.add(cc+'.diagnostic.'+locale+'.'+n)
        for n in ['syntax','redeclaration','arity','type','repair','runtime']:s.add(cc+'.permissive.'+n)
        s|={cc+'.'+n for n in ['constraint.warning','extension.strict','extension.gnu','runtime.compile','runtime.valid','runtime.sanitized.compile','runtime.invalid']}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();failure=None
    strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic']
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version']);loc,_=run('locales',['locale','-a']);assert 'C.utf8' in loc.splitlines()
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for profile,flags in PROFILES:
                t=cc+'.'+profile
                def build(tag,stub=False,extra=()):
                    exe=BUILD/tag;inc=PUBLIC if stub else PRIVATE
                    run(tag+'.compile',[cc,*strict,*flags,*extra,'-I'+str(inc),PUBLIC/'consumer_a.c',PUBLIC/'consumer_b.c',PUBLIC/'tests.c',PUBLIC/'learner.c' if stub else PRIVATE/'headers.c','-o',exe]);return exe
                exe=build(t);run(t+'.definitions',[exe,0],marker='HH_DEFINITIONS_PASS');run(t+'.state',[exe,1],marker='HH_LOCAL_STATE_PASS')
                exe=build(t+'.stub',True)
                for i,name in enumerate(['definitions','state']):run(t+'.stub.'+name,[exe,i],41,'HH_ASSERT:')
                for m,(selector,marker) in MUTANTS.items():exe=build(t+'.mutant.'+m,extra=['-DHH_MUTANT_'+m]);run(t+'.mutant.'+m,[exe,selector],41,'HH_ASSERT:'+marker)
            for opt in ['O0','O3']:
                t=cc+'.static.'+opt;objects=[]
                for part,source in [('a',PUBLIC/'consumer_a.c'),('b',PUBLIC/'consumer_b.c'),('external',PRIVATE/'headers.c')]:
                    obj=BUILD/(t+'.'+part+'.o');objects.append(obj);run(t+'.'+part,[cc,*strict,'-'+opt,'-I'+str(PRIVATE),'-c',source,'-o',obj]);run(t+'.'+part+'.nm',['nm','-P',obj])
                run(t+'.symbols',[sys.executable,PRIVATE/'symbols.py','static',BUILD/(t+'.a.nm.log'),BUILD/(t+'.b.nm.log'),BUILD/(t+'.external.nm.log')],marker='"status": "pass"')
                run(t+'.missing',[cc,*strict,'-I'+str(PRIVATE),PUBLIC/'tests.c',*objects[:2],'-o',BUILD/(t+'.missing')],1,'hh_read')
                run(t+'.duplicate',[cc,*strict,'-I'+str(PRIVATE),PUBLIC/'tests.c',*objects,objects[2],'-o',BUILD/(t+'.duplicate')],1,'hh_read')
                for dialect,dflags in DIALECTS:
                    for form in ['inline','extern']:
                        tag=cc+'.'+dialect+'.'+opt+'.'+form;defines=['-DHH_EXTERN='+str(int(form=='extern')),'-DHH_CALL_ONLY=0'];objs=[]
                        for part in ['a','b']:
                            obj=BUILD/(tag+'.'+part+'.o');objs.append(obj);run(tag+'.'+part,[cc,*dflags,'-'+opt,'-Wall','-Wextra','-Werror',*defines,'-c',PROBES/('inline_'+part+'.c'),'-o',obj]);run(tag+'.'+part+'.nm',['nm','-P',obj])
                        emits=(form=='extern')!=(dialect=='gnu89-inline');kind='T' if emits else 'U'
                        run(tag+'.symbols',[sys.executable,PRIVATE/'symbols.py',kind,BUILD/(tag+'.a.nm.log'),BUILD/(tag+'.b.nm.log')],marker='"expected": "'+kind+'"')
                        run(tag+'.link',[cc,*dflags,*defines,PROBES/'inline_main.c',*objs,'-o',BUILD/tag],1,'hh_experiment')
                        if not emits:
                            obj=BUILD/(tag+'.definition.o');run(tag+'.definition',[cc,*dflags,'-'+opt,'-c',PROBES/'inline_external.c','-o',obj]);exe=BUILD/(tag+'.repaired');run(tag+'.repaired.link',[cc,*dflags,*defines,PROBES/'inline_main.c',*objs,obj,'-o',exe]);run(tag+'.repaired.run',[exe])
                        OBSERVATIONS.append(dict(id=tag,expected_definition=kind,missing_or_duplicate='duplicate' if emits else 'missing',address_taken=True))
                    tag=cc+'.'+dialect+'.'+opt+'.callonly';objs=[]
                    for part in ['a','b']:
                        obj=BUILD/(tag+'.'+part+'.o');objs.append(obj);run(tag+'.'+part,[cc,*dflags,'-'+opt,'-DHH_EXTERN=0','-DHH_CALL_ONLY=1','-c',PROBES/('inline_'+part+'.c'),'-o',obj])
                    _,code=run(tag+'.link',[cc,*dflags,'-DHH_CALL_ONLY=1',PROBES/'inline_main.c',*objs,'-o',BUILD/tag],None)
                    assert code in [0,1];OBSERVATIONS.append(dict(id=tag,observed_link_exit=code,executed=False,iso_external_definition_obligation='still present; no definition supplied',gnu89_limit='duplicate-definition dialect experiment' if dialect=='gnu89-inline' else None))
            for name,file in [('first','header_first.c'),('reverse','header_reverse.c'),('masked','missing_masked.c'),('missing','missing_first.c')]:run(cc+'.header.'+name,[cc,*strict,'-I'+str(PRIVATE),'-fsyntax-only',PROBES/file],1 if name=='missing' else 0,'size_t' if name=='missing' else None)
            install=BUILD/(cc+'.install');install.mkdir();shutil.copyfile(PRIVATE/'headers.h',install/'headers.h');obj=install/'headers.o';archive=install/'libhh.a'
            run(cc+'.header.object',[cc,*strict,'-O2','-I'+str(install),'-c',PRIVATE/'headers.c','-o',obj]);run(cc+'.header.archive',['ar','rcs',archive,obj]);shutil.copyfile(PROBES/'installed.c',install/'consumer.c')
            run(cc+'.header.installed.compile',[cc,*strict,'-I.', 'consumer.c','libhh.a','-o','consumer'],cwd=install);run(cc+'.header.installed.run',[install/'consumer'],cwd=install)
            for locale in ['C','C.utf8']:
                for name in ['syntax','redeclaration','arity','type','repair']:
                    text,_=run(cc+'.diagnostic.'+locale+'.'+name,[cc,*strict,'-fsyntax-only',PROBES/(name+'.c')],0 if name=='repair' else 1,None if name=='repair' else name+'.c:',locale)
                    if name!='repair':
                        assert 'error:' in text
                        assert {'syntax':'expected','redeclaration':'hh_conflict','arity':'hh_arity','type':'int-conversion'}[name] in text
                        assert not any(x in text.lower() for x in ['no such file','unknown argument','unrecognized command-line'])
            for name in ['syntax','redeclaration','arity','type','repair','runtime']:
                run(cc+'.permissive.'+name,[cc,'-std=gnu17','-Wno-error=int-conversion','-fsyntax-only',PROBES/(name+'.c')],1 if name in ['syntax','redeclaration','arity'] else 0, name+'.c:' if name in ['syntax','redeclaration','arity','type'] else None)
            run(cc+'.constraint.warning',[cc,'-std=c17','-pedantic','-Wno-error=int-conversion','-fsyntax-only',PROBES/'type.c'],0,'int-conversion')
            run(cc+'.extension.strict',[cc,*strict,'-fsyntax-only',PROBES/'extension.c'],1,'extension.c:');run(cc+'.extension.gnu',[cc,'-std=gnu17','-Wall','-Wextra','-Werror','-fsyntax-only',PROBES/'extension.c'])
            exe=BUILD/(cc+'.runtime');run(cc+'.runtime.compile',[cc,*strict,'-O0',PROBES/'runtime.c',PROBES/'runtime_main.c','-o',exe]);run(cc+'.runtime.valid',[exe,3],marker='HH_VALUE:40')
            exe=BUILD/(cc+'.runtime.sanitized');run(cc+'.runtime.sanitized.compile',[cc,*strict,'-O1','-g','-fsanitize=address','-fno-omit-frame-pointer',PROBES/'runtime.c',PROBES/'runtime_main.c','-o',exe]);run(cc+'.runtime.invalid',[exe,4],77,'AddressSanitizer: stack-buffer-overflow')
        run('analyzer.capability',['clang','-cc1','-analyzer-checker-help'],marker='security.ArrayBound')
        for mode,name in [(0,'unknown'),(1,'known'),(2,'repaired')]:
            text,_=run('analyzer.'+name,['clang','--analyze','-std=c17','-Xanalyzer','-analyzer-checker=security.ArrayBound','-Xanalyzer','-analyzer-output=text','-DHH_MODEL='+str(mode),PROBES/'analyzer.c'],marker='security.ArrayBound' if mode==1 else None)
            assert ('security.ArrayBound' in text)==(mode==1);OBSERVATIONS.append(dict(id='analyzer.'+name,warning=mode==1,scope='selected Clang checker with stated path constraint; silence is not memory safety'))
        assert {q['id'] for q in COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if before!=pins() or ext!=external():failure='Scoped source identity changed'
    limits=['Hosted C17 baseline; GNU17 and explicit -fgnu89-inline are isolated dialect experiments. N1570 is inspected C11 draft, not exact published C17/C23.','GNU89 external-body behavior and ELF nm letters are implementation observations; ISO obligations are classified separately.','Call-only missing-definition executables are never executed even if optimization permits linking.','Only original local array target receives invalid index4, in a separate AddressSanitizer process; no arbitrary-code-execution claim.','C and C.utf8 are distinct installed locales but both produce English diagnostics here; no untranslated language coverage claim.','Static analyzer uncertainty is demonstrated, not a proof of absence; finite compiler/ABI/input matrix does not establish mastery.','No external review, independent delayed learner assessment, or physical observation.']
    report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=COMMANDS,observations=OBSERVATIONS,limits=limits)
    (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
