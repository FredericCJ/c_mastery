"""Generate the active contextual route and lossless objective/migration maps.

This producer reads public canonical sources only. Historical catalog records
are never edited. --check compares expected output without modifying any file.
"""
from __future__ import annotations
import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import sys
from common import ROOT, load_taxonomy, parse

BASE = 'curriculum/v5/routes/'


def read(name):
    return json.loads((ROOT/name).read_text(encoding='utf-8-sig'))


def generate():
    source = read(BASE+'contextual-v1.source.json')
    units = read('curriculum/v5/units.json')['units']
    exercises = read('curriculum/v5/exercises.json')['exercises']
    project_entries={}
    for unit in units:project_entries.setdefault(unit['project'],unit)
    artifacts = {r['id']:r for r in read('curriculum/v5/artifacts.json')['artifacts']}
    for artifact in source.get('component_contracts', []):
        if artifact['id'] in artifacts:
            raise ValueError('Active component must not replace historical identity')
        artifacts[artifact['id']] = artifact
        artifact['required_header_sha256'] = hashlib.sha256((ROOT/artifact['required_header']).read_bytes()).hexdigest()
    domains = {d:c['id'] for c in source['contexts'] for d in c['domains']}
    contexts = {c['id']:c for c in source['contexts']}
    from native_successors import resolve
    resolutions=[]
    for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')):
        resolutions+=resolve(json.loads(path.read_text(encoding='utf-8-sig')),ROOT,source.get('successor_dispositions'))
    resolution_by_project=defaultdict(list)
    for row in resolutions:resolution_by_project[row['project']].append(row)
    nodes=[]
    for kind, rows in [('unit',units),('exercise',exercises)]:
        for value in rows:
            key=value['id'];project=value['project']
            domain=value.get('domain') or (project.split('-')[1] if project.startswith('PR-T') else 'INTEGRATION')
            context=source['project_context'].get(project,domains.get(domain,'integration'))
            required=value.get('requires',[]) if kind=='unit' else value.get('prerequisite_exercises',[])+value.get('prerequisite_units',[])
            edges=[dict(kind='concept',target=k,reason=value.get('requires_reasons',{}).get(k,'Pass the referenced mechanism assessment before independent reuse.')) for k in required]
            if kind=='exercise' and source.get('exercise_prerequisite_policy',{}).get('inherit_parent_entry_upstream'):
                policy=source['exercise_prerequisite_policy']
                if key not in policy.get('portable_core_exceptions',{}):
                    parent=project_entries[project]
                    for prior in parent.get('requires',[]):
                        if prior not in required:
                            edges.append(dict(kind='concept',target=prior,derived_from=parent['id'],
                                              reason='Reuse the parent entry prerequisite: '+parent.get('requires_reasons',{}).get(prior,'assessed prior mechanism')+'. This is conceptual preparation for the extension; it does not import a hidden implementation.'))
            imports=value.get('imports',[]) if kind=='unit' else value.get('prerequisite_artifacts',[])
            for aid in imports:
                a=artifacts[aid]
                # Preserve inherited import gating: a later unit still carries its actual import.
                edges=[e for e in edges if e['target']!=a['producer_unit']]
                edges.append(dict(kind='component',target=a['producer_unit'],artifact=aid,contract_version=a['contract_version'],reason=a['public_contract']))
            family=value.get('family') if kind=='unit' else value['lab']['family']
            if family:
                edges.append(dict(kind='environment',target='cc',reason='Build the component with an identified C17 compiler.'))
            supplied=[]
            contracts=[];sources=['curriculum/v5/units.json' if kind=='unit' else value['brief'],'curriculum/projects/'+project+'.md']
            assessment=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
            if assessment.exists():
                data=json.loads(assessment.read_text(encoding='utf-8-sig'));contracts+=data['shared_contracts'];sources.append(assessment.relative_to(ROOT).as_posix())
            if kind=='exercise':contracts+=value['shared_contracts']
            if project=='PR-T24-05':
                contracts += ['labs/hosted_systems/timer_depth/CONTRACTS.md']
                optional=ROOT/'labs/hosted_systems/timer_depth/COLLECTION.md'
                if optional.exists():contracts.append(optional.relative_to(ROOT).as_posix())
                supplied += [p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/hosted_systems/timer_depth').glob('*') if p.is_file() and p.suffix in {'.c','.h','.md','.py'}]
            if project=='PR-T24-03':contracts+=['labs/hosted_systems/stream_depth/CONTRACTS.md']
            if project=='PR-T43-09':
                contracts += ['labs/performance_followthrough/PMU_PROTOCOL.md']
                supplied += ['labs/performance_followthrough/'+name for name in ['pmu.h','pmu.c','pmu_linux.c','pmu_tests.c','publication.c','publication.h','build.py','PMU_PROTOCOL.md']]
            public_areas=[]
            if project in {'PR-T14-01','PR-T14-05','PR-T14-08'}:public_areas=['labs/containers_depth']
            if project=='PR-T15-05':public_areas=['labs/algorithm_depth']
            if project=='PR-T36-07':public_areas=['labs/algorithm_frontier']
            if project=='PR-T03-02' and kind=='unit':public_areas=['labs/object_followthrough/cursor']
            for area in public_areas:
                supplied += [p.relative_to(ROOT).as_posix() for p in (ROOT/area).glob('*') if p.is_file() and p.suffix in {'.c','.h','.md','.py'}]
            supplied += [a['required_header'] for a in source.get('component_contracts', []) if a['producer_unit']==key]
            supplied=list(dict.fromkeys(supplied))
            selected_resolutions=[r for r in resolution_by_project[project] if r['objective'] in value['objectives']]
            for resolution in selected_resolutions:contracts+=resolution['public_contracts']
            contracts=list(dict.fromkeys(contracts))
            nodes.append(dict(id=key,kind=kind,project=project,title=value['title'],context=context,
                              objectives=value['objectives'],prerequisites=edges,contracts=contracts,sources=sources,supplied_artifacts=supplied,criterion_resolutions=selected_resolutions,
                              why_now=contexts[context]['problem']+' This increment: '+value['title']+'.',
                              next_consumer='Selected after prerequisite ordering.',required=value.get('required',value.get('status')!='elective'),
                              assessment=dict(kind='existing-scoped-assessment',tests=value.get('tests',[]) if kind=='unit' else value['lab']['tests'],
                                              limits=value.get('remaining_evidence', 'Reasoning, independence and changed conditions require explicit evidence.'))))
            if project=='PR-T15-05':
                nodes[-1]['contract_scope']='Implement the selected graph objectives using the imported struct cd_csr and local traversal/worklist storage. The shared T15 document also covers byte and sorting producers: its fr_* and heap-sort imports apply to those operations, not to ad_bfs/ad_dijkstra. The later frontier refinement separately imports the assessed struct cd_ring/struct cd_heap producers.'
    byid={n['id']:n for n in nodes}
    # Place the compact vocabulary after actual C setup and bounds work.
    first='U-T01-01-01'
    for prior in ['U-T01-03-03','U-T03-02-03','U-T04-01-04']:
        if prior in byid and first in byid:
            byid[first]['prerequisites'].append(dict(kind='concept',target=prior,reason='Use defined indexing and checked arithmetic when proving the first algorithms.'))
    byid['U-T43-09-03']['prerequisites'].append(dict(kind='concept',target='U-T55-01-03',reason='Sampled-ring consumption requires the acquired publication-order argument; earlier counting work does not require it.'))
    for identifier, edges in source.get('additional_prerequisites', {}).items():
        byid[identifier]['prerequisites'] += edges
    for group in source.get('exercise_concept_groups', []):
        for identifier in group['exercises']:
            if byid[identifier]['kind']!='exercise':raise ValueError('Exercise concept group names a baseline unit')
            for prior in group['requires']:
                if not any(edge['target']==prior and edge['kind'] in {'concept','component'} for edge in byid[identifier]['prerequisites']):
                    byid[identifier]['prerequisites'].append(dict(kind='concept',target=prior,reason=group['reason'],
                                                               derived_from=group['source']))
    for entry in nodes:
        gates={}
        for edge in entry['prerequisites']:
            signature=(edge['kind'],edge['target'],edge.get('artifact'),edge.get('contract_version'))
            gates.setdefault(signature,edge)
        entry['prerequisites']=list(gates.values())
    for identifier, increment in source.get('increments', {}).items():
        byid[identifier]['active_increment'] = increment
        byid[identifier]['why_now'] = increment['task']
    hard={n['id']:{e['target'] for e in n['prerequisites'] if e['kind'] in {'concept','component'}} for n in nodes}
    original={n['id']:i for i,n in enumerate(nodes)}
    preferred={k:i for i,k in enumerate(source['foundation_order'])}
    # Promote real ancestors, never future artifacts. Round-robin contexts among
    # ready work means the subject catalog cannot dictate the encounter order.
    ranks=dict(preferred)
    def promote(key,rank,seen):
        if key in seen:return
        seen.add(key);ranks[key]=min(ranks.get(key,1e9),rank)
        for p in sorted(hard[key]):promote(p,rank-.01,seen)
    for key,rank in preferred.items():
        if key in hard:promote(key,rank,set())
    pending=set(byid);order=[];done=set();last=defaultdict(lambda:-1)
    while pending:
        ready=[k for k in pending if hard[k]<=done]
        if not ready:raise ValueError('Route cycle/dangling prerequisites: '+str({k:sorted(hard[k]-done) for k in sorted(pending)[:8]}))
        ready.sort(key=lambda k:(ranks.get(k,1e9),last[byid[k]['context']],original[k]))
        key=ready[0];pending.remove(key);done.add(key);order.append(key);last[byid[key]['context']]=len(order)
    for key in order:
        consumers=[k for k in order if key in hard[k]]
        byid[key]['next_consumer']=', '.join(consumers[:4]) or 'Independent integrated defense in the '+byid[key]['context']+' context.'
    coverage=[]
    for domain in load_taxonomy():
        for topic in domain['subtopics']:
            for objective in topic['objectives']:
                entries=[k for k in order if objective['id'] in byid[k]['objectives']]
                if not entries:raise ValueError('Lost objective: '+objective['id'])
                criteria=[]
                for project in sorted({byid[k]['project'] for k in entries}):
                    path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
                    if path.exists():
                        criteria += [dict(id=c['id'],source=path.relative_to(ROOT).as_posix()) for c in json.loads(path.read_text(encoding='utf-8-sig'))['criteria'] if c['objective']==objective['id']]
                coverage.append(dict(objective=objective['id'],text=objective['text'],encounters=entries,criteria=criteria,
                                     contexts=sorted({byid[k]['context'] for k in entries}),disposition='retained',
                                     limits='Coverage links identify authored obligations, not completed assessments.'))
    mechanisms=[]
    for mechanism in source['mechanisms']:
        bound=dict(mechanism)
        project_nodes=[[k for k in order if byid[k]['project']==project] for project in mechanism['projects']]
        project_nodes=[values for values in project_nodes if values]
        flat=[key for values in project_nodes for key in values]
        bound['encounter_map']={
            'motivated_introduction':project_nodes[0][:1],
            'derivation':project_nodes[0][:2],
            'implementation':project_nodes[0],
            'reuse':[values[0] for values in project_nodes[1:]],
            'changed_assumption':[values[-1] for values in project_nodes],
            'independent_transfer':{'contract':mechanism['independent_transfer'], 'assessment_contexts':flat[-2:], 'status':'authored requirement; no independent result inferred'}}
        mechanisms.append(bound)
    migration=dict(schema_version=1,from_route='v5-catalog-order',to_route=source['id'],
                   records=[dict(id=n['id'],successor=n['id'],disposition='reordered-context',acceptance='Retain the original contract and receipt; added route prerequisites do not grant new acceptance.') for n in nodes],
                   objective_ids=[c['objective'] for c in coverage],ledger_policy='Do not rewrite old ledger entries. Explicit adoption records a route-migrated event; new profiles select contextual-v1.')
    route=dict(schema_version=1,id=source['id'],catalog='curriculum/v5',contexts=source['contexts'],nodes=nodes,order=order,
               coverage=BASE+'coverage.json',migration=BASE+'migration.json',mechanisms=mechanisms,
               component_contracts=source.get('component_contracts', []),
               pilots=[dict(id='streaming-to-perf',teaching=BASE+'streaming.md',units=[k for k in order if byid[k]['project'] in {'PR-T14-01','PR-T24-03','PR-T55-01','PR-T43-09'}]),
                       dict(id='deadline-structures',teaching=BASE+'deadlines.md',units=[k for k in order if byid[k]['project'] in {'PR-T14-05','PR-T24-05','PR-T75-03'}])],
               policy=source['policy'],semantic_review=dict(record=BASE+'semantic-review.md',status='authored-review-with-explicit-remaining-work'))
    return {'criterion_resolutions.json':dict(schema_version=1,criteria=resolutions,limits='Exact explicit successor links resolved to public teaching; unresolved metadata bindings remain visible.'),'contextual-v1.json':route,'coverage.json':dict(schema_version=1,objectives=coverage), 'migration.json':migration}


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');args=parser.parse_args()
    errors=[]
    for name,value in generate().items():
        text=json.dumps(value,ensure_ascii=False,indent=2)+'\n';path=ROOT/BASE/name
        if args.check:
            if not path.exists() or path.read_text(encoding='utf-8')!=text:errors.append('Stale generated route: '+name)
        else:path.write_text(text,encoding='utf-8')
    if errors:raise SystemExit('\n'.join(errors))
    print('Active route generation '+('verified' if args.check else 'complete'))

if __name__=='__main__':main()
