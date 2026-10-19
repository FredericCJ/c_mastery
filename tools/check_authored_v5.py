"""Check canonical new-domain teaching contracts independently of generated counts."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery import exercises
from common import ROOT, dump, parse, read_json
from v5_common import RECEIPTS, domains_manifest, identity, require_domain_research
from author_v4 import authored_unit_minutes


def canonical_resources():
    """Resolve active additions before regeneration; retain accepted identities."""
    from resource_inputs_v5 import discover
    resources={r['id']:r for r in read_json(ROOT/'references/resources.json')['resources']}
    additions, _ = discover(ROOT)
    for row in additions:
        if not row.get('reuse'):resources[row['id']]=row
    return resources


def unit_source_errors(units,resources):
    errors=[]
    for unit in units:
        for ref in unit.get('references',[]):
            source=resources.get(ref.get('resource_id'),{})
            if not source.get('inspection') or ref.get('locator') not in source.get('locators',[]):
                errors.append(unit['id']+': source passage unresolved: '+str(ref.get('resource_id'))+' / '+str(ref.get('locator')))
    return errors


def required_assessment_errors(unit, catalog):
    """A later assessment must retain its own rubric, prerequisites and lab."""
    errors = []
    seen = set()
    for stage in unit.get('required_exercise_assessments', []):
        key = stage.get('exercise')
        record = catalog.get(key, {})
        expected = {
            'objectives': record.get('objectives'),
            'prerequisite_exercises': record.get('prerequisite_exercises'),
            'family': record.get('lab', {}).get('family'),
            'tests': record.get('lab', {}).get('tests'),
            'assessor_analysis': record.get('assessor_analysis'),
            'criteria': [r['id'] for r in record.get('rubric', [])],
        }
        if (key in seen or not record or record.get('project') != unit['project']
                or any(stage.get(field) != value for field, value in expected.items())
                or not set(stage.get('objectives', [])) <= set(unit['objectives'])
                or any(not stage.get(field) for field in ('prerequisite_policy', 'assessment', 'accounting'))):
            errors.append(unit['id']+': later assessment contract differs: '+str(key))
        seen.add(key)
    return errors


def inspect():
    recipes=domains_manifest();require_domain_research(recipes)
    errors=[];rows=[]
    b06_rows={row['objective']:row for row in read_json(ROOT/'research/v5/expansion/b06/coverage.json')['objective_map']}
    resources=canonical_resources()
    catalog = {record['id']: record for record in read_json(ROOT/'curriculum/v5/exercises.json')['exercises']}
    for domain,recipe in recipes['domains'].items():
        module,_=parse(ROOT/'curriculum/modules'/f'{domain}.md')
        registrations=[recipe,*recipe.get('additional_laboratories',[])]
        families={r['family']:read_json(ROOT/'labs'/r['family']/'family.json') for r in registrations}
        terms=read_json(ROOT/'curriculum/vocabulary'/f'{domain}.json')['entries']
        inspected=set();projects=[]
        for pid in module['projects']:
            meta,body=parse(ROOT/'curriculum/projects'/f'{pid}.md')
            owners=[r['family'] for r in recipe.get('additional_laboratories',[]) if pid in r['projects']]
            if len(owners)>1:errors.append(pid+': ambiguous laboratory ownership')
            family=families[owners[0] if owners else recipe['family']]
            assigned=[u for u in family.get('units',[]) if u['project']==pid]
            errors.extend(unit_source_errors(assigned,resources))
            for unit in assigned:
                errors.extend(required_assessment_errors(unit, catalog))
            try:authored_unit_minutes(meta,assigned)
            except (ValueError,KeyError,TypeError) as e:errors.append(str(e))
            briefs=[]
            for path in (ROOT/'curriculum/v5/exercises').glob(f'X-{domain}-*.md'):
                value,text=exercises.parse_brief(path)
                if value['project']!=pid:continue
                try:exercises.validate(value,text)
                except (ValueError,KeyError,TypeError) as e:errors.append(str(e))
                briefs.append(value['id'])
                inspected.update(r['id'] for r in value['source_records'])
                owned={o for u in assigned for o in u['objectives']}
                if not set(value['objectives'])<=owned:errors.append(pid+': exercise objectives absent from authored units')
            if not briefs and 'curriculum/v5/contracts/b06.md' in meta.get('shared_contracts', []):
                # B06 uses the existing project + authored-unit schema rather than
                # duplicating the same problem as an X sidecar. Resolve every facet.
                for heading in ('Contract','Inputs and expected results','Steps','Failure cases','Acceptance and evidence','Hints and recovery','Transfer challenge','Environment, security and validation'):
                    if '## '+heading not in body:errors.append(pid+': missing project exercise section '+heading)
                for u in assigned:
                    for field in ('reasoning','deliverables','rubric','variants','assessor_analysis','remaining_evidence','references'):
                        if not u.get(field):errors.append(u['id']+': missing exercise field '+field)
                    criteria={r['objective']:r for r in u['rubric']}
                    for oid in u['objectives']:
                        row=b06_rows.get(oid,{})
                        if oid not in criteria or set(criteria[oid].get('levels',{}))!={'0','1','2','3'}:
                            errors.append(u['id']+': incomplete leaf rubric '+oid)
                        for facet in ('source_topic','teaching','implementation_analysis','boundary_adversarial','independent_assessment','assessor'):
                            if not row.get(facet):errors.append(pid+': unresolved '+facet+' for '+oid)
                    for ref in u.get('references',[]):
                        inspected.add(ref['resource_id'])
                briefs=[pid+' (authored project/unit exercise contract)']
            if not briefs:errors.append(pid+': no complete substantive exercise')
            if not assigned:errors.append(pid+': no authored learner units')
            projects.append(dict(project=pid,units=[u['id'] for u in assigned],exercises=briefs))
        if int(domain[1:])>=65:
            if len(terms)<15:errors.append(domain+': fewer than15 precise glossary terms')
            if len(inspected)<3:errors.append(domain+': fewer than3 inspected referenced sources')
            if not recipe.get('decisions'):errors.append(domain+': no sealed design decision')
            if not (ROOT/'labs'/recipe['family']/'COST_REVIEW.md').is_file():errors.append(domain+': no13-dimension cost/enforcement evidence template')
        rows.append(dict(domain=domain,projects=projects,glossary_terms=len(terms),inspected_source_ids=sorted(inspected)))
    return dict(status='passed' if not errors else 'failed',passed=not errors,errors=errors,domains=rows,
                scope='Canonical authored contracts and resolvable sources only. Mechanism correctness, source completeness, execution and independent assessment require separate evidence.')


def main():
    result=inspect();result['input_sha256']=identity()['sha256'];dump(ROOT/RECEIPTS/'authored_content.json',result)
    print(dict(passed=result['passed'],domains=len(result['domains']),errors=result['errors']))
    return 0 if result['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
