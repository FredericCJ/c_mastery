"""Independent scoped kernel regression run; execute in Linux/WSL after freeze."""
from pathlib import Path
from datetime import datetime,timezone
import hashlib,json,os,subprocess

ROOT=Path(__file__).resolve().parents[2]

def hashes():
    return {p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
            for parent in ['labs/kernel_smp','assessor/kernel_smp']
            for p in sorted((ROOT/parent).rglob('*'))
            if p.is_file() and not {'__pycache__','build'}.intersection(p.parts)}

def main():
    before=hashes(); folder=ROOT/'build/kernel-review'
    report={'scope':'Independent host contracts and actual integrated QEMU1/2/4CPU boots, with the supplied finite user regressions',
            'recorded_at':datetime.now(timezone.utc).isoformat(),'inspected_build_inputs':before,'runs':[],
            'physical_timing':'not-run'}
    commands=[['cmake','-S',str(ROOT/'labs/kernel_smp'),'-B',str(folder),'-G','Ninja',
               '-DCMAKE_C_COMPILER=gcc','-DCMASTERY_CPUS=4','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/kernel_smp'),
               '-DCMASTERY_RUNTIME_ROOT='+str(ROOT/'assessor/runtime'),'-DCMASTERY_RUNTIME_INCLUDE='+str(ROOT/'labs/runtime/include'),
               '-DCMASTERY_RUNTIME_ADAPTERS='+str(ROOT/'labs/runtime/adapters'),'-DCMASTERY_STORAGE_ROOT='+str(ROOT/'assessor/storage'),
               '-DCMASTERY_STORAGE_INCLUDE='+str(ROOT/'labs/storage/include')],
              ['cmake','--build',str(folder)],
              ['ctest','--test-dir',str(folder),'-R','^(kernel_smp_(madt|channel)|kernel_(boot|frames|elf|archive|handles|ranges|mutations))$',
               '--no-tests=error','--output-on-failure']]
    for command in commands:
        result=subprocess.run(command,capture_output=True,text=True,timeout=180)
        report['runs'].append({'command':command,'exit_code':result.returncode,'stdout':result.stdout,'stderr':result.stderr})
        print(command[0],result.returncode,flush=True)
        if result.returncode: break
    if all(r['exit_code']==0 for r in report['runs']) and len(report['runs'])==3:
        for cpus in [1,2,4]:
            command=['python3',str(ROOT/'labs/kernel_smp/tools/guest.py'),'test','--require','storage',
                     '--lab',str(ROOT/'labs/kernel_smp'),'--source',str(ROOT/'assessor/kernel_smp'),'--build',str(folder/'guest')]
            result=subprocess.run(command,capture_output=True,text=True,timeout=100,env=dict(os.environ,CMASTERY_CPUS=str(cpus)))
            item={'command':command,'CMASTERY_CPUS':cpus,'exit_code':result.returncode,'stdout':result.stdout,'stderr':result.stderr}
            item['guest_receipt']=json.loads((folder/'guest/execution.json').read_text())
            item['serial_logs']={run['mode']:(folder/'guest'/(run['mode']+'.log')).read_text() for run in item['guest_receipt']['runs']}
            report['runs'].append(item); print('integrated CPUs',cpus,result.returncode,flush=True)
            if result.returncode: break
    report['source_unchanged']=before==hashes()
    report['passed']=len(report['runs'])==6 and all(r['exit_code']==0 for r in report['runs']) and report['source_unchanged']
    if (folder/'guest/build.json').exists(): report['image_build']=json.loads((folder/'guest/build.json').read_text())
    if (folder/'guest/strict-link.txt').exists(): report['strict_link']=(folder/'guest/strict-link.txt').read_text()
    (ROOT/'reviews/v3/kernel_independent_runs.json').write_text(json.dumps(report,indent=2)+'\n')
    print('scoped kernel regression passed',report['passed'])
    return 0 if report['passed'] else 1

if __name__=='__main__': raise SystemExit(main())
