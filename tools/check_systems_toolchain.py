"""Check current T07 experiment receipts; negative controls cannot be crashes."""
from copy import deepcopy
import argparse,json
from common import ROOT,read_json,dump,sha

OBJECTIVES={f'T07.{topic:02}.{leaf}' for topic in range(1,13) for leaf in 'abc'}
GROUPS={'tool-identities','stages','diagnostics','profiles','make-graph','ninja-database',
 'generated-resource','packages','generation-phase','foreign','target-roots',
 'named-emulator','offline-dependencies','matrix-verdicts','reproducibility'}

def validate(data):
    errors=[]
    if data.get('passed') is not True or data.get('error') is not None:errors.append('Campaign did not pass')
    pins=data.get('input_sha256',{})
    if not pins:errors.append('Missing source identity')
    for p,h in pins.items():
        path=(ROOT/p).resolve()
        if not path.is_relative_to(ROOT.resolve()) or not path.is_file() or sha(path)!=h:errors.append('Stale source: '+p)
    observations=data.get('observations',[])
    if len(observations)!=72 or {(r.get('objective'),r.get('variant')) for r in observations}!={(o,v) for o in OBJECTIVES for v in (False,True)}:errors.append('Missing exact objective/transfer observations')
    commands=data.get('commands',[])
    if not commands or {r.get('group') for r in commands}!=GROUPS:errors.append('Missing experiment groups')
    for row in commands:
        rc,expected=row.get('exit'),row.get('expected')
        valid=isinstance(rc,int) and rc!=0 if expected=='nonzero' else rc==expected
        if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):valid=False
        if row.get('passed') is not True or not valid:errors.append('Command result inconsistent')
    checks=data.get('checks',[])
    if not checks or any(r.get('passed') is not True for r in checks):errors.append('Failed/missing property checks')
    required={'first difference is preprocessed macro','assembly entry runtime closure',
      'Ninja no-op','flag-only change invalidates','same header has distinct absolute/relative graph nodes',
      'resource byte change invalidates generated header and consumer',
      'native/foreign ELF identity independently decoded','mislabeled target rejected before execution',
      'attribute distinguishes same-ISA calling convention','memory map change reuses unchanged compile artifact',
      'emulated image has no unresolved runtime helpers','offline missing cache rejects before build',
      'header-only change rejects old binary receipt','path mapping alone cannot normalize generated timestamp',
      'declared identical normalized inputs reproduce whole artifact','changed input rejects provenance reuse'}
    if not required<={r.get('name') for r in checks}:errors.append('Missing discriminating property checks')
    emulator=[r for r in commands if r.get('argv',[None])[0]=='qemu-system-arm' and '-M' in r['argv']]
    if len(emulator)!=1 or emulator[0].get('exit')!=0 or 'TOOLCHAIN_EMULATOR_PASS' not in emulator[0].get('stdout','')+emulator[0].get('stderr',''):errors.append('Missing actual emulator execution')
    semantic=[r for r in commands if r.get('argv',[None])[0].endswith('/leaked')]
    if len(semantic)!=1 or semantic[0].get('exit')!=1:errors.append('Wrong private-header semantic rejection')
    case=[r for r in commands if r.get('argv',[None])[0].endswith('/failure')]
    if len(case)!=2 or any(r.get('exit')!=1 or 'CHECK:wrong expected value' not in r.get('stderr','') for r in case):errors.append('Missing reproduced intended failure')
    if not data.get('generated_inputs'):errors.append('Missing inspectable generated inputs')
    return errors

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default='reviews/v5/expansion/legacy_systems_toolchain/toolchain_validation.json');args=p.parse_args()
    data=read_json(ROOT/args.report);errors=validate(data);guards=[]
    if not errors:
        bad=deepcopy(data);bad['observations'].pop();assert 'Missing exact objective/transfer observations' in validate(bad);guards.append('missing delayed variant rejected')
        bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale source') for e in validate(bad));guards.append('stale input rejected')
        bad=deepcopy(data)
        for row in bad['commands']:
            if row['argv'][0].endswith('/leaked'):row['exit']=-11
        assert 'Wrong private-header semantic rejection' in validate(bad);guards.append('crash is not semantic rejection')
        bad=deepcopy(data)
        for row in bad['commands']:
            if row.get('diagnostic')=='unused parameter':row['stdout']='';row['stderr']='compiler unavailable'
        assert 'Command result inconsistent' in validate(bad);guards.append('wrong diagnostic rejected')
        bad=deepcopy(data);bad['checks']=[r for r in bad['checks'] if r['name']!='resource byte change invalidates generated header and consumer'];assert 'Missing discriminating property checks' in validate(bad);guards.append('missing resource invalidation rejected')
    result={'passed':not errors,'errors':errors,'negative_guards':guards,'input_sha256':{args.report:sha(ROOT/args.report)}}
    dump(ROOT/'reviews/v5/expansion/legacy_systems_toolchain/validation_check.json',result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
