"""Pin every original T20 criterion/delayed task to current bounded author evidence."""
import datetime,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_architecture'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 spec=importlib.util.spec_from_file_location('architecture_bindings',FOLDER/'bindings.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);bindings=m.BINDINGS
 report_path=FOLDER/'validation.json';report=json.loads(report_path.read_text(encoding='utf8'));assert report['status']=='pass' and report['commands'] and all(x['passed'] for x in report['commands'])
 for key in ['source_sha256','outputs_sha256']:
  for name,value in report[key].items():assert sha(ROOT/name)==value,'stale '+key+' '+name
 gate=json.loads((FOLDER/'research_gate.json').read_text(encoding='utf8'))
 for name,value in gate['research_input_sha256'].items():assert sha(ROOT/name)==value,'stale source gate '+name
 commands={r['id'] for r in report['commands']};updates=[];objectives=set();files={ROOT/x for x in report['source_sha256']}|{report_path,FOLDER/'bindings.py',FOLDER/'research_gate.json',FOLDER/'register_sources.py',FOLDER/'sources.md',pathlib.Path(__file__),ROOT/'tools/integrate_architecture_followthrough.py',ROOT/'tools/check_architecture_followthrough.py'}
 public='labs/api_depth/architecture/CONTRACTS.md';analysis='assessor/api_depth/architecture/ANALYSIS.md';receipt=report_path.relative_to(ROOT).as_posix();export_path=FOLDER/'exports.json';export=json.loads(export_path.read_text(encoding='utf8'));assert export['status']=='pass' and len(export['commands'])==12 and all(x['passed'] for x in export['commands']);files.add(export_path)
 for key in ['source_sha256','outputs_sha256']:
  for name,value in export[key].items():assert sha(ROOT/name)==value,'stale export '+name
 files|={ROOT/x for x in export['source_sha256']}
 closure_path=FOLDER/'closure.json';closure=json.loads(closure_path.read_text(encoding='utf8'));assert closure['status']=='pass' and len(closure['commands'])==52 and all(x['passed'] for x in closure['commands']);files.add(closure_path)
 for key in ['source_sha256','outputs_sha256']:
  for name,value in closure[key].items():assert sha(ROOT/name)==value,'stale closure '+name
 files|={ROOT/x for x in closure['source_sha256']}
 for parent in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T20-*.json')):
  data=json.loads(parent.read_text(encoding='utf8'));files.add(parent)
  for criterion in data['criteria']:
   objective=criterion['objective'];objectives.add(objective);ordinary,transfer,reason=bindings[objective]
   for delayed,selectors in [(False,ordinary),(True,transfer)]:
    checks=[]
    for selector in selectors:
     matches=sorted(x for x in commands if x.startswith(selector) and not x.endswith('.compile'));assert matches,(objective,selector);checks.extend(matches)
    variant=criterion['variants'][0];ident=variant['id'] if delayed else criterion['id'];task=variant['task'] if delayed else criterion['task'];physical=objective=='T20.04.b' or objective.startswith('T20.05.')
    dimensions={'teaching':{'state':'complete','evidence':[parent.relative_to(ROOT).as_posix(),public]},'implementation':{'state':'complete','evidence':[public,analysis,*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'assessor/api_depth/architecture').glob('*.c')),'assessor/api_depth/architecture/models.py']},'author_validation':{'state':'passed','evidence':[receipt]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-observed' if physical else 'not-applicable-to-host-or-analysis-contract','evidence':[public]}}
    updates.append(dict(id=ident,parent_project=data['project'],objective=objective,original_task=task,original_delayed_variant=variant['id'],minimum_days=variant['minimum_days'],dimensions=dimensions,closure_rationale=reason,artifacts=[public,analysis],author_validation=dict(report=receipt,checks=sorted(set(checks)),scope='Actual bounded C/Linux execution, selected ARM adapter cross-build and explicitly identified finite/design analysis; not learner transfer or physical target validation'),remaining_author_work=[],evidence_limits=['Original independent learner and delayed criteria remain mandatory.','Hardware-related rows retain unobserved physical dimension; model/cross-build is not peripheral execution.','Finite tests, model enumeration and source inspection do not prove arbitrary fault containment, universal latency or absolute mastery.']))
 assert objectives==set(bindings) and len(updates)==60
 for row in updates:
  row['dimensions']['author_validation']['evidence'].append(closure_path.relative_to(ROOT).as_posix())
  row['author_validation']['additional_reports']=[closure_path.relative_to(ROOT).as_posix()]
 result=dict(schema_version=1,scope='T20 original30 ordinary and30 delayed assessments; author implementation/execution',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,export_path.relative_to(ROOT).as_posix()],updates=updates,summary=dict(original_criteria=30,original_delayed_variants=30,commands=len(report['commands']),export_commands=12,external_review='not-run',independent_learner='not-assessed',physical='F439 not observed'),evidence_limits=report['limitations'])
 result['current_reports'].append(closure_path.relative_to(ROOT).as_posix());result['summary']['closure_commands']=52
 (FOLDER/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']))
if __name__=='__main__':main()
