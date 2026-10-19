"""Strict acceptance of both actual network provider campaigns as one family."""
import argparse,copy,json,os,subprocess,sys
from pathlib import Path
from common import ROOT,sha,dump
REPORT='reviews/v5/expansion/integrated/network_followthrough_campaign.json'
PROVIDERS={
 'ip':('network_followthrough.ip','reviews/v5/expansion/legacy_network_ip/current_validation.json',230),
 'congestion':('network_followthrough.congestion','reviews/v5/expansion/legacy_network_congestion/current_validation.json',220)}
TOOLS=['tools/run_network_followthrough.py','tools/check_network_followthrough.py','tools/run_network_ip_depth.py',
       'tools/check_network_ip_depth.py','tools/run_network_congestion.py','tools/check_network_congestion.py',
       'labs/network_followthrough/family.json']
def normalized(kind,report):
    rows=[]
    for row in report['checks']:
        if 'command' not in row:continue
        item=copy.deepcopy(row);item.update(provider=kind,log=row['raw'],log_sha256=row['raw_sha256'])
        rows.append(item)
    return rows
def validate(d,files=True):
    # The mechanism guards verify exact Linux tool identities and target paths.
    # Windows callers use the same guard in the actual owning Ubuntu environment.
    if files and os.name=='nt':
        from v5_common import python_command
        r=subprocess.run(python_command('tools/check_network_followthrough.py','--stdin',platform='wsl'),
            cwd=ROOT,input=json.dumps(d),text=True,capture_output=True,timeout=240)
        try:return json.loads(r.stdout)['errors']
        except (ValueError,KeyError):return ['Owning-environment network guard failed: '+r.stdout[-1000:]+r.stderr[-1000:]]
    errors=[]
    def need(ok,message):
        if not ok:errors.append(message)
    need(d.get('family')=='network_followthrough' and d.get('passed') is True and d.get('scope_source_stable') is True,'passed stable exact network composition')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external observation boundary')
    refs=d.get('provider_reports',{});need(set(refs)==set(PROVIDERS),'both exact providers')
    need(d.get('anchors')==[x[0] for x in PROVIDERS.values()],'both exact public anchors')
    pins=d.get('inspected_paths',{});need(set(TOOLS)<=set(pins),'producer and guard source identities')
    expected_exec=[];expected_pins={}
    if files:
        from check_network_ip_depth import validate as ip_guard
        from check_network_congestion import validate as congestion_guard
        guards={'ip':ip_guard,'congestion':congestion_guard}
        for kind,(anchor,path,count) in PROVIDERS.items():
            ref=refs.get(kind,{})
            need(ref.get('path')==path and ref.get('anchor')==anchor,'exact provider binding '+kind)
            try:
                need(ref.get('sha256')==sha(ROOT/path),'provider receipt identity '+kind)
                value=json.loads((ROOT/path).read_text())
                need(len(value.get('checks',[]))==count,'exact provider check count '+kind)
                errors += [kind+': '+e for e in guards[kind](value)]
                expected_exec+=normalized(kind,value);expected_pins.update(value['input_sha256'])
                expected_pins[path]=sha(ROOT/path)
            except (OSError,ValueError,KeyError) as e:errors.append(kind+': '+str(e))
        expected_pins.update({p:sha(ROOT/p) for p in TOOLS})
        need(pins==expected_pins,'exact consumed source closure')
        need(d.get('executions')==expected_exec,'unchanged exact provider observations')
    else:
        for kind,(anchor,path,count) in PROVIDERS.items():
            ref=refs.get(kind,{})
            need(ref.get('path')==path and ref.get('anchor')==anchor and ref.get('checks')==count,'exact provider declaration '+kind)
        rows=d.get('executions',[])
        for kind in PROVIDERS:
            need(sum(r.get('provider')==kind for r in rows)==PROVIDERS[kind][2]-1,'all actual provider commands '+kind)
            for tag in ['gcc-debug','gcc-release','gcc-asan','clang-debug','clang-release','clang-asan']:
                need(any(r.get('provider')==kind and r.get('label')==tag+' core execute' and r.get('passed') is True for r in rows),'observed reference '+kind+' '+tag)
                need(any(r.get('provider')==kind and r.get('label')==tag+' stub execute' and r.get('returncode')==1 and r.get('expected')==1 for r in rows),'observed unsolved rejection '+kind+' '+tag)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);p.add_argument('--stdin',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--output');a=p.parse_args()
    d=json.load(sys.stdin) if a.stdin else json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        changes=[('provider',lambda x:x['provider_reports'].pop('ip')),('anchor',lambda x:x['anchors'].pop()),
          ('provider-path',lambda x:x['provider_reports']['ip'].update(path='wrong.json')),
          ('source',lambda x:x['inspected_paths'].pop('tools/run_network_ip_depth.py')),
          ('observations',lambda x:x['executions'].pop()),
          ('host-case',lambda x:next(r for r in x['executions'] if r['provider']=='ip' and r['label']=='clang-release core execute').update(passed=False)),
          ('learner-control',lambda x:next(r for r in x['executions'] if r['provider']=='congestion' and r['label']=='gcc-debug stub execute').update(returncode=0)),
          ('stability',lambda x:x.update(scope_source_stable=False)),
          ('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        for name,change in changes:
            candidate=copy.deepcopy(d);change(candidate);reject=bool(validate(candidate,False));controls.append(dict(name=name,rejected=reject))
            if not reject:errors.append('accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls)
    if not a.stdin:result.update(report=a.report,report_sha256=sha(ROOT/a.report),source_sha256={p:sha(ROOT/p) for p in TOOLS})
    if a.output:dump(ROOT/a.output,result)
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
