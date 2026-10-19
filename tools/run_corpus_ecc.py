"""Established six-profile campaign plus scoped strict target object/link audits."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import run_expansion_family as base
ROOT=pathlib.Path(__file__).resolve().parents[1]
DEFAULT='reviews/v5/expansion/corpus_ecc/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(report):
    extra=[pathlib.Path(__file__),ROOT/'tools/check_corpus_ecc.py',ROOT/'tools/run_expansion_family.py',ROOT/'tools/verify_corpus_ecc_fixtures.py']
    extra_pins={p.relative_to(ROOT).as_posix():sha(p) for p in extra}
    result=base.run('corpus_ecc',report);path=ROOT/report;d=json.loads(path.read_text(encoding='utf8'))
    d['inspected_paths'].update(extra_pins);d['target_audits']=[];outputs={};build=ROOT/'build/corpus-ecc-target';build.mkdir(parents=True,exist_ok=True)
    def execute(args,name):
        r=subprocess.run([str(x) for x in args],cwd=ROOT,capture_output=True,text=True,timeout=90)
        d['executions'].append(dict(command=[str(x) for x in args],stdout=r.stdout+'\n'+r.stderr,exit_code=r.returncode,expected_exit=0,passed=r.returncode==0,id=name))
        assert r.returncode==0,(name,r.stdout,r.stderr)
        return r.stdout
    try:
        if result:raise RuntimeError('Hosted campaign failed')
        for cc in ['arm-none-eabi-gcc','clang']:
            execute([cc,'--version'],cc+'.version')
            for opt in ['-O0','-O3']:
                tag=cc+opt;obj=build/(tag+'.o');image=build/(tag+'.elf');mapping=build/(tag+'.map')
                execute([cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-I',ROOT/'labs/corpus_ecc','-c',ROOT/'assessor/corpus_ecc/ecc.c','-o',obj],tag+'.compile')
                assert not execute(['arm-none-eabi-nm','-u',obj],tag+'.undefined').strip()
                execute(['arm-none-eabi-ld','--entry=ec_decode','-Map='+str(mapping),obj,'-o',image],tag+'.link')
                assert not execute(['arm-none-eabi-nm','-u',image],tag+'.link-undefined').strip()
                text=execute(['arm-none-eabi-objdump','-dr',image],tag+'.disassembly');assert '<ec_decode>' in text and '<ec_interleave_phase>' in text
                for p in [obj,image,mapping]:outputs[p.relative_to(ROOT).as_posix()]=sha(p)
                d['target_audits'].append(dict(compiler=cc,optimization=opt,language='c17',undefined_symbols=[],execution='not-run',libraries=[]))
        for p,h in d['inspected_paths'].items():assert sha(ROOT/p)==h,('changed input',p)
    except Exception as error:
        d['passed']=False;d['failure']=str(error)
    d['status']='passed' if d['passed'] else 'failed';d['generated_artifact_sha256']=outputs
    d['scope']='Six C17 host profiles, exact compiled controls, independent small-code nearest-codeword oracle, and four strict M4 object/link audits. No target execution or physical channel observation.'
    path.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=d['passed'],profiles=len(d['profiles']),commands=len(d['executions']),failure=d.get('failure'))));return int(not d['passed'])
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();return run(a.report)
if __name__=='__main__':raise SystemExit(main())
