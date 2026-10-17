"""Execute the two additive project exports outside the repository under Linux."""
from pathlib import Path
import hashlib
import json
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[4]
sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'))
from cmastery import cli, assessments
from check_exercise_integration import check_exported_stub

def main():
 observations=[]
 with tempfile.TemporaryDirectory(prefix='cmastery lifecycle export ') as temp:
  temporary=Path(temp).resolve()
  assert not temporary.is_relative_to(ROOT.resolve())
  fake=temporary/'negative-root';destination=temporary/'negative-export'
  source=fake/'curriculum/v5/expansion/legacy_assessments/PR-T23-01.json'
  source.parent.mkdir(parents=True);destination.mkdir()
  answer=fake/'assessor/answer.md';answer.parent.mkdir();answer.write_text('private assessor sentinel',encoding='utf-8')
  lab=fake/'labs/example';lab.mkdir(parents=True)
  contract=lab/'CONTRACTS.md';contract.write_text('public learner contract',encoding='utf-8')
  link=lab/'answer-link.md';link.symlink_to(answer)
  outside=temporary/'outside.md';outside.write_text('outside sentinel',encoding='utf-8')
  (lab/'outside-link.md').symlink_to(outside)
  negative=[]
  for name in ['assessor/answer.md','labs/../assessor/answer.md',str(contract.resolve()),'labs/example/answer-link.md','labs/example/outside-link.md','labs/example','labs/example/missing.md','labs\\example\\CONTRACTS.md']:
   source.write_text(json.dumps(dict(project='PR-T23-01',shared_contracts=[name])),encoding='utf-8')
   try:assessments.export(fake,'PR-T23-01',destination)
   except ValueError as error:
    assert str(error)=='Invalid shared assessment contract'
    negative.append(dict(contract=name,rejected=True))
   else:raise AssertionError('unsafe shared contract exported: '+name)
  assert answer.read_text(encoding='utf-8')=='private assessor sentinel'
  assert contract.read_text(encoding='utf-8')=='public learner contract'
  assert not list(destination.rglob('*'))
  outside_directory=temporary/'outside-directory';outside_directory.mkdir()
  (destination/'labs').symlink_to(outside_directory,target_is_directory=True)
  source.write_text(json.dumps(dict(project='PR-T23-01',shared_contracts=['labs/example/CONTRACTS.md'])),encoding='utf-8')
  try:assessments.export(fake,'PR-T23-01',destination)
  except ValueError as error:
   assert str(error)=='Invalid shared assessment contract'
   negative.append(dict(contract='destination symlink outside export',rejected=True))
  else:raise AssertionError('destination traversal accepted')
  assert not list(outside_directory.rglob('*'))
  assert outside.read_text(encoding='utf-8')=='outside sentinel'
  for project in ['PR-T23-01','PR-T25-01']:
   work=temporary/project
   cli.instantiate(project,work,family_name='posix_lifecycle_depth')
   files=[p.relative_to(work).as_posix() for p in work.rglob('*') if p.is_file()]
   assert not any('assessor' in Path(p).parts for p in files)
   source='src/posix_lifecycle.c'
   assert (work/source).read_bytes()==(ROOT/'labs/posix_lifecycle_depth'/source).read_bytes()
   assert (work/source).read_bytes()!=(ROOT/'assessor/posix_lifecycle_depth/posix_lifecycle.c').read_bytes()
   assert (work/'ASSESSMENT.md').is_file()
   contract=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')).read_text(encoding='utf-8'))
   body=(work/'ASSESSMENT.md').read_text(encoding='utf-8')
   assert all(c['id'] in body and c['task'] in body for c in contract['criteria'])
   result=check_exported_stub(work,temporary)
   assert result['status']=='passed',result
   observations.append(dict(project=project,status='passed',criteria=len(contract['criteria']),assessor_absent=True,canonical_unsolved_source=True,stub_execution=result))
 paths=['research/v5/expansion/legacy_runtime/check_export.py','tools/check_exercise_integration.py','cmastery/cli.py','cmastery/assessments.py','labs/posix_lifecycle_depth/family.json']
 paths += [p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/posix_lifecycle_depth').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 paths += ['assessor/posix_lifecycle_depth/posix_lifecycle.c','curriculum/v5/expansion/legacy_assessments/shared-contract.md']
 paths += ['curriculum/v5/expansion/legacy_assessments/'+p+'.json' for p in ['PR-T23-01','PR-T25-01']]
 report=dict(status='passed',command='python3 research/v5/expansion/legacy_runtime/check_export.py',observations=observations,negative_controls=negative,input_sha256={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths},limitations=['Temporary Linux project exports and intended unsolved-stub rejection only. No learner acceptance or complete assessor portfolio is inferred.'])
 destination=ROOT/'reviews/v5/expansion/legacy_runtime/export_validation.json'
 destination.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps(dict(status=report['status'],projects=[x['project'] for x in observations],negative_controls=len(negative),claim='Both exported learner scaffolds compile and reach intended semantic assertions; assessor source absent.')))

if __name__=='__main__':main()
