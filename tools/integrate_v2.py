"""Reconcile v2 research identities, project dependencies and learner directories."""
from __future__ import annotations
from collections import defaultdict
import hashlib
import json
import re
from common import ROOT, dump, load, read_json, write

def records(value, key):
    return value if isinstance(value,list) else value.get(key,[])

def register_addition(seen, resource):
    previous=seen.get(resource['id'])
    if previous is not None and previous!=resource:
        raise ValueError('Conflicting second-edition source ID: '+resource['id'])
    seen[resource['id']]=resource

def registry():
    path=ROOT/'references/resources.json'
    value=read_json(path)
    resources={r['id']:r for r in value['resources']}
    additions_seen={}
    for additions in sorted((ROOT/'research/v2').glob('*_resources.json')):
        for resource in records(read_json(additions),'resources'):
            if resource.get('reuse'):
                if resource['id'] not in resources: raise ValueError('Unknown reused resource: '+resource['id'])
                continue
            register_addition(additions_seen,resource)
            resources[resource['id']]=resource
    works, editions, inspections, aliases = {}, {}, {}, {}
    for key, resource in sorted(resources.items()):
        resource.setdefault('kind','manual')
        resource['assignable']=resource.get('assignable',bool(resource.get('locators') and resource.get('inspection',{}).get('scope')))
        resource['assignable']=bool(resource['assignable'])
        identity=re.sub(r'\W+',' ',resource['title'].casefold()).strip()+'|'+str(resource.get('authors',[]))
        work='W-'+hashlib.sha256(identity.encode()).hexdigest()[:16]
        edition='E-'+hashlib.sha256((work+'|'+str(resource['edition'])).encode()).hexdigest()[:16]
        event='I-'+hashlib.sha256(json.dumps(resource['inspection'],sort_keys=True).encode()).hexdigest()[:16]
        works.setdefault(work,{'id':work,'title':resource['title'],'authors':resource.get('authors',[])})
        editions.setdefault(edition,{'id':edition,'work_id':work,'edition':resource['edition'],'year':resource.get('year'),'resource_aliases':[]})['resource_aliases'].append(key)
        inspections.setdefault(event,{'id':event,'edition_ids':[],'inspection':resource['inspection'],'locators':[]})
        if edition not in inspections[event]['edition_ids']: inspections[event]['edition_ids'].append(edition)
        inspections[event]['locators']=sorted(set(inspections[event]['locators']+resource.get('locators',[])))
        aliases[key]={'work_id':work,'edition_id':edition,'inspection_ids':[event],'assignable':resource['assignable']}
        resource['work_id']=work; resource['edition_id']=edition; resource['inspection_ids']=[event]
    value['resources']=list(resources.values()); dump(path,value)
    dump(ROOT/'references/registry_v2.json',{'schema_version':2,'identity_policy':'Exact normalized title/authors identity; ambiguous similar titles retain distinct works until inspected reconciliation.',
        'works':list(works.values()),'editions':list(editions.values()),'inspections':list(inspections.values()),'aliases':aliases})

def taxonomy():
    domains=[]
    for path in sorted((ROOT/'research/v2').glob('*_taxonomy.json')):
        value=read_json(path)
        domains+=value.get('domains',[value] if 'id' in value else [])
    if domains: dump(ROOT/'curriculum/v2_taxonomy.json',{'edition':2,'domains':sorted(domains,key=lambda d:d['id'])})

