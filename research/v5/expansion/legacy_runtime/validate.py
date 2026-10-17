"""Read-only scoped checks plus explicit local report; never publish shared catalogs."""
from pathlib import Path
import copy
import hashlib
import json
import subprocess
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT/'tools'))
import check_legacy_assessments as checker
from common import load, read_json
from verify_systems_fixtures import oracle
from verify_embedded_fixtures import recompute

def sha(path):
 return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path,value):
 path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')

def main():
 taxonomy,_,projects,_=load()
 objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
 units={u['id']:u for u in read_json(ROOT/'curriculum/v5/units.json')['units']}
 schema=read_json(checker.BASE/'schema.json')
 results=[];paths=set();seen=set();leaves=set();records=[];trace=[]
 for domain in [f'T{n}' for n in range(23,30)]:
  for path in sorted(checker.BASE.glob('PR-'+domain+'-*.json')):
   data=read_json(path);records.append(data);paths.add(path)
   errors=checker.validate(data,schema,projects,objectives,units)
   expected={o for o in objectives if o.rsplit('.',1)[0] in projects[data['project']]['meta']['topics']}
   actual={c['objective'] for c in data['criteria']}
   if actual!=expected:errors.append('actual parent leaf coverage mismatch')
   leaves.update(actual)
   for c in data['criteria']:
    if c['id'] in seen:errors.append('duplicate criterion')
    seen.add(c['id'])
    if not c.get('source_records'):errors.append('leaf lacks explicit source/topic mapping')
    if any(s not in data['source_records'] for s in c.get('source_records',[])):errors.append('leaf source not in validated parent register')
    trace.append(dict(source_records=c['source_records'],topic=c['objective'].rsplit('.',1)[0],objective=c['objective'],criterion=c['id'],project=data['project'],contract=path.relative_to(ROOT).as_posix(),learner_artifacts=c['deliverables'],assessment=dict(rubric=c['rubric'],variant_ids=[v['id'] for v in c['variants']]),facet_scope=c['facet_scope']))
   f=read_json(ROOT/projects[data['project']]['meta']['fixtures'][0])
   result=oracle(f['oracle_type'],f['input']) if 'oracle_type' in f else recompute(f['input'])
   if result!=f['expected']:errors.append('independent synthetic oracle disagrees')
   for s in data['source_records']:paths.add(ROOT/s['path'])
   paths.add(ROOT/data['assessor_analysis'])
   paths.update(ROOT/b['path'] for b in data['baseline_binding']['files'])
   paths.update(ROOT/p for p in data['shared_contracts'])
   results.append(dict(project=data['project'],criteria=len(actual),fixture=f.get('oracle_type',f['input'].get('kind')),status='failed' if errors else 'passed',errors=errors))
 controls=[]
 mutations=[('missing-title',lambda d:d['criteria'][0].pop('title')),
  ('changed-baseline',lambda d:d['baseline_binding']['files'][0].update(sha256='0'*64)),
  ('foreign-objective',lambda d:d['criteria'][0].update(objective='T01.01.a')),
  ('missing-assessor-anchor',lambda d:d['criteria'][0].update(assessor_anchor='#missing')),
  ('undelayed-transfer',lambda d:d['criteria'][0]['variants'][0].update(minimum_days=0)),
  ('uninspected-passage',lambda d:d['source_records'][0].update(passages=['invented passage']))]
 for name,mutate in mutations:
  data=copy.deepcopy(records[0]);mutate(data)
  errors=checker.validate(data,schema,projects,objectives,units)
  controls.append(dict(name=name,status='passed' if errors else 'failed',observed_rejection=errors))
 real_read=checker.read_json
 def unavailable(path):
  value=real_read(path)
  if isinstance(value,dict) and 'resources' in value:
   value=copy.deepcopy(value)
   for resource in value['resources']:
    if isinstance(resource.get('inspection'),dict):resource['inspection']['status']='unavailable'
  return value
 with patch.object(checker,'read_json',side_effect=unavailable):
  errors=checker.validate(copy.deepcopy(records[0]),schema,projects,objectives,units)
 controls.append(dict(name='explicitly-unavailable-source',status='passed' if any('uninspected' in x for x in errors) else 'failed',observed_rejection=errors))
 campaign_path=ROOT/'reviews/v5/expansion/legacy_runtime/posix_lifecycle_depth_campaign.json'
 campaign=read_json(campaign_path)
 stale=[p for p,h in campaign['inspected_paths'].items() if sha(ROOT/p)!=h]
 campaign_ok=campaign.get('passed') and not stale and len(campaign['profiles'])==8 and all(p['passed'] and p['ctest_count']==6 for p in campaign['profiles'])
 controls.append(dict(name='actual-lifecycle-campaign-current',status='passed' if campaign_ok else 'failed',profiles=len(campaign['profiles']),stale_paths=stale))
 paths.update(ROOT/p for p in campaign['inspected_paths']);paths.add(campaign_path)
 export_path=ROOT/'reviews/v5/expansion/legacy_runtime/export_validation.json'
 exported=read_json(export_path)
 stale_export=[p for p,h in exported['input_sha256'].items() if sha(ROOT/p)!=h]
 export_ok=exported.get('status')=='passed' and not stale_export and len(exported['observations'])==2 and len(exported['negative_controls'])==9
 controls.append(dict(name='standalone-export-current',status='passed' if export_ok else 'failed',projects=len(exported['observations']),negative_controls=len(exported['negative_controls']),stale_paths=stale_export))
 paths.add(export_path);paths.update(ROOT/p for p in exported['input_sha256'])
 baseline_command=['git','diff','--quiet','HEAD','--','curriculum/modules/T2[3-9].md','curriculum/projects/PR-T2[3-9]-*.md','curriculum/v4/units.json','fixtures/systems/PR-T2[3-6]-*.json','fixtures/embedded/PR-T2[7-9]-*.json','labs/hosted_systems','labs/firmware_io','assessor/hosted_systems','assessor/firmware_io','assessor/execution','labs/execution']
 baseline=subprocess.run(baseline_command,cwd=ROOT,capture_output=True,text=True)
 controls.append(dict(name='accepted-owned-baseline-unchanged',status='passed' if baseline.returncode==0 else 'failed',command=baseline_command,exit_code=baseline.returncode,stderr=baseline.stderr))
 write(HERE/'traceability.json',dict(schema_version=1,scope='Source passage → topic → objective → criterion → learner D1–D4 → explicit rubric/changed condition. Authored opportunities are not learner evidence.',rows=trace))
 for p in HERE.glob('*.py'):paths.add(p)
 for p in HERE.glob('T*_audit.json'):paths.add(p)
 for p in HERE.glob('T*_remaining.json'):paths.add(p)
 for name in ['research_gate.json','AUDIT.md','gap_ledger.json','traceability.json']:paths.add(HERE/name)
 paths.update([checker.BASE/'schema.json',ROOT/'tools/check_legacy_assessments.py'])
 passed=all(r['status']=='passed' for r in results+controls) and len(records)==40 and len(seen)==258 and len(leaves)==240
 report=dict(status='passed' if passed else 'failed',command='.conda/python.exe research/v5/expansion/legacy_runtime/validate.py',counts=dict(projects=len(records),unique_objectives=len(leaves),criteria=len(seen),synthetic_fixtures=len(results),negative_controls=7,campaign_integrity_checks=1,export_integrity_checks=1,accepted_baseline_checks=1),checks=results,controls=controls,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},limitations=['40 exact synthetic oracle comparisons and structural/source bindings do not establish the broader OS, concurrency or hardware claims.','The separately linked lifecycle campaign is actual assessor execution only; no complete learner attempt, delayed assessment, independent human review or physical validation.','No accepted baseline source or receipt is rewritten by this validator.'])
 write(HERE/'validation.json',report)
 print(json.dumps({k:v for k,v in report.items() if k not in ['checks','input_sha256']},ensure_ascii=False))
 return not passed

if __name__=='__main__':
 sys.stdout.reconfigure(encoding='utf-8')
 raise SystemExit(main())
