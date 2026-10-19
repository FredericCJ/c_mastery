"""Check exact later-exercise ownership without inventing completed assessments."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
from register_modern_unit_refinements import ROOT,REFINEMENTS,refine
from cmastery.exercises import parse_brief


def inspect():
    records=[];inputs={};controls=0
    for family,spec in REFINEMENTS.items():
        path=ROOT/'labs'/family/'family.json'
        data=json.loads(path.read_text(encoding='utf8'))
        before_path=ROOT/'reviews/v5/expansion/modern_unit_mapping/before'/(family+'_family.json')
        before=json.loads(before_path.read_text(encoding='utf8'))
        assert data==refine(before,family) and data==refine(data,family)
        target=next(u for u in data['units'] if u['id']==spec['unit'])
        old=next(u for u in before['units'] if u['id']==spec['unit'])
        assert [(u['id'],u['minutes'],u['tests']) for u in data['units']]==[(u['id'],u['minutes'],u['tests']) for u in before['units']]
        assert {k:v for k,v in data.items() if k!='units'}=={k:v for k,v in before.items() if k!='units'}
        assert [u for u in data['units'] if u['id']!=spec['unit']]==[u for u in before['units'] if u['id']!=spec['unit']]
        brief_path=ROOT/'curriculum/v5/exercises'/(spec['exercise']+'.md')
        brief,_=parse_brief(brief_path)
        for row in brief['rubric']:
            assert row in target['rubric'] and row['objective'] in target['objectives']
        assert set(brief['objectives'])<=set(target['objectives'])
        stage=next(s for s in target['required_exercise_assessments'] if s['exercise']==brief['id'])
        assert stage['prerequisite_exercises']==brief['prerequisite_exercises']
        assert stage['assessor_analysis']==brief['assessor_analysis'] and (ROOT/stage['assessor_analysis']).is_file()
        assert stage['criteria']==[r['id'] for r in brief['rubric']]
        for source in brief['source_records']:
            resource=json.loads((ROOT/source['path']).read_text(encoding='utf8'))
            row=next(r for r in resource['resources'] if r['id']==source['id'])
            assert row.get('inspection') and set(source['passages'])<=set(row['locators'])
            inputs[source['path']]=hashlib.sha256((ROOT/source['path']).read_bytes()).hexdigest()
        for p in [path,before_path,brief_path,ROOT/stage['assessor_analysis']]:
            inputs[p.relative_to(ROOT).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
        for kind in ['missing-unit','wrong-project','changed-criterion']:
            bad=copy.deepcopy(data)
            unit=next(u for u in bad['units'] if u['id']==spec['unit'])
            if kind=='missing-unit':bad['units'].remove(unit)
            elif kind=='wrong-project':unit['project']='PR-T00-00'
            else:next(r for r in unit['rubric'] if r['id']==brief['rubric'][0]['id'])['criterion']+=' unsupported claim'
            try:refine(bad,family)
            except AssertionError:controls+=1
            else:raise AssertionError('Accepted '+kind)
        records.append(dict(unit=spec['unit'],exercise=brief['id'],added_objectives=sorted(set(target['objectives'])-set(old['objectives'])),
                            exact_criteria=stage['criteria'],prerequisite_exercises=stage['prerequisite_exercises'],minutes_preserved=target['minutes'],
                            learner_assessment='not-assessed',deferred_stage_explicit=True))
    for name in ['tools/register_modern_unit_refinements.py','tools/check_modern_unit_refinements.py','research/v5/modern/author_replicated_support.py']:
        inputs[name]=hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
    return dict(passed=True,accepted=False,negative_controls=controls,records=records,input_sha256=inputs,
                scope='Exact authored-unit integration and idempotent regeneration only; current family execution and independent learner assessments are separate evidence.')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args()
    result=inspect();path=ROOT/args.report;path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,negative_controls=result['negative_controls'],units=len(result['records']))))
