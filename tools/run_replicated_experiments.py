"""Execute bounded T76 mutation, resource and local effect-adapter experiments."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from common import ROOT, dump, sha
from run_expansion_family import family_inputs
from v5_common import pins

def main():
    report=ROOT/'reviews/v5/expansion/b07/replicated_experiments.json'
    run_dir=report.parent/'runs'/('replicated-experiments-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run_dir.mkdir(parents=True)
    inputs=family_inputs('replicated')+[Path(__file__).resolve()];source_pins=pins(inputs)
    records=[];checks=[]
    def run(command,label):
        process=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,encoding='utf-8',timeout=120)
        log=run_dir/(f'{len(records):03}-'+label+'.log');log.write_text(process.stdout+'\n'+process.stderr,encoding='utf-8',newline='\n')
        records.append(dict(command=[str(x) for x in command],exit_code=process.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        return process
    def check(name,passed,observation):checks.append(dict(name=name,passed=bool(passed),observation=observation))
    flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/replicated/include','-Ilabs/dod/include']
    core=ROOT/'assessor/replicated/replicated.c';dependency=ROOT/'assessor/dod/dod.c'
    with tempfile.TemporaryDirectory(prefix='cmastery-rp-experiments-') as temp:
        work=Path(temp)
        for index,case in enumerate(['log','projection','clocks','counters','sets','sequence','merkle','wire','replay'],1):
            for mutation in [2*index-1,2*index]:
                target=work/f'mutant-{mutation}'
                compiled=run(['gcc',*flags,'-O0','-g',f'-DMOD_MUTATION={mutation}',core,dependency,'labs/replicated/tests/tests.c','-pthread','-o',target],f'mutant-{mutation}-compile')
                check(f'mutant-{mutation}-compiles',compiled.returncode==0,'Strict C17 compile; a compile failure cannot count as a killed semantic mutant.')
                if compiled.returncode==0:
                    observed=run([target,case],f'mutant-{mutation}-run')
                    check(f'mutant-{mutation}-named-assertion',observed.returncode==1 and 'RP_CHECK line' in observed.stderr,observed.stderr.strip())
        for compiler in ['gcc','clang']:
            obj=work/(compiler+'.o')
            compiled=run([compiler,*flags,'-O2','-fstack-usage','-c',core,'-o',obj],compiler+'-object')
            check(compiler+'-strict-object',compiled.returncode==0,'Hosted core object with optimization and stack-use reporting.')
            if compiled.returncode==0:
                symbols=run(['nm','-u',obj],compiler+'-undefined');names={line.split()[-1] for line in symbols.stdout.splitlines() if line.strip()}
                allowed={'d_hamt_init','d_hamt_get','d_hamt_update','memcpy','memset','memmove','__stack_chk_fail'}
                check(compiler+'-dependency-audit',symbols.returncode==0 and names<=allowed,dict(observed=sorted(names),allowed=sorted(allowed),limits='Hosted dependencies, not strict-freestanding certification; imported producer separately pinned. GCC may inject __stack_chk_fail through its enabled stack protector; this libc/compiler hardening dependency is explicitly allowed and recorded, never described as a runtime-free target. Clock, random and I/O routines remain forbidden.'))
                size=run(['size',obj],compiler+'-object-size');check(compiler+'-object-size-recorded',size.returncode==0,size.stdout.strip())
                stackfiles=list(work.glob(compiler+'*.su'));stack='\n'.join(p.read_text(encoding='utf-8') for p in stackfiles)
                check(compiler+'-stack-recorded',bool(stack),stack)
            adapter=work/(compiler+'-effects')
            compiled=run([compiler,*flags,'-O2',core,dependency,'labs/replicated/tests/effect_adapter.c','-o',adapter],compiler+'-effects-compile')
            check(compiler+'-effect-adapter-compiles',compiled.returncode==0,'Original local synthetic receiver; no external effects.')
            if compiled.returncode==0:
                observed=run([adapter],compiler+'-effects-run')
                check(compiler+'-effect-counterexample-and-repair',observed.returncode==0 and 'deduplicated_effects=8' in observed.stdout,observed.stdout.strip())
    result=dict(schema_version=1,passed=all(c['passed'] for c in checks),checks=checks,commands=records,source_sha256=source_pins,scope_sha256=hashlib.sha256(json.dumps(source_pins,sort_keys=True).encode()).hexdigest(),limits=['Author-executed bounded experiments; no independent learner or human assessment.','Local receiver state is memory only; crash-atomic external effects remain a required unexecuted investigation.','Object size and static stack reports are compiler observations, not end-to-end latency or physical measurements.'])
    dump(report,result);print(json.dumps(dict(passed=result['passed'],checks=len(checks),commands=len(records),report=report.relative_to(ROOT).as_posix())))
    return 0 if result['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
