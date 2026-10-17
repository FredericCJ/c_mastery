"""Check canonical new-domain teaching contracts independently of generated counts."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery import exercises
from common import ROOT, dump, parse, read_json
from v5_common import RECEIPTS, domains_manifest, identity, require_domain_research
from author_v4 import authored_unit_minutes


def inspect():
    recipes=domains_manifest();require_domain_research(recipes)
    errors=[];rows=[]
    b06_rows={row['objective']:row for row in read_json(ROOT/'research/v5/expansion/b06/coverage.json')['objective_map']}
    b06_resources={row['id']:row for row in read_json(ROOT/'research/v5/b06_resources.json')['resources']}
    for domain,recipe in recipes['domains'].items():
        module,_=parse(ROOT/'curriculum/modules'/f'{domain}.md')
        family=read_json(ROOT/'labs'/recipe['family']/'family.json')
        terms=read_json(ROOT/'curriculum/vocabulary'/f'{domain}.json')['entries']
        inspected=set();projects=[]
        for pid in module['projects']:
            meta,body=parse(ROOT/'curriculum/projects'/f'{pid}.md')
            assigned=[u for u in family.get('units',[]) if u['project']==pid]
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
                        source=b06_resources.get(ref['resource_id'],{})
                        if not source.get('inspection') or ref['locator'] not in source.get('locators',[]):
                            errors.append(u['id']+': source passage unresolved')
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
