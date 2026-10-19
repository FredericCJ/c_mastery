"""Validate source-bound partial closures without conflating model/OS observations."""
import argparse,copy,json,pathlib
from common import ROOT,sha
from register_source_reconciliation import BASE,PRIOR,SOURCES,read
from check_posix_edition import validate as posix
SUPPORT=BASE+'/completion_dispositions.json'
IDS=set(PRIOR)|{'G-DG-EXACT-RUNTIME-SOURCE','LIO-POSIX2024-CLOSE','G-SOURCE-POSIX2024-RDLOCK','SE-POSIX-CLOSE-01','SE-POSIX-RDLOCK-01'}
def validate(d,files=True):
 errors=[]
 def need(ok,s):
  if not ok:errors.append(s)
 rows=d.get('updates',[]);need(len(rows)==10 and {r['id'] for r in rows}==IDS,'exact10 identities')
 need(bool(d.get('source_sha256')),'source pins')
 for r in rows:
  need(r['dimensions']['external_review']['state']=='not-run','no external review '+r['id'])
  need(r['dimensions']['independent_learner']['state']=='not-assessed','no learner evidence '+r['id'])
  need(r['dimensions']['physical_observation']['state']=='not-applicable-to-source-or-pure-model','no physical claim '+r['id'])
  need(bool(r.get('evidence_limits')),'limits '+r['id'])
  if r['id'] in PRIOR:need({s['id'] for s in r.get('source_records',[])}==set(PRIOR[r['id']][1]),'exact sources '+r['id']);need(PRIOR[r['id']][2] in r['evidence_limits'],'bounded parent scope '+r['id'])
 if files:
  for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'current '+p)
  errors.extend(posix(read(BASE+'/posix_validation.json')))
  exports=read(BASE+'/exports.json');need(exports.get('passed') is True and len(exports.get('commands',[]))==4 and exports.get('private_sources_copied') is False,'four separated public-export commands')
  for field in ['source_sha256','export_files']:
   for p,h in exports[field].items():need(sha(ROOT/p)==h,'current export '+p)
  for r in exports['commands']:need(r['passed'] and r['exit']==r['expected'] and r['marker'] in r['stdout'],'observed export '+str(r['command']))
  from check_systems_abi import validate as abi
  from check_systems_elf import validate as elf
  for f,v in [('legacy_systems_abi',abi),('legacy_systems_elf',elf)]:errors.extend(v(read('reviews/v5/expansion/'+f+'/full_validation.json')))
  for r in read(BASE+'/resources.json')['resources']:
   p=r['inspection']['capture'];need(sha(ROOT/p['path'])==p['sha256'],'inspected capture '+r['id']);need(bool(r['locators']),'actual locators '+r['id'])
  k=read('research/v5/expansion/legacy_advanced_diagnostic/kernel_source_identity.json')
  for key in ['package','archive']:need(pathlib.Path(k[key]['path']).is_file() and sha(pathlib.Path(k[key]['path']))==k[key]['sha256'],'exact kernel '+key)
  for r in k['comparisons']:need(sha(pathlib.Path(r['inspected']))==r['inspected_sha256']==r['package_sha256'],'selected kernel source '+r['member'])
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/source_reconciliation/package_guard.json');a=p.parse_args();d=read(SUPPORT);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for name in ['missing-source-child','lost-pins','lost-source','overstated-parent','false-review','false-learner','false-physical','lost-limit']:
   b=copy.deepcopy(d)
   if name=='missing-source-child':b['updates'].pop()
   elif name=='lost-pins':b['source_sha256']={}
   elif name=='lost-source':next(r for r in b['updates'] if r['id'] in PRIOR)['source_records']=[]
   elif name=='overstated-parent':next(r for r in b['updates'] if r['id'] in PRIOR)['evidence_limits']=['Full current ABI conformance']
   elif name=='lost-limit':b['updates'][0]['evidence_limits']=[]
   else:b['updates'][0]['dimensions'][{'false-review':'external_review','false-learner':'independent_learner','false-physical':'physical_observation'}[name]]['state']='passed'
   rejected=bool(validate(b,False));controls.append(dict(name=name,rejected=rejected))
   if not rejected:errors.append('accepted '+name)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={SUPPORT:sha(ROOT/SUPPORT)});dest=ROOT/a.report;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
