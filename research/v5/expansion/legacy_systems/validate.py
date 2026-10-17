"""Scoped structural, fixture and immutable-baseline checks; no learner promotion."""
from pathlib import Path
import copy,hashlib,json,sys
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'tools'))
import check_legacy_assessments as checker
from common import load,read_json
from verify_systems_fixtures import oracle

def main():
 taxonomy,_,projects,_=load()
 objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
 units={u['id']:u for u in read_json(ROOT/'curriculum/v5/units.json')['units']}
 schema=read_json(checker.BASE/'schema.json')
 results=[];paths=set();seen=set();records=[]
 for domain in ['T07','T08','T09','T10','T11']:
  for path in sorted(checker.BASE.glob('PR-'+domain+'-*.json')):
   data=read_json(path);records.append(data);paths.add(path)
   errors=checker.validate(data,schema,projects,objectives,units)
   expected={o for o in objectives if o.rsplit('.',1)[0] in projects[data['project']]['meta']['topics']}
   actual={c['objective'] for c in data['criteria']}
   if actual!=expected:errors.append('leaf coverage differs from actual parent topics')
   for c in data['criteria']:
    if c['id'] in seen:errors.append('duplicate criterion')
    seen.add(c['id'])
   for source in data['source_records']:
    register=read_json(ROOT/source['path']);resource=next(r for r in register['resources'] if r['id']==source['id'])
    ins=resource['inspection']
    if not all(ins.get(k) for k in ['date','method','scope','limits']):errors.append('incomplete inspection')
    if ins.get('status') in ['unavailable','not-read','not-inspected']:errors.append('uninspected source')
    if any(p not in resource['locators'] for p in source['passages']):errors.append('unlocated source passage')
    paths.add(ROOT/source['path'])
   fixture=read_json(ROOT/projects[data['project']]['meta']['fixtures'][0])
   if oracle(fixture['oracle_type'],fixture['input'])!=fixture['expected']:errors.append('synthetic oracle disagreement')
   paths.add(ROOT/data['assessor_analysis'])
   paths.update(ROOT/b['path'] for b in data['baseline_binding']['files'])
   paths.update(ROOT/p for p in data['shared_contracts'])
   results.append(dict(project=data['project'],criteria=len(actual),fixture=fixture['oracle_type'],status='failed' if errors else 'passed',errors=errors))
 # Mutations are in-memory controls for validation, never forged assessment evidence.
 controls=[]
 for name,mutate in [
  ('missing-title',lambda d:d['criteria'][0].pop('title')),
  ('wrong-baseline-hash',lambda d:d['baseline_binding']['files'][0].update(sha256='0'*64)),
  ('foreign-objective',lambda d:d['criteria'][0].update(objective='T01.01.a')),
  ('missing-assessor-anchor',lambda d:d['criteria'][0].update(assessor_anchor='#does-not-exist')),
  ('undelayed-transfer',lambda d:d['criteria'][0]['variants'][0].update(minimum_days=0)),
 ]:
  d=copy.deepcopy(records[0]);mutate(d);errors=checker.validate(d,schema,projects,objectives,units)
  controls.append(dict(name=name,status='passed' if errors else 'failed',observed_rejection=errors))
 for p in HERE.glob('*.py'):paths.add(p)
 for p in HERE.glob('T*_audit.json'):paths.add(p)
 for p in HERE.glob('T*_remaining.json'):paths.add(p)
 paths.add(HERE/'research_gate.json');paths.add(checker.BASE/'schema.json');paths.add(ROOT/'tools/check_legacy_assessments.py')
 status='passed' if all(r['status']=='passed' for r in results+controls) and len(seen)==162 and len(records)==24 else 'failed'
 report=dict(status=status,command='.conda/python.exe research/v5/expansion/legacy_systems/validate.py',counts=dict(projects=len(records),criteria=len(seen),synthetic_fixtures=len(results),negative_controls=len(controls)),checks=results,negative_controls=controls,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)},limitations=['Structural and current-source integrity plus24 exact synthetic oracle comparisons only.','No new target experiment, delayed learner attempt, independent human review or physical validation.','Baseline file hashes are current inspected bytes; historical acceptance receipts are untouched.'])
 (HERE/'validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({k:v for k,v in report.items() if k not in ['checks','input_sha256']},ensure_ascii=False))
 return status!='passed'
if __name__=='__main__':raise SystemExit(main())
