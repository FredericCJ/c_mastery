"""Required service successor campaign; exact author-reference provenance."""
import argparse
from datetime import datetime,timezone
import hashlib,json,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CONTROLS=['LOAD_LEAK','RIGHTS','OFFSET','CANCEL','FILE_FORK','SNAPSHOT','PARTIAL','IMMUTABLE']
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',type=Path);args=parser.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    evidence=ROOT/'reviews/v5/expansion/followthrough/service_adapters'/stamp;evidence.mkdir(parents=True)
    inputs=[p for folder in ['labs/service_depth/adapters','assessor/service_depth/kernel'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    inputs += [ROOT/'tools'/name for name in ['run_service_adapters.py','run_service_kernel.py','run_service_receipts_host.py','check_service_adapters_v5.py']]
    report={'schema_version':1,'passed':False,'observed_utc':datetime.now(timezone.utc).isoformat(),
      'input_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in inputs},'commands':[],'runs':[],
      'limits':['Author reference execution only; actual learner imports require a new campaign.','QEMU guest recovery and declared media models are not physical power-loss validation.','External review and delayed independent learner evidence remain pending.']}
    def invoke(name,command,expect=0):
        target=evidence/(name+'.json');argv=[sys.executable,*command,'--report',str(target)]
        r=subprocess.run(argv,cwd=ROOT,text=True,capture_output=True,timeout=1800)
        row={'command':argv,'exit':r.returncode,'expected':expect,'passed':r.returncode==expect,'stdout':r.stdout,'stderr':r.stderr}
        report['commands'].append(row)
        if not row['passed'] or not target.is_file():raise RuntimeError(name+': '+r.stdout+r.stderr)
        data=json.loads(target.read_text())
        report['runs'].append({'id':name,'path':target.relative_to(ROOT).as_posix(),'sha256':sha(target)})
        return data
    try:
        for compiler in ['gcc','clang']:
            for optimization in ['-O0','-O2']:
                for cpus in ['2','4']:
                    invoke(compiler+optimization+'-'+cpus,['tools/run_service_kernel.py','--compiler',compiler,'--optimization='+optimization,'--cpus',cpus])
        for control in CONTROLS:invoke(control,['tools/run_service_kernel.py','--only','SDK_MUTANT_'+control])
        # Both stubs must translate/link and reach their distinct target failure.
        for kind in ['loader','services']:
            command=['labs/service_depth/adapters/guest.py','--kernel-source','assessor/kernel_smp','--kernel-lab','labs/kernel_smp',
              '--adapter-source','labs/service_depth/adapters','--store-source','assessor/storage/store.c','--storage-lab','labs/storage',
              '--build-root','build/service_depth/kernel-stub']
            if kind=='services':command+=['--loader-source','assessor/service_depth/kernel/load.inc']
            data=invoke('stub-'+kind,command,1)
            marker='FAIL kernel: sd init ELF' if kind=='loader' else 'SD_CHECK:load rollback'
            qemu=[r for r in data['commands'] if r['command'][0]=='qemu-system-x86_64' and '-machine' in r['command']]
            if len(qemu)!=1 or qemu[0]['exit']!=35 or marker not in qemu[0]['stdout']:raise RuntimeError('Wrong scaffold rejection: '+kind)
        invoke('receipt-host',['tools/run_service_receipts_host.py'])
        invoke('receipt-guests',['tools/run_service_kernel.py','--suite','receipts'])
        if any(sha(ROOT/p)!=h for p,h in report['input_sha256'].items()):raise RuntimeError('Inputs changed during campaign')
        report['passed']=True
        from check_service_adapters_v5 import validate
        errors=validate(report)
        if errors:raise RuntimeError(str(errors))
    except Exception as error:report.update(passed=False,error=str(error))
    target=args.report or evidence/'campaign.json';target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'passed':report['passed'],'report':str(target),'error':report.get('error')}));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
