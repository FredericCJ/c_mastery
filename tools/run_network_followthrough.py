"""Execute the two real provider producers, then bind their exact observations.

--reuse-current revalidates already executed exact reports and makes no new C
execution claim. Both modes preserve designated predecessor bytes.
"""
import argparse,json,subprocess,sys
from datetime import datetime,timezone
from common import ROOT,sha,dump
from check_network_followthrough import PROVIDERS,TOOLS,normalized,validate
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--reuse-current',action='store_true');a=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/followthrough'/('network_composed_'+stamp);base.mkdir()
    records=[];inputs={p:sha(ROOT/p) for p in TOOLS}
    for kind,(_,path,_) in PROVIDERS.items():
        if a.reuse_current:continue
        target=ROOT/path
        if target.is_file():
            backup=base/(kind+'_previous.json');backup.write_bytes(target.read_bytes());assert sha(backup)==sha(target)
        producer='tools/run_network_ip_depth.py' if kind=='ip' else 'tools/run_network_congestion.py'
        cmd=[sys.executable,producer,'--report',path]
        print('PROVIDER_START',kind,flush=True)
        r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=2400)
        log=base/(kind+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf-8',newline='\n')
        records.append(dict(command=cmd,exit_code=r.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        dump(base/'invocations.json',dict(executions=records))
        if r.returncode:print('PROVIDER_FAIL',kind,r.stdout[-2000:],r.stderr[-1000:]);return 1
        print('PROVIDER_PASS',kind,flush=True)
    pins={};references={};executions=[]
    for kind,(anchor,path,count) in PROVIDERS.items():
        value=json.loads((ROOT/path).read_text());pins.update(value['input_sha256']);pins[path]=sha(ROOT/path)
        references[kind]=dict(path=path,sha256=sha(ROOT/path),anchor=anchor,checks=count)
        executions+=normalized(kind,value)
    pins.update(inputs)
    d=dict(schema_version=1,family='network_followthrough',status='passed',passed=True,
        scope_source_stable=all(sha(ROOT/p)==h for p,h in pins.items()),inspected_paths=pins,
        provider_reports=references,anchors=[r[0] for r in PROVIDERS.values()],executions=executions,
        producer_invocations=records,reused_current_provider_observations=a.reuse_current,
        physical_validation='not-run',learner_mastery='not-assessed',independent_review='not-established',
        scope='Exact composition of actual IP230 and congestion220 checks; provider observations retain their identities and are not counted as new runs when reused.',
        limitations=['Synthetic host and Cortex-M4 CPU execution is not physical Ethernet, Internet fairness or hardware timing.',
          'Two current provider guards retain original native profiles, compiled mutants, actual oracle traces, target closures and learner rejections.',
          'No independent learner, external human or elapsed delayed assessment inferred.'])
    errors=validate(d);d['passed']=not errors;d['status']='passed' if not errors else 'failed';d['errors']=errors
    target=ROOT/a.report
    if target.is_file():
        old=base/'aggregate_previous.json';old.write_bytes(target.read_bytes())
    dump(target,d);dump(base/'result.json',dict(passed=not errors,report=a.report,sha256=sha(target),errors=errors,invocations=records))
    print(json.dumps(dict(passed=not errors,commands=len(executions),errors=errors)));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
