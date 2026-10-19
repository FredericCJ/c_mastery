"""Validate the exact T21 failure contracts and actual scoped campaign evidence."""
from copy import deepcopy
from pathlib import Path
import argparse,itertools,json,re
from common import ROOT,read_json,dump,sha
MODULES={m:{f'T21.{n:02}.{letter}' for n in numbers for letter in 'abc'} for m,numbers in {'taxonomy':[1,2],'construction':[3,4],'transaction':[5],'cancellation':[6],'observability':[7,8,9],'crash':[10],'target':[]}.items()}
NEGATIVES={'taxonomy':['taxonomy-primary','taxonomy-ack','taxonomy-learner','taxonomy-abi-learner'],'construction':['construction-missed','construction-dependency','construction-learner'],'transaction':['mixed','replay','unpin','transaction-stub','publication-stub'],'cancellation':['early','stale','cancel-stub'],'observability':['fallback','observability-stub','disabled-eval','diagnostics-stub'],'crash':['checksum','crash-stub']}
MARKERS={'taxonomy':'TAXONOMY-PASS:categories=4;','construction':'CONSTRUCTION-PASS:cases=16896;','transaction':'TRANSACTION-PASS:cases=528;','cancellation':'CANCEL-PASS:histories=5461;','observability':'OBSERVABILITY-PASS:codec_cases=240;clock_pairs=65536;','crash':'CRASH-PASS:slot_cuts=33;'}
def validate(d):
 errors=[];selected=d.get('selected_modules',[])
 if not selected or len(set(selected))!=len(selected) or not set(selected)<=MODULES.keys():return ['Invalid module scope']
 if not d.get('passed') or d.get('error') is not None:errors.append('Failed campaign')
 if d.get('topic_complete') is not False:errors.append('Inflated mastery claim')
 expected={(o,v) for m in selected for o in MODULES[m] for v in (False,True)};obs=d.get('observations',[])
 if len(obs)!=len(expected) or {(r.get('objective'),r.get('variant')) for r in obs}!=expected:errors.append('Missing exact ordinary/changed observation')
 if not d.get('input_sha256'):errors.append('Missing source pins')
 for name,h in d.get('input_sha256',{}).items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=h:errors.append('Stale input: '+name)
 if not d.get('retained_artifacts'):errors.append('Missing retained artifacts')
 for name,row in d.get('retained_artifacts',{}).items():
  p=(ROOT/name).resolve()
  if not p.is_relative_to((ROOT/'build/legacy_failure').resolve()) or not p.is_file() or sha(p)!=row.get('sha256') or p.stat().st_size!=row.get('bytes'):errors.append('Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 if not commands or not checks or any(r.get('passed') is not True for r in checks):errors.append('Missing/failed checks')
 for r in commands:
  rc=r.get('exit');wanted=r.get('expected');ok=isinstance(rc,int) and rc!=0 if wanted=='nonzero' else isinstance(rc,int) and rc==wanted
  if not ok or r.get('passed') is not True or (r.get('diagnostic') and r['diagnostic'] not in r.get('stdout','')+r.get('stderr','')):errors.append('Inconsistent command')
 def runs(name):return [r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name]
 def good(name,code=0,marker=''):
  rows=runs(name);return len(rows)==1 and rows[0]['exit']==code and marker in rows[0].get('stdout','')+rows[0].get('stderr','')
 for m in selected:
  if m=='target':continue
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   name=cc+'-'+profile+('-cancel-core' if m=='cancellation' else '-'+m)
   if not good(name,0,MARKERS[m]):errors.append('Missing actual profile: '+name)
  for name in NEGATIVES[m]:
   category='PUBLICATION' if name in ('unpin','publication-stub') else 'DIAGNOSTICS' if name in ('disabled-eval','diagnostics-stub') else 'CANCEL' if m=='cancellation' else m.upper()
   if not good(name,1,category+'-CHECK'):errors.append('Missing semantic rejection: '+name)
 names={r.get('name') for r in checks}
 for module,required in {'taxonomy':['actual old public consumers with copied diagnostic snapshot'],'construction':['actual T18 resource ownership provider imported'],'cancellation':['Actual FAsync producer functions imported'],'target':['all named bounded failure-core API roots selected']}.items():
  if module in selected and not set(required)<=names:errors.append('Missing import/bound: '+module)
 if 'transaction' in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   if not good(cc+'-'+profile+'-publication',0,'PUBLICATION-PASS:replacements=1000;'):errors.append('Missing concurrent publication profile')
  if not good('publication-tsan',0,'PUBLICATION-PASS:'):errors.append('Missing actual publication TSan')
 if 'cancellation' in selected:
  for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
   if not good(cc+'-'+profile+'-cancel-threads',0,'CANCEL-THREAD-PASS:races=256;'):errors.append('Missing threaded cancellation profile')
  if not good('cancel-tsan',0,'CANCEL-THREAD-PASS:'):errors.append('Missing actual cancellation TSan')
 if 'observability' in selected:
  for cc,dev,integ,opt in itertools.product(('gcc','clang'),(0,1),(0,1),('-O0','-O2')):
   if not good(cc+'-diagnostics-'+str(dev)+str(integ)+opt,0,'DIAGNOSTICS-PASS:'):errors.append('Missing feature matrix')
  for cc in ('gcc','clang'):
   reader=runs(cc+'-diagnostic-reader')
   if len(reader)!=22 or sum(r['exit']==7 for r in reader)!=20:errors.append('Missing actual diagnostic process boundary')
   if not {'actual assert preprocessing work difference '+cc+' False','actual assert preprocessing work difference '+cc+' True','disabled optional expression absent '+cc}<=names:errors.append('Missing actual preprocessing')
   for take in (False,True):
    if 'address-taking changes retained function but not compiled-out buffer '+cc+' '+str(take) not in names:errors.append('Missing address-taking experiment')
 if 'crash' in selected:
  for cc in ('gcc','clang'):
   if not {'actual differing ELF build identities '+cc,'matching-build admission '+cc+'4','matching-build admission '+cc+'5','relocated PIE normalized address remains same '+cc}<=names:errors.append('Missing matching artifact symbolization')
   negatives=[r for r in checks if r.get('name')=='matching-build admission '+cc+'5']
   if len(negatives)!=1 or negatives[0]['observed'].get('symbolizer_called') is not False:errors.append('Stale artifact symbolized')
 if 'target' in selected:
  header_names={name for h in ['taxonomy','construction','transaction','cancellation','observability','diagnostics','crash','public_error'] for name in re.findall(r'\b(ae_[a-z_]+)\s*\(', (ROOT/'labs/api_depth/failure'/(h+'.h')).read_text())}
  roots=[r for r in checks if r.get('name')=='all named bounded failure-core API roots selected']
  if len(roots)!=1 or set(roots[0].get('observed',[]))!=header_names:errors.append('Incomplete target root set')
  for target in ('m4-O0','m4-O2','x86_64-O2'):
   if not {'actual failure-core final closure empty '+target,'all selected roots survive linked image '+target,'actual failure-core stack records '+target}<=names:errors.append('Missing actual target closure')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_failure/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing criterion',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated mastery',lambda x:x.update(topic_complete=True),'Inflated mastery claim'),('lost binaries',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts')]
  for m in d['selected_modules']:
   if m not in NEGATIVES:continue
   name=NEGATIVES[m][0];cases.append((m+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name==n],'Missing semantic rejection: '+name))
  if 'transaction' in d['selected_modules']:cases.append(('missing threaded publication',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='publication-tsan']),'Missing actual publication TSan'))
  if 'observability' in d['selected_modules']:cases.append(('weakened feature matrix',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='gcc-diagnostics-11-O2']),'Missing feature matrix'))
  if 'crash' in d['selected_modules']:cases.append(('stale artifact admitted',lambda x:[r['observed'].update(symbolizer_called=True) for r in x['checks'] if r.get('name')=='matching-build admission gcc5'],'Stale artifact symbolized'))
  if 'target' in d['selected_modules']:cases.append(('missing generated image closure',lambda x:x.update(checks=[r for r in x['checks'] if not r.get('name','').startswith('actual failure-core final closure empty')]),'Missing actual target closure'))
  for title,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),title;controls.append(title+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_failure_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
