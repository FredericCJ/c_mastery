"""Check exact T08 evidence; neither source counts nor crashes replace semantics."""
from copy import deepcopy
import argparse,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={f'T08.{topic:02}.{leaf}' for topic in range(1,11) for leaf in 'abc'}
def validate(data):
    errors=[]
    if data.get('passed') is not True or data.get('error') is not None:errors.append('Campaign failed')
    pins=data.get('input_sha256',{})
    if not pins:errors.append('Missing pins')
    for p,h in pins.items():
        file=(ROOT/p).resolve()
        if not file.is_relative_to(ROOT.resolve()) or not file.is_file() or sha(file)!=h:errors.append('Stale source: '+p)
    observations=data.get('observations',[])
    if len(observations)!=60 or {(o.get('objective'),o.get('variant')) for o in observations}!={(o,v) for o in OBJECTIVES for v in (False,True)}:errors.append('Missing exact ordinary or delayed objective')
    commands=data.get('commands',[]);checks=data.get('checks',[])
    if not commands or not checks:errors.append('Missing execution evidence')
    for row in commands:
        rc=row.get('exit');expected=row.get('expected')
        valid=isinstance(rc,int) and rc>0 if expected=='nonzero' else rc==expected
        if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):valid=False
        if not valid or row.get('passed') is not True:errors.append('Command evidence inconsistent')
    if any(r.get('passed') is not True for r in checks):errors.append('Failed property check')
    required={'LP64 manifest','ordinary packed reordered layout','changed Cell address derivation','independent subtraction flags',
      'object data model ilp32','object data model llp64','layout gate rejects incompatible before object access',
      'ninth AVX argument requires32-byte stack boundary','CFA matches actual push plus32-byte frame',
      'soft-only compiler helper dependency','actual Arm guest runtime closure','independent modular benchmark result',
      'all30 exact objectives and30 transfers observed'}
    if not required<={r.get('name') for r in checks}:errors.append('Missing discriminating property')
    core=[r for r in commands if r.get('argv',[None])[0].split('/')[-1] in {'AD_MUTANT_PRESERVED','AD_MUTANT_ALIGNMENT','AD_MUTANT_EARLY'}]
    if len(core)!=3 or any(r.get('exit')!=1 or 'AD_CHECK:' not in r.get('stderr','') for r in core):errors.append('Incorrect semantic controls')
    guests=[r for r in commands if r.get('argv',[None])[0]=='qemu-system-arm' and '-kernel' in r['argv']]
    if len(guests)!=4 or sorted(r.get('exit') for r in guests)!=[0,0,1,1] or any(('ARM_ABI_PASS' if r['exit']==0 else 'ARM_ABI_PRESERVATION_FAIL') not in r.get('stdout','')+r.get('stderr','') for r in guests):errors.append('Missing actual guest positive/negative cells')
    matrices=[r for r in commands if any('/matrix-' in a and a.endswith('.elf') for a in r.get('argv',[]))]
    if len(matrices)!=9 or sum(r['exit']==0 for r in matrices)!=5:errors.append('Incomplete PCS matrix')
    if any('DO_NOT_EXECUTE' in r.get('argv',[''])[0] for r in commands):errors.append('Privileged specimen was executed')
    if not data.get('generated_inputs'):errors.append('Missing original specimen bytes')
    return errors
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default='reviews/v5/expansion/legacy_systems_abi/full_validation.json');args=p.parse_args()
    data=read_json(ROOT/args.report);errors=validate(data);guards=[]
    if not errors:
        bad=deepcopy(data);bad['observations'].pop();assert 'Missing exact ordinary or delayed objective' in validate(bad);guards.append('missing changed condition rejected')
        bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale source') for e in validate(bad));guards.append('stale sources rejected')
        bad=deepcopy(data)
        for row in bad['commands']:
            if row['argv'][0].endswith('/AD_MUTANT_ALIGNMENT'):row['exit']=-11
        assert 'Incorrect semantic controls' in validate(bad);guards.append('crash is not alignment proof')
        bad=deepcopy(data);bad['commands']=[r for r in bad['commands'] if 'guest-softfp-bad.elf' not in ' '.join(r['argv'])];assert 'Missing actual guest positive/negative cells' in validate(bad);guards.append('missing Arm mutant rejected')
        bad=deepcopy(data);bad['checks']=[r for r in bad['checks'] if r['name']!='layout gate rejects incompatible before object access'];assert 'Missing discriminating property' in validate(bad);guards.append('missing layout gate rejected')
    result={'passed':not errors,'errors':errors,'negative_guards':guards,'input_sha256':{args.report:sha(ROOT/args.report)}}
    dump(ROOT/'reviews/v5/expansion/legacy_systems_abi/validation_check.json',result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
