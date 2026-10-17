"""Audit actual helper dependencies and link closure; no target execution."""
import argparse,datetime,os,subprocess
from pathlib import Path
from common import ROOT,dump,sha
from v5_common import python_command


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if os.name=='nt':
        return subprocess.run(python_command('tools/run_api_depth_experiments.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode
    work=ROOT/'build/api-depth-dependencies';work.mkdir(parents=True,exist_ok=True)
    inputs=['tools/run_api_depth_experiments.py','labs/api_depth/include/api_depth.h','assessor/api_depth/api_depth.c']
    before={p:sha(ROOT/p) for p in inputs};rows=[]
    def run(argv,expected_empty=False):
        p=subprocess.run([str(a) for a in argv],cwd=ROOT,capture_output=True,text=True)
        ok=p.returncode==0 and (not expected_empty or not p.stdout.strip())
        rows.append(dict(command=[str(a) for a in argv],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=ok,requires_empty_stdout=expected_empty))
        return ok
    for compiler,nm,flags in [('gcc','nm',[]),('clang','nm',[]),
                              ('arm-none-eabi-gcc','arm-none-eabi-nm',['-mcpu=cortex-m4','-mthumb'])]:
        obj=work/(compiler+'.o');image=work/(compiler+'.elf')
        run([compiler,'--version'])
        ok=run([compiler,*flags,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
                '-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/api_depth/include','-c',
                'assessor/api_depth/api_depth.c','-o',obj])
        if ok:
            run([nm,'-u',obj],True)
            # A dependency-audit ELF with a named function entry, not bootable firmware.
            if run([compiler,*flags,'-nostdlib','-Wl,-e,api_append','-Wl,-Map,'+str(work/(compiler+'.map')),
                    obj,'-o',image]):run([nm,'-u',image],True)
    stable=before=={p:sha(ROOT/p) for p in inputs}
    passed=stable and all(r['passed'] for r in rows) and len(rows)==15
    report=dict(schema_version=1,passed=passed,status='passed' if passed else 'failed',
                recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                inspected_paths=before,source_stable=stable,commands=rows,
                scope='Actual GCC/Clang hosted and Cortex-M4 object/link helper audit; all functions retained.',
                limits=['No target execution; named-function entry is not a bootable firmware startup.',
                        'Caller callbacks are not linked and require their own transitive dependency audit.',
                        'No stack worst-case, timing, independent review or learner assessment is inferred.'])
    dump(ROOT/args.report,report);print(dict(passed=passed,commands=len(rows)))
    return 0 if passed else 1


if __name__=='__main__':raise SystemExit(main())
