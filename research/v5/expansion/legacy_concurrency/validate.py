"""Validate this additive scope, without touching shared catalogs or old receipts."""
from pathlib import Path
import copy,hashlib,importlib.util,json,subprocess,sys
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'tools'));sys.path.insert(0,str(HERE))
import check_legacy_assessments as checker
from common import load,read_json
from verify_claims import observations

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,x):p.write_text(json.dumps(x,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 taxonomy,_,projects,_=load();objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
 units={u['id']:u for u in read_json(ROOT/'curriculum/v5/units.json')['units']};schema=read_json(checker.BASE/'schema.json')
 spec=importlib.util.spec_from_file_location('lcon_fixture_oracle',ROOT/'research/v3/verify_fixtures.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 records=[];results=[];paths=set();seen=set();leaves=set();trace=[];tasks=set()
 for d in ['T51','T52','T53']:
  for p in sorted(checker.BASE.glob('PR-'+d+'-*.json')):
   data=read_json(p);records.append(data);paths.add(p);errors=checker.validate(data,schema,projects,objectives,units)
   actual={c['objective'] for c in data['criteria']};expected={o for o in objectives if o.rsplit('.',1)[0] in projects[data['project']]['meta']['topics']}
   if actual!=expected:errors.append('parent leaf coverage mismatch')
   leaves.update(actual)
   for c in data['criteria']:
    if c['id'] in seen:errors.append('duplicate criterion')
    if c['task'] in tasks:errors.append('repeated task text')
    if not c.get('source_records') or any(s not in data['source_records'] for s in c['source_records']):errors.append('invalid explicit leaf source mapping')
    seen.add(c['id']);tasks.add(c['task'])
    trace.append(dict(source_records=c['source_records'],topic=c['objective'].rsplit('.',1)[0],objective=c['objective'],criterion=c['id'],project=data['project'],contract=p.relative_to(ROOT).as_posix(),learner_artifacts=c['deliverables'],assessment=dict(rubric=c['rubric'],variant_ids=[v['id'] for v in c['variants']]),facet_scope=c['facet_scope']))
   f=read_json(ROOT/projects[data['project']]['meta']['fixtures'][0]);result=oracle.compute(f['oracle'],f['input'])
   if result!=f['expected']:errors.append('synthetic fixture oracle disagrees')
   results.append(dict(project=data['project'],criteria=len(actual),fixture=f['oracle'],status='failed' if errors else 'passed',errors=errors))
   paths.update(ROOT/s['path'] for s in data['source_records']);paths.add(ROOT/data['assessor_analysis']);paths.update(ROOT/b['path'] for b in data['baseline_binding']['files']);paths.update(ROOT/s for s in data['shared_contracts']+data['supplied_artifacts'])
 controls=[]
 mutations=[('missing-title',lambda d:d['criteria'][0].pop('title')),('changed-baseline',lambda d:d['baseline_binding']['files'][0].update(sha256='0'*64)),('foreign-objective',lambda d:d['criteria'][0].update(objective='T01.01.a')),('missing-assessor-anchor',lambda d:d['criteria'][0].update(assessor_anchor='#missing')),('undelayed-transfer',lambda d:d['criteria'][0]['variants'][0].update(minimum_days=0)),('uninspected-passage',lambda d:d['source_records'][0].update(passages=['invented passage'])),('assessor-export',lambda d:d['supplied_artifacts'].append('assessor/execution/cmx.c')),('traversal-export',lambda d:d['supplied_artifacts'].append('labs/execution/../../assessor/execution/cmx.c'))]
 for name,change in mutations:
  data=copy.deepcopy(records[0]);change(data);errors=checker.validate(data,schema,projects,objectives,units)
  controls.append(dict(name=name,status='passed' if errors else 'failed',observed_rejection=errors))
 real_read=checker.read_json
 def unavailable(p):
  value=real_read(p)
  if isinstance(value,dict) and 'resources' in value:
   value=copy.deepcopy(value)
   for source in value['resources']:
    if isinstance(source.get('inspection'),dict):source['inspection']['status']='unavailable'
  return value
 with patch.object(checker,'read_json',side_effect=unavailable):errors=checker.validate(copy.deepcopy(records[0]),schema,projects,objectives,units)
 controls.append(dict(name='explicitly-unavailable-source',status='passed' if any('uninspected' in x for x in errors) else 'failed',observed_rejection=errors))
 gate=read_json(HERE/'research_gate.json');stale=[p for p,h in gate['research_input_sha256'].items() if sha(ROOT/p)!=h]
 controls.append(dict(name='research-gate-current',status='passed' if not stale and gate['status']=='passage-inspected' else 'failed',stale_paths=stale))
 command=['git','diff','--quiet','HEAD','--','curriculum/modules/T5[1-3].md','curriculum/projects/PR-T5[1-3]-*.md','curriculum/v4/units.json','fixtures/os_v3/PR-T5[1-3]-*.json','labs/execution','assessor/execution','labs/kernel_smp','assessor/kernel_smp']
 baseline=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
 controls.append(dict(name='accepted-baseline-unchanged',status='passed' if baseline.returncode==0 else 'failed',command=command,exit_code=baseline.returncode,stderr=baseline.stderr))
 calculations=observations()
 write(HERE/'traceability.json',dict(schema_version=1,scope='Source passage → topic → objective → criterion → learner D1–D4 → rubric and delayed changed condition. Authored opportunities, not assessed learner evidence.',rows=trace))
 paths.update(HERE.glob('*.py'));paths.update(HERE.glob('T*_audit.json'));paths.update(HERE.glob('T*_remaining.json'))
 paths.update([HERE/n for n in ['research_gate.json','AUDIT.md','gap_ledger.json','traceability.json']]);paths.update([checker.BASE/'schema.json',ROOT/'tools/check_legacy_assessments.py',ROOT/'research/v3/verify_fixtures.py'])
 passed=len(records)==38 and len(seen)==114 and len(leaves)==114 and all(r['status']=='passed' for r in results+controls) and all(x['passed'] for x in calculations)
 report=dict(status='passed' if passed else 'failed',command='.conda/python.exe -X utf8 research/v5/expansion/legacy_concurrency/validate.py',counts=dict(projects=len(records),unique_objectives=len(leaves),criteria=len(seen),synthetic_fixtures=len(results),new_example_calculations=len(calculations),negative_controls=9,research_gate_checks=1,accepted_baseline_checks=1),checks=results,controls=controls,example_calculations=calculations,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},limitations=['Structural/source/fixture/arithmetic checks do not execute the full new learner investigations.','No fresh complete target campaign, physical measurements, learner attempts, delayed transfer or independent human review is claimed.','Exact unavailable and broader research/experiment obligations remain in the required gap ledger.'])
 write(HERE/'validation.json',report)
 print(json.dumps({k:v for k,v in report.items() if k not in ['checks','input_sha256','example_calculations']},ensure_ascii=False))
 return not passed
if __name__=='__main__':
 sys.stdout.reconfigure(encoding='utf8');raise SystemExit(main())
