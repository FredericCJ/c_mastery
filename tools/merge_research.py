"""Merge bounded research evidence; do not substitute this structural check for review."""
from collections import Counter
import json
from pathlib import Path
from common import ROOT, dump, load_taxonomy

# Independently inspected scopes of the same edition share one public identity.
# Windows filenames are case-insensitive; reject other collisions rather than
# silently replacing a reading page.
ALIASES = {'R-higham-asna-2': 'R-HIGHAM-ASNA-2'}

def extend_inspection(prior,incoming,key,label):
    existing=prior.get(key,''); added=incoming.get(key,'Bounded inspection only.')
    if added in existing: return
    # A reused record may carry the entire accepted scope plus a short extension.
    # Keep its extension without duplicating the already retained source history.
    prior[key]=added if added.startswith(existing) else existing+label+added

def main():
    files=sorted((ROOT/'research/contributions').glob('*_resources.json'))
    if len(files)!=3:
        raise ValueError('Need all three research contributions')
    supplemental=ROOT/'references/supplemental_resources.json'
    if supplemental.exists(): files.append(supplemental)
    enhancement=[ROOT/f'research/enhancement/{group}_resources.json' for group in ['algorithms','communications','networking']]
    if any(p.exists() for p in enhancement):
        if not all(p.exists() for p in enhancement):
            raise ValueError('Need all three enhancement research contributions')
        files += enhancement
        files += sorted((ROOT/'research/enhancement').glob('*_additional_resources.json'))
    extra = ROOT/'references/enhancement_supplemental_resources.json'
    if extra.exists(): files.append(extra)
    new=[ROOT/f'research/performance_debugging/{group}_resources.json' for group in ['performance','debugging','memory']]
    if any(p.exists() for p in new):
        if not all(p.exists() for p in new): raise ValueError('Need all three performance/debugging research contributions')
        files+=new
        files+=sorted((ROOT/'research/performance_debugging').glob('*_additional_resources.json'))
    extra=ROOT/'references/performance_debugging_supplemental_resources.json'
    if extra.exists(): files.append(extra)
    result={}; sources={}; logs=[]
    for path in files:
        data=json.loads(path.read_text(encoding='utf-8-sig'))
        if isinstance(data,dict): data=data.get('resources',data.get('records',[]))
        for r in data:
            r['id'] = ALIASES.get(r['id'], r['id'])
            if r['id'] in result:
                prior=result[r['id']]
                if prior['url'].rstrip('/')!=r['url'].rstrip('/') or prior['edition']!=r['edition']:
                    raise ValueError('Conflicting resource identity '+r['id'])
                prior['domains']=sorted(set(prior['domains']+r['domains']))
                prior['locators']=list(dict.fromkeys(prior['locators']+r['locators']))
                extend_inspection(prior['inspection'],r['inspection'],'scope',' Additional independent researcher scope: ')
                extend_inspection(prior['inspection'],r['inspection'],'limits',' Additional independent researcher limits: ')
                prior['inspection']['date']=max(prior['inspection']['date'],r['inspection']['date'])
                if r['inspection']['method'] not in prior['inspection']['method']:
                    prior['inspection']['method']+='; '+r['inspection']['method']
                prior['inspection']['evidence_urls']=list(dict.fromkeys(prior['inspection'].get('evidence_urls',[])+r['inspection'].get('evidence_urls',[])))
                if r['inspection'].get('captures'):
                    captures=prior['inspection'].setdefault('captures',[])
                    for capture in r['inspection']['captures']:
                        if capture not in captures: captures.append(capture)
                sources[r['id']].append(path.name)
            else:
                result[r['id']]=r; sources[r['id']]=[path.name]
    for path in sorted((ROOT/'research/contributions').glob('*_search_log.json')) + sorted((ROOT/'research/enhancement').glob('*_search_log.json')) + sorted((ROOT/'research/performance_debugging').glob('*_search_log.json')):
        data=json.loads(path.read_text(encoding='utf-8-sig'))
        logs+=data if isinstance(data,list) else data.get('records',data.get('entries',[]))
    folded = [key.casefold() for key in result]
    if len(folded) != len(set(folded)):
        raise ValueError('Case-insensitive resource filename collision')
    counts=Counter(d for r in result.values() for d in r['domains'])
    domain_ids=[d['id'] for d in load_taxonomy()]
    missing=[d for d in domain_ids if not counts[d]]
    if missing: raise ValueError('No research support for '+str(missing))
    dump(ROOT/'references/resources.json',{'resources':sorted(result.values(),key=lambda r:r['id'])})
    dump(ROOT/'references/inspection_ledger.json',{'records':[{'resource_id':r['id'],**r['inspection'],'locators':r['locators'],'contributions':sources[r['id']]} for r in result.values()]})
    dump(ROOT/'research/search_log.json',{'entries':logs})
    dump(ROOT/'reviews/research_structure.json',{'sources':len(result),'search_entries':len(logs),'domain_support':dict(counts),'all_domains_supported':not missing})
    print(f'Merged {len(result)} resources and {len(logs)} research entries. All {len(domain_ids)} domains supported. Research acceptance still required.')

if __name__=='__main__': main()
