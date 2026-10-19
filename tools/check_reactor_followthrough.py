"""Validate exact reactor mechanisms, native modes, providers and negative controls."""
from copy import deepcopy
from pathlib import Path
import argparse,itertools,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={f'T24.{n:02}.{x}' for n in (7,8,11) for x in 'abc'}
PUBLIC=['labs/hosted_systems/reactor_depth/'+n for n in ('CONTRACTS.md','READINESS.md','LIFETIME.md','SHUTDOWN.md','reactor.h','reactor.c','tests.c','waiters.h','waiters.c','waiters_tests.c')]+['labs/hosted_systems/stream_depth/'+n for n in ('stream.h','stream.c','CONTRACTS.md','STREAMS.md')]
PRIVATE=['ANALYSIS.md','reactor.c','waiters.c','native.c','schedule_oracle.py','schedule_trace.c','known_race.c','campaign.py']
MODES={'cursor':'REACTOR-CURSOR:progress=2,again,1,eof;','duplex':'REACTOR-DUPLEX:rx=3;tx=2;','lt':'REACTOR-EPOLL:et=0;oneshot=0;after_partial_notification=1;','et':'REACTOR-EPOLL:et=1;oneshot=0;after_partial_notification=0;','oneshot':'REACTOR-EPOLL:et=1;oneshot=1;after_partial_notification=0;','reuse':'REACTOR-REUSE:slot=2;old=7;new=8;same_descriptor=1;','shutdown':'REACTOR-SHUTDOWN:accepted=32;completed=1;cancelled=31;','policies':'REACTOR-POLICIES:actual_drop=1;actual_disconnect=1;pause_interest_suppressed=1;resumed_frame=42;'}
NEGATIVES={**{n:(None,'REACTOR-CHECK:') for n in ('again','generation','operation','wrap','work','borrow','early-messages','learner-core','learner-provider')},'learner-native':('cursor','REACTOR-NATIVE-CHECK:'),**{n:(None,'REACTOR-WAIT-CHECK:') for n in ('wait-stop','wait-wake','learner-waiters')}}
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True and d.get('error') is None,'Failed campaign');need(d.get('topic_complete') is False,'Inflated topic claim')
 obs=d.get('observations',[]);need(len(obs)==18 and {(r.get('objective'),r.get('variant')) for r in obs}==set(itertools.product(OBJECTIVES,(False,True))),'Missing exact ordinary/changed observation')
 required=['tools/run_reactor_followthrough.py','assessor/legacy_systems/toolchain/common.py','assessor/hosted_systems/stream_depth/stream.c',*PUBLIC]+['assessor/hosted_systems/reactor_depth/'+n for n in PRIVATE]+['research/v5/expansion/legacy_reactor/'+n for n in ('resources.json','research_gate.json','source_access.json','author_support.py')]
 pins=d.get('input_sha256',{});need(set(required)<=pins.keys(),'Missing required source pins')
 for name,h in pins.items():
  p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()) and p.is_file() and sha(p)==h,'Stale input: '+name)
 artifacts=d.get('retained_artifacts',{});need(bool(artifacts),'Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve();need(p.is_relative_to((ROOT/'build/legacy_reactor').resolve()) and p.is_file() and sha(p)==row.get('sha256') and p.stat().st_size==row.get('bytes'),'Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[]);need(len(commands)==134 and all(r.get('passed') is True for r in commands),'Exact 134 passing commands');need(bool(checks) and all(r.get('passed') is True for r in checks),'Missing/failed checks')
 for r in commands:need(r.get('exit')==r.get('expected') and (not r.get('diagnostic') or r['diagnostic'] in r.get('stdout','')+r.get('stderr','')),'Inconsistent command')
 def good(name,mode=None,code=0,marker='',sources=(),options=()):
  rows=[r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name and (mode is None or r['argv'][1:]==[mode])]
  if len(rows)!=1:return False
  r=rows[0]
  if r.get('exit')!=code or marker not in r.get('stdout','')+r.get('stderr',''):return False
  builds=[b for b in commands if Path(b['argv'][0]).name in ('gcc','clang') and '-o' in b['argv'] and b['argv'][b['argv'].index('-o')+1]==r['argv'][0] and b.get('exit')==0]
  if len(builds)!=1 or commands.index(builds[0])>=commands.index(r):return False
  argv=builds[0]['argv'];return all(any(a.replace('\\','/').endswith('/'+s) for a in argv) for s in sources) and all(o in argv for o in options)
 core=['assessor/hosted_systems/reactor_depth/reactor.c','assessor/hosted_systems/stream_depth/stream.c'];wait=['labs/hosted_systems/reactor_depth/waiters_tests.c','assessor/hosted_systems/reactor_depth/waiters.c']
 for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
  prefix=cc+'-'+profile;options=['-std=c17']+(['-O0'] if profile=='debug' else ['-O2','-DNDEBUG'] if profile=='release' else ['-O1','-fsanitize=address,undefined'])
  need(good(prefix+'-core',marker='byte_budget_cells=297;shutdown_unique=6;',sources=core+['labs/hosted_systems/reactor_depth/tests.c'],options=options),'Missing core/provider/profile: '+prefix)
  for mode,marker in MODES.items():need(good(prefix+'-native',mode,marker=marker,sources=core+['assessor/hosted_systems/reactor_depth/native.c'],options=options),'Missing native mode: '+prefix+'/'+mode)
  need(good(prefix+'-schedule',sources=core+['assessor/hosted_systems/reactor_depth/schedule_trace.c'],options=options),'Missing scheduler trace: '+prefix)
  need(any(r.get('name')=='independent scheduler oracle '+prefix and r.get('observed')==dict(cases=12288,independent_representation='ordered participant set and quota arithmetic') for r in checks),'Missing independent oracle: '+prefix)
  need(good(prefix+'-waiters',marker='REACTOR-WAIT:enrolled_empty_full_peer=3;predicate_recheck=1;races=200;',sources=wait,options=options),'Missing actual waiters: '+prefix)
 for name,(mode,marker) in NEGATIVES.items():need(good('reactor-'+name,mode,1,marker),'Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  need(good(cc+'-waiters-tsan',marker='REACTOR-WAIT:',sources=wait,options=['-fsanitize=thread']),'Missing actual waiter TSan: '+cc)
  need(good(cc+'-known-race',code=66,marker='WARNING: ThreadSanitizer: data race',sources=['assessor/hosted_systems/reactor_depth/known_race.c'],options=['-fsanitize=thread']),'Missing known-race TSan control: '+cc)
  need(any(r.get('name')=='no heap service '+cc and r.get('passed') is True for r in checks),'Missing object import check: '+cc)
  for tool in ('nm','size'):need(any(Path(r['argv'][0]).name==tool and Path(r['argv'][-1]).name==cc+'-reactor.o' and r.get('exit')==0 for r in commands),'Missing object tool: '+cc+'/'+tool)
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_reactor/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing delayed observation',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated topic claim'),('missing objects',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('missing sources',lambda x:x.update(input_sha256={}),'Missing required source pins')]
  for name in NEGATIVES:cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name=='reactor-'+n],'Missing semantic rejection: '+name))
  cases += [('missing mutation compile',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='reactor-operation')]),'Missing semantic rejection: operation'),('missing actual pause adapter',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='clang-release-native' and r['argv'][1:]==['policies'])]),'Missing native mode: clang-release/policies'),('missing scheduler oracle',lambda x:x.update(checks=[r for r in x['checks'] if r.get('name')!='independent scheduler oracle gcc-debug']),'Missing independent oracle: gcc-debug'),('missing actual TSan',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='clang-waiters-tsan']),'Missing actual waiter TSan: clang'),('missing known-race control',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='gcc-known-race']),'Missing known-race TSan control: gcc'),('missing sanitizer flags',lambda x:[r['argv'].remove('-fsanitize=address,undefined') for r in x['commands'] if '-fsanitize=address,undefined' in r['argv']],'Missing core/provider/profile: gcc-sanitize')]
  for name,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(d)
  for r in bad['commands']:
   if '-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='gcc-release-core':r['argv']=[v.replace('/assessor/hosted_systems/stream_depth/stream.c','/labs/hosted_systems/stream_depth/stream.c') for v in r['argv']]
  assert 'Missing core/provider/profile: gcc-release' in validate(bad);controls.append('wrong imported provider rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 out=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_reactor_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,out);print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
