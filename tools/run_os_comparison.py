"""Run identical bounded Linux/FreeRTOS workloads with pinned external sources."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from common import ROOT, dump
from v2_common import source_identity
COMMIT='0adc196d4bd52a2d91102b525b0aafc1e14a2386'

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--freertos-root',default=str(ROOT/'build/dependencies/freertos-11.2.0'))
    p.add_argument('--native',action='store_true'); args=p.parse_args()
    if os.name=='nt' and not args.native:
        def linux(path): return subprocess.check_output(['wsl','--exec','wslpath','-a',str(path)],text=True).strip()
        raise SystemExit(subprocess.run(['wsl','--exec','python3',linux(ROOT/'tools/run_os_comparison.py'),
            '--native','--freertos-root',linux(Path(args.freertos_root))],cwd=ROOT).returncode)
    identity=source_identity(); dep=Path(args.freertos_root).resolve()
    report={'status':'unavailable','input_sha256':identity['sha256'],'upstream_commit':COMMIT,
            'physical_timing':'not-run','scope':'Actual hosted Linux and FreeRTOS POSIX behavior for the same bounded channel workload.'}
    if not (dep/'tasks.c').is_file():
        report['reason']='Explicit FreeRTOS V11.2.0 source checkout unavailable'
        dump(ROOT/'reviews/v3/os_comparison.json',report); return 77
    actual=subprocess.check_output(['git','-C',str(dep),'rev-parse','HEAD'],text=True).strip()
    dirty=subprocess.check_output(['git','-C',str(dep),'status','--porcelain'],text=True).strip()
    if actual!=COMMIT or dirty: raise ValueError('FreeRTOS checkout is not clean pinned V11.2.0')
    files=['tasks.c','queue.c','list.c','portable/ThirdParty/GCC/Posix/port.c',
           'portable/ThirdParty/GCC/Posix/portmacro.h','portable/ThirdParty/GCC/Posix/utils/wait_for_event.c']
    report['external_sources']={name:hashlib.sha256((dep/name).read_bytes()).hexdigest() for name in files}
    folder=ROOT/'build/comparison'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    commands=[['cmake','-S',str(ROOT/'labs/os_observatory'),'-B',str(folder),'-G','Ninja',
               '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/os_observatory'),'-DCMASTERY_FREERTOS_ROOT='+str(dep)],
              ['cmake','--build',str(folder)],
              ['ctest','--test-dir',str(folder),'-R','comparison','--no-tests=error','--output-on-failure','-V']]
    report['steps']=[]
    for command in commands:
        run=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=120)
        report['steps'].append({'command':command,'exit_code':run.returncode,'stdout':run.stdout,'stderr':run.stderr})
        if run.returncode: break
    passed=all(s['exit_code']==0 for s in report['steps']) and len(report['steps'])==3
    if passed:
        output=report['steps'][-1]['stdout']
        passed=all(token in output for token in ['platform=Linux-pthreads count=256 checksum=32640',
            'platform=FreeRTOS-11.2.0-POSIX count=256 checksum=32640'])
    report['source_changed_during_run']=source_identity()['sha256']!=identity['sha256']
    passed=passed and not report['source_changed_during_run']
    report.update(status='passed' if passed else 'failed',passed=passed)
    dump(ROOT/'reviews/v3/os_comparison.json',report)
    print(json.dumps({k:v for k,v in report.items() if k!='steps'},indent=2)); return 0 if passed else 1
if __name__=='__main__': raise SystemExit(main())
