"""Check exact source-bound service evidence; malformed controls never count."""
from copy import deepcopy
import argparse,json
from common import ROOT,sha,read_json,dump

def validate(report,override=None):
    errors=[]
    def pins(data):
        values=data.get('input_sha256',{})
        if not isinstance(values,dict) or not values:errors.append('Missing source pins');return
        for name,digest in values.items():
            path=ROOT/name
            if not path.is_file() or sha(path)!=digest:errors.append('Stale source: '+name)
    pins(report)
    if report.get('passed') is not True:errors.append('Campaign failed')
    required={cc+opt+'-'+cpu for cc in ['gcc','clang'] for opt in ['-O0','-O2'] for cpu in ['2','4']}
    controls={'LOAD_LEAK':'load rollback','RIGHTS':'directional rights','OFFSET':'read rejection preserves offset','CANCEL':'cancel clears waiter',
      'FILE_FORK':'child shared offset','SNAPSHOT':'snapshot retains old backing','PARTIAL':'partial retry exact suffix','IMMUTABLE':'immutable write admitted'}
    expected=required|set(controls)|{'stub-loader','stub-services','receipt-host','receipt-guests'}
    runs=report.get('runs',[])
    if len(runs)!=len(expected) or {r['id'] for r in runs}!=expected:errors.append('Required run identities incomplete/duplicated')
    for row in runs:
        path=ROOT/row['path']
        if not path.is_file() or sha(path)!=row.get('sha256'):errors.append('Receipt identity changed: '+row['id']);continue
        data=(override or {}).get(row['id'],read_json(path));pins(data)
        ident=row['id'];commands=data.get('commands',[])
        qemu=[r for r in commands if r.get('command',[None])[0]=='qemu-system-x86_64' and '-machine' in r['command']]
        if ident.startswith('stub-'):
            marker='FAIL kernel: sd init ELF' if ident=='stub-loader' else 'SD_CHECK:load rollback'
            if data.get('passed') is not False or len(qemu)!=1 or qemu[0].get('exit')!=35 or marker not in qemu[0].get('stdout',''):errors.append('Wrong stub rejection: '+ident)
            commands=[r for r in commands if r not in qemu]
        else:
            if data.get('passed') is not True:errors.append('Failed child: '+ident)
        for command in commands:
            if command.get('exit')!=command.get('expected') or command.get('exit') is None:errors.append('Unexpected command exit: '+ident)
        if ident in required:
            if len(qemu)!=1 or qemu[0].get('exit')!=33 or len(data.get('markers',[]))!=8 or any(m not in qemu[0].get('stdout','') for m in data.get('markers',[])):errors.append('Missing service observations: '+ident)
        if ident in controls:
            compilations=[r for r in commands if '-DSDK_MUTANT_'+ident in r.get('command',[]) and '-c' in r['command']]
            if not compilations or any(r.get('exit')!=0 for r in compilations):errors.append('Mutant did not compile: '+ident)
            if len(qemu)!=1 or qemu[0].get('exit')!=35 or 'SD_CHECK:'+controls[ident] not in qemu[0].get('stdout',''):errors.append('Wrong semantic rejection: '+ident)
        if ident=='receipt-guests':
            cases=data.get('cases',[])
            if len(cases)!=152 or {(r['cut'],r['mode']) for r in cases}!={(cut,mode) for cut in range(76) for mode in ['write','verify']}:errors.append('Missing required receipt cut')
            if len(qemu)!=152 or any(r.get('exit')!=33 for r in qemu):errors.append('Missing actual receipt VM executions')
            if data.get('disk_environment',{}).get('guest_deadline_ticks')!=100:errors.append('Guest deadline contract changed')
        if ident=='receipt-host':
            profiles=data.get('profiles',[])
            if len(profiles)!=6 or {(r['compiler'],r['profile']) for r in profiles}!={(cc,mode) for cc in ['gcc','clang'] for mode in ['debug','release','sanitize']}:errors.append('Host profiles incomplete')
            if any(r.get('cases')!=185 or r.get('events')!=37 for r in profiles):errors.append('Host fault model bounds changed')
            sector=[r for r in commands if len(r.get('command',[]))==2 and r['command'][1]=='sectors']
            if len(sector)!=2 or any('cases=18176 events=37 masks=256' not in r.get('stdout','') or r['exit']!=0 for r in sector):errors.append('Sector model incomplete')
            if len(data.get('controls',[]))!=3 or any(not r.get('compiled') or not r.get('semantic_rejection') for r in data.get('controls',[])):errors.append('Receipt controls incomplete')
            if data.get('strict_target',{}).get('execution')!='not-run':errors.append('Cross-link relabeled as target execution')
    return errors

def regressions(report):
    omitted=deepcopy(report);omitted['runs']=[r for r in omitted['runs'] if r['id']!='PARTIAL']
    assert 'Required run identities incomplete/duplicated' in validate(omitted)
    rows={r['id']:read_json(ROOT/r['path']) for r in report['runs']}
    cut=deepcopy(rows['receipt-guests']);cut['cases'].pop()
    assert 'Missing required receipt cut' in validate(report,{'receipt-guests':cut})
    mutant=deepcopy(rows['PARTIAL'])
    for command in mutant['commands']:
        if command['command'][0]=='qemu-system-x86_64' and '-machine' in command['command']:command['exit']=139
    assert 'Wrong semantic rejection: PARTIAL' in validate(report,{'PARTIAL':mutant})
    stale=deepcopy(report);stale['input_sha256'][next(iter(stale['input_sha256']))]='0'*64
    assert any(e.startswith('Stale source:') for e in validate(stale))
    return ['missing variant run rejected','missing reboot cut rejected','crashing mutant is not semantic rejection','stale source rejected']

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--report',default='reviews/v5/expansion/followthrough/service_adapter_campaign.json');args=parser.parse_args()
    report=read_json(ROOT/args.report);errors=validate(report)
    controls=regressions(report) if not errors else []
    result={'passed':not errors,'errors':errors,'controls':controls,'input_sha256':{args.report:sha(ROOT/args.report)}}
    dump(ROOT/'reviews/v5/expansion/followthrough/service_adapter_check.json',result);print(json.dumps(result));raise SystemExit(bool(errors))
