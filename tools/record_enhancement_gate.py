"""Record the integrator's explicit source review; not an automatic content review."""
from datetime import datetime, timezone
import argparse
from common import ROOT, read_json, dump, sha

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('group',choices=['algorithms','communications','networking'])
    parser.add_argument('--rationale',required=True)
    args=parser.parse_args()
    groups={'algorithms':['T36','T37'],'communications':['T38','T39'],'networking':['T40','T41','T42']}
    paths=[f'research/enhancement/{args.group}_{suffix}' for suffix in
           ['reflection.md','synthesis.md','resources.json','search_log.json','taxonomy.json']]
    if args.group=='networking': paths.append('research/enhancement/networking_contracts.json')
    additions=sorted((ROOT/'research/enhancement').glob(f'{args.group}*_additional_resources.json'))
    paths += [p.relative_to(ROOT).as_posix() for p in additions]
    if (ROOT/'references/enhancement_supplemental_resources.json').exists():
        paths.append('references/enhancement_supplemental_resources.json')
    if any(not (ROOT/p).is_file() for p in paths):
        raise ValueError('Missing staged research artifact; no gate recorded')
    resources=read_json(ROOT/f'research/enhancement/{args.group}_resources.json')
    resources=resources if isinstance(resources,list) else resources.get('resources',resources.get('records',[]))
    for path in additions:
        added=read_json(path)
        resources += added if isinstance(added,list) else added.get('resources',added.get('records',[]))
    if any(not r.get('locators') or not r.get('inspection',{}).get('scope') for r in resources):
        raise ValueError('Missing inspected source scope')
    target=ROOT/'reviews/enhancement_research_gate.json'
    gate=read_json(target) if target.exists() else {'reviewer':'/root','groups':{}}
    gate['groups'][args.group]={'accepted':True,'domains':groups[args.group],
        'review_date_utc':datetime.now(timezone.utc).isoformat(),'rationale':args.rationale,
        'research_input_sha256':{p:sha(ROOT/p) for p in paths}}
    accepted=sorted({d for g in gate['groups'].values() for d in g['domains'] if g['accepted']})
    gate.update({'accepted_domains':accepted,'accepted':accepted==[f'T{i:02}' for i in range(36,43)],
                 'scope':'Staged source support for teaching; no learner implementation, hardware or protocol conformance acceptance',
                 'research_input_sha256':{p:h for g in gate['groups'].values() for p,h in g['research_input_sha256'].items()}})
    dump(target,gate)
    print('Research domains accepted:',accepted,'; aggregate publication gate:',gate['accepted'])

if __name__=='__main__':
    main()
