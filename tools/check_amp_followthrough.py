"""Read-only exact-ID, source/producer, machine, semantic-export guard."""
from pathlib import Path
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_amp_followthrough.author_support import OPEN,SUPPORT,CAMPAIGN,NOTES,records,attach,public_paths
from research.v3.verify_fixtures import compute
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(v,m):
 if not v:raise ValueError(m)
def pins(values):
 for p,h in values.items():need(sha(ROOT/p)==h,'changed bytes '+p)
def load(p):return json.loads((ROOT/p).read_text())
def validate_campaign(v):
 need(v['passed'] and v['open_gates']==OPEN,'incomplete/inflated gates')
 need(v['coverage']=={'original_criteria':30,'delayed_variants':30,'mechanism_commands':147,'supplement_commands':24,'isolated_packages':10,'export_commands':56},'coverage mismatch')
 need(set(v['components'])=={'mechanisms','audit','exports'},'missing producer')
 data={}
 for name,r in v['components'].items():need(sha(ROOT/r['path'])==r['sha256'],'changed receipt '+name);data[name]=load(r['path'])
 pins(v['source_sha256'])
 for name,r in data.items():need(r['passed'] and r['error'] is None,'failed '+name);pins(r['source_sha256']);pins(r['artifacts'])
 m=data['mechanisms'];a=data['audit'];e=data['exports'];need(len(m['rows'])==147 and len(m['checks'])==5 and all(x['passed'] for x in m['checks']),'mechanism matrix missing');need(len(a['commands'])==24 and len(a['images'])==28 and all(x['passed'] for x in a['checks']),'audit/supplement missing');need(len(e['rows'])==56 and len(e['packages'])==10,'export matrix missing')
 rows={r['name']:r for r in m['rows']};supp={r['name']:r for r in a['commands']};need(all(x in rows or x in supp for values in NOTES.values() for x in values[3].split()),'unmapped exact check')
 for row in m['rows']+e['rows']:
  need(row['passed'] and row['returncode']==row['expected'],'wrong exit '+row['name']);out=(ROOT/row['log']).read_text();need(row['marker'] is None or row['marker'] in out,'missing cause '+row['name']);need('runtime error:' not in out and 'ERROR: AddressSanitizer' not in out,'unexpected sanitizer failure')
 for row in a['commands']:need(row['returncode']==0 and sha(ROOT/row['log'])==row['sha256'],'supplement source/log mismatch')
 for image in a['images']:
  b=image['linked_stack_bounds'];need(b['MSP_selected_bytes']<8192 and (b['PSP_selected_bytes'] is None or b['PSP_selected_bytes']<2048),'selected stack capacity')
 resources=load('research/v5/expansion/legacy_amp_followthrough/resources.json')
 for source in resources['resources']:need(sha(ROOT/source['path'])==source['sha256'],'manual identity drift')
 pins(resources['frozen_provider_sha256'])
 base=(ROOT/v['components']['mechanisms']['path']).parent
 for directory in base.iterdir():
  if not directory.is_dir() or not (directory/'report.json').exists():continue
  r=json.loads((directory/'report.json').read_text());negative='mutant' in directory.name;need(r['passed']!=negative,'wrong image status');q=[c for c in r['rows'] if c['label'].endswith('emulator')]
  if q:
   need(len(q)==1 and '-M' in q[0]['command'] and 'mps2-an386' in q[0]['command'] and '-nic' in q[0]['command'] and 'none' in q[0]['command'],'wrong machine command')
   need(q[0]['returncode']==(229 if 'mutant1' in directory.name else 237) if negative else q[0]['returncode']==0,'wrong semantic target exit')
   if not negative:
    words=[int(x,16) for x in (ROOT/q[0]['log']).read_text().splitlines() if re.fullmatch('[0-9a-f]{8}',x)];need(len(words)==41,'missing actual ledger');offered,done,dropped,epoch,traffic,*rest=words;need(offered==512 and done+dropped==512 and epoch==2,'wrong observed conservation');need(traffic==(0 if 'local' in directory.name else done),'wrong moved-service traffic')
  for image in r['images']:need(image['physical_status']=='not-run' and not image['external_runtime'] and not image['unresolved'],'physical/dependency overclaim')
 for pkg in e['packages']:
  root=ROOT/pkg['public_root'];need(not pkg['private_reference_in_public'] and not any('assessor' in p.relative_to(root).parts or 'reference' in p.relative_to(root).parts for p in root.rglob('*')),'private public package');need(all(sha(root/p)==h for p,h in pkg['public_sha256'].items()),'changed package')
  need(all(p in pkg['public_sha256'] and sha(ROOT/p)==pkg['public_sha256'][p] for p in public_paths()),'incomplete relocated public inventory')
 for f in v['fixtures']:d=load(f['path']);need(sha(ROOT/f['path'])==f['sha256'] and compute(d['oracle'],d['input'])==f['actual']==d['expected'],'original fixture mismatch')
 need(len(v['fixtures'])==10,'missing fixtures')
 return data
