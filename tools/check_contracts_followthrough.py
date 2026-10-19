"""Require exact T18 ordinary/changed evidence, real profiles and semantic controls."""
from copy import deepcopy
from pathlib import Path
import argparse,json
from common import ROOT,read_json,dump,sha
MODULES={
 'bounds':{'T18.'+x for x in ('01.a','01.c','02.a','02.b','07.a','07.b','07.c','09.a','09.b','09.c')},
 'ownership':{'T18.'+x for x in ('01.b','02.c','03.a','03.b','03.c','04.b','04.c','05.a','05.b','05.c')},
 'representation':{'T18.06.'+x for x in 'abc'},
 'async':{'T18.'+x for x in ('04.a','08.a','08.b','08.c','10.a','10.b','10.c')}}
def validate(d):
 errors=[];selected=d.get('selected_modules',[])
 if len(selected)!=len(set(selected)) or not selected or not set(selected)<=MODULES.keys():return ['Unknown/empty/duplicate scope']
 if d.get('passed') is not True or d.get('error') is not None:errors.append('Campaign failed')
 if d.get('topic_complete') is not False:errors.append('Inflated mastery/completion claim')
 expected={(x,v) for m in selected for x in MODULES[m] for v in (False,True)}
 obs=d.get('observations',[])
 if len(obs)!=len(expected) or {(x.get('objective'),x.get('variant')) for x in obs}!=expected:errors.append('Missing exact ordinary/changed observation')
 if not d.get('input_sha256'):errors.append('Missing source pins')
 for path,h in d.get('input_sha256',{}).items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=h:errors.append('Stale input: '+path)
 if not d.get('retained_artifacts'):errors.append('Missing retained artifacts')
 for path,r in d.get('retained_artifacts',{}).items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to((ROOT/'build/legacy_contracts').resolve()) or not p.is_file() or p.stat().st_size!=r.get('bytes') or sha(p)!=r.get('sha256'):errors.append('Changed artifact: '+path)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 if not commands or not checks or any(r.get('passed') is not True for r in checks):errors.append('Missing/failed checks')
 for r in commands:
  rc=r.get('exit');expected_rc=r.get('expected');valid=isinstance(rc,int) and rc>0 if expected_rc=='nonzero' else rc==expected_rc
  if not valid or r.get('passed') is not True or r.get('diagnostic') and r['diagnostic'] not in r.get('stdout','')+r.get('stderr',''):errors.append('Inconsistent command')
 def runs(name):return [r for r in commands if r.get('argv') and r['argv'][0].split('/')[-1]==name]
 def good(name,code=0,marker=None):
  rows=runs(name)
  return len(rows)==1 and rows[0]['exit']==code and (marker is None or marker in rows[0]['stdout']+rows[0]['stderr'])
 for m in selected:
  for cc in ('gcc','clang'):
   for profile in ('debug','release','sanitize'):
    suffixes=['-instrumented-bounds','-silent-bounds'] if m=='bounds' else ['-'+m]
    for suffix in suffixes:
     name=cc+'-'+profile+suffix
     if not good(name,0,m.upper()+'-PASS:'):errors.append('Missing actual profile: '+name)
  negatives={'bounds':['bounds-wrap','bounds-sentinel','bounds-assert','bounds-learner'],'ownership':['ownership-release','ownership-close','ownership-generation','ownership-learner'],'representation':['representation-endian','representation-learner','opaque-output','opaque-learner'],'async':['async-request-is-ack','async-failure-output','async-learner','async-v1-learner','async-descendant-drop','async-parent-early-drop']}[m]
  for name in negatives:
   if not good(name,5 if name=='opaque-output' else 1,None if name=='opaque-learner' else m.upper()+'-CHECK:'):errors.append('Missing semantic rejection: '+name)
 names={r.get('name') for r in checks}
 if 'bounds' in selected:
  for cc in ('gcc','clang'):
   rows=[r for r in checks if r.get('name')=='independent absolute-time oracle '+cc]
   if len(rows)!=1 or rows[0]['observed'].get('cases')!=65536:errors.append('Missing independent deadline bound')
  if 'actual original append provider imported' not in names:errors.append('Missing append prerequisite')
  service=runs('bounds-service')
  if {(r['argv'][-1],r['exit']) for r in service}!={('capacity',0),('external',0),('internal',70)}:errors.append('Missing isolated service distinction')
 if 'ownership' in selected:
  rows=[r for r in checks if r.get('name','').startswith('actual ownership bounds ')]
  if len(rows)!=6 or any(any(r['observed'].get(k)!=v for k,v in {'constructor_paths':10,'lease_cases':32768,'generation_allocations':6,'state_operations':24,'resource_kinds':4}.items()) for r in rows):errors.append('Missing ownership state bounds')
  if 'actual original view provider imported' not in names:errors.append('Missing view prerequisite')
 if 'representation' in selected:
  if not {'actual generated typed-storage contract','actual typed-storage final target closure empty'}<=names:errors.append('Missing representation target audit')
  wrong=[r for r in commands if r.get('expected')=='nonzero' and 'storage contract' in r.get('stderr','')]
  if len(wrong)!=3 or not any('stale version' in r['stderr'] for r in wrong):errors.append('Missing storage translation rejections')
  for cc in ('gcc','clang'):
   for ver in ('version0','version1','relocated'):
    rows=[r for r in runs(ver) if ('/'+cc+'-consumer/') in r['argv'][0]]
    if len(rows)!=1 or rows[0]['exit']!=0:errors.append('Missing opaque external consumer')
 if 'async' in selected:
  oracles=[r for r in commands if len(r.get('argv',[]))>2 and r['argv'][1].endswith('/compare_async.py')]
  positives=[r for r in oracles if r['expected']==0];negative=[r for r in oracles if r['expected']==1]
  if len(positives)!=6 or any(r['exit']!=0 or 'ASYNC-ORACLE 3906 3906 0' not in r['stdout'] for r in positives):errors.append('Missing independent async traces')
  if len(negative)!=1 or negative[0]['exit']!=1:errors.append('Missing rollback counterexample')
  for r in oracles:
   artifact=next((p for p in d['retained_artifacts'] if p.endswith('/'+r['argv'][-1].split('/')[-1])),None)
   if not artifact:errors.append('Missing raw async oracle');continue
   raw=read_json(ROOT/artifact)
   if raw.get('exit')!=0 or raw.get('cases')!=3906 or raw.get('observed')!=3906 or bool(raw.get('mismatch_indices'))!=(r['expected']==1):errors.append('Invalid async semantic comparison')
  for cc in ('gcc','clang'):
   for version in ('v1','v2','relocated'):
    if not good(cc+'-consumer-'+version):errors.append('Missing old API consumer')
  if not {'whole reachable transform+recorder image has empty runtime closure','actual stack usage files'}<=names:errors.append('Missing transitive effect target audit')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_contracts/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('criterion absent',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated mastery',lambda x:x.update(topic_complete=True),'Inflated mastery/completion claim'),('artifacts absent',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts')]
  if 'bounds' in d['selected_modules']:cases.append(('weaker deadline oracle',lambda x:[r['observed'].update(cases=1) for r in x['checks'] if r['name'].startswith('independent absolute-time oracle')],'Missing independent deadline bound'))
  if 'ownership' in d['selected_modules']:cases.append(('weaker lease oracle',lambda x:[r['observed'].update(lease_cases=1) for r in x['checks'] if r['name'].startswith('actual ownership bounds')],'Missing ownership state bounds'))
  if 'representation' in d['selected_modules']:cases.append(('unrelated translation failure',lambda x:[r.update(stderr='compiler missing') for r in x['commands'] if r['expected']=='nonzero'],'Missing storage translation rejections'))
  if 'async' in d['selected_modules']:cases.append(('missing async profile',lambda x:x.update(commands=[r for r in x['commands'] if not (len(r['argv'])>2 and r['argv'][1].endswith('/compare_async.py') and r['expected']==0)]),'Missing independent async traces'))
  for module in d['selected_modules']:
   name={'bounds':'bounds-wrap','ownership':'ownership-release','representation':'representation-endian','async':'async-request-is-ack'}[module]
   cases.append((module+' crash treated as semantic control',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if r['argv'][0].endswith('/'+n)],'Missing semantic rejection: '+name))
  for title,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),title;controls.append(title+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale input rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_contracts_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
