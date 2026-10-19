"""Fail closed on incomplete, stale or non-semantic request observations."""
import json
from copy import deepcopy
from common import ROOT,dump,read_json,sha
def validate(report,overrides=None):
    errors=[];overrides=overrides or {}
    def pins(data):
        mapping=data.get('input_sha256',{})
        if not isinstance(mapping,dict) or not mapping:errors.append('Missing source pins');return
        for p,h in mapping.items():
            path=(ROOT/p).resolve()
            if not path.is_relative_to(ROOT.resolve()) or not path.is_file() or sha(path)!=h:errors.append('Stale source: '+p)
    pins(report)
    if report.get('passed') is not True:errors.append('Campaign not passed')
    if {(p['compiler'],p['profile']) for p in report.get('profiles',[])}!={(c,m) for c in ['gcc','clang'] for m in ['debug','release','sanitize']} or len(report.get('profiles',[]))!=6:errors.append('Missing model profile')
    commands=report.get('commands',[])
    if not commands or any(r.get('passed') is not True for r in commands):errors.append('Failed/missing command')
    model=[r for r in commands if r.get('marker')=='RQ_OK:196 transitions;']
    if len(model)!=6 or any(r.get('exit')!=0 or 'RQ_OK:196 transitions;' not in r.get('stdout','') for r in model):errors.append('Missing model execution')
    if len([r for r in commands if r.get('exit')==1 and r.get('marker')=='RQ_CHECK:transition table'])!=2:errors.append('Missing model controls')
    ids={c+o+'-'+n for c in ['gcc','clang'] for o in ['-O0','-O2'] for n in ['2','4']}
    controls={'RQ_MUTANT_CANCEL':'cancel intent','RQ_MUTANT_RESET':'reset intent retains request','RQA_MUTANT_ACK':'withheld acknowledgement retains request','RQA_MUTANT_STATUS':'device error propagated'}
    guests=report.get('guests',[])
    error_ids={'gcc-device-error','clang-device-error'}
    if len(guests)!=15 or {g['id'] for g in guests}!=ids|set(controls)|{'learner-stub'}|error_ids:errors.append('Missing guest scope')
    for row in guests:
        if not (ROOT/row['path']).is_file():errors.append('Missing child receipt');continue
        if sha(ROOT/row['path'])!=row['sha256']:errors.append('Changed child receipt')
        data=overrides.get(row['id'],read_json(ROOT/row['path']));pins(data)
        q=[r for r in data.get('commands',[]) if r.get('command',[None])[0]=='qemu-system-x86_64' and '-machine' in r['command']]
        if len(q)!=1:errors.append('Missing actual guest');continue
        if row['id'] in ids|error_ids:
            markers=['PASS rqa-two-outstanding-cancel-coalesced','PASS rqa-two-blocked-waiters','PASS rqa-reset-ack-unknown-reuse','PASS rqa-131074-transfers-two-wraps','PASS rqa-owner-exit-keeps-request','PASS rqa-guest-complete']
            if row['id'] in error_ids:
                markers=['PASS rqa-device-error-retry','PASS rqa-guest-complete']
                if data.get('scenario')!='device-error' or 'sector = "96"' not in data.get('injection',{}).get('contents',''):errors.append('Missing actual device error injection')
            if data.get('passed') is not True or q[0].get('exit')!=33 or any(m not in q[0].get('stdout','') for m in markers):errors.append('Missing guest observations')
        else:
            marker='RQA_CHECK:'+controls.get(row['id'],'hold observer')
            compiled=[r for r in data.get('commands',[]) if '-c' in r['command']]
            if not compiled or any(r['exit']!=0 for r in compiled):errors.append('Control not compiled')
            if q[0].get('exit')!=35 or marker not in q[0].get('stdout',''):errors.append('Wrong semantic rejection')
    if report.get('model_target',{}).get('undefined_symbols')!=[] or report.get('model_target',{}).get('execution')!='not-run':errors.append('Missing bounded target audit')
    return errors
def main():
    path='reviews/v5/expansion/followthrough/request_adapter_campaign.json';r=read_json(ROOT/path);errors=validate(r);controls=[]
    if not errors:
        missing=deepcopy(r);missing['guests'].pop();assert 'Missing guest scope' in validate(missing);controls.append('missing target profile rejected')
        row=next(x for x in r['guests'] if x['id']=='RQA_MUTANT_ACK');bad=deepcopy(read_json(ROOT/row['path']))
        for command in bad['commands']:
            if command['command'][0]=='qemu-system-x86_64' and '-machine' in command['command']:command['exit']=139
        assert 'Wrong semantic rejection' in validate(r,{'RQA_MUTANT_ACK':bad});controls.append('crash is not semantic rejection')
        stale=deepcopy(r);stale['input_sha256'][next(iter(stale['input_sha256']))]='0'*64;assert any(e.startswith('Stale source:') for e in validate(stale));controls.append('stale source rejected')
    result={'passed':not errors,'errors':errors,'controls':controls,'input_sha256':{path:sha(ROOT/path)}}
    dump(ROOT/'reviews/v5/expansion/followthrough/request_adapter_check.json',result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
