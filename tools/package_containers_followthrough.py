"""Validate and bind every original T14 leaf/variant to specific evidence."""
import datetime,hashlib,importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
FOLDER=ROOT/'research/v5/expansion/legacy_containers'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 spec=importlib.util.spec_from_file_location('containers_bindings',FOLDER/'bindings.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);bindings=module.BINDINGS
 report_path=FOLDER/'validation.json';report=json.loads(report_path.read_text());assert report['status']=='pass' and all(r['passed'] for r in report['commands'])
 pins=report['source_sha256']
 for name,value in pins.items():assert sha(ROOT/name)==value,'stale report source '+name
 for name,value in report['outputs_sha256'].items():assert sha(ROOT/name)==value,'stale observation output '+name
 gate=json.loads((FOLDER/'research_gate.json').read_text())
 for name,value in gate['research_input_sha256'].items():assert sha(ROOT/name)==value,'stale source gate '+name
 command_ids={x['id'] for x in report['commands']};updates=[];all_objectives=set();files={ROOT/x for x in pins}|{report_path,FOLDER/'bindings.py',FOLDER/'research_gate.json',FOLDER/'register_sources.py',FOLDER/'sources.md',pathlib.Path(__file__),ROOT/'tools/integrate_containers_followthrough.py',ROOT/'tools/check_containers_followthrough.py'}
 for parent in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T14-*.json')):
  data=json.loads(parent.read_text());files.add(parent)
  for criterion in data['criteria']:
   objective=criterion['objective'];all_objectives.add(objective);ordinary,transfer,analysis=bindings[objective]
   for delayed,selectors in [(False,ordinary),(True,transfer)]:
    checks=[]
    for selector in selectors:
     matches=sorted(x for x in command_ids if x.startswith(selector) if not x.endswith('.compile'))
     if not matches:matches=sorted(x for x in command_ids if x==selector)
     assert matches,(objective,selector);checks.extend(matches)
    variant=criterion['variants'][0];ident=variant['id'] if delayed else criterion['id'];task=variant['task'] if delayed else criterion['task']
    public='labs/containers_depth/CONTRACTS.md';technical='assessor/containers_depth/ANALYSIS.md';receipt=report_path.relative_to(ROOT).as_posix()
    dimensions={'teaching':{'state':'complete','evidence':[parent.relative_to(ROOT).as_posix(),public]},'implementation':{'state':'complete','evidence':[public,technical,'assessor/containers_depth/sequence.c','assessor/containers_depth/ordered.c','assessor/containers_depth/indexed.c','assessor/containers_depth/transfer.c','assessor/containers_depth/graph_consumer.c']},'author_validation':{'state':'passed','evidence':[receipt]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-applicable-to-host-or-object-contract','evidence':[public]}}
    updates.append({'id':ident,'parent_project':data['project'],'objective':objective,'original_task':task,'original_delayed_variant':variant['id'],'minimum_days':variant['minimum_days'],'dimensions':dimensions,'closure_rationale':analysis,'artifacts':[public,technical],'author_validation':{'report':receipt,'checks':sorted(set(checks)),'scope':'Author execution plus explicit source/finite-model analysis; not delayed learner performance'},'remaining_author_work':[],'evidence_limits':['Original rubric and changed-condition task remain required for independent learner assessment.','Exact published C17/C23 access, physical-cache causation, actual concurrent/persistent implementations and all-platform behavior are not closed by these bounded observations.']})
 assert all_objectives==set(bindings) and len(updates)==90
 result={'schema_version':1,'scope':'T14 original45 leaf assessments and45 original delayed variants, author implementation/execution only','recorded_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'source_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},'current_reports':[report_path.relative_to(ROOT).as_posix()],'updates':updates,'summary':{'original_criteria':45,'original_delayed_variants':45,'commands':len(report['commands']),'external_review':'not-run','independent_learner':'not-assessed'},'evidence_limits':report['limitations']}
 (FOLDER/'completion_dispositions.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result['summary']))
if __name__=='__main__':main()
