"""Check exact timer mechanisms, imported provider and compiled semantic controls."""
from copy import deepcopy
from pathlib import Path
import argparse,importlib.util,itertools,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={'T24.09.'+x for x in 'abc'}
PUBLIC=['labs/hosted_systems/timer_depth/'+n for n in ('CONTRACTS.md','TIMERS.md','timer.h','timer.c','tests.c','race_tests.c','native.c','run.py')]+['labs/hosted_systems/'+n for n in ('include/hosted.h','src/deadline.c','src/common.h','CONTRACTS.md')]
PRIVATE=['timer.c','native.c','policy_oracle.py','policy_trace.c','campaign.py','ANALYSIS.md']
MODES={'event':'TIMER-EVENT:sum=5;short_rejected=1;saturation_preserved=1;semaphore_ones=3;','periodic':'TIMER-PERIODIC:first=','rearm':'TIMER-REARM:earlier_selected=1;restored_late=1;empty_disarmed=1;provider_cancel=1;','cached':'TIMER-CACHED:old=1;new=2;old_rejected=1;new_fired=1;repeat_cancel=0;','failure':'TIMER-FAILURE:acquisition_cuts=3;invalid_arm=1;success=1;closed=9;'}
NEGATIVES={**{n:(None,'TIMER-CHECK:') for n in ('generation','wrap','count','drift','early','learner-core')},'race-early':(None,'TIMER-RACE-CHECK:'),'learner-races':(None,'TIMER-RACE-CHECK:'),'learner-native':('event','TIMER-NATIVE-CHECK:'),'learner-provider':('rearm','TIMER-NATIVE-CHECK:')}
EVIDENCE={'a':['native:event','native:failure','native:rearm','import:assessor/hosted_systems/deadline.c','tests.c:earliest'],'b':['native:periodic','independent-calendar:14112','tests.c:edges'],'c':['native:cached','tests.c:history','race_tests.c:one','tsan:correct-and-known-race']}
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True and d.get('error') is None,'Failed campaign');need(d.get('topic_complete') is False,'Inflated topic claim')
 obs=d.get('observations',[]);need(len(obs)==6 and {(r.get('objective'),r.get('variant')) for r in obs}==set(itertools.product(OBJECTIVES,(False,True))),'Missing exact ordinary/changed observation')
 need(all(r.get('evidence')==EVIDENCE.get(r.get('objective','')[-1]) for r in obs),'Incorrect criterion evidence mapping')
 required=['tools/run_timers_followthrough.py','assessor/legacy_systems/toolchain/common.py','assessor/hosted_systems/deadline.c','assessor/hosted_systems/common.h','assessor/hosted_systems/reactor_depth/known_race.c','research/v5/expansion/legacy_reactor/resources.json',*PUBLIC]+['assessor/hosted_systems/timer_depth/'+n for n in PRIVATE]+['research/v5/expansion/legacy_timers/'+n for n in ('resources.json','research_gate.json','source_access.json','author_support.py')]
 pins=d.get('input_sha256',{});need(set(required)<=pins.keys(),'Missing required source pins')
 for name,h in pins.items():
  p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()) and p.is_file() and sha(p)==h,'Stale input: '+name)
 artifacts=d.get('retained_artifacts',{});need(bool(artifacts),'Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve();need(p.is_relative_to((ROOT/'build/legacy_timers').resolve()) and p.is_file() and sha(p)==row.get('sha256') and p.stat().st_size==row.get('bytes'),'Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[]);need(len(commands)==110 and all(r.get('passed') is True for r in commands),'Exact110 passing commands');need(bool(checks) and all(r.get('passed') is True for r in checks),'Missing/failed checks')
 for r in commands:need(r.get('exit')==r.get('expected') and (not r.get('diagnostic') or r['diagnostic'] in r.get('stdout','')+r.get('stderr','')),'Inconsistent command')
 def good(name,mode=None,code=0,marker='',sources=(),options=()):
  rows=[r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name and (mode is None or r['argv'][1:]==[mode])]
  if len(rows)!=1:return False
  r=rows[0]
  if r.get('exit')!=code or marker not in r.get('stdout','')+r.get('stderr',''):return False
  builds=[b for b in commands if Path(b['argv'][0]).name in ('gcc','clang') and '-o' in b['argv'] and b['argv'][b['argv'].index('-o')+1]==r['argv'][0] and b.get('exit')==0]
  if len(builds)!=1 or commands.index(builds[0])>=commands.index(r):return False
  argv=builds[0]['argv'];return all(any(a.replace('\\','/').endswith('/'+s) for a in argv) for s in sources) and all(o in argv for o in options)
 spec=importlib.util.spec_from_file_location('timer_report_oracle',ROOT/'assessor/hosted_systems/timer_depth/policy_oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 core=['assessor/hosted_systems/timer_depth/timer.c'];race=core+['labs/hosted_systems/timer_depth/race_tests.c']
 for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
  prefix=cc+'-'+profile;options=['-std=c17']+(['-O0'] if profile=='debug' else ['-O2','-DNDEBUG'] if profile=='release' else ['-O1','-fsanitize=address,undefined'])
  need(good(prefix+'-core',marker='TIMER-PASS:history=3;overflow_edges=3;earliest=4;',sources=core+['labs/hosted_systems/timer_depth/tests.c'],options=options),'Missing core/profile: '+prefix)
  for mode,marker in MODES.items():need(good(prefix+'-native',mode,marker=marker,sources=core+['assessor/hosted_systems/timer_depth/native.c','assessor/hosted_systems/deadline.c'],options=options),'Missing native/provider: '+prefix+'/'+mode)
  need(good(prefix+'-calendar',sources=core+['assessor/hosted_systems/timer_depth/policy_trace.c'],options=options),'Missing calendar trace: '+prefix)
  need(any(r.get('name')=='independent calendar '+prefix and r.get('observed')==dict(cases=14112,independent_representation='enumerated absolute calendar instants') for r in checks),'Missing independent oracle: '+prefix)
  for r in commands:
   if Path(r['argv'][0]).name==prefix+'-calendar':
    try:oracle.check(r.get('stdout',''))
    except (ValueError,AssertionError):need(False,'Incorrect calendar output: '+prefix)
   if Path(r['argv'][0]).name==prefix+'-native' and r['argv'][1:]==['periodic']:
    try:result=oracle.check_native(r.get('stdout',''))
    except (ValueError,AssertionError):need(False,'Incorrect native clock bracket: '+prefix)
    else:need(any(x.get('name')=='native clock-bracket calendar '+prefix and x.get('observed')==result for x in checks),'Missing native clock bracket: '+prefix)
  need(good(prefix+'-races',marker='TIMER-RACE:forced_running=1;forced_cancelled=1;races=200;',sources=race,options=options),'Missing actual races: '+prefix)
 for name,(mode,marker) in NEGATIVES.items():need(good('timer-'+name,mode,1,marker),'Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  need(good(cc+'-races-tsan',marker='TIMER-RACE:forced_running=1;forced_cancelled=1;races=200;',sources=race,options=['-fsanitize=thread']),'Missing actual TSan: '+cc)
  need(good(cc+'-known-race',code=66,marker='WARNING: ThreadSanitizer: data race',sources=['assessor/hosted_systems/reactor_depth/known_race.c'],options=['-fsanitize=thread']),'Missing known-race control: '+cc)
  need(any(r.get('name')=='no heap service '+cc and r.get('passed') is True for r in checks),'Missing object import check: '+cc)
  for tool in ('nm','size'):need(any(Path(r['argv'][0]).name==tool and Path(r['argv'][-1]).name==cc+'-timer.o' and r.get('exit')==0 for r in commands),'Missing object tool: '+cc+'/'+tool)
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_timers/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing changed observation',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('incorrect mapping',lambda x:x['observations'][0].update(evidence=['heading']),'Incorrect criterion evidence mapping'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated topic claim'),('missing objects',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('missing sources',lambda x:x.update(input_sha256={}),'Missing required source pins')]
  for name in NEGATIVES:cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name=='timer-'+n],'Missing semantic rejection: '+name))
  cases += [('missing mutation compile',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='timer-generation')]),'Missing semantic rejection: generation'),('vacuous calendar',lambda x:[r.update(stdout='') for r in x['commands'] if Path(r['argv'][0]).name=='gcc-release-calendar'],'Incorrect calendar output: gcc-release'),('fabricated timestamp',lambda x:[r.update(stdout='TIMER-PERIODIC:first=10;period=2000000;read_before=9;dispatch=8;count=6;catchup=6;skip=1;next=7;') for r in x['commands'] if Path(r['argv'][0]).name=='clang-release-native' and r['argv'][1:]==['periodic']],'Incorrect native clock bracket: clang-release'),('missing actual TSan',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='clang-races-tsan']),'Missing actual TSan: clang'),('missing known-race',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='gcc-known-race']),'Missing known-race control: gcc'),('missing native cleanup',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='gcc-release-native' and r['argv'][1:]==['failure'])]),'Missing native/provider: gcc-release/failure')]
  for name,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(d)
  for r in bad['commands']:
   if '-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='gcc-release-native':r['argv']=[v.replace('/assessor/hosted_systems/deadline.c','/labs/hosted_systems/src/deadline.c') for v in r['argv']]
  assert 'Missing native/provider: gcc-release/rearm' in validate(bad);controls.append('wrong imported provider rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 out=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_timers_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,out);print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
