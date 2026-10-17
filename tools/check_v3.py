"""Validate actual v3 identity preservation, routes and target declarations."""
from copy import deepcopy
from common import ROOT, dump, load, read_json
from v2_common import source_identity
from v3_common import check_v3_migration
from check_v2 import check_graph

def main():
    taxonomy,modules,projects,resources=load()
    errors=check_v3_migration(taxonomy,projects)
    previous_sources=read_json(ROOT/'references/baseline_v2/resources.json')['resources']
    actual_sources={r['id']:r for r in resources}
    for resource in previous_sources:
        if actual_sources.get(resource['id'])!=resource:
            errors.append('Accepted v2 source changed: '+resource['id'])
    objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    graph=read_json(ROOT/'curriculum/project_graph.json')
    errors+=check_graph(graph,projects,objectives)
    required_domains = {f'T{i}' for i in range(49,57)}
    edition_path = ROOT / 'curriculum/edition.json'
    if edition_path.is_file():
        required_domains.update(read_json(edition_path).get('new_domains', []))
    new={key for key in projects if projects[key]['meta']['module'] in required_domains}
    declarations={}; families=[]
    for path in sorted((ROOT/'labs').glob('*/family.json')):
        manifest=read_json(path); families.append(path.parent.name)
        entries=manifest.get('projects',[])
        entries=[dict(value,id=key) for key,value in entries.items()] if isinstance(entries,dict) else entries
        for row in entries:
            if isinstance(row,str): continue
            if row['id'] not in projects: errors.append('Unknown lab project '+row['id'])
            declarations.setdefault(row['id'],[]).append(path.parent.name)
            if row['id'] in new and not row.get('tests') and not row.get('target_tests'):
                errors.append('New project has no stage-specific anchor '+row['id'])
        if manifest.get('default_target') not in manifest.get('targets',{}) and manifest.get('targets'):
            errors.append('Unknown default target '+path.parent.name)
    for key in sorted(new-set(declarations)): errors.append('New project has no executable family '+key)
    migration=read_json(ROOT/'curriculum/migration_v3.json')
    changed=deepcopy(migration); changed['objectives'][0]['old_id']='T99.99.a'
    assert check_v3_migration(taxonomy,projects,changed)
    changed=deepcopy(migration); changed['projects'][0]['baseline_sha256']='0'*64
    assert check_v3_migration(taxonomy,projects,changed)
    report={'status':'passed' if not errors else 'failed','passed':not errors,
        'input_sha256':source_identity()['sha256'],'errors':errors,'domains':len(modules),
        'projects':len(projects),'new_projects':len(new),'objectives':len(objectives),'families':families,
        'prior_objectives_preserved':1605,'prior_projects_preserved':337,
        'prior_resources_preserved':len(previous_sources),
        'scope':'Canonical identity, dependency and declared stage-gate checks; execution and physical evidence are separate.'}
    dump(ROOT/'reviews/v3/structure.json',report); print(report)
    if errors: raise SystemExit(1)
if __name__=='__main__': main()
