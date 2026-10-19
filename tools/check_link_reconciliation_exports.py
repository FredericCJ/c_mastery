"""Run the real initializer with narrowly source-current exercise catalog entries.
Does not write the shared catalog or claim a globally generated edition passed.
"""
import datetime,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import cli,exercises,project_assets
from close_link_reconciliation import MAP,BASE,brief,sha,write,HERE

def main():
 folder=ROOT/'build'/('link-reconciliation-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));values={};pins={};outputs={};commands=[]
 for suffix in MAP:
  x='X-T'+suffix;p,d,t=brief(x);bound=d['shared_contracts']+d['fixtures']+[r['path'] for r in d['source_records']]
  value=dict(d,brief=p.relative_to(ROOT).as_posix(),brief_sha256=sha(p),bound_files={n:sha(ROOT/n) for n in sorted(set(bound))});values[x]=value;pins[value['brief']]=sha(p)
  for n in bound:pins[n]=sha(ROOT/n)
 # Explicit local catalog input lets this test inspect current canonical exports
 # while root alone owns global regeneration. All normal exercise/hash/harness
 # and supplement-export checks remain in the actual initialization path.
 exercises.catalog=lambda:dict(exercises=values,order=list(values))
 for x,d in values.items():
  declared=project_assets.prepare(ROOT,d['project']);project_brief='curriculum/projects/'+d['project']+'.md';pins[project_brief]=sha(ROOT/project_brief)
  workspace=folder/x;result=cli.instantiate(d['project'],workspace,exercise_id=x);assert result['exercise']==x
  assert result.get('project_assets',[])==declared
  for asset in declared:
   n=asset['path'];p=workspace/n;assert p.is_file() and sha(p)==asset['sha256'],n
   pins[n]=asset['sha256'];outputs[p.relative_to(ROOT).as_posix()]=sha(p)
  for n in d['shared_contracts']+d['fixtures']:
   p=workspace/n;assert p.is_file() and sha(p)==sha(ROOT/n),n;outputs[p.relative_to(ROOT).as_posix()]=sha(p)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  for token in [d['id'],*[q['id'] for q in d['rubric']]]:assert token in (workspace/'EXERCISE.md').read_text(encoding='utf8'),token
  pins['curriculum/v5/expansion/legacy_assessments/'+d['project']+'.json']=sha(ROOT/('curriculum/v5/expansion/legacy_assessments/'+d['project']+'.json'))
  commands.append(dict(id=x+'.initialize',command=['cmastery.cli.instantiate',d['project'],str(workspace),'exercise_id='+x],passed=True,exercise=x,contracts=len(d['shared_contracts']),assessor_files=0,workspace=workspace.relative_to(ROOT).as_posix(),project_assets=declared))
 for n in ['cmastery/cli.py','cmastery/exercises.py','cmastery/assessments.py','cmastery/project_assets.py','cmastery/learning.py','tools/check_link_reconciliation_exports.py','tools/close_link_reconciliation.py']:pins[n]=sha(ROOT/n)
 assert all(sha(ROOT/n)==h for n,h in pins.items())
 write(HERE/'exports.json',dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual12 initializer executions using a scoped canonical-current catalog input; shared edition catalog is not regenerated or accepted.','Contract copying and assessor separation checked; no independent learner success.']))
 print('PASS12 actual exercise initializers and shared-contract copies')
if __name__=='__main__':main()
