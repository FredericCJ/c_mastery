"""Read-only exact T55 mapping, raw receipt, source and canonical export guard."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_concurrency_followthrough.author_support import OPEN,SUPPORT,CAMPAIGN,NOTES,records,attach,public_paths
from research.v3.verify_fixtures import compute
from audit_concurrency_followthrough import inspect,FAMILIES
COVERAGE=dict(original_criteria=30,delayed_variants=30,mechanism_commands=311,measurement_commands=357,observations=340,native_groups=64,paired_comparisons=8,isolated_packages=10,export_commands=180,provider_guards=5)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads((ROOT/p).read_text())
def need(value,message):
 if not value:raise ValueError(message)
def pins(mapping):
 for p,h in mapping.items():need(sha(ROOT/p)==h,'changed bytes '+p)
def validate_campaign(v):
 need(v['passed'] and v['open_gates']==OPEN,'incomplete/inflated gates');need(v['coverage']==COVERAGE,'coverage mismatch');need(set(v['components'])=={'mechanisms','measurements','audit','exports'},'missing producer');pins(v['source_sha256']);data={}
 for name,row in v['components'].items():
  need(sha(ROOT/row['path'])==row['sha256'],'changed receipt '+name);d=load(row['path']);need(d['passed'] and d['error'] is None,'failed '+name);pins(d['source_sha256']);pins(d['artifacts']);data[name]=d
 a=data['audit'];need(all(a['components'][n]==v['components'][n] for n in ('mechanisms','measurements','exports')),'audit different campaign');checks=inspect(data['mechanisms'],data['measurements'],data['exports']);need(checks==a['checks'],'raw audit mismatch')
 need({r['name'] for r in a['commands']}==set(FAMILIES) and len(a['commands'])==5,'missing provider guard')
 for row in a['commands']:
  d=load(row['log']);need(row['passed'] and row['returncode']==0 and row['command'][1:]==['tools/check_advanced_'+row['name']+'.py','--self-test'] and d['passed'] and d['errors']==[] and len(d['negative_controls'])==row['negative_controls'] and all(x['rejected'] for x in d['negative_controls']),'failed provider guard')
 for pkg in data['exports']['packages']:
  root=ROOT/pkg['public_root'];need(not pkg['private_reference_in_public'] and not any('assessor' in p.relative_to(root).parts or 'reference' in p.relative_to(root).parts for p in root.rglob('*')),'private public package');need(all(sha(root/p)==h for p,h in pkg['public_sha256'].items()),'changed package');need(all(p in pkg['public_sha256'] and sha(ROOT/p)==pkg['public_sha256'][p] for p in public_paths()),'incomplete relocated public inventory')
 for f in v['fixtures']:
  d=load(f['path']);need(sha(ROOT/f['path'])==f['sha256'] and compute(d['oracle'],d['input'])==f['actual']==d['expected'],'original fixture mismatch')
 need(len(v['fixtures'])==10,'missing fixture');resources=load('research/v5/expansion/legacy_concurrency_followthrough/resources.json');pins(resources['frozen_provider_sha256']);need(sha(ROOT/resources['original_fixture_oracle']['path'])==resources['original_fixture_oracle']['sha256'],'changed original oracle')
 for row in resources['resources']:need(sha(ROOT/row['path'])==row['sha256'],'changed primary source registration')
 return data
def main(report):
 v=load(CAMPAIGN);data=validate_campaign(v);checks=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 check('source-bound distinct mechanisms, raw measurements, provider guards and exports',True)
 for name,mutate in [('missing-variant',lambda x:x['coverage'].update(delayed_variants=29)),('external-promotion',lambda x:x.update(open_gates=[])),('changed-receipt',lambda x:x['components']['mechanisms'].update(sha256='0'*64)),('missing-measurements',lambda x:x['components'].pop('measurements')),('changed-source',lambda x:x.update(source_sha256={'labs/concurrency_followthrough/core/core.h':'0'*64}))]:
  bad=copy.deepcopy(v);mutate(bad)
  try:validate_campaign(bad);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 support=load(SUPPORT);mapping=load('labs/concurrency_followthrough/criteria.json')['criteria'];expected=set()
 for r in records():
  path='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path],cwd=ROOT,text=True));stripped=copy.deepcopy(r['criteria'])
  for c in stripped:c.pop('successor_support',None)
  check(r['project']+' original criterion objects',stripped==old['criteria']);check(r['project']+' historical bindings',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(r['project']+' idempotent attachment',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(v['id'] for v in c['variants']);row=next(x for x in mapping if x['id']==c['id']);note=NOTES[c['objective'].removeprefix('T55.')];check(c['id']+' original task and changed condition',row['original_task']==c['task'] and row['original_expected']==c['verification']['expected'] and row['changed_conditions']==c['variants'] and row['selectors']==note[3])
   for ident,rationale in [(c['id'],note[1]),(c['variants'][0]['id'],note[2])]:
    update=next(x for x in support['updates'] if x['id']==ident);check(ident+' exact derivation and selectors',update['closure_rationale']==rationale and update['selectors']==note[3])
 check('60 exact identities',len(support['updates'])==60 and {x['id'] for x in support['updates']}==expected);pins(support['source_sha256']);check('support identities',True);check('independent and physical gates retained',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state']=='not-run' for x in support['updates']));check('author work accounted',support['remaining_author_work']==[]);check('all dimension evidence concrete files',all((ROOT/p).is_file() for row in support['updates'] for value in row['dimensions'].values() for p in value['evidence']))
 from cmastery import assessments
 with tempfile.TemporaryDirectory(prefix='concurrency-canonical-export-') as temporary:
  for r in records():
   dest=Path(temporary)/r['project'];assessments.export(ROOT,r['project'],dest);check(r['project']+' canonical export',all((dest/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*')))
 result=dict(passed=all(x['passed'] for x in checks),checks=checks,source_sha256={p:sha(ROOT/p) for p in (CAMPAIGN,SUPPORT,'tools/check_concurrency_followthrough.py')},scope='All60 exact T55 IDs, with source-bound native, model, oracle, timing and export evidence; independent/physical gates retained.')
 dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_concurrency_followthrough') and not dest.exists(),'preserve owned historical receipt');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),failed=[x['name'] for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
