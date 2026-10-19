"""Reject missing backends, vacuous models and inflated completion-I/O evidence."""
from copy import deepcopy
from functools import lru_cache
from pathlib import Path
import argparse,hashlib,importlib.util,itertools,json,os,re
from common import ROOT,read_json,dump,sha
OBJECTIVES={'T24.12.'+x for x in 'abc'}
PUBLIC=['labs/hosted_systems/completion_depth/'+n for n in ('completion.h','completion.c','tests.c','native.c','workload.c','stall.c','run.py','seal.py','CONTRACTS.md','COMPLETION.md')]+['labs/linux_io_depth/targets/'+n for n in ('raw_ring.h','raw_ring.c')]
PRIVATE=['completion.c','native.c','order_trace.c','oracle.py','stall.c','workload.c','environment.py','campaign.py','ANALYSIS.md']
spec=importlib.util.spec_from_file_location('completion_guard_oracle',ROOT/'assessor/hosted_systems/completion_depth/oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
@lru_cache(None)
def check_workload(text):return oracle.check_workload(text)
@lru_cache(None)
def check_order(text):return oracle.check_order(text)
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True and d.get('error') is None,'Failed campaign');need(d.get('topic_complete') is False,'Inflated topic claim')
 obs=d.get('observations',[]);need(len(obs)==6 and {(r.get('objective'),r.get('variant')) for r in obs}==set(itertools.product(OBJECTIVES,(False,True))),'Missing exact ordinary/changed observations')
 mappings={'T24.12.a':['native:cancel','native:boundary','native:failure','oracle:event-sets4320','import:T62'],'T24.12.b':['workload:192cells','stall:both-backends','native:cancel','ANALYSIS.md:resource-bound'],'T24.12.c':['prediction_seal.json','workload:192cells','oracle:independent-bytes','native:failure']}
 need(all(r.get('evidence')==mappings.get(r.get('objective')) for r in obs),'Incorrect criterion evidence map')
 required=['tools/run_completion_followthrough.py','assessor/legacy_systems/toolchain/common.py','assessor/linux_io_depth/targets/raw_ring.c','research/v5/b06_resources.json',*PUBLIC]+['assessor/hosted_systems/completion_depth/'+n for n in PRIVATE]+['research/v5/expansion/legacy_completion_io/'+n for n in ('resources.json','research_gate.json','source_access.json','author_support.py','prediction.json')]
 pins=d.get('input_sha256',{});need(set(required)<=pins.keys(),'Missing source/provider pins')
 for name,h in pins.items():
  p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()) and p.is_file() and sha(p)==h,'Stale source: '+name)
 artifacts=d.get('retained_artifacts',{});need(bool(artifacts),'Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve();need(p.is_relative_to((ROOT/'build/legacy_completion_io').resolve()) and p.is_file() and sha(p)==row.get('sha256') and p.stat().st_size==row.get('bytes'),'Stale artifact: '+name)
 need(sum(name.endswith('.su') for name in artifacts)>=10,'Missing actual stack-usage artifacts')
 for cc in ('gcc','clang'):
  usage=[p for p in artifacts if p.endswith('/'+cc+'-workload-stack.su')];need(len(usage)==1 and 'main' in (ROOT/usage[0]).read_text(),'Missing workload main stack audit '+cc)
 commands=d.get('commands',[]);checks=d.get('checks',[]);need(len(commands)==298 and all(r.get('passed') is True for r in commands),'Exact298 passing commands');need(bool(checks) and all(r.get('passed') is True for r in checks),'Missing/failed checks')
 for r in commands:need(r.get('exit')==r.get('expected') and (not r.get('diagnostic') or r['diagnostic'] in r.get('stdout','')+r.get('stderr','')),'Inconsistent command result')
 envs=[r.get('observed',{}) for r in checks if r.get('name')=='actual guest ABI identity'];need(len(envs)==1,'Missing installed ABI identity')
 for e in envs:
  need(e.get('header_query_exit')==0 and e.get('physical_measurement') is False and e.get('backend')=='actual raw T62 io_uring syscalls; no liburing or fallback','Inflated backend evidence')
  path=Path('//wsl.localhost/Ubuntu/usr/include/linux/io_uring.h') if os.name=='nt' else Path('/usr/include/linux/io_uring.h');need(path.is_file() and sha(path)==e.get('header_sha256'),'Stale installed header')
 def rows(name):return [r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name]
 def compiled(r,sources=(),options=()):
  builds=[b for b in commands if Path(b['argv'][0]).name in ('gcc','clang') and '-o' in b['argv'] and b['argv'][b['argv'].index('-o')+1]==r['argv'][0] and b.get('exit')==0]
  if len(builds)!=1 or commands.index(builds[0])>=commands.index(r):return False
  argv=builds[0]['argv'];return all(any(a.replace('\\','/').endswith('/'+s) for a in argv) for s in sources) and all(o in argv for o in options)
 def good(name,code=0,marker='',sources=(),options=()):
  rs=rows(name);return len(rs)==1 and rs[0].get('exit')==code and marker in rs[0].get('stdout','')+rs[0].get('stderr','') and compiled(rs[0],sources,options)
 for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
  prefix=cc+'-'+profile;options=['-std=c17']+(['-O0'] if profile=='debug' else ['-O2','-DNDEBUG'] if profile=='release' else ['-O1','-fsanitize=address,undefined'])
  need(good(prefix+'-core',marker='COMPLETION-PASS:cancel_target_orders=6;',sources=['labs/hosted_systems/completion_depth/tests.c','assessor/hosted_systems/completion_depth/completion.c'],options=options),'Missing core/profile '+prefix)
  rr=rows(prefix+'-orders');need(len(rr)==1 and compiled(rr[0],['assessor/hosted_systems/completion_depth/order_trace.c','assessor/hosted_systems/completion_depth/completion.c'],options),'Missing event model execution '+prefix)
  if len(rr)==1:
   try:check_order(rr[0]['stdout'])
   except (AssertionError,ValueError,KeyError):need(False,'Incorrect event-set output '+prefix)
  rr=rows(prefix+'-native');need(len(rr)==3 and {r['argv'][-1] for r in rr}=={'cancel','boundary','failure'},'Missing actual native mode '+prefix)
  for r in rr:
   output=r.get('stdout','');mode=r['argv'][-1];need(r.get('exit')==0 and compiled(r,['assessor/hosted_systems/completion_depth/native.c','assessor/hosted_systems/completion_depth/completion.c','assessor/linux_io_depth/targets/raw_ring.c'],options),'Wrong native/provider '+prefix)
   need('COMPLETION-PROBE:READ=1;READ_FIXED=1;ASYNC_CANCEL=1;' in output,'Missing actual operation probe '+prefix)
   if mode=='cancel':need(all('COMPLETION-CANCEL:fixed='+str(i)+';' in output for i in (0,1)) and output.count('old_cancel=-ENOENT;dispatch_delayed=1;')==2 and len(re.findall(r'latency_ns=[1-9][0-9]*;',output))==2,'Missing ordinary/fixed cancellation '+prefix)
   elif mode=='boundary':need(all('COMPLETION-BOUNDARY:fixed='+str(i)+';short=3;EOF=0;bad_fd=-EBADF;subrange=17;range_rejected='+str(i)+';' in output for i in (0,1)),'Missing operation boundaries '+prefix)
   else:need('COMPLETION-FAILURE:acquisition_cuts=5;normal_cleanup=1;registered_release_before_unmap=1;' in output,'Missing acquisition cleanup '+prefix)
  rr=rows(prefix+'-stall');need(good(prefix+'-stall',sources=['assessor/hosted_systems/completion_depth/stall.c'],options=options),'Missing stalled receiver '+prefix)
  if len(rr)==1:
   found=re.findall(r'STALL-PASS:backend=(copy|sendfile);sent=(\d+);consumed=(\d+);would_block=(\d+);partial=(\d+);cancel_to_drained_ns=(\d+);source_unchanged_until_drained=1;',rr[0].get('stdout',''))
   need(len(found)==2 and {x[0] for x in found}=={'copy','sendfile'} and all(0<int(x[1])==int(x[2])<1048576 and int(x[3])==1 and int(x[5])>0 for x in found),'Vacuous stalled receiver '+prefix)
 for name,code,marker in [('completion-token',1,'COMPLETION-CHECK:'),('completion-cancel',1,'COMPLETION-CHECK:'),('completion-pending',1,'COMPLETION-CHECK:'),('stall-early',1,'STALL-CHECK:'),('completion-learner-core',1,'COMPLETION-CHECK:'),('completion-learner-native',1,'COMPLETION-NATIVE-CHECK:'),('completion-learner-provider',77,'COMPLETION-UNAVAILABLE:'),('completion-learner-stall',1,'STALL-CHECK:'),('completion-learner-workload',1,'WORKLOAD-CHECK:')]:need(good(name,code,marker),'Missing compiled semantic rejection '+name)
 for cc,name in itertools.product(('gcc','clang'),('core','raw')):
  need(any(r.get('name')=='no heap '+cc+'-'+name and r.get('passed') is True for r in checks),'Missing heap audit '+cc+'-'+name)
  for tool in ('nm','size'):need(any(Path(r['argv'][0]).name==tool and Path(r['argv'][-1]).name==cc+'-'+name+'.o' and r.get('exit')==0 for r in commands),'Missing object audit '+cc+'-'+name+'/'+tool)
 workloads=[]
 for cc in ('gcc','clang'):
  rr=rows(cc+'-workload');expected=set(itertools.product(('readiness','ring','fixed','sendfile'),('1','4'),('64','4096'),('warm','cold'),('0','1','2')))
  need(len(rr)==96 and {tuple(r['argv'][1:6]) for r in rr}==expected,'Incomplete workload matrix '+cc)
  for r in rr:
   need(r.get('exit')==0 and compiled(r,['assessor/hosted_systems/completion_depth/workload.c','assessor/hosted_systems/completion_depth/completion.c','assessor/linux_io_depth/targets/raw_ring.c'],['-O2','-DNDEBUG']),'Wrong workload provider '+cc)
   try:
    check_workload(r['stdout']);v=json.loads(r['stdout']);need([v['backend'],str(v['depth']),str(v['length']),v['cache_request'],str(v['trial'])]==r['argv'][1:6],'Workload arguments/output mismatch');workloads.append(r)
   except (AssertionError,ValueError,KeyError):need(False,'Invalid workload byte/cache/resource evidence '+cc)
 seals=[r for r in commands if any(str(a).endswith('/completion_depth/seal.py') for a in r['argv'])];need(len(seals)==1 and bool(workloads) and all(commands.index(seals[0])<commands.index(r) for r in workloads),'Prediction missing or after matrix')
 if len(seals)==1:
  sealpath=next((n for n in artifacts if n.endswith('/prediction_seal.json')),None);need(bool(sealpath),'Missing sealed artifact')
  if sealpath:
   value=read_json(ROOT/sealpath);prediction='research/v5/expansion/legacy_completion_io/prediction.json';need(value.get('prediction')==read_json(ROOT/prediction),'Wrong sealed prediction');need(any(p.endswith('/'+prediction) and h==sha(ROOT/prediction) for p,h in value.get('input_sha256',{}).items()),'Wrong seal input identity')
 rr=rows('workload-offset');need(good('workload-offset',sources=['assessor/hosted_systems/completion_depth/workload.c'],options=['-DCO_WORKLOAD_MUTANT_OFFSET']),'Missing compiled offset mutant')
 if len(rr)==1:
  rejected=False
  try:check_workload(rr[0]['stdout'])
  except (AssertionError,ValueError,KeyError):rejected=True
  need(rejected and any(r.get('name')=='compiled offset mutant rejected by independent byte oracle' and r.get('passed') is True for r in checks),'Vacuous offset mutant')
 return errors

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_completion_io/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing changed objective',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observations'),('wrong objective map',lambda x:x['observations'][0].update(evidence=['heading']),'Incorrect criterion evidence map'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated topic claim'),('missing source/provider',lambda x:x.update(input_sha256={}),'Missing source/provider pins'),('missing artifacts',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('vacuous event output',lambda x:[r.update(stdout='') for r in x['commands'] if Path(r['argv'][0]).name=='gcc-release-orders'],'Incorrect event-set output gcc-release'),('missing cold backend',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='gcc-workload' and r['argv'][1:6]==['fixed','4','64','cold','2'])]),'Incomplete workload matrix gcc'),('false cold snapshot',lambda x:[r.update(stdout=json.dumps(dict(json.loads(r['stdout']),resident_pages_before=1))) for r in x['commands'] if Path(r['argv'][0]).name=='gcc-workload' and r['argv'][4]=='cold'],'Invalid workload byte/cache/resource evidence gcc'),('missing prediction',lambda x:x.update(commands=[r for r in x['commands'] if not any(str(v).endswith('/completion_depth/seal.py') for v in r['argv'])]),'Prediction missing or after matrix'),('false guest physical',lambda x:[r['observed'].update(physical_measurement=True) for r in x['checks'] if r.get('name')=='actual guest ABI identity'],'Inflated backend evidence'),('vacuous stall',lambda x:[r.update(stdout=r['stdout'].replace('would_block=1','would_block=0')) for r in x['commands'] if Path(r['argv'][0]).name=='gcc-release-stall'],'Vacuous stalled receiver gcc-release'),('lost fixed cancellation',lambda x:[r.update(stdout=r['stdout'].replace('COMPLETION-CANCEL:fixed=1;','OMITTED:')) for r in x['commands'] if Path(r['argv'][0]).name=='gcc-release-native' and r['argv'][-1]=='cancel'],'Missing ordinary/fixed cancellation gcc-release')]
  for name in ('completion-token','completion-cancel','completion-pending','stall-early','completion-learner-core','completion-learner-native','completion-learner-provider','completion-learner-stall','completion-learner-workload'):
   cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name==n],'Missing compiled semantic rejection '+name))
  cases.append(('missing compiling control',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='completion-token')]),'Missing compiled semantic rejection completion-token'))
  for name,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale source:') for e in validate(bad));controls.append('stale source rejected')
 out=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_completion_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,out);print(dict(passed=not errors,checks=len(controls),errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
