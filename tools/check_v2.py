"""Validate migration, source identities, project routing and lab declarations."""
from __future__ import annotations
from copy import deepcopy
import json
from common import ROOT, dump, load, read_json
from v2_common import check_migration, source_identity, receipt_current
from integrate_v2 import register_addition

def check_graph(graph, projects, objectives):
    errors=[]
    rows=graph.get('projects',[]); by_id={r['id']:r for r in rows}
    if len(rows)!=len(by_id) or set(by_id)!=set(projects): errors.append('Project graph does not cover unique canonical projects')
    done=set()
    for key in graph.get('order',[]):
        if key not in by_id or key in done:
            errors.append('Invalid or repeated project in route: '+key); continue
        row=by_id[key]
        if not set(row['requires'])<=done: errors.append('Project prerequisites not ready: '+key)
        if not set(row['prelude_objectives'])<=objectives: errors.append('Unknown selected objective: '+key)
        if row['hours']!=projects[key]['meta']['hours']: errors.append('Stale project workload: '+key)
        if row.get('lab_family') and not (ROOT/'labs'/row['lab_family']/'family.json').is_file():
            errors.append('Missing executable family: '+key)
        done.add(key)
    if done!=set(projects): errors.append('Project route incomplete')
    return errors

def main():
    taxonomy,modules,projects,resources=load(); errors=check_migration(taxonomy)
    migration=read_json(ROOT/'curriculum/migration.json')
    baseline=read_json(ROOT/'curriculum/edition.json')['baseline_counts']
    previous=migration['projects']
    if len({r['old_id'] for r in previous})!=baseline['projects']: errors.append('Prior project migration count differs')
    for row in previous:
        if not row.get('new_ids') or not set(row['new_ids'])<=set(projects): errors.append('Missing migrated project: '+row['old_id'])
    objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    graph=read_json(ROOT/'curriculum/project_graph.json'); errors+=check_graph(graph,projects,objectives)
    registry=read_json(ROOT/'references/registry_v2.json'); aliases=registry['aliases']
    for resource in resources:
        if type(resource.get('assignable')) is not bool: errors.append('Implicit reading eligibility: '+resource['id'])
        if not resource.get('kind') or resource['id'] not in aliases: errors.append('Incomplete source identity: '+resource['id'])
    declarations=[]
    for path in sorted((ROOT/'labs').glob('*/family.json')):
        family=read_json(path); entries=family.get('projects',{})
        ids=set(entries if isinstance(entries,dict) else [p if isinstance(p,str) else p['id'] for p in entries])
        if not ids<=set(projects): errors.append('Unknown lab project: '+str(sorted(ids-set(projects))))
        if not (path.parent/'CMakeLists.txt').is_file(): errors.append('Missing family build: '+path.parent.name)
        declarations.append({'family':path.parent.name,'projects':len(ids),
            'reference_present':(ROOT/'assessor'/path.parent.name).is_dir(),
            'scope':'Declaration/build presence only; execution receipts are separate.'})
    negative=[]
    for field,value in [('old_id','T99.99.a'),('old_text','Substituted historical claim'),('disposition','mapped')]:
        changed=deepcopy(migration); changed['objectives'][0][field]=value
        assert check_migration(taxonomy,changed)
        negative.append('immutable objective baseline rejects '+field+' substitution')
    changed=deepcopy(migration); changed['projects'][0]['old_id']='PR-T99-01'
    assert check_migration(taxonomy,changed)
    negative.append('equal-count prior project substitution rejected')
    source_ids={}
    register_addition(source_ids,{'id':'R-CONTROL','edition':'one'})
    try:
        register_addition(source_ids,{'id':'R-CONTROL','edition':'two'})
    except ValueError:
        negative.append('conflicting new source identities rejected')
    else: raise AssertionError('Conflicting source ID accepted')
    altered=deepcopy(taxonomy); altered[0]['subtopics'][0]['objectives'].pop()
    assert check_migration(altered); negative.append('missing prior objective rejected')
    changed=deepcopy(migration); changed['objectives'][0]['new_ids']=['T99.99.z']
    assert check_migration(taxonomy,changed); negative.append('dangling migration rejected')
    changed=deepcopy(graph); changed['projects'][0]['requires']=[changed['projects'][0]['id']]
    assert check_graph(changed,projects,objectives); negative.append('project self-cycle rejected')
    identity=source_identity()
    assert not receipt_current({'status':'passed','input_sha256':'0'*64},identity)
    negative.append('stale passed receipt rejected')
    assert not receipt_current({'status':'unavailable','input_sha256':identity['sha256']},identity)
    negative.append('unavailable receipt is not passed')
    # More than three objectives is valid when identities are unique and no old
    # claim is silently removed. Validate this independently of publication data.
    extended=deepcopy(taxonomy)
    first=extended[0]['subtopics'][0]
    first['objectives'].append({'id':first['id']+'.extra','text':'A distinct test-only claim.'})
    assert not check_migration(extended)
    negative.append('variable objective cardinality supported')
    report={'schema_version':2,'passed':not errors,'status':'passed' if not errors else 'failed',
        'input_sha256':identity['sha256'],'errors':errors,'domains':len(modules),'objectives':len(objectives),
        'projects':len(projects),'baseline_objectives_mapped':baseline['objectives'],
        'baseline_projects_mapped':baseline['projects'],'guard_cases':negative,'laboratory_declarations':declarations,
        'physical_validation':'not-run','learner_time_calibration':'not-run'}
    dump(ROOT/'reviews/v2/structure.json',report); print(json.dumps(report,indent=2))
    if errors: raise SystemExit(1)

if __name__=='__main__': main()
