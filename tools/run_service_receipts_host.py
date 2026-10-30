"""Exercise actual retained store transactions under its explicit torn-write model."""
import argparse
from datetime import datetime,timezone
import hashlib,json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',type=Path);args=parser.parse_args()
    folder=ROOT/'build/service_depth/receipt-host'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');folder.mkdir(parents=True)
    paths=['assessor/service_depth/kernel/receipt.c','assessor/service_depth/kernel/receipt_faults.c','assessor/storage/store.c',
      'labs/service_depth/adapters/receipt.h','labs/service_depth/adapters/receipt.c','labs/storage/tests/test_store.c',
      'labs/storage/src/store_file.c','labs/storage/include/store.h','labs/storage/include/store_file.h',
      'labs/protocols/targets/runtime.c','tools/run_service_receipts_host.py']
    paths += ['labs/service_depth/adapters/freestanding/'+n for n in ['string.h','bytes.c']]
    report={'schema_version':1,'passed':False,'commands':[],'started_utc':datetime.now(timezone.utc).isoformat(),
      'input_sha256':{p:sha(ROOT/p) for p in paths},'profiles':[],
      'limits':['The five persistence modes are explicit device models, not hardware observations.','A compiled semantic mutant must execute and fail the stated assertion.','Reference artifacts are author work; no learner or independent reviewer evidence.']}
    def run(command,expect=0,marker=None,env=None):
        r=subprocess.run(list(map(str,command)),cwd=ROOT,text=True,capture_output=True,timeout=600,env=dict(os.environ,**(env or {})))
        output=r.stdout+r.stderr;ok=r.returncode==expect and (marker is None or ''.join(marker.split()) in ''.join(output.split()))
        report['commands'].append({'command':list(map(str,command)),'exit':r.returncode,'expected':expect,'marker':marker,'passed':ok,'stdout':r.stdout,'stderr':r.stderr,'environment':env or {}})
        if not ok:raise RuntimeError('Command failed: '+str(command)+'\n'+output)
        return output
    try:
        common=['-std=c17','-Wall','-Wextra','-Werror','-Ilabs/storage/include','-Ilabs/service_depth/adapters']
        sources=['assessor/service_depth/kernel/receipt_faults.c','assessor/service_depth/kernel/receipt.c','assessor/storage/store.c','labs/storage/src/store_file.c']
        for cc in ['gcc','clang']:
            run([cc,'--version'])
            for profile,flags in [('debug',['-O0','-g']),('release',['-O2']),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
                exe=folder/(cc+'-'+profile);run([cc,*common,*flags,*sources,'-o',exe])
                run([exe],marker='PASS receipt-fault-model cases=185 events=37',env={'ASAN_OPTIONS':'detect_stack_use_after_return=1'})
                report['profiles'].append({'compiler':cc,'profile':profile,'cases':185,'events':37,'passed':True})
                if profile=='release':run([exe,'sectors'],marker='PASS receipt-sector-model cases=18176 events=37 masks=256 prefixes=all-dirty-page-boundaries')
        controls=[]
        for macro,marker in [('SDK_MUTANT_REAPPLY','out==7'),('SDK_MUTANT_PAYLOAD_BINDING','SDR_CONFLICT'),('SDK_MUTANT_SPLIT_RECEIPT','out==7')]:
            exe=folder/macro;run(['gcc',*common,'-O2','-D'+macro,*sources,'-o',exe]);run([exe],expect=1,marker=marker);controls.append({'macro':macro,'marker':marker,'compiled':True,'semantic_rejection':True})
        stub=folder/'learner-stub';run(['gcc',*common,'-O2',sources[0],'labs/service_depth/adapters/receipt.c',*sources[2:],'-o',stub]);run([stub],expect=1,marker='sdr_initialize')
        objects=[]
        for name,path in [('receipt',sources[1]),('store',sources[2]),('runtime','labs/protocols/targets/runtime.c'),('bytes','labs/service_depth/adapters/freestanding/bytes.c')]:
            obj=folder/(name+'.o');run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb',*common,'-Ilabs/service_depth/adapters/freestanding','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-c',path,'-o',obj]);objects.append(obj)
        elf=folder/'receipt.elf';run(['arm-none-eabi-ld',*objects,'-e','model_entry','-Map='+str(folder/'receipt.map'),'-o',elf])
        if run(['arm-none-eabi-nm','-u',elf]).strip():raise RuntimeError('Unclosed target runtime')
        symbols=run(['arm-none-eabi-nm',elf]);run(['arm-none-eabi-size',elf])
        if any(sha(ROOT/p)!=h for p,h in report['input_sha256'].items()):raise RuntimeError('Inputs changed during campaign')
        report.update(passed=True,status='passed',sector_model={'profiles':2,'cases_per_profile':18176,'masks':256,'prefixes':'all observed dirty-page boundaries','events':37,'limits':'All512-byte masks for the current write and reverse page-prefix plus one torn page for flush; not all joint arbitrary media states. Counts retained in raw output.'},controls=controls,strict_target={'target':'Cortex-M4','execution':'not-run','closure':'Full image linked without section garbage collection; explicit original byte helpers only.','symbols':symbols,'sha256':sha(elf)})
    except Exception as error:report.update(status='failed',error=str(error))
    dest=args.report or folder/'report.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'passed':report['passed'],'report':str(dest),'error':report.get('error')}));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
