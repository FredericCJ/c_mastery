"""Validate exact T19 successor evidence without converting it into learner mastery."""
from copy import deepcopy
from pathlib import Path
import argparse,json,itertools
from common import ROOT,read_json,dump,sha
MODULES={m:{'T19.'+f'{n:02}.'+letter for n in numbers for letter in 'abc'} for m,numbers in {'session':[1,2],'registry':[3],'generic':[4,5],'reducer':[6],'traversal':[7],'handles':[9],'dispatch':[10]}.items()}
NEGATIVES={'session':['session-overflow','session-publication','session-learner'],'registry':['registry-early','registry-leak','registry-learner'],'generic':['generic-leak','generic-range','generic-learner'],'reducer':['reducer-replay','reducer-retry','reducer-learner'],'traversal':['traversal-lifetime','traversal-stop','traversal-cancel','traversal-learner'],'handles':['handles-wrap','handles-quiescence','handles-resurrection','handles-learner'],'dispatch':['dispatch-table','dispatch-bytes','dispatch-learner','backend-learner']}
MARKERS={'session':'SESSION-PASS:state_events=384;constructor_callbacks=2;','registry':'REGISTRY-PASS:histories=10;terminal_free=1;capacity=4;policy_count=2','generic':'GENERIC-PASS:forms=3;capacity=4;staged_rollback=1;transfer_modes=2;tokens=10;fields=4;','reducer':'REDUCER-PASS:cells=256;effect_histories=3;hot_backends=3;','traversal':'TRAVERSAL-PASS:pull_push_pairs=50;tree_bounds=32;depth=3;','handles':'HANDLES-PASS:epochs=3;identities=27;','dispatch':'DISPATCH-PASS:conformance=455;fault_paths=12;third_backend=all;'}
def validate(d):
 errors=[];selected=d.get('selected_modules',[])
 if not selected or len(set(selected))!=len(selected) or not set(selected)<=MODULES.keys():return ['Invalid module scope']
 if d.get('passed') is not True or d.get('error') is not None:errors.append('Failed campaign')
 if d.get('topic_complete') is not False:errors.append('Inflated mastery claim')
 expected={(x,v) for m in selected for x in MODULES[m] for v in (False,True)};obs=d.get('observations',[])
 if len(obs)!=len(expected) or {(r.get('objective'),r.get('variant')) for r in obs}!=expected:errors.append('Missing exact ordinary/changed observation')
 if not d.get('input_sha256'):errors.append('Missing input pins')
 for name,h in d.get('input_sha256',{}).items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=h:errors.append('Stale input: '+name)
 artifacts=d.get('retained_artifacts',{})
 if not artifacts:errors.append('Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to((ROOT/'build/legacy_abstractions').resolve()) or not p.is_file() or sha(p)!=row.get('sha256') or p.stat().st_size!=row.get('bytes'):errors.append('Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 if not commands or not checks or any(r.get('passed') is not True for r in checks):errors.append('Missing/failed checks')
 for r in commands:
  rc=r.get('exit');wanted=r.get('expected');ok=isinstance(rc,int) and rc>0 if wanted=='nonzero' else isinstance(rc,int) and rc==wanted
  if not ok or r.get('passed') is not True or r.get('diagnostic') and r['diagnostic'] not in r.get('stdout','')+r.get('stderr',''):errors.append('Inconsistent command')
 def runs(name):return [r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name]
 def good(name,code=0,marker=None):
  rows=runs(name);return len(rows)==1 and rows[0]['exit']==code and (marker is None or marker in rows[0]['stdout']+rows[0]['stderr'])
 for m in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   if not good(cc+'-'+profile+'-'+m,0,MARKERS[m]):errors.append('Missing actual profile: '+cc+'-'+profile+'-'+m)
  for name in NEGATIVES[m]:
   if not good(name,1,m.upper()+'-CHECK:'):errors.append('Missing semantic rejection: '+name)
 names={r.get('name') for r in checks}
 required={'session':['actual T18 resource ledger imported'],'registry':['actual registry provider imported'],'reducer':['pure dispatch relation and effect identity bounded'],'traversal':['actual traversal stack use records'],'dispatch':['full build/workload matrix']}
 for m in selected:
  for name in required.get(m,[]):
   if name not in names:errors.append('Missing provider/bound check: '+name)
 if 'generic' in selected:
  for cc in ('gcc','clang'):
   rows=[r for r in checks if r.get('name')=='independent decimal grammar oracle '+cc]
   if len(rows)!=1 or rows[0]['observed'].get('cases')!=66388:errors.append('Missing decimal oracle bound')
  forms=[r for r in checks if r.get('name','').startswith('actual generic form ')]
  if len(forms)!=32:errors.append('Missing generated representation builds')
  rejected=[r for r in commands if r.get('expected')=='nonzero' and any(x in r.get('stderr','') for x in ('compatible','member','AV_OWNED'))]
  if len(rejected)<6:errors.append('Missing type/enum negative translations')
 if 'handles' in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   if not good(cc+'-'+profile+'-threads',0,'HANDLE-THREAD-PASS:promotion_races=256;'):errors.append('Missing threaded lifetime profile')
  if not good('clang-tsan-handles-racy',66,'WARNING: ThreadSanitizer: data race') or not good('clang-tsan-handles-repaired',0,'HANDLE-THREAD-PASS:'):errors.append('Missing intended TSan race/repair')
 if 'dispatch' in selected:
  paths=[p for p in artifacts if p.endswith('/dispatch_measurements.json')]
  if len(paths)!=1:errors.append('Missing raw dispatch measurements')
  else:
   report=read_json(ROOT/paths[0]);rows=report.get('rows',[]);code=report.get('code',[])
   builds={cc+opt+('-lto' if lto else '')+('-shared' if shared else '-static') for cc,opt,lto,shared in itertools.product(('gcc','clang'),('-O0','-O2'),(False,True),(False,True))};expected={(b,f,m,n) for b in builds for f,m in [(0,0),(1,0),(2,0),(0,1),(1,1),(0,2),(1,2)] for n in (0,64)}
   if len(rows)!=224 or {(r['build'],r['form'],r['mix'],r['payload']) for r in rows}!=expected or len(code)!=16:errors.append('Incomplete dispatch cost matrix')
   if any(len(r.get('samples_ns',[]))!=9 or any(t<=0 for t in r['samples_ns']) or r.get('rounds')!=3000 or r.get('warmups')!=1 or r.get('target_allocations')!=0 for r in rows):errors.append('Invalid raw dispatch bounds')
   matrix=[r for r in checks if r.get('name')=='full build/workload matrix']
   if len(matrix)!=1 or matrix[0].get('observed')!=224:errors.append('Weakened dispatch matrix bound')
   if len([r for r in checks if r.get('name','').startswith('actual shared backend NEEDED ')])!=8:errors.append('Missing actual shared loading')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_abstractions/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing criterion',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated mastery',lambda x:x.update(topic_complete=True),'Inflated mastery claim'),('lost binaries',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts')]
  for m in d['selected_modules']:
   name=NEGATIVES[m][0];cases.append((m+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name==n],'Missing semantic rejection: '+name))
  if 'generic' in d['selected_modules']:cases.append(('shallow numeric oracle',lambda x:[r['observed'].update(cases=10) for r in x['checks'] if r['name'].startswith('independent decimal grammar oracle')],'Missing decimal oracle bound'))
  if 'handles' in d['selected_modules']:cases.append(('unrelated TSan failure',lambda x:[r.update(stdout='',stderr='runtime unavailable',diagnostic=None) for r in x['commands'] if Path(r['argv'][0]).name=='clang-tsan-handles-racy'],'Missing intended TSan race/repair'))
  if 'dispatch' in d['selected_modules']:cases.append(('incomplete cost scope',lambda x:[r.update(observed=1) for r in x['checks'] if r['name']=='full build/workload matrix'],'Weakened dispatch matrix bound'))
  for title,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),title;controls.append(title+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_abstractions_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
