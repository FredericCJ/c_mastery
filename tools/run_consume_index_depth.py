"""Run only the original consume/index companion; never execute ordinary races."""
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import time

ROOT=pathlib.Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/consume_index_depth'
PRIVATE=ROOT/'assessor/consume_index_depth'
HERE=ROOT/'research/v5/expansion/consume_index_depth'
BUILD=ROOT/'build/consume-index-depth'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
    files=[p for folder in [PUBLIC,PRIVATE] for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [pathlib.Path(__file__),ROOT/'tools/check_consume_index_depth.py',HERE/'sources.md',HERE/'resources.json']
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}

def main():
    BUILD.mkdir(parents=True,exist_ok=True); before=pins();commands=[];outputs={};observations=[];failure=None
    def run(name,args,code=0,marker=None):
        args=[str(x) for x in args];started=time.time()
        result=subprocess.run(args,cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=90,
                              env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
        passed=result.returncode==code and (marker is None or marker in result.stdout)
        commands.append(dict(id=name,command=args,expected_returncode=code,returncode=result.returncode,
                             marker=marker,stdout=result.stdout,passed=passed,seconds=time.time()-started))
        if not passed: raise RuntimeError(name+': '+result.stdout[-2000:])
        return result.stdout
    try:
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                tag=cc+'.'+profile
                common=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I',PUBLIC]
                binary=BUILD/tag
                run(tag+'.compile',common+[PRIVATE/'consume_index.c',PUBLIC/'tests.c','-o',binary])
                outputs[binary.relative_to(ROOT).as_posix()]=sha(binary)
                for case in ['value','branch','other']:run(tag+'.'+case,[binary,case],marker='CI_PASS')
                run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',binary],marker='"graphs": 729')
                stub=BUILD/(tag+'.learner')
                run(tag+'.learner.compile',common+[PUBLIC/'learner.c',PUBLIC/'tests.c','-o',stub])
                for case in ['value','branch','other']:run(tag+'.learner.'+case,[stub,case],41,'CI_CHECK:')
                for number,case in [(1,'value'),(2,'branch'),(3,'graphs')]:
                    mutant=BUILD/(tag+'.mutant'+str(number))
                    run(tag+'.mutant'+str(number)+'.compile',common+['-DCI_MUTATION='+str(number),PRIVATE/'consume_index.c',PUBLIC/'tests.c','-o',mutant])
                    if case=='graphs':run(tag+'.mutant3.reject',[sys.executable,PRIVATE/'oracle.py',mutant],41,'CI_ORACLE:')
                    else:run(tag+'.mutant'+str(number)+'.reject',[mutant,case],41,'CI_CHECK:')
                observations.append(dict(profile=tag,language='c17',functional_cases=3,independent_graphs=729,compiled_controls=3,learner_rejections=3))
                if profile!='sanitized':
                    obj=BUILD/(tag+'.o');run(tag+'.object',common+['-c',PRIVATE/'consume_index.c','-o',obj])
                    outputs[obj.relative_to(ROOT).as_posix()]=sha(obj)
                    run(tag+'.disassembly',['objdump','-dr',obj],marker='<ci_branch_index>')
        for cc in ['arm-none-eabi-gcc','clang']:
            for opt in ['-O0','-O3']:
                tag=cc+'.m4'+opt
                args=[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Wpedantic','-Werror','-I',PUBLIC]
                obj=BUILD/(tag+'.o');image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map')
                run(tag+'.compile',args+['-c',PRIVATE/'consume_index.c','-o',obj])
                assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',obj]).strip()
                run(tag+'.link',['arm-none-eabi-ld','--entry=ci_pointer','-Map='+str(mapping),obj,'-o',image])
                assert not run(tag+'.link-undefined',['arm-none-eabi-nm','-u',image]).strip()
                assembly=run(tag+'.disassembly',['arm-none-eabi-objdump','-dr',image],marker='<ci_acquire_branch>')
                for symbol in ['ci_pointer','ci_value_index','ci_branch_index','ci_killed_index','ci_acquire_branch']:
                    section=run(tag+'.'+symbol,['arm-none-eabi-objdump','-dr','--disassemble='+symbol,image],marker='<'+symbol+'>')
                    assert re.search(r'\bdmb\b',section),(tag,symbol,'expected observed strengthened lowering changed')
                for p in [obj,image,mapping]:outputs[p.relative_to(ROOT).as_posix()]=sha(p)
                observations.append(dict(profile=tag,language='c17',object_link_audit='zero undefined symbols; DMB observed in each accessor',execution='not-run'))
        # An actual isolated public export with no hidden assessor fallback.
        export=BUILD/'isolated-public'
        export.mkdir(exist_ok=True)
        for p in PUBLIC.iterdir():
            if p.is_file():shutil.copy2(p,export/p.name)
        result=run('export.learner',[sys.executable,export/'run.py'],1,'CI_CHECK:')
        run('export.explicit-reference',[sys.executable,export/'run.py','--source',PRIVATE/'consume_index.c'],marker='Functional cases passed;')
        for p in export.iterdir():
            if p.is_file():outputs[p.relative_to(ROOT).as_posix()]=sha(p)
    except Exception as error:failure=str(error)
    after=pins()
    if before!=after:failure='Scoped source changed during campaign'
    report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,
                commands=commands,observations=observations,outputs_sha256=outputs,
                limits=['Defined immutable-data execution only; flawed concurrent ordinary reads are analysis-only.',
                        'Finite labeled DAG oracle is not a C parser or full memory-model checker.',
                        'M4 objects/links/disassembly were inspected, not executed; no physical or independent learner/reviewer evidence.',
                        'N1570 is a C11 committee draft; exact final C17/C23 texts remain unavailable.'])
    HERE.mkdir(parents=True,exist_ok=True);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(status=report['status'],commands=len(commands),failure=failure)))
    return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