def main(report):
 v=load(CAMPAIGN);validate_campaign(v);checks=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 check('all exact source-bound mechanisms and exports',True)
 for name,mutate in [('missing-variant',lambda x:x['coverage'].update(delayed_variants=29)),('external-promotion',lambda x:x.update(open_gates=[])),('changed-receipt',lambda x:x['components']['mechanisms'].update(sha256='0'*64)),('missing-audit',lambda x:x['components'].pop('audit')),('changed-source',lambda x:x.update(source_sha256={'labs/amp_followthrough/core/amp.h':'0'*64}))]:
  bad=copy.deepcopy(v);mutate(bad)
  try:validate_campaign(bad);rejected=False
  except (ValueError,KeyError):rejected=True
  check(name,rejected)
 support=load(SUPPORT);mapping=load('labs/amp_followthrough/criteria.json')['criteria'];expected=set()
 for r in records():
  path='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path],cwd=ROOT,text=True));stripped=copy.deepcopy(r['criteria'])
  for c in stripped:c.pop('successor_support',None)
  check(r['project']+' original criterion objects',stripped==old['criteria']);check(r['project']+' historical bindings',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(r['project']+' idempotent attachment',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(v['id'] for v in c['variants']);row=next(x for x in mapping if x['id']==c['id']);note=NOTES[c['objective'].removeprefix('T54.')]
   check(c['id']+' original task and actual variant',row['original_task']==c['task'] and row['original_expected']==c['verification']['expected'] and row['changed_conditions']==c['variants'] and row['mechanism_checks']==note[3].split())
 check('60 exact original identities',len(support['updates'])==60 and {x['id'] for x in support['updates']}==expected);pins(support['source_sha256']);check('support source identities',True)
 check('independent/physical gates retained',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state'].startswith('not-run') for x in support['updates']));check('author work accounted',support['remaining_author_work']==[]);check('all dimension evidence concrete files',all((ROOT/p).is_file() for row in support['updates'] for value in row['dimensions'].values() for p in value['evidence']))
 from cmastery import assessments
 with tempfile.TemporaryDirectory(prefix='amp-canonical-export-') as temp:
  for r in records():
   dest=Path(temp)/r['project'];assessments.export(ROOT,r['project'],dest);check(r['project']+' canonical export',all((dest/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*')))
 result=dict(passed=all(x['passed'] for x in checks),checks=checks,source_sha256={p:sha(ROOT/p) for p in (CAMPAIGN,SUPPORT,'tools/check_amp_followthrough.py')},scope='All60 exact T54 IDs; source-bound distinct native/model/image/emulator mechanisms and exports, physical and independent gates open.')
 dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_amp_followthrough') and not dest.exists(),'preserve owned historical receipt');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),failed=[x['name'] for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
