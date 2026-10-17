"""Scoped structural and source-binding checks; not independent teaching review."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import vocabulary
def read(path):return json.loads(path.read_text(encoding='utf-8'))
def front(path):return json.loads(path.read_text(encoding='utf-8').split('```json\n',1)[1].split('\n```',1)[0])
def main():
    errors=[];checked=[];seen=set();topics=read(ROOT/'research/v5/b06_taxonomy.json')['domains'];resources=read(ROOT/'research/v5/b06_resources.json')['resources'];entries=[]
    for domain in topics:
        did=domain['id'];fragment=read(ROOT/'curriculum/v5/domains'/(did+'.json'));family=fragment['domain']['family'];data=read(ROOT/'labs'/family/'family.json')
        module=front(ROOT/'curriculum/modules'/(did+'.md'));recall=read(ROOT/'curriculum/v5/recall'/(did+'.json'))['sets'];vocab=read(ROOT/'curriculum/vocabulary'/(did+'.json'));entries+=vocab['entries']
        if len(data['projects'])!=8 or len(data['units'])!=16:errors.append(did+': project/unit count')
        if set(vocab.get('recall',{}))!={'define','distinguish','apply'}:errors.append(did+': vocabulary recall')
        objectives={o['id'] for t in domain['subtopics'] for o in t['objectives']}
        if set(o for u in data['units'] for o in u['objectives'])!=objectives:errors.append(did+': unit objective coverage')
        if set(o for r in recall for o in r['objectives'])!=objectives:errors.append(did+': recall objective coverage')
        for unit in data['units']:
            if unit['id'] in seen:errors.append('duplicate '+unit['id'])
            seen.add(unit['id'])
            for key in ['reasoning','deliverables','rubric','variants','assessor_analysis','remaining_evidence','scope']:
                if not unit.get(key):errors.append(unit['id']+': missing '+key)
            if {r['objective'] for r in unit['rubric']}!=set(unit['objectives']):errors.append(unit['id']+': objective rubric gap')
            if any(v.get('minimum_days',0)<7 for v in unit['variants']):errors.append(unit['id']+': delayed variant missing')
            if not (ROOT/unit['assessor_analysis']).is_file():errors.append(unit['id']+': missing assessor')
        for row in data['projects']:
            path=ROOT/'curriculum/projects'/(row['id']+'.md');meta=front(path);text=path.read_text(encoding='utf-8')
            for header in ['Contract','Inputs and expected results','Steps','Failure cases','Acceptance and evidence','Hints and recovery','Transfer challenge']:
                if '## '+header not in text:errors.append(row['id']+': '+header)
            if sum(u['minutes'] for u in data['units'] if u['project']==row['id'])!=meta['hours']*60:errors.append(row['id']+': descriptive budget mismatch')
            for rel in meta['fixtures']+meta['shared_contracts']:
                if not (ROOT/rel).is_file():errors.append(row['id']+': missing '+rel)
            for prereq in meta['requires_projects']:
                if not (ROOT/'curriculum/projects'/(prereq+'.md')).is_file():errors.append(row['id']+': missing prerequisite '+prereq)
        checked.append(dict(domain=did,projects=len(data['projects']),units=len(data['units']),objectives=len(objectives),terms=len(vocab['entries'])))
    verrors,pending=vocabulary.validate(entries,resources,topics);errors+=verrors
    if pending:errors.append('pending glossary authorities')
    # Global names still matter before catalog integration.
    all_entries,_=vocabulary.load();names={}
    for entry in all_entries:
        key=entry['term'].casefold()
        if key in names and (entry['owner'] in {d['id'] for d in topics} or names[key]['owner'] in {d['id'] for d in topics}):errors.append('global term collision '+entry['term'])
        names[key]=entry
    gate=read(ROOT/'research/v5/expansion/b06/research_gate.json')
    for rel,want in gate['research_input_sha256'].items():
        if hashlib.sha256((ROOT/rel).read_bytes()).hexdigest()!=want:errors.append('source pin '+rel)
    coverage=read(ROOT/'research/v5/expansion/b06/coverage.json')
    if len(coverage['objective_map'])!=sum(r['objectives'] for r in checked):errors.append('coverage cardinality')
    report=dict(passed=not errors,domains=checked,errors=errors,limits='Scoped author structure/source checks only; campaign, learner, independent reviewer and physical claims remain separate.')
    print(json.dumps(report,indent=2));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
