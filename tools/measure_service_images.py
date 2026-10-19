"""Observe sizes and compiler stack records from the exact executed service images."""
import hashlib,json,subprocess
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    path=ROOT/'reviews/v5/expansion/followthrough/service_adapter_campaign.json'
    campaign=json.loads(path.read_text());assert campaign['passed']
    report={'passed':False,'observed_utc':datetime.now(timezone.utc).isoformat(),'commands':[],'images':[],
      'input_sha256':{path.relative_to(ROOT).as_posix():sha(path),Path(__file__).relative_to(ROOT).as_posix():sha(Path(__file__))},
      'limits':['ELF section sizes and compiler static stack estimates, not measured worst-case latency or whole-call-chain stack.',
                'Fixed tables and stack extents are target-profile costs; hosted struct layouts and other ISAs require new observations.']}
    for row in campaign['runs']:
        if not row['id'].startswith(('gcc-','clang-')):continue
        receipt=ROOT/row['path'];data=json.loads(receipt.read_text());assert sha(receipt)==row['sha256']
        report['input_sha256'][row['path']]=sha(receipt)
        command=next(r['command'] for r in data['commands'] if r['command'][0]=='qemu-system-x86_64' and '-cdrom' in r['command'])
        folder=Path(command[command.index('-cdrom')+1]).parent
        assert sha(folder/'kernel.elf')==data['kernel_sha256']
        assert sha(folder/'init.cmka')==data['archive_sha256']
        image={'profile':row['id'],'kernel_sha256':data['kernel_sha256'],'archive_sha256':data['archive_sha256'],'elfs':[],'stack_records':[]}
        for elf in sorted(folder.glob('*.elf')):
            argv=['size','-A',str(elf)];run=subprocess.run(argv,text=True,capture_output=True,check=False)
            report['commands'].append({'command':argv,'exit':run.returncode,'expected':0,'stdout':run.stdout,'stderr':run.stderr})
            assert not run.returncode
            image['elfs'].append({'name':elf.name,'sha256':sha(elf),'file_bytes':elf.stat().st_size,'sections':run.stdout})
        for stack in sorted(folder.glob('*.su')):
            image['stack_records'].append({'name':stack.name,'sha256':sha(stack),'contents':stack.read_text()})
        assert image['stack_records']
        report['images'].append(image)
    assert len(report['images'])==8
    report['passed']=True
    target=ROOT/'reviews/v5/expansion/followthrough/service_image_costs.json';target.write_text(json.dumps(report,indent=2)+'\n')
    print({'passed':True,'profiles':len(report['images']),'report':str(target)})
if __name__=='__main__':main()
