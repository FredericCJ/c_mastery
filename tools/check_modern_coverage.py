"""Resolve census/SWE dispositions to actual assessed artifacts, not mentions."""
import re
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery import exercises
from common import ROOT, dump, read_json
from v5_common import RECEIPTS, identity


def planned_elements():
    plan=(ROOT/'planning/modern_techniques_plan.md').read_text(encoding='utf-8')
    result=set()
    for group in re.findall(r'\[([^\]\n]+)\]',plan):
        for value in group.split(','):
            value=value.strip()
            if re.fullmatch(r'[a-z][a-z0-9]*(?:-[a-z0-9]+)*',value):result.add(value)
    return result


def inspect():
    index=(ROOT/'research/v5/modern/census/INDEX.md').read_text(encoding='utf-8')
    census=set(re.findall(r'^- \[([MCPD]\d\d) ',index,re.M))
    if len(census)!=100:raise ValueError('Census index does not contain100 distinct stable identities')
    selected=planned_elements();seen={'census':set(),'swe':set()};errors=[];rows=[]
    for path in sorted((ROOT/'research/v5/modern/coverage').glob('*.json')):
        data=read_json(path)
        for kind in ('census','swe'):
            for entry in data.get(kind,[]):
                key=entry.get('technique') if kind=='census' else entry.get('element',entry.get('term',entry.get('technique')))
                label=path.name+':'+str(key);fail=[]
                if not key:fail.append('Missing stable discovery identity')
                if kind=='census' and key not in census:fail.append('Unknown census identity')
                disposition=entry.get('disposition')
                if disposition not in ('project','imported','non-equivalence','excluded'):fail.append('Unknown disposition')
                if not isinstance(entry.get('reason'),str) or not entry['reason'].strip():fail.append('Missing mechanism-specific rationale')
                assessment=None
                if disposition=='excluded':
                    if not entry.get('boundary') or not entry.get('remaining_work'):fail.append('Exclusion needs exact scope boundary and remaining-work disposition')
                else:
                    xid=entry.get('exercise');target=entry.get('target')
                    if not xid or not target:fail.append('Disposition needs an explicit exercise and learner project')
                    else:
                        brief=ROOT/'curriculum/v5/exercises'/f'{xid}.md'
                        try:
                            value,body=exercises.parse_brief(brief)
                            exercises.validate(value,body)
                            if value['project']!=target:fail.append('Exercise belongs to another project')
                            criteria={r['id']:r for r in value['rubric']}
                            rubric=entry.get('rubric',[])
                            if not rubric or not set(rubric)<=set(criteria):fail.append('No valid explicit rubric subset')
                            assessment=dict(exercise=xid,project=target,criteria=[criteria[k] for k in rubric if k in criteria],
                                            objectives=sorted({criteria[k]['objective'] for k in rubric if k in criteria}),
                                            learner_artifacts=[c for c in value['objective_coverage'] if any(criteria[k]['objective']==c['objective'] for k in rubric if k in criteria)],
                                            inspected_sources=value['source_records'],assessor=value['assessor_analysis'])
                        except (OSError,ValueError,KeyError,TypeError) as e:fail.append(str(e))
                if not fail:seen[kind].add(key)
                errors += [label+': '+e for e in fail]
                rows.append(dict(register=path.relative_to(ROOT).as_posix(),kind=kind,id=key,disposition=disposition,
                                 valid=not fail,assessment=assessment,scope=entry.get('scope'),errors=fail))
    missing_census=sorted(census-seen['census']);missing_swe=sorted(selected-seen['swe'])
    errors += ['Unresolved census: '+', '.join(missing_census)] if missing_census else []
    errors += ['Unresolved selected SWE elements: '+', '.join(missing_swe)] if missing_swe else []
    return dict(schema_version=1,status='passed' if not errors else 'failed',passed=not errors,
                census_total=len(census),resolved_census=len(census&seen['census']),selected_swe=sorted(selected),
                unresolved_census=missing_census,unresolved_swe=missing_swe,dispositions=rows,errors=errors,
                limits=['A resolved mapping is an authored assessment obligation, not execution, learner mastery or a full external-source audit.',
                        'Partial contributions retain their named limits. Exclusions do not assert covered competence.',
                        'SWE/census discovery metadata is not an inspected primary source.'])


def main():
    report=inspect();report['input_sha256']=identity()['sha256']
    dump(ROOT/RECEIPTS/'modern_coverage.json',report)
    print({k:v for k,v in report.items() if k!='dispositions'})
    return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
