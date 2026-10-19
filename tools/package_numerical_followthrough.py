"""Pin exact T17 original criteria and delayed variants to observed author work."""
import datetime,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_numerics'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 spec=importlib.util.spec_from_file_location('numerical_bindings',FOLDER/'bindings.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);bindings=m.BINDINGS
 report_path=FOLDER/'validation.json';report=json.loads(report_path.read_text(encoding='utf8'));assert report['status']=='pass' and all(x['passed'] for x in report['commands'])
 for key in ['source_sha256','outputs_sha256']:
  for name,value in report[key].items():assert sha(ROOT/name)==value,'stale '+key+' '+name
 gate=json.loads((FOLDER/'research_gate.json').read_text(encoding='utf8'))
 for name,value in gate['research_input_sha256'].items():assert sha(ROOT/name)==value,'stale gate '+name
 commands={r['id'] for r in report['commands']};updates=[];objectives=set();files={ROOT/x for x in report['source_sha256']}|{report_path,FOLDER/'bindings.py',FOLDER/'research_gate.json',FOLDER/'register_sources.py',FOLDER/'sources.md',pathlib.Path(__file__),ROOT/'tools/integrate_numerical_followthrough.py',ROOT/'tools/check_numerical_followthrough.py'}
 public='labs/numerical_depth/CONTRACTS.md';technical='assessor/numerical_depth/ANALYSIS.md';receipt=report_path.relative_to(ROOT).as_posix()
 export_path=FOLDER/'exports.json';export=json.loads(export_path.read_text(encoding='utf8'));assert export['status']=='pass' and len(export['commands'])==12 and all(x['passed'] for x in export['commands']);files.add(export_path)
 for key in ['source_sha256','outputs_sha256']:
  for name,value in export[key].items():assert sha(ROOT/name)==value,'stale export '+name
 files|={ROOT/x for x in export['source_sha256']}
 for parent in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T17-*.json')):
  data=json.loads(parent.read_text(encoding='utf8'));files.add(parent)
  for criterion in data['criteria']:
   objective=criterion['objective'];objectives.add(objective);ordinary,transfer,reason=bindings[objective]
   for delayed,selectors in [(False,ordinary),(True,transfer)]:
    checks=[]
    for selector in selectors:
     matches=sorted(x for x in commands if x.startswith(selector) and not x.endswith('.compile'));assert matches,(objective,selector);checks.extend(matches)
    variant=criterion['variants'][0];ident=variant['id'] if delayed else criterion['id'];task=variant['task'] if delayed else criterion['task']
    dimensions={'teaching':{'state':'complete','evidence':[parent.relative_to(ROOT).as_posix(),public]},'implementation':{'state':'complete','evidence':[public,technical,*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'assessor/numerical_depth').glob('*.c')),'assessor/numerical_depth/finite_oracles.py']},'author_validation':{'state':'passed','evidence':[receipt]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-host-or-analysis-contract','evidence':[public]}}
    updates.append(dict(id=ident,parent_project=data['project'],objective=objective,original_task=task,original_delayed_variant=variant['id'],minimum_days=variant['minimum_days'],dimensions=dimensions,closure_rationale=reason,artifacts=[public,technical],author_validation=dict(report=receipt,checks=sorted(set(checks)),scope='Author C execution and explicitly identified exact analysis models; not delayed learner performance'),remaining_author_work=[],evidence_limits=['Original rubric and independent changed-condition learner artifact remain mandatory.','Analysis-only design tasks are named separately from C execution; finite tests and observed compiler profiles do not prove universal numerical guarantees.']))
 assert objectives==set(bindings) and len(updates)==60
 result=dict(schema_version=1,scope='T17 original30 leaf assessments and30 original delayed variants; author implementation/execution',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt,export_path.relative_to(ROOT).as_posix()],updates=updates,summary=dict(original_criteria=30,original_delayed_variants=30,commands=len(report['commands']),export_commands=12,external_review='not-run',independent_learner='not-assessed'),evidence_limits=report['limitations'])
 (FOLDER/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']))
if __name__=='__main__':main()
