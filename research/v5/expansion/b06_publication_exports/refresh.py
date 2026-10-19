"""Re-export four project consumers after the explicit B06 prose attachment.

No campaign is rerun or rebound. Existing exporters execute their own public
workspace controls, then unchanged mechanism and package guards are invoked.
"""
from pathlib import Path
import datetime
import hashlib
import json
import os
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[4]
BASE=ROOT/'reviews/v5/expansion/b06_publication_exports'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def read(path):return json.loads(path.read_text(encoding='utf8'))
def put(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')


def main():
    assert os.name!='nt','Use WSL for actual native/target exported programs'
    packages=[p for p in read(ROOT/'reviews/v5/expansion/author_envelope_refresh_01/manifest.json')['packages'] if p['name'] in ['dma','g474_uart','layers']]
    packages.append(dict(name='lease',support='research/v5/expansion/legacy_advanced_lease/completion_dispositions.json',
        exporter='tools/check_interrupt_lease_exports.py',close='tools/close_interrupt_lease_depth.py',
        guard='tools/check_interrupt_lease_depth.py',package_guard='tools/check_interrupt_lease_package.py',
        review='reviews/v5/expansion/legacy_advanced_lease',campaign='reviews/v5/expansion/legacy_advanced_lease/current_validation.json'))
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work=BASE/'runs'/stamp;work.mkdir(parents=True)
    completed=[];history={};commands=[]
    for p in packages:
        old=read(ROOT/p['support']);campaign_hash=sha(ROOT/p['campaign'])
        original_rows=old['updates']
        for name in {p['support'],*old['current_reports'],p['review']+'/package_validation.json'}:
            path=ROOT/name
            if not path.exists():continue
            digest=sha(path);target=BASE/'history'/digest/path.name
            target.parent.mkdir(parents=True,exist_ok=True)
            if target.exists():assert sha(target)==digest
            else:target.write_bytes(path.read_bytes())
            history[name]=dict(path=target.relative_to(ROOT).as_posix(),sha256=digest)
        flag='--report' if p['name']=='g474_uart' else '--output'
        steps=[[sys.executable,p['exporter']],
            [sys.executable,p['guard'],'--self-test',flag,p['review']+'/support_validation.json'],
            [sys.executable,p['close']],
            [sys.executable,p['package_guard'],'--self-test',flag,p['review']+'/package_validation.json']]
        for i,command in enumerate(steps):
            result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=1800)
            log=work/(p['name']+'-'+str(i)+'.log');log.write_text(result.stdout+'\n'+result.stderr,encoding='utf8')
            commands.append(dict(command=command,exit_code=result.returncode,passed=result.returncode==0,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
            put(BASE/'progress.json',dict(passed=result.returncode==0,completed=completed,commands=commands,history=history))
            if result.returncode:
                print(result.stdout+result.stderr,flush=True);return result.returncode
        new=read(ROOT/p['support'])
        assert new['updates']==original_rows,'Publication metadata must not change original assessment/dimension rows'
        assert sha(ROOT/p['campaign'])==campaign_hash,'Original C campaign overwritten'
        completed.append(dict(support=p['support'],sha256=sha(ROOT/p['support']),rows=len(new['updates']),
            campaign=p['campaign'],campaign_sha256=campaign_hash,campaign_unchanged=True,
            package_guard=p['review']+'/package_validation.json'))
        print('REFRESH_PASS',p['name'],flush=True)
    result=dict(passed=True,packages=completed,commands=commands,history=history,
        source_sha256={Path(__file__).relative_to(ROOT).as_posix():sha(Path(__file__)),
            'research/v5/expansion/b06_document_utf8/completion_dispositions.json':sha(ROOT/'research/v5/expansion/b06_document_utf8/completion_dispositions.json')},
        limits=['Actual exports and fresh metadata guards only. Original C campaign bytes and exact assessment/dimension rows remain unchanged.',
                'No external review, learner acceptance or physical result inferred.'])
    put(BASE/'refreshed_packages.json',result);print('PASS',len(completed),'packages',len(commands),'commands');return 0


if __name__=='__main__':raise SystemExit(main())
