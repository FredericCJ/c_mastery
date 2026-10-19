"""Versioned metadata-only refresh through the 25 established export/close/guard producers."""
import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from common import ROOT,sha,dump

BASE='reviews/v5/expansion/envelope_refresh_01'
MANIFEST=BASE+'/owned_manifest.json'


def prepare():
    assert not (ROOT/MANIFEST).exists(),'Preparation already recorded; do not replace history'
    v=json.loads((ROOT/'research/v5/expansion/vm_source_metadata/refreshed_packages.json').read_text())
    s=json.loads((ROOT/'research/v5/expansion/resource_merge_repair/refreshed_packages.json').read_text())['packages']
    packages=[];archive={}
    for item in v+s:
        support=item['support'];data=json.loads((ROOT/support).read_text());sources=data['source_sha256']
        candidates=[p for p in sources if p.startswith('tools/')]
        exporter=next(p for p in candidates if p.endswith('_exports.py'))
        close=next(p for p in candidates if Path(p).name.startswith('close_'))
        package_guard=next(p for p in candidates if p.endswith('_package.py'))
        guard=next(p for p in candidates if Path(p).name.startswith('check_') and not any(
            tag in p for tag in ['_exports.py','_package.py','_capture.py','_supplement.py']))
        review=Path(item['guard']).parent.as_posix()
        campaign=review+'/current_validation.json'
        assert campaign in data['current_reports']
        paths={support,exporter,*data['current_reports'],item['guard']}
        for path in paths:
            source=ROOT/path
            assert source.is_file(),path
            destination=ROOT/BASE/'history'/path
            destination.parent.mkdir(parents=True,exist_ok=True)
            destination.write_bytes(source.read_bytes());archive[path]=sha(source)
        text=(ROOT/exporter).read_text(encoding='utf-8')
        assert 'envelope_export_inputs' not in text and 'catalog' not in text,exporter
        lines=text.splitlines()
        start=next(i for i,line in enumerate(lines) if line.lstrip().startswith('d=dict(passed='))
        indent=lines[start][:-len(lines[start].lstrip())]
        lines[start:start]=[indent+'from envelope_export_inputs import bind_project_exports',
                            indent+'bind_project_exports(ROOT,pins,rows,errors)']
        (ROOT/exporter).write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
        packages.append(dict(support=support,exporter=exporter,guard=guard,close=close,
            package_guard=package_guard,review=review,campaign=campaign,campaign_sha256=sha(ROOT/campaign),
            original_support_sha256=sha(ROOT/support),rows=len(data['updates'])))
    dump(ROOT/MANIFEST,dict(schema_version=1,packages=packages,history_sha256=archive,
        scope='Original C campaign bytes retained; actual canonical exports then support/close/package guards rerun. Added project-frontmatter asset verification, without whole-catalog pinning.'))
    print(json.dumps(dict(prepared=len(packages),historical_files=len(archive))))


def run(resume=False):
    if os.name=='nt':raise RuntimeError('Run this producer under WSL to retain hosted/target tool contracts')
    manifest=json.loads((ROOT/MANIFEST).read_text());results=[];previous=None
    if resume:
        previous=json.loads((ROOT/BASE/'owned_progress.json').read_text())
        assert previous['passed'] is False,'Resume only the preserved failed batch'
        history=ROOT/BASE/'owned_failure_01.json'
        assert not history.exists(),'Use a new explicitly versioned attempt if another failure occurs'
        history.write_bytes((ROOT/BASE/'owned_progress.json').read_bytes())
        results=previous['completed']
        for item in results:
            assert sha(ROOT/item['support'])==item['sha256'],'Completed support changed before resume'
            guard=json.loads((ROOT/item['guard']).read_text());assert guard['passed']
            for path,digest in guard.get('input_sha256',{}).items():
                assert sha(ROOT/path)==digest,'Completed package guard changed before resume'
    for package in manifest['packages']:
        assert sha(ROOT/package['campaign'])==package['campaign_sha256'],'C campaign changed before refresh'
        if any(item['support']==package['support'] for item in results):continue
        commands=[['python3',package['exporter']],
                  ['python3',package['guard'],'--self-test','--output',package['review']+'/support_validation.json'],
                  ['python3',package['close']],
                  ['python3',package['package_guard'],'--self-test','--output',package['review']+'/package_validation.json']]
        executions=[];start=0
        if previous and previous['failed_package']['support']==package['support']:
            old=previous['executions'];assert old[-1]['exit_code']!=0
            start=len(old)-1;executions=old[:start]
            for row in executions:assert sha(ROOT/row['log'])==row['log_sha256']
            for index in range(start):
                assert index in (0,1),'This recovery supports an export/guard prefix only'
                report=package['review']+('/exports.json' if index==0 else '/support_validation.json')
                value=json.loads((ROOT/report).read_text());assert value['passed']
                for path,digest in value.get('input_sha256',{}).items():
                    assert sha(ROOT/path)==digest,('Passed prefix became stale',report,path)
        for index,command in enumerate(commands):
            if index<start:continue
            process=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=900)
            log=BASE+'/runs/'+Path(package['support']).parent.name+f'-{index}'+('-resume1' if resume else '')+'.log'
            assert not (ROOT/log).exists(),'Never overwrite a previous command log'
            (ROOT/log).parent.mkdir(parents=True,exist_ok=True)
            (ROOT/log).write_text(process.stdout+'\n'+process.stderr,encoding='utf-8',newline='\n')
            executions.append(dict(command=command,exit_code=process.returncode,log=log,log_sha256=sha(ROOT/log)))
            if process.returncode:
                dump(ROOT/BASE/'owned_progress.json',dict(passed=False,completed=results,failed_package=package,executions=executions))
                print(process.stdout+process.stderr,flush=True);return 1
        assert sha(ROOT/package['campaign'])==package['campaign_sha256'],'C campaign changed during refresh'
        result=dict(support=package['support'],sha256=sha(ROOT/package['support']),
            guard=package['review']+'/package_validation.json',rows=package['rows'],
            campaign_unchanged=True,campaign_sha256=package['campaign_sha256'],executions=executions)
        results.append(result);dump(ROOT/BASE/'owned_progress.json',dict(passed=True,completed=results))
        print('REFRESH_PASS '+Path(package['support']).parent.name,flush=True)
    dump(ROOT/BASE/'refreshed_packages.json',dict(passed=True,packages=results,
        source_sha256={'tools/refresh_owned_envelopes.py':sha(Path(__file__)),MANIFEST:sha(ROOT/MANIFEST),
                       'tools/envelope_export_inputs.py':sha(ROOT/'tools/envelope_export_inputs.py')},
        limits='No original C campaign overwritten; actual exports and guard checks only. External dimensions unchanged.'))
    return 0


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--prepare',action='store_true');parser.add_argument('--resume',action='store_true');args=parser.parse_args()
    if args.prepare:prepare()
    else:raise SystemExit(run(args.resume))
