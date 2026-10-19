"""Read-only original-contract, source, actual-machine and isolated-export guard."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_mcu_followthrough.author_support import OPEN,records,attach,public_paths,SUPPORT
from research.v5.expansion.legacy_mcu_followthrough.evidence_map import EVIDENCE
from audit_mcu_followthrough import fixture,serial_block
CAMPAIGN='reviews/v5/expansion/legacy_mcu_followthrough/campaign.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads((ROOT/p).read_text())
def need(value,message):
 if not value:raise ValueError(message)
def pins(mapping):
 for p,h in mapping.items():need(sha(ROOT/p)==h,'Changed bytes: '+p)
def row_valid(r):
 need(r['passed'],'Recorded failure: '+r['name']);need(r['returncode']==0 if r['expected']==0 else r['returncode']!=0 and r['returncode']!=124,'Wrong exit: '+r['name'])
 log=(ROOT/r['log']).read_text();need(r.get('marker') is None or r['marker'] in log,'Missing cause: '+r['name'])
 return log
def validate_campaign(v,files=True):
 need(v['passed'] and v['open_gates']==OPEN,'Inflated/failed composed claim')
 need(v['coverage']=={'original_criteria':36,'delayed_variants':36,'mechanism_commands':119,'supplement_commands':12,'isolated_packages':12,'export_commands':96},'Incomplete coverage')
 need(set(v['components'])=={'mechanisms','audit','exports'},'Missing separate mechanism')
 if files:pins(v['source_sha256'])
 data={}
 for name,r in v['components'].items():need(sha(ROOT/r['path'])==r['sha256'],'Changed receipt');data[name]=load(r['path'])
 m=data['mechanisms'];need(m['passed'] and m['error'] is None and len(m['rows'])==119 and len(m['checks'])==8 and all(x['passed'] for x in m['checks']),'Incomplete119 command matrix')
 if files:pins(m['source_sha256']);pins(m['artifacts'])
 rows={r['name']:r for r in m['rows']};need(len(rows)==119,'Duplicate command');need(all(s in rows for names,scope in EVIDENCE.values() for s in names.split()),'Missing exact criterion evidence')
 for row in rows.values():row_valid(row)
 base=(ROOT/v['components']['mechanisms']['path']).parent
 for directory in sorted(base.iterdir()):
  if not (directory/'report.json').exists() or directory.name=='retained-stage1':continue
  r=json.loads((directory/'report.json').read_text());need(r['physical_status']=='not-run','Physical overclaim')
  if files:
   pins(r['inputs']);pins({(directory/p).relative_to(ROOT).as_posix():h for p,h in r['artifacts'].items()})
  q=[c for c in r['commands'] if c['command'][0]=='qemu-system-arm' and '-M' in c['command']]
  if directory.name.startswith('missing-helper-'):need(not q and 'undefined reference' in r['error'],'Helper omission not detected at link');continue
  need(len(q)==1,'Missing actual emulator');q=q[0];command=q['command'];need('-icount' in command and 'shift=0,align=off,sleep=off' in command and command[command.index('-nic')+1]=='none','Wrong emulator profile')
  need(q['returncode'] in (0,1,2,3),'Deadline or crash credited as semantic detection')
  expected=rows[directory.name]['expected'];need((q['returncode']==0)==(expected==0),'Machine status disagrees')
  if expected==0:need(r['status']=='passed' and all(x['passed'] for x in r['checks']) and 'PASS MF ' in q['output'],'Missing target checks')
  need(any('-std=c17' in c['command'] and '-Werror' in c['command'] for c in r['commands']),'Wrong C profile')
  need(any('closed symbols'==x['name'] and x['passed'] for x in r['checks']),'Unresolved target symbols')
 for cc in ('gcc','clang'):
  exact={'bad-lma':'00000001','bad-bss':'00000003','bad-helper':'00000005','direct-r8':'0000002c','direct-return':'FAIL MF fault','preempt-r8':'00000086','preempt-exc':'00000072','preempt-s31':'0000009b','bad-mask':'000000a0','canary':'0000006a','stack-bound':'00000069','sleep-stopped':'000000d5'}
  for name,tag in exact.items():need(tag in row_valid(rows[name+'-'+cc]),'Wrong negative cause '+name)
  need('retained resources\n00000003\n00000003' in row_valid(rows['owner-fault-'+cc]),'Owner lost protected resources')
  need('00000082' in row_valid(rows['mpu-'+cc]),'No actual MPU violation')
  for profile in ('O0','O2'):
   log=row_valid(rows['preempt-fp-'+cc+'-'+profile]);basic,extended,before,after,msp=serial_block(log,'MF basic/extended/lazy-before/lazy-after/MSP-min',5)
   need(extended>0 and before==(profile=='O2') and after==0 and msp%8==0,'FP ownership/lazy mismatch')
   need(serial_block(log,'MF painted extension words before/after first FP instruction',2)==([18,1] if profile=='O2' else [1,1]),'Lost reserved/written distinction')
  uart=row_valid(rows['uart-borrowed-'+cc]);counts=serial_block(uart,'MF attempted/accepted/rejected/processed/delivered/output-drop/log-drop/max-ISR-batch',8)
  need(counts[0]==64 and counts[0]==counts[1]+counts[2] and counts[1]==counts[3] and counts[3]==counts[4]+counts[5] and counts[6]>0 and counts[7]<=4,'UART conservation/backpressure failed')
  sleep=row_valid(rows['sleep-'+cc+'-O2']);need(sleep.count('MF wake-boundary/elapsed/remaining/slept')==8 and 'MF split-three elapsed counts' in sleep,'Missing WFI boundary/split')
 a=data['audit'];need(a['passed'] and a['error'] is None and len(a['commands'])==12 and len(a['fixtures'])==12 and all(x['passed'] for x in a['checks']),'Incomplete fixture/stack/native supplement')
 if files:pins(a['source_sha256']);pins(a['artifacts'])
 for r in a['fixtures']:need(sha(ROOT/r['path'])==r['sha256'] and fixture(load(r['path']))==r['actual']==load(r['path'])['expected'],'Fixture oracle mismatch')
 need(all(c['passed'] and c['returncode']==0 for c in a['commands']),'Supplement native failed')
 for r in a['images']:
  need(sha(ROOT/r['report'])==r['report_sha256'],'Unbound resource image')
  if 'linked_call_path_bounds' in r:need(r['linked_call_path_bounds']['MSP_nested_conservative_bytes']<8192 and r['linked_call_path_bounds']['PSP_conservative_bytes']<4096,'Stack capacity unmet')
 e=data['exports'];need(e['passed'] and e['error'] is None and len(e['packages'])==12 and len(e['rows'])==96,'Incomplete isolated exports')
 if files:pins(e['source_sha256']);pins(e['artifacts'])
 for row in e['rows']:
  row_valid(row)
  if '-public-' in row['name']:need(row['expected']==1 and row['returncode']==1,'Learner completed silently')
 for pkg in e['packages']:
  need(not pkg['private_reference_in_public'],'Reference leaked')
  root=ROOT/pkg['public_root'];need(not any('assessor' in p.relative_to(root).parts or 'reference' in p.relative_to(root).parts for p in root.rglob('*')),'Private public path')
  if files:need(all(sha(root/p)==h for p,h in pkg['public_sha256'].items()),'Changed public package')
 for r in load('research/v5/expansion/legacy_mcu_followthrough/resources.json')['resources']:
  if files:pins(r['sha256'])
 registration=load('research/v5/expansion/legacy_mcu_followthrough/scaffold_registration.json')
 if files:need(sha(ROOT/registration['original_source'])==registration['original_sha256'],'Original learner mutated');pins(registration['generated'])
 return data
def main(report):
 v=load(CAMPAIGN);validate_campaign(v);checks=[]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 check('all-source-bound-mechanisms',True)
 for name,mutate in [('missing-variant',lambda x:x['coverage'].update(delayed_variants=35)),('external-promotion',lambda x:x.update(open_gates=[])),('changed-receipt',lambda x:x['components']['mechanisms'].update(sha256='0'*64)),('missing-target-campaign',lambda x:x['components'].pop('mechanisms')),('stale-source',lambda x:x['source_sha256'].update({'labs/mcu_followthrough/target/build.py':'0'*64}))]:
  changed=copy.deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 mapping=load('labs/mcu_followthrough/criteria.json')['criteria'];support=load(SUPPORT);expected=set()
 for r in records():
  p='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+p],cwd=ROOT,text=True));stripped=copy.deepcopy(r['criteria'])
  for c in stripped:c.pop('successor_support',None)
  check(r['project']+' original criteria',stripped==old['criteria']);check(r['project']+' frozen historical bindings',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(r['project']+' additive idempotence',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(x['id'] for x in c['variants']);row=next(x for x in mapping if x['id']==c['id'])
   check(c['id']+' exact task/variant',row['original_task']==c['task'] and row['original_expected']==c['verification']['expected'] and row['changed_conditions']==c['variants'] and row['mechanism_checks']==EVIDENCE[c['objective'][4:]][0].split())
 check('72 exact identities',len(support['updates'])==72 and {x['id'] for x in support['updates']}==expected)
 pins(support['source_sha256']);check('support current source identities',True)
 check('independent gates retained',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state'].startswith('not-run') for x in support['updates']))
 check('author work accounted',support['remaining_author_work']==[])
 from cmastery import assessments
 with tempfile.TemporaryDirectory(prefix='mcu-canonical-export-') as temp:
  for r in records():
   target=Path(temp)/r['project'];assessments.export(ROOT,r['project'],target)
   check(r['project']+' canonical export',all((target/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(target).parts for p in target.rglob('*')))
 result={'passed':all(x['passed'] for x in checks),'checks':checks,'source_sha256':{p:sha(ROOT/p) for p in (CAMPAIGN,SUPPORT,'tools/check_mcu_followthrough.py')},'scope':'72 original IDs, exact mechanism and changed-condition evidence; no external/physical promotion.'}
 destination=(ROOT/report).resolve();need(destination.is_relative_to(ROOT/'reviews/v5/expansion/legacy_mcu_followthrough'),'Report outside owned path');need(not destination.exists(),'Preserve old guard receipt');destination.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[x['name'] for x in checks if not x['passed']]}));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
