"""Require exact T09 ordinary/changed evidence and reject weakened report scopes."""
from copy import deepcopy
import argparse,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={f'T09.{topic:02}.{leaf}' for topic in range(1,13) for leaf in 'abc'}
MODULES={'decoder','linker','dynamic','layout','interfaces'}

def validate(data):
    errors=[]
    if data.get('passed') is not True or data.get('error') is not None:errors.append('Campaign failed')
    if set(data.get('selected_modules',[]))!=MODULES:errors.append('Focused run is not full evidence')
    pins=data.get('input_sha256',{})
    if not pins:errors.append('Missing pins')
    for p,h in pins.items():
        file=(ROOT/p).resolve()
        if not file.is_relative_to(ROOT.resolve()) or not file.is_file() or sha(file)!=h:errors.append('Stale source: '+p)
    observations=data.get('observations',[])
    if len(observations)!=72 or {(r.get('objective'),r.get('variant')) for r in observations}!={(o,v) for o in OBJECTIVES for v in (False,True)}:errors.append('Missing exact ordinary or delayed objective')
    commands=data.get('commands',[]);checks=data.get('checks',[])
    if not commands or not checks:errors.append('Missing execution evidence')
    for row in commands:
        rc=row.get('exit');expected=row.get('expected');valid=isinstance(rc,int) and rc>0 if expected=='nonzero' else rc==expected
        if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):valid=False
        if not valid or row.get('passed') is not True:errors.append('Command evidence inconsistent')
    if any(r.get('passed') is not True for r in checks):errors.append('Failed property check')
    required={'independent valid decoder declared-load-ranges','intended rejection ELF32-address-wrap',
      'semantic range mutant or learner rejection mutant','semantic range mutant or learner rejection learner',
      'Arm decoder has no implicit runtime helper','independent absolute and PC-relative fields',
      'static PIE still has relocation obligations','hidden internal target eliminates PLT relocation in these final images',
      'declared GNU priority and reverse finalization order','metadata needs explicitly selected consumer 0',
      'metadata needs explicitly selected consumer 1','metadata needs explicitly selected consumer 2',
      'independently derived run/load intervals','post-link policy rejects unapproved allocatable orphan before use',
      'registry stage plain/unextracted','registry stage lto/extracted','registry stage plain/collected',
      'registry stage lto/not-enumerated','numeric values do not create callback reachability edges',
      'GDB rejects wrong named companion for intended CRC reason','actual patched file matches independent excluded-field model',
      'nonzero HEX fill changes actual records/checksums and addressed byte count','HEX checksum missing-EOF and overlap controls rejected',
      'only intended operation is exported'}
    if not required<={r.get('name') for r in checks}:errors.append('Missing discriminating property')
    named=lambda name:[r for r in commands if r.get('argv') and r['argv'][0].split('/')[-1]==name]
    profiles=[r for r in commands if r.get('stdin_file')]
    if len(profiles)!=6 or any(r['exit']!=0 or not r.get('stdin_sha256') for r in profiles):errors.append('Missing six decoder profile executions')
    if len(named('tls-host'))!=1 or 'TLS-LIVE-PASS' not in named('tls-host')[0]['stdout']:errors.append('Missing late TLS execution')
    probe=named('probe')
    if len(probe)!=2 or sorted(r['exit'] for r in probe)!=[-11,0] or 'ABI-GATE-PASS' not in next(r['stdout'] for r in probe if r['exit']==0):errors.append('Missing repaired and intentionally faulty ABI gate pair')
    if len(named('client-old'))!=4 or len(named('client-new'))!=2 or sorted(r['exit'] for r in named('client-old')+named('client-new'))!=[0,0,0,1,1,33]:errors.append('Missing actual six-cell version matrix')
    generated=data.get('generated_inputs',{})
    if not generated or 'layout/image.ld' not in generated:errors.append('Missing original specimen/script bytes')
    for path in ('dynamic/startup-samples.json','dynamic/binding-samples.json'):
        try:
            samples=json.loads(generated[path]['content'])
            if len(samples)!=22 or sum(not r['warmup'] for r in samples)!=18:errors.append('Missing paired timing samples')
        except (KeyError,ValueError):errors.append('Missing paired timing samples')
    return errors

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default='reviews/v5/expansion/legacy_systems_elf/full_validation.json');a=p.parse_args()
    data=read_json(ROOT/a.report);errors=validate(data);guards=[]
    if not errors:
        mutations=[('missing changed objective',lambda d:d['observations'].pop(),'Missing exact ordinary or delayed objective'),
          ('focused scope',lambda d:d['selected_modules'].pop(),'Focused run is not full evidence'),
          ('missing TLS run',lambda d:d.update(commands=[r for r in d['commands'] if not r['argv'][0].endswith('/tls-host')]),'Missing late TLS execution'),
          ('missing property',lambda d:d.update(checks=[r for r in d['checks'] if r['name']!='numeric values do not create callback reachability edges']),'Missing discriminating property'),
          ('missing sample',lambda d:d['generated_inputs'].pop('dynamic/binding-samples.json'),'Missing paired timing samples'),
          ('version semantic failure replaced with crash',lambda d:[r.update(exit=-11) for r in d['commands'] if r['argv'][0].endswith('/client-old') and r['exit']==33],'Missing actual six-cell version matrix')]
        for name,mutate,expected in mutations:
            bad=deepcopy(data);mutate(bad);assert expected in validate(bad),name;guards.append(name+' rejected')
        bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale source') for e in validate(bad));guards.append('stale source rejected')
    result={'passed':not errors,'errors':errors,'negative_guards':guards,'input_sha256':{a.report:sha(ROOT/a.report)}}
    dump(ROOT/'reviews/v5/expansion/legacy_systems_elf/validation_check.json',result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
