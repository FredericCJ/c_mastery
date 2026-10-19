"""Pin exact T16 criterion and original-delayed successor evidence."""
import datetime,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_allocation'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 spec=importlib.util.spec_from_file_location('allocation_bindings',FOLDER/'bindings.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);bindings=module.BINDINGS
 report_path=FOLDER/'validation.json';report=json.loads(report_path.read_text(encoding='utf8'));assert report['status']=='pass' and all(x['passed'] for x in report['commands'])
 for key in ['source_sha256','outputs_sha256']:
  for name,value in report[key].items():assert sha(ROOT/name)==value,'stale '+key+' '+name
 gate=json.loads((FOLDER/'research_gate.json').read_text(encoding='utf8'))
 for name,value in gate['research_input_sha256'].items():assert sha(ROOT/name)==value,'stale source gate '+name
 command_ids={r['id'] for r in report['commands']};updates=[];objectives=set();files={ROOT/x for x in report['source_sha256']}|{report_path,FOLDER/'bindings.py',FOLDER/'research_gate.json',FOLDER/'register_sources.py',FOLDER/'sources.md',pathlib.Path(__file__),ROOT/'tools/integrate_allocation_followthrough.py',ROOT/'tools/check_allocation_followthrough.py'}
 public='labs/allocation_depth/CONTRACTS.md';technical='assessor/allocation_depth/ANALYSIS.md';receipt=report_path.relative_to(ROOT).as_posix()
 for parent in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T16-*.json')):
  data=json.loads(parent.read_text(encoding='utf8'));files.add(parent)
  for criterion in data['criteria']:
   objective=criterion['objective'];objectives.add(objective);ordinary,transfer,reason=bindings[objective]
   for delayed,selectors in [(False,ordinary),(True,transfer)]:
    checks=[]
    for selector in selectors:
     matches=sorted(x for x in command_ids if x.startswith(selector) and not x.endswith('.compile'));assert matches,(objective,selector);checks.extend(matches)
    variant=criterion['variants'][0];ident=variant['id'] if delayed else criterion['id'];task=variant['task'] if delayed else criterion['task']
    dimensions={'teaching':{'state':'complete','evidence':[parent.relative_to(ROOT).as_posix(),public]},'implementation':{'state':'complete','evidence':[public,technical,*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'assessor/allocation_depth').glob('*.c')),'assessor/allocation_depth/finite_oracles.py']},'author_validation':{'state':'passed','evidence':[receipt]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-observed' if objective=='T16.12.a' else 'not-applicable-to-host-or-object-contract','evidence':[] if objective=='T16.12.a' else [public]}}
    updates.append(dict(id=ident,parent_project=data['project'],objective=objective,original_task=task,original_delayed_variant=variant['id'],minimum_days=variant['minimum_days'],dimensions=dimensions,closure_rationale=reason,artifacts=[public,technical],author_validation=dict(report=receipt,checks=sorted(set(checks)),scope='Actual author execution, source analysis and explicitly identified finite derivations; not delayed learner performance'),remaining_author_work=[],evidence_limits=['Original objective-specific rubric and changed-condition artifact remain mandatory for independent learner assessment.','Finite models are identified separately from executed C; firmware observations, external review and exact published-ISO access are not invented.']))
 assert objectives==set(bindings) and len(updates)==84
 result=dict(schema_version=1,scope='T16 original42 leaf assessments and42 original delayed variants; author implementation/execution',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[receipt],updates=updates,summary=dict(original_criteria=42,original_delayed_variants=42,commands=len(report['commands']),external_review='not-run',independent_learner='not-assessed'),evidence_limits=report['limitations'])
 (FOLDER/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(result['summary']))
if __name__=='__main__':main()
