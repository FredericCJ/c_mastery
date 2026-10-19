"""Refresh five network envelopes through real exports and existing guards."""
import argparse
from datetime import datetime,timezone
import hashlib,json,subprocess,sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'reviews/v5/expansion/envelope_refresh_01/network'

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def read(path):return json.loads(path.read_text(encoding='utf8'))
def write(path,value):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')

def commands(name):
    review='reviews/v5/expansion/legacy_network_'+name+'/'
    guard='tools/check_network_'+name+('_depth' if name in ['ip','tcp'] else '')+'.py'
    close='tools/close_network_'+name+('_depth' if name in ['ip','tcp'] else '')+'.py'
    exporter='tools/check_network_'+name+('_exercise' if name=='congestion' else '_exports')+'.py'
    out=[[sys.executable,exporter]]
    if name=='ip':out.append([sys.executable,'tools/check_network_ip_exercise.py'])
    out.append([sys.executable,guard,'--self-test','--output',review+'support_validation.json'])
    if name=='ip':
        out.extend([[sys.executable,'tools/close_network_ip_exercise.py'],
                    [sys.executable,'tools/check_network_ip_exercise_package.py','--self-test','--output',review+'exercise_package_validation.json']])
    out.extend([[sys.executable,close],
                [sys.executable,'tools/check_network_'+name+'_package.py','--self-test','--output',review+'package_validation.json']])
    return out

def main():
    global BASE
    p=argparse.ArgumentParser();p.add_argument('--resume',action='store_true');p.add_argument('--base');p.add_argument('--begin',action='store_true');args=p.parse_args()
    template=BASE/'manifest.json'
    if args.base:
        BASE=(ROOT/args.base).resolve()
        assert BASE.is_relative_to((ROOT/'reviews/v5/expansion').resolve())
    if args.begin:
        assert not args.resume and not BASE.exists(),'A new cycle requires a fresh scoped review directory'
        prior=read(template);BASE.mkdir(parents=True)
        old_manifest=BASE/'previous_manifest.json';old_manifest.write_bytes(template.read_bytes())
        history=[]
        for entry in prior['historical_files']:
            path=ROOT/entry['path'];saved=BASE/'history'/entry['path'];saved.parent.mkdir(parents=True,exist_ok=True)
            saved.write_bytes(path.read_bytes());assert sha(saved)==sha(path)
            history.append(dict(path=entry['path'],sha256=sha(saved),historical_path=saved.relative_to(ROOT).as_posix()))
        packages=prior['packages']
        for package in packages:
            # These are actual newly executed producer receipts, not rehashed
            # predecessor observations. The original mechanism guard is run
            # below before any new envelope is generated.
            package['campaign_sha256']=sha(ROOT/package['campaign'])
        write(BASE/'manifest.json',dict(packages=packages,historical_files=history,
            prior_manifest=dict(path=old_manifest.relative_to(ROOT).as_posix(),sha256=sha(old_manifest))))
    manifest=read(BASE/'manifest.json');previous=None;done=[]
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run=BASE/stamp;run.mkdir()
    if args.resume:
        previous=read(BASE/'result.json');assert not previous['passed']
        write(run/'prior_result.json',previous);done=previous['packages']
        for row in done:
            for n,h in row['support_sha256'].items():assert sha(ROOT/n)==h,('completed support changed',n)
    elif (BASE/'result.json').exists():raise ValueError('Use explicit resume for a preserved failed attempt')
    for package in manifest['packages']:
        name=package['name'];assert sha(ROOT/package['campaign'])==package['campaign_sha256']
        if any(row['name']==name for row in done):continue
        executions=[]
        for index,command in enumerate(commands(name)):
            result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=900)
            log=run/(name+'-'+str(index)+'.log');log.write_text(result.stdout+'\n'+result.stderr,encoding='utf8')
            executions.append(dict(command=command,exit_code=result.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
            if result.returncode:
                write(BASE/'result.json',dict(passed=False,packages=done,failed=name,executions=executions))
                print(result.stdout+result.stderr,flush=True);return 1
        assert sha(ROOT/package['campaign'])==package['campaign_sha256']
        supports=[package['support']]
        if name=='ip':supports.append('research/v5/expansion/legacy_network_ip/exercise_completion_dispositions.json')
        row=dict(name=name,support_sha256={n:sha(ROOT/n) for n in supports},
                 rows=sum(len(read(ROOT/n)['updates']) for n in supports),
                 guards=['reviews/v5/expansion/legacy_network_'+name+'/package_validation.json']+
                    (['reviews/v5/expansion/legacy_network_ip/exercise_package_validation.json'] if name=='ip' else []),
                 campaign_unchanged=True,campaign_sha256=package['campaign_sha256'],executions=executions)
        done.append(row);write(BASE/'result.json',dict(passed=True,packages=done))
        print('REFRESH_PASS '+name,flush=True)
    write(BASE/'result.json',dict(passed=True,packages=done,
          source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [Path(__file__),BASE/'manifest.json',ROOT/'tools/envelope_export_inputs.py',ROOT/'tools/selected_export_bindings.py']},
          limits='Actual selected exports, native/target stub/reference execution and exact guards. Consumed C campaign identities remain unchanged during this envelope cycle; no learner, external-review or physical observation inferred.'))
    return 0

if __name__=='__main__':raise SystemExit(main())
