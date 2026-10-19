"""Six actual C17 host profiles and eight strict target object/link audits."""
import argparse
import json
from pathlib import Path
import subprocess
import run_expansion_family as base
from common import ROOT,sha
DEFAULT='reviews/v5/expansion/corpus_limbs/current_validation.json'
def run(report):
    extras=['tools/run_corpus_limbs.py','tools/check_corpus_limbs.py','tools/verify_corpus_limbs.py']
    pins={p:sha(ROOT/p) for p in extras}
    result=base.run('corpus_limbs',report);path=ROOT/report;d=json.loads(path.read_text())
    d['inspected_paths'].update(pins);d['target_audits']=[];artifacts={}
    build=ROOT/'build/corpus-limbs-target';build.mkdir(parents=True,exist_ok=True)
    def execute(args,name):
        args=list(map(str,args));r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=90)
        d['executions'].append(dict(id=name,command=args,exit_code=r.returncode,stdout=r.stdout+r.stderr,passed=r.returncode==0,expected_exit=0))
        assert r.returncode==0,(name,r.stdout,r.stderr)
        return r.stdout
    try:
        assert result==0,'hosted campaign'
        for cc in ['arm-none-eabi-gcc','clang']:
            execute([cc,'--version'],cc+'.version')
            for opt in ['-O0','-O3']:
                for bits in [16,8]:
                    tag=cc+opt+'-'+str(bits);obj=build/(tag+'.o');elf=build/(tag+'.elf');mapping=build/(tag+'.map')
                    execute([cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DBL_BITS='+str(bits),'-I',ROOT/'labs/corpus_limbs','-c',ROOT/'assessor/corpus_limbs/limbs.c','-o',obj],tag+'.compile')
                    assert not execute(['arm-none-eabi-nm','-u',obj],tag+'.undefined').strip()
                    execute(['arm-none-eabi-ld','--entry=bl_div','-Map='+str(mapping),obj,'-o',elf],tag+'.link')
                    assert not execute(['arm-none-eabi-nm','-u',elf],tag+'.link-undefined').strip()
                    asm=execute(['arm-none-eabi-objdump','-dr',elf],tag+'.disassembly');assert '<bl_div>' in asm and '<bl_owner_set>' in asm
                    for p in [obj,elf,mapping]:artifacts[p.relative_to(ROOT).as_posix()]=sha(p)
                    d['target_audits'].append(dict(compiler=cc,optimization=opt,bits=bits,language='c17',execution='not-run',undefined_symbols=[],libraries=[],section_sizes=execute(['arm-none-eabi-size','-A',elf],tag+'.sizes'),stack_usage=obj.with_suffix('.su').read_text(),limits='Actual per-function emitted frames; no physical execution, allocator implementation or whole-caller stack claim.'))
        assert all(sha(ROOT/p)==h for p,h in d['inspected_paths'].items())
    except Exception as error:d['passed']=False;d['failure']=str(error)
    d['status']='passed' if d['passed'] else 'failed';d['generated_artifact_sha256']=artifacts
    d['scope']='Variable-count core, two radices, actual hosted allocation and independent arithmetic; strict target object/link closure only.'
    path.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=d['passed'],commands=len(d['executions']),targets=len(d['target_audits']),failure=d.get('failure'))))
    return int(not d['passed'])
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();return run(a.report)
if __name__=='__main__':raise SystemExit(main())
