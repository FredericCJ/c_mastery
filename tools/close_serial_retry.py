"""Bind three exact original delayed IDs to new service mechanisms."""
import argparse,copy,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_retry';REPORT='research/v5/expansion/legacy_serial_retry/validation.json';SUPPORT='research/v5/expansion/legacy_serial_retry/completion_dispositions.json'
from research.v5.expansion.legacy_serial_retry.author_support import attach,PROJECTS
SPECS=[('X-T38-001','V1','stretch'),('X-T38-001','V2','transaction'),('X-T38-004','V1','retry')]
ARTIFACTS=['labs/serial_retry/CONTRACTS.md','labs/serial_retry/retry.h','labs/serial_retry/tests.c','assessor/serial_retry/retry.c','assessor/serial_retry/ANALYSIS.md','assessor/serial_retry/oracle.py']
MUTANT=dict(stretch='STRETCH',transaction='READBIT',retry='UNCERTAIN')

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def brief(x):
 t=(ROOT/('curriculum/v5/exercises/'+x+'.md')).read_text(encoding='utf8');m=json.loads(t.split('```json')[1].split('```')[0]);return m,t
def task(t,v):
 section=t.split('## Variants\n',1)[1].split('\n## ',1)[0];return re.search(r'\b'+v+r':\s*(.*?)(?=\s+V[12]:|\n\n|$)',section,re.S).group(1).strip()
def rows(r):
 out=[]
 for x,v,case in SPECS:
  m,t=brief(x);checks=[q['id'] for q in r['commands'] if '.'+case in q['id'] or '.mutation.'+MUTANT[case] in q['id'] or q['id'].endswith('.execute')];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:3]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
  out.append(dict(id=x+'-'+v,original_exercise=x,original_variant=v,original_task=task(t,v),minimum_days=m['delayed_transfer']['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks,case=case),closure_rationale='Exact '+case+' changed service is implemented with actual imported primitives and bounded native/Thumb observations; original independent delayed attempt remains separate.',remaining_author_work=[],evidence_limits=r['limits']))
 return out
def integrate():
 import run_serial_retry as runner
 public=sorted({n for n in runner.pins() if n.startswith('labs/')}|{'labs/serial_depth/src/serial_depth.c','labs/b045_systems/src/systems.c','labs/constrained_depth/timing/timing.c','labs/peripheral_depth/i2c/i2c.c'})
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-variants.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  ids=[c['id'] for c in d['criteria']]
  write(HERE/('attachment_'+project+'.json'),dict(project=project,supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],support=SUPPORT,criterion_ids=ids,scope='Additive original X changed-condition serial package; original parent criteria unchanged.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 for x in sorted({q[0] for q in SPECS}):
  p=ROOT/('curriculum/v5/exercises/'+x+'.md');old=HERE/(x+'.before-variants.md')
  if not old.exists():old.write_bytes(p.read_bytes())
  m,t=brief(x);m['shared_contracts']=list(dict.fromkeys(m['shared_contracts']+[ARTIFACTS[0]]));m['source_records']=[z for z in m['source_records'] if not z['id'].startswith('R-SR-')]+[dict(id=z['id'],path='research/v5/expansion/legacy_serial_retry/resources.json',passages=z['locators']) for z in read(HERE/'resources.json')['resources']]
  body=t.split('```',2)[2];heading='\n## Required serial changed-condition successor\n';body=body.split(heading)[0];vs=', '.join(v for a,v,_ in SPECS if a==x)
  body+=heading+'\nThe original '+vs+' task is concretely specified by [the required changed-condition contract](labs/serial_retry/CONTRACTS.md). Implement its unfinished public scaffold with the actual learner producer interfaces; the parent project export includes these files. The original family and rubric remain required. The successor author campaign does not administer the seven-day independent assessment or supply physical observations.\n'
  p.write_text('```json\n'+json.dumps(m,indent=2)+'\n```'+body,encoding='utf8')
 print('PASS additive X/project I2C retry integration')
def package():
 import run_serial_retry as runner
 r=read(HERE/'validation.json');e=read(HERE/'exports.json');files={pathlib.Path(__file__),ROOT/'tools/check_serial_retry.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for report in [r,e]:
  assert report['status']=='pass' and report['commands'] and all(q['passed'] for q in report['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in report[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in report['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-variants.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 for x in {q[0] for q in SPECS}:files.add(ROOT/('curriculum/v5/exercises/'+x+'.md'))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three exact original X delayed variants; no root/rubric/physical closure inferred',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_retry/exports.json'],updates=rows(r),remaining_parents=['Other X roots/rubrics/variants require their own exact existing-package reconciliation.','Independent delayed assessment, external review and physical observations remain separate.']));print('PASS3 exact serial variant dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
