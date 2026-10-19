"""Actual project/exercise fixture exports; no new mechanism execution claim."""
from pathlib import Path
import copy, datetime, hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[6]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT/'tools'))
from common import parse

MAP = {
 'PR-T69-01': ('fixtures/dispatch/extensions.json', 'X-T69-001', ['R2','R3'], 'exactly-once partial-progress callback observations'),
 'PR-T69-03': ('fixtures/dispatch/dictionary.json', 'X-T69-003', ['R2','R3'], 'ordered fold lengths0..5 and operand/status traces'),
 'PR-T69-05': ('fixtures/dispatch/extensions.json', 'X-T69-005', ['R2','R3'], 'clone acquisition cuts and inline/spilled capture rollback'),
 'PR-T69-06': ('fixtures/dispatch/resolution.json', 'X-T69-006', ['R2','R3','R4'], 'four-type partial-order applicability, ambiguity and cache invalidation'),
 'PR-T70-01': ('fixtures/dod/histories.json', 'X-T70-001', ['R2','R3'], 'signed column updates, product bounds and transactional rejection'),
 'PR-T70-04': ('fixtures/dod/histories.json', 'X-T70-004', ['R2','R3'], 'length7 sparse histories, generation retirement and rank bijection'),
 'PR-T70-06': ('fixtures/dod/histories.json', 'X-T70-006', ['R2','R3'], 'persistent root histories and every cloned-node failure cut'),
 'PR-T70-07': ('fixtures/dod/dictionary.json', 'X-T70-007', ['R2','R3','R4'], 'canonical-key equality, immutable versions and bounded constant-hash collisions'),
}

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p): return json.loads(p.read_text(encoding='utf8'))

def main():
 stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
 work=ROOT/'build/fixture-consumer-exports'/stamp;work.mkdir(parents=True)
 rows=[];bindings=[];inputs={Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/project_assets.py',ROOT/'cmastery/exercises.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/project_graph.json',ROOT/'curriculum/v5/exercises.json'}
 for project,(fixture,xid,criteria,meaning) in MAP.items():
  name='curriculum/projects/'+project+'.md';path=ROOT/name;before=HERE/'before'/name
  original,old_body=parse(before);current,body=parse(path)
  expected=copy.deepcopy(original);expected['fixtures'].append(fixture)
  assert current==expected and body==old_body
  assert path.read_bytes().split(b'```',2)[2]==before.read_bytes().split(b'```',2)[2]
  brief=ROOT/'curriculum/v5/exercises'/(xid+'.md');exercise,_=parse(brief)
  selected=[r for r in exercise['rubric'] if r['id'] in {xid+'-'+c for c in criteria}]
  assert len(selected)==len(criteria)
  family=exercise['lab']['family'];inputs.update({path,before,brief,ROOT/fixture,ROOT/'labs'/family/'family.json'})
  private={sha(p) for p in (ROOT/'assessor'/family).rglob('*') if p.is_file() and p.suffix in {'.c','.md'} and '__pycache__' not in p.parts}
  exported=[]
  for mode in ['project','exercise']:
   destination=work/(project+'-'+mode)
   command=[sys.executable,'-m','cmastery','init',project,str(destination)]+(['--exercise',xid] if mode=='exercise' else [])
   run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='utf8',timeout=120)
   record=dict(project=project,exercise=xid if mode=='exercise' else None,command=command,exit_code=run.returncode,stdout=run.stdout,stderr=run.stderr)
   rows.append(record);assert run.returncode==0,record
   exported_files=[p for p in destination.rglob('*') if p.is_file()]
   assert not any('assessor' in p.relative_to(destination).parts or sha(p) in private for p in exported_files)
   for required in current['fixtures']:
    assert (destination/required).is_file() and sha(destination/required)==sha(ROOT/required),required
   record.update(passed=True,fixture_sha256=sha(destination/fixture),assessor_leak=False)
   exported.append(str(destination))
  bindings.append(dict(project=project,exercise=xid,fixture=fixture,mechanism=meaning,rubric=selected,project_body_unchanged=True,only_fixture_metadata_added=True,exports=exported))
 p4='curriculum/v5/expansion/legacy_assessments/PR-P4-02.json';old=read(HERE/'before'/p4);new=read(ROOT/p4)
 expected=copy.deepcopy(old);expected['supplied_artifacts'].append('fixtures/legacy_checkpoints/transfer/P4_timing.json')
 assert new==expected
 p4export=read(ROOT/'reviews/v5/expansion/legacy_checkpoint_firmware/exports.json')
 assert p4export['passed'] and len(p4export['commands'])==14
 p4init=next(c for c in p4export['commands'] if c['label']=='PR-P4-02:init')
 p4work=Path(p4init['command'][-1]);timing='fixtures/legacy_checkpoints/transfer/P4_timing.json'
 assert sha(p4work/timing)==sha(ROOT/timing)
 inputs.update({ROOT/p4,ROOT/timing,ROOT/'tools/close_checkpoint_firmware_followthrough.py',ROOT/'reviews/v5/expansion/legacy_checkpoint_firmware/exports.json'})
 bindings.append(dict(project='PR-P4-02',fixture=timing,criterion='AX-PR-P4-02-T30.05.b',variant='AX-PR-P4-02-T30.05.b-DELAYED',mechanism='fixed-priority response recurrence, higher-priority interference and finite queue/UART service budget',historical_criteria_unchanged=True,export=str(p4work)))
 result=dict(passed=True,commands=rows,consumer_bindings=bindings,unique_fixtures=6,project_and_exercise_initializers=16,p4_export_checks=14,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(inputs)},limits=['Packaging/integration correction only; no new C correctness, physical timing, independent learner or release acceptance claim.','Existing source-bound campaigns and closed publication correction remain unchanged.'])
 (HERE/'result.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
 print(json.dumps(dict(passed=True,initializers=len(rows),unique_fixtures=6,p4_export_checks=14)))

if __name__=='__main__':main()
