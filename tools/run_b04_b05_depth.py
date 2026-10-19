#!/usr/bin/env python3
"""Exact host campaigns for additive depth families, never a target receipt."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
CASES={'b04_depth':['serial','certificate','framing','dma','memory'],
       'b05_depth':['verify','recover','copy','timer','response','grace','request'],
       'b045_systems':['pipeline','i2c','checksum','paired','reduce','replace','wait','shoot','peer']}

def main(default=None):
    p=argparse.ArgumentParser();p.add_argument('--family',choices=CASES,default=default);p.add_argument('--report',type=Path)
    p.add_argument('--build',type=Path,help='Fresh scoped build directory; preserve prior raw campaign artifacts')
    args=p.parse_args()
    if not args.family:p.error('--family is required')
    family=args.family;build=(args.build.resolve() if args.build else ROOT/'build'/family/'campaign');build.mkdir(parents=True,exist_ok=True)
    research='b04/systems' if family=='b045_systems' else family[:3]
    inspected={str(x.relative_to(ROOT)).replace(os.sep,'/'):hashlib.sha256(x.read_bytes()).hexdigest()
               for base in [ROOT/'labs'/family,ROOT/'assessor'/family,ROOT/'fixtures'/family,ROOT/'research/v5/expansion'/research]
               for x in base.rglob('*') if x.is_file() and '__pycache__' not in x.parts}
    for name in ['tools/run_b04_b05_depth.py','tools/verify_b04_b05_depth.py','tools/verify_b04_b05_sources.py','tools/run_'+family+'.py']:
        inspected[name]=hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
    if family=='b045_systems':inspected['tools/verify_b045_systems.py']=hashlib.sha256((ROOT/'tools/verify_b045_systems.py').read_bytes()).hexdigest()
    for path in json.loads((ROOT/'research/v5/expansion'/research/'research_gate.json').read_text())['research_input_sha256']:
        inspected[path]=hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
    report={'schema_version':5,'family':family,'status':'failed','passed':False,'inspected_paths':inspected,'commands':[],
            'limits':['Core observations are host models; b05 request successor separately binds actual QEMU guest evidence. No physical validation.','Mutation sensitivity is not completeness or independent learner assessment.']}
    try:
        from v5_common import identity
        report['input_sha256']=identity()['sha256']
    except ImportError:pass
    def run(command,expected=0,marker=None):
        r=subprocess.run([str(x) for x in command],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=1800 if any('run_request_adapters.py' in str(x) for x in command) else 240)
        index=len(report['commands']);log=build/f'{index:03}.log';log.write_text(r.stdout)
        ok=r.returncode==expected and (marker is None or marker in r.stdout)
        report['commands'].append({'command':[str(x) for x in command],'returncode':r.returncode,'expected':expected,'passed':ok,
                                   'log':log.relative_to(ROOT).as_posix(),'stdout':r.stdout})
        if not ok:raise RuntimeError(f'Command {index} failed: {command}: {r.stdout[-1500:]}')
        return r.stdout
    try:
        run([sys.executable,ROOT/'tools/verify_b04_b05_sources.py',research])
        for cc in ['gcc','clang']:
            run([cc,'--version'])
            for mode in ['Debug','Release','Sanitize']:
                folder=build/(cc+'-'+mode.lower())
                run(['cmake','-S',ROOT/'labs'/family,'-B',folder,'-DCMAKE_C_COMPILER='+cc,
                     '-DCMAKE_BUILD_TYPE='+('Debug' if mode=='Sanitize' else mode),
                     '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'/family),
                     '-DCMASTERY_SANITIZE='+('ON' if mode=='Sanitize' else 'OFF')])
                run(['cmake','--build',folder,'-j4'])
                run(['ctest','--test-dir',folder,'--output-on-failure'])
        stub=build/'stub'
        run(['cmake','-S',ROOT/'labs'/family,'-B',stub,'-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='])
        run(['cmake','--build',stub,'-j4'])
        for case in CASES[family]:run([stub/'depth-tests',case],1,'DEPTH_CHECK:')
        shared=build/'oracle.so'
        source='systems.c' if family=='b045_systems' else 'depth.c'
        run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-shared','-fPIC','-O2','-I'+str(ROOT/'labs'/family/'include'),ROOT/'assessor'/family/source,'-o',shared])
        oracle=run([sys.executable,ROOT/'tools/verify_b045_systems.py',shared] if family=='b045_systems' else
                   [sys.executable,ROOT/'tools/verify_b04_b05_depth.py',family,shared])
        report['oracle']=json.loads(oracle)
        if family=='b05_depth':
            receipt=ROOT/'reviews/v5/expansion/followthrough/request_adapter_campaign.json'
            run([sys.executable,ROOT/'tools/run_request_adapters.py','--report',receipt])
            run([sys.executable,ROOT/'tools/check_request_adapters.py'])
            report['request_adapter_campaign']={'path':receipt.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(receipt.read_bytes()).hexdigest()}
        if any(hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=h for path,h in inspected.items()):raise RuntimeError('Inputs changed during campaign')
        report.update(status='passed',passed=True)
    except Exception as e:report['error']=str(e)
    dest=args.report or ROOT/'build'/family/'report.json';dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'family':family,'passed':report['passed'],'report':str(dest),'error':report.get('error')}))
    return 0 if report['passed'] else 1
if __name__=='__main__':sys.exit(main())