def graph(preserved=None, preserved_order=None):
    taxonomy, modules, projects, resources=load()
    module_projects={key:list(value['meta']['projects']) for key,value in modules.items()}
    integration_families=read_json(ROOT/'curriculum/integration_families.json')['families']
    integration_owner={p:f for f in integration_families for p in f['milestones']}
    producer=defaultdict(list)
    for key, project in projects.items():
        for topic in project['meta']['topics']: producer[topic].append(key)
    family_map=defaultdict(list); laboratory_contracts={}
    for path in sorted((ROOT/'labs').glob('*/family.json')):
        entries=read_json(path).get('projects',{})
        items=entries.keys() if isinstance(entries,dict) else [x if isinstance(x,str) else x['id'] for x in entries]
        for key in items: family_map[key].append(path.parent.name)
        detailed=[dict(value,id=key) for key,value in entries.items()] if isinstance(entries,dict) else entries
        for entry in detailed:
            if isinstance(entry,dict): laboratory_contracts[(path.parent.name,entry['id'])]=entry
    nodes=[]
    for key,project in sorted(projects.items()):
        meta=project['meta']; owner=meta['module']
        if 'requires_projects' in meta:
            deps=list(meta['requires_projects'])
        else:
            deps=[]
            if owner in modules:
                sequence=module_projects[owner]; position=sequence.index(key)
                if position: deps.append(sequence[position-1])
                for prerequisite in modules[owner]['meta']['prerequisites']:
                    seq=module_projects[prerequisite]
                    required=[p for p in seq if projects[p]['meta'].get('status')!='elective']
                    if required: deps.append(required[-1])
            elif owner=='INTEGRATION':
                # A capstone is a consumer of components, never an entry task.
                deps += [pid for pid,p in projects.items() if p['meta']['module']!='INTEGRATION'
                         and set(p['meta']['topics']) & set(meta['topics']) and p['meta']['status']!='elective']
                family=integration_owner[key]; sequence=family['milestones']; position=sequence.index(key)
                if position: deps.append(sequence[position-1])
                reuse={'CAP-LINUX':['P1','P2','P3','P5','P6'],'CAP-MCU':['P1','P3','P4','P6']}.get(family['id'],[])
                deps += [f['milestones'][-1] for f in integration_families if f['id'] in reuse]
        # Selected objectives are evidence prerequisites, not a demand to finish
        # the owning domain. Their producer links support targeted study.
        inherited=modules[owner]['meta'].get('prelude_objectives',[]) if owner in modules else []
        if 'prelude_override' in meta:
            inherited=[]
        preludes=list(dict.fromkeys(inherited+meta.get('prelude_override',meta.get('prelude_objectives',[]))))
        if key in deps: raise ValueError('Project requires itself: '+key)
        objectives=[o['id'] for d in taxonomy for t in d['subtopics'] if t['id'] in meta['topics'] for o in t['objectives']]
        manual=['mechanism explanation','independent changed-condition evidence','delayed transfer']
        if owner in {'T27','T28','T29','T30','T31','T38','T39','T40','T41'}: manual.append('physical observation for every applicable hardware claim')
        if owner in {'T43','T44','T45'}: manual.append('actual tool/measurement dossier under the assigned conditions')
        if owner in {'T50','T54'} or key=='PR-T56-12': manual.append('independent physical target observations for board claims; image builds alone do not close this gate')
        if owner=='T52': manual.append('task-model assumptions and independent scheduling argument; measured maxima are not universal WCET bounds')
        candidates=family_map.get(key,[])
        preferred='algorithms' if owner in {'T14','T15','T16','T17','T36','T37'} else 'foundations'
        primary=preferred if preferred in candidates else (candidates[0] if candidates else None)
        nodes.append({'id':key,'title':meta['title'],'module':owner,'status':meta['status'],'hours':meta['hours'],
            'requires':sorted(set(deps)-{key}), 'prelude_objectives':preludes,
            'prelude_producers':{leaf:producer.get(leaf.rsplit('.',1)[0],[]) for leaf in preludes},
            'objectives':objectives,'produces':[key+'/accepted-component-and-dossier'],
            'consumes':[p+'/accepted-component-and-dossier' for p in sorted(set(deps)-{key})],
            'fixtures':meta['fixtures'],'lab_family':primary,'lab_families':candidates,
            'manual_evidence':manual,'evidence_template':meta.get('evidence_template','component')})
    # Later editions may retain a historical project view while adding explicit
    # unit/exercise adapters. Discovering a new lab does not rewrite that view.
    if preserved:
        missing = set(preserved) - {node['id'] for node in nodes}
        if missing:
            raise ValueError('Accepted projects disappeared from canonical input: ' + ', '.join(sorted(missing)))
        if preserved_order is not None and (len(preserved_order) != len(set(preserved_order)) or set(preserved_order) != set(preserved)):
            raise ValueError('Accepted project route must contain each preserved project exactly once')
        for node in nodes:
            old = preserved.get(node['id'])
            if old is None:
                continue
            # Missing and present-with-None are different contracts as well.
            changed = (set(node) ^ set(old)) | {key for key in set(node) & set(old) if node[key] != old[key]}
            if changed - {'lab_family', 'lab_families'}:
                raise ValueError('Accepted project semantics changed: ' + node['id'])
            for key in ('lab_family', 'lab_families'):
                if key in old:
                    node[key] = old[key]
                else:
                    node.pop(key, None)
    by_id={n['id']:n for n in nodes}; done=set(); order=[]
    while len(done)<len(nodes):
        ready=[n for n in nodes if n['id'] not in done and set(n['requires'])<=done]
        if not ready: raise ValueError('Project prerequisite cycle or missing project: '+str(sorted(set(by_id)-done)))
        early={'PR-T44-01','PR-T44-02','PR-T44-03','PR-T45-01','PR-T45-02','PR-T45-03','PR-T43-01','PR-T43-02'}
        ready.sort(key=lambda n:(0 if n['id'] in early else 1,n['module'],n['id']))
        key=ready[0]['id']
        if preserved_order and len(order) < len(preserved_order):
            key = preserved_order[len(order)]
            if key not in {row['id'] for row in ready}:
                raise ValueError('Accepted project route is no longer ready: ' + key)
        done.add(key);order.append(key)
    if preserved_order is not None and order[:len(preserved_order)] != preserved_order:
        raise ValueError('Generated project route does not retain the complete accepted prefix')
    dump(ROOT/'curriculum/project_graph.json',{'schema_version':2,'order':order,'projects':nodes,
        'policy':'Whole-project dependencies and selected-objective evidence gates are distinct. Reading a prelude alone does not mark it mastered.',
        'completion':'All required projects, objective evidence, delayed transfer and physical interfaces; automated suites establish only their own scope.'})
    lines=['# Project route and reusable artifacts','',
        'Follow this ordered project route after the entry diagnostic. Complete selected objective cards before their dependent experiments; a card does not require completing its entire domain. Early diagnosis and memory experiments recur later at greater depth.', '',
        'Each accepted artifact carries its source revision, interface, ownership contract and evidence. Reusing it transfers only claims whose assumptions remain true. The directory does not add workload to the owning projects.', '',
        '| Step | Project | New work | Prerequisite projects | Executable family |','|---|---|---|---|---|']
    for i,key in enumerate(order,1):
        node=by_id[key]
        lines.append('| '+str(i)+' | {ref}`'+key+'` | '+node['title'].replace('|','/')+' | '+', '.join('{ref}`'+p+'`' for p in node['requires'])+' | '+(node['lab_family'] or 'See project-specific evidence contract')+' |')
    write(ROOT/'curriculum/guides/project-route.md','\n'.join(lines)+'\n')
    evidence=['# Executable evidence directory','',
        'Every row describes supplied executable support, with its exact limits. The canonical project contract still governs the full assignment. Tests of visible anchors and supplied observation controls are not automatic objective or whole-project acceptance. Projects without a row retain their independent project-specific implementation/evidence contracts.', '',
        'When a project has more than one family, choose the additional workspace explicitly with `cmastery init PROJECT PATH --family NAME`. Reuse completed components and charge their work once.', '',
        '| Project | Family | Named tests | Scope and remaining evidence |','|---|---|---|---|']
    for (family,key),contract in sorted(laboratory_contracts.items(),key=lambda item:(item[0][1],item[0][0])):
        if preserved and key in preserved and family not in preserved[key].get('lab_families', []):
            continue
        scope=contract.get('scope','Finite mechanism checks; full project dossier and transfer remain required.')
        scope+=' '+contract.get('remaining_evidence','')
        evidence.append('| {ref}`'+key+'` | '+family+' | '+', '.join('`'+t+'`' for t in contract.get('tests',[]))+' | '+scope.replace('|','/').replace('\n',' ')+' |')
    write(ROOT/'curriculum/guides/executable-evidence.md','\n'.join(evidence)+'\n')
    print({'projects':len(nodes),'ordered':len(order),'executable_family_links':sum(bool(n['lab_family']) for n in nodes)})

def main():
    taxonomy(); registry(); graph()

if __name__=='__main__': main()
