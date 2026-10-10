"""Record explicit integrator source acceptance after substantive review."""
import argparse
from datetime import datetime,timezone
from common import ROOT,read_json,dump,sha

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('group',choices=['performance','debugging','memory'])
    parser.add_argument('--rationale',required=True)
    args=parser.parse_args(); keys={'performance':['T43'],'debugging':['T44'],'memory':['T45']}
    paths=[f'research/performance_debugging/{args.group}_{suffix}' for suffix in ['reflection.md','search_log.json','resources.json','synthesis.md','taxonomy.json']]
    paths += [f'research/performance_debugging/{name}' for name in ['protocol.md','integrator_reflection.md','integrator_search_log.json']]
    protocol=ROOT/f'research/performance_debugging/{args.group}_protocol.md'
    if protocol.exists(): paths.append(protocol.relative_to(ROOT).as_posix())
    additions=sorted((ROOT/'research/performance_debugging').glob(f'{args.group}*_additional_resources.json'))
    paths+=[p.relative_to(ROOT).as_posix() for p in additions]
    supplemental=ROOT/'references/performance_debugging_supplemental_resources.json'
    if supplemental.exists(): paths.append(supplemental.relative_to(ROOT).as_posix())
    if any(not (ROOT/path).is_file() for path in paths): raise ValueError('Missing staged source artifact')
    resources=[]
    for path in [ROOT/f'research/performance_debugging/{args.group}_resources.json']+additions:
        value=read_json(path)
        resources+=value if isinstance(value,list) else value.get('resources',value.get('records',[]))
    if not resources or any(not r.get('locators') or not r.get('inspection',{}).get('scope') or not r.get('inspection',{}).get('limits') for r in resources):
        raise ValueError('Missing bounded, inspected source support')
    target=ROOT/'reviews/performance_debugging_research_gate.json'
    gate=read_json(target) if target.exists() else {'reviewer':'/root','groups':{}}
    gate['groups'][args.group]={'accepted':True,'domains':keys[args.group],
        'review_date_utc':datetime.now(timezone.utc).isoformat(),'rationale':args.rationale,
        'research_input_sha256':{path:sha(ROOT/path) for path in paths}}
    accepted=sorted({d for g in gate['groups'].values() if g['accepted'] for d in g['domains']})
    gate.update({'accepted_domains':accepted,'accepted':accepted==['T43','T44','T45'],
        'scope':'Inspected source support for curriculum authoring, not learner execution or diagnostic effectiveness',
        'research_input_sha256':{p:h for g in gate['groups'].values() for p,h in g['research_input_sha256'].items()}})
    dump(target,gate); print('Research accepted:',accepted,'Aggregate publication gate:',gate['accepted'])

if __name__=='__main__': main()
