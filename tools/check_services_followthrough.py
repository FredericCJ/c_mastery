"""Read-only exact T56 source, raw receipt, variant and canonical export guard."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_services_followthrough.author_support import OPEN,SUPPORT,CAMPAIGN,NOTES,records,attach,public_paths
from research.v3.verify_fixtures import compute
from audit_services_followthrough import inspect,sha
COVERAGE=dict(original_criteria=33,delayed_variants=33,positive_guest_profiles=216,guest_mutants=5,guest_scaffold_rejections=2,native_profiles=18,native_negative_controls=4,target_images=32,mechanism_commands=271,isolated_packages=11,export_commands=44,retained_provider_guards=2)

def load(p):return json.loads((ROOT/p).read_text())
def need(value,message):
 if not value:raise ValueError(message)
def pins(mapping):
 for p,h in mapping.items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'changed bytes '+p)

def validate_campaign(v):
 need(v['passed'] and v['open_gates']==OPEN,'incomplete/inflated gates');need(v['coverage']==COVERAGE,'coverage mismatch');need(set(v['components'])=={'mechanisms','audit','exports'},'missing producer');pins(v['source_sha256']);data={}
 for name,row in v['components'].items():
  need(sha(ROOT/row['path'])==row['sha256'],'changed receipt '+name);d=load(row['path']);need(d['passed'] and d['error'] is None,'failed '+name);pins(d['source_sha256']);pins(d['artifacts']);data[name]=d
 a=data['audit'];need(all(a['components'][n]==v['components'][n] for n in ('mechanisms','exports')),'audit different campaign');checks,observations=inspect(data['mechanisms'],data['exports']);need(checks==a['checks'] and observations==a['observations'] and all(x['passed'] for x in checks),'raw audit mismatch')
 need({x['name'] for x in a['providers']}=={'service-adapters','amp-followthrough'} and len(a['providers'])==2,'missing provider guard')
 for row in a['providers']:need(row['passed'] and sha(ROOT/row['path'])==row['sha256'],'changed retained provider')
 from check_service_adapters_v5 import validate,regressions
 old=load('reviews/v5/expansion/followthrough/service_adapter_campaign.json');need(not validate(old),'retained service guard failed');need(regressions(old)==next(x['controls'] for x in a['providers'] if x['name']=='service-adapters'),'retained service negative controls changed')
 from check_amp_followthrough import validate_campaign as amp_validate
 amp_validate(load('reviews/v5/expansion/legacy_amp_followthrough/campaign.json'))
 for pkg in data['exports']['packages']:
  root=ROOT/pkg['public_root'];need(not pkg['private_reference_in_public'] and not any('assessor' in p.relative_to(root).parts for p in root.rglob('*')),'private public package');need(all(p in pkg['public_sha256'] and sha(ROOT/p)==pkg['public_sha256'][p] for p in public_paths()),'incomplete relocated public inventory')
 for f in v['fixtures']:
  d=load(f['path']);need(sha(ROOT/f['path'])==f['sha256'] and compute(d['oracle'],d['input'])==f['actual']==d['expected'],'original fixture mismatch')
 need(len(v['fixtures'])==11,'missing fixture');resources=load('research/v5/expansion/legacy_services_followthrough/resources.json');pins(resources['frozen_provider_sha256']);pins(resources['retained_receipts']);need(sha(ROOT/resources['original_fixture_oracle']['path'])==resources['original_fixture_oracle']['sha256'],'changed original oracle')
 for row in resources['resources']:
  need(sha(ROOT/row['path'])==row['sha256'],'changed primary source registration')
  for passage in row['registered_passages']:need(sha(ROOT/passage['path'])==passage['sha256'],'changed registered manual bytes')
 return data

def main(report):
 v=load(CAMPAIGN);data=validate_campaign(v);checks=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 check('distinct source-bound mechanisms, raw logs, two retained guards and exports',True)
 for name,mutate in [('missing-variant',lambda x:x['coverage'].update(delayed_variants=32)),('physical-promotion',lambda x:x.update(open_gates=[])),('changed-receipt',lambda x:x['components']['mechanisms'].update(sha256='0'*64)),('missing-exports',lambda x:x['components'].pop('exports')),('changed-source',lambda x:x.update(source_sha256={'labs/services_followthrough/kernel/abi.h':'0'*64}))]:
  bad=copy.deepcopy(v);mutate(bad)
  try:validate_campaign(bad);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 # Mutate raw semantic rows before source/receipt checks, proving the audit reads outcomes.
 raw=copy.deepcopy(data['mechanisms']);raw['components']=[x for x in raw['components'] if x.get('case')!=15];audit,_=inspect(raw,data['exports']);check('omitted simultaneous-fault mechanism rejected',not all(x['passed'] for x in audit))
 raw=copy.deepcopy(data['mechanisms']);next(x for x in raw['rows'] if x['name']=='negative-vm-run')['returncode']=0;audit,_=inspect(raw,data['exports']);check('foreign-instance control success rejected',not all(x['passed'] for x in audit))
 support=load(SUPPORT);mapping=load('labs/services_followthrough/criteria.json')['criteria'];expected=set()
 for r in records():
  path='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path],cwd=ROOT,text=True));stripped=copy.deepcopy(r['criteria'])
  for c in stripped:c.pop('successor_support',None)
  check(r['project']+' original criterion objects',stripped==old['criteria']);check(r['project']+' historical bindings',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(r['project']+' idempotent attachment',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(x['id'] for x in c['variants']);row=next(x for x in mapping if x['id']==c['id']);note=NOTES[c['objective'].removeprefix('T56.')];check(c['id']+' exact task and changed condition',row['original_task']==c['task'] and row['original_expected']==c['verification']['expected'] and row['changed_conditions']==c['variants'] and row['guest_cases']==note[3])
   for ident,rationale in [(c['id'],note[1]),(c['variants'][0]['id'],note[2])]:
    update=next(x for x in support['updates'] if x['id']==ident);check(ident+' exact derivation and selectors',update['closure_rationale']==rationale and update['guest_cases']==note[3])
 check('66 exact identities',len(support['updates'])==66 and {x['id'] for x in support['updates']}==expected);pins(support['source_sha256']);check('support identities',True);check('physical and independent gates retained',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state']=='not-run-H745-capture-required' for x in support['updates']));check('author work accounted',support['remaining_author_work']==[]);check('dimension evidence concrete files',all((ROOT/p).is_file() for row in support['updates'] for value in row['dimensions'].values() for p in value['evidence']))
 from cmastery import assessments
 with tempfile.TemporaryDirectory(prefix='services-canonical-export-') as temporary:
  for r in records():
   dest=Path(temporary)/r['project'];assessments.export(ROOT,r['project'],dest);check(r['project']+' canonical export',all((dest/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*')))
 result=dict(passed=all(x['passed'] for x in checks),checks=checks,source_sha256={p:sha(ROOT/p) for p in (CAMPAIGN,SUPPORT,'tools/check_services_followthrough.py')},scope='66 exact T56 IDs with distinct native/guest/image/export evidence. Retained PR07 unchanged; physical and independent gates open.')
 dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_services_followthrough') and not dest.exists(),'preserve owned historical receipt');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),failed=[x['name'] for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
