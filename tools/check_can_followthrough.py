"""Validate adapter evidence without promoting modeled frames to physical CAN."""
from copy import deepcopy
from pathlib import Path
import argparse,importlib.util,itertools,json,os
from common import ROOT,read_json,dump,sha
OBJECTIVES={'T24.10.c'}
PUBLIC=['labs/hosted_systems/can_depth/'+n for n in ('frames.h','frames.c','tests.c','CONTRACTS.md','CAN.md','BENCH.md','bench_plan.json','check_bench.py')]
PRIVATE=['frames.c','filter_trace.c','filter_oracle.py','environment.py','campaign.py','ANALYSIS.md']
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True and d.get('error') is None,'Failed campaign');need(d.get('topic_complete') is False,'Inflated topic claim')
 obs=d.get('observations',[]);need(len(obs)==2 and {(r.get('objective'),r.get('variant')) for r in obs}=={('T24.10.c',False),('T24.10.c',True)},'Missing ordinary/changed observation')
 need(all(r.get('evidence')==(['BENCH.md','bench_plan.json','check_bench.py:self-test-and-unfilled-rejection'] if r.get('variant') else ['tests.c','independent-tuples:32768','environment.py']) for r in obs),'Incorrect criterion evidence mapping')
 required=['tools/run_can_followthrough.py','assessor/legacy_systems/toolchain/common.py',*PUBLIC]+['assessor/hosted_systems/can_depth/'+n for n in PRIVATE]+['research/v5/expansion/legacy_can/'+n for n in ('resources.json','research_gate.json','source_access.json','author_support.py')]
 pins=d.get('input_sha256',{});need(set(required)<=pins.keys(),'Missing required source pins')
 for name,h in pins.items():
  p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()) and p.is_file() and sha(p)==h,'Stale input: '+name)
 artifacts=d.get('retained_artifacts',{});need(bool(artifacts),'Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve();need(p.is_relative_to((ROOT/'build/legacy_can').resolve()) and p.is_file() and sha(p)==row.get('sha256') and p.stat().st_size==row.get('bytes'),'Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[]);need(len(commands)==47 and all(r.get('passed') is True for r in commands),'Exact47 passing commands');need(bool(checks) and all(r.get('passed') is True for r in checks),'Missing/failed checks')
 for r in commands:need(r.get('exit')==r.get('expected') and (not r.get('diagnostic') or r['diagnostic'] in r.get('stdout','')+r.get('stderr','')),'Inconsistent command')
 envs=[r.get('observed',{}) for r in checks if r.get('name')=='explicit software backend'];need(len(envs)==1,'Missing environment record')
 for e in envs:
  need(e.get('kernel_can_execution') is False and e.get('selected_backend')=='original-frame-adapter','Inflated kernel CAN evidence')
  need(e.get('virtual_interfaces')==[],'Source-access absence needs reconciliation')
  need(set(e.get('headers_sha256',{}))=={'/usr/include/linux/'+n for n in ('can.h','can/raw.h','can/error.h')},'Missing header identities')
  for path,h in e.get('headers_sha256',{}).items():
   p=Path('//wsl.localhost/Ubuntu'+path) if os.name=='nt' else Path(path)
   need(p.is_file() and sha(p)==h,'Stale installed header: '+path)
 def good(name,code=0,marker='',sources=(),options=()):
  rows=[r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name]
  if len(rows)!=1:return False
  r=rows[0]
  if r.get('exit')!=code or marker not in r.get('stdout','')+r.get('stderr',''):return False
  builds=[b for b in commands if Path(b['argv'][0]).name in ('gcc','clang') and '-o' in b['argv'] and b['argv'][b['argv'].index('-o')+1]==r['argv'][0] and b.get('exit')==0]
  if len(builds)!=1 or commands.index(builds[0])>=commands.index(r):return False
  argv=builds[0]['argv'];return all(any(a.replace('\\','/').endswith('/'+s) for a in argv) for s in sources) and all(o in argv for o in options)
 spec=importlib.util.spec_from_file_location('can_report_oracle',ROOT/'assessor/hosted_systems/can_depth/filter_oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
  prefix=cc+'-'+profile;options=['-std=c17']+(['-O0'] if profile=='debug' else ['-O2','-DNDEBUG'] if profile=='release' else ['-O1','-fsanitize=address,undefined'])
  need(good(prefix+'-core',marker='CAN-PASS:lengths=256;raw_dlc=256;roundtrips=36;filter_cells=8192;loopback_model_cells=16;physical=0;',sources=['assessor/hosted_systems/can_depth/frames.c','labs/hosted_systems/can_depth/tests.c'],options=options),'Missing core/profile: '+prefix)
  need(good(prefix+'-tuples',sources=['assessor/hosted_systems/can_depth/frames.c','assessor/hosted_systems/can_depth/filter_trace.c'],options=options),'Missing tuple execution: '+prefix)
  need(any(r.get('name')=='independent tuple oracle '+prefix and r.get('observed')==dict(cases=32768,independent_representation='format/request/identifier tuple equality') for r in checks),'Missing independent oracle: '+prefix)
  for r in commands:
   if Path(r['argv'][0]).name==prefix+'-tuples':
    try:oracle.check(r.get('stdout',''))
    except (ValueError,AssertionError):need(False,'Incorrect tuple output: '+prefix)
 for name in ('filter','length','error','own','learner'):need(good('can-'+name,1,'CAN-CHECK:'),'Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  need(any(r.get('name')=='no heap service '+cc and r.get('passed') is True for r in checks),'Missing object import check: '+cc)
  for tool in ('nm','size'):need(any(Path(r['argv'][0]).name==tool and Path(r['argv'][-1]).name==cc+'-frames.o' and r.get('exit')==0 for r in commands),'Missing object tool: '+cc+'/'+tool)
 math=[r for r in commands if any(str(v).endswith('/can_depth/check_bench.py') for v in r['argv']) and '--self-test' in r['argv']]
 try:value=json.loads(math[0]['stdout']) if len(math)==1 else {}
 except ValueError:value={}
 need(value==dict(passed=True,synthetic_math_cases=6,device_legality=False,physical_observation=False),'Missing bounded math evidence')
 need(any('--plan' in r['argv'] and any(str(v).endswith('/can_depth/bench_plan.json') for v in r['argv']) and r.get('exit')==1 and 'positive integer decoded timing parameters required' in r.get('stderr','') for r in commands),'Missing unfilled-plan rejection')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_can/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing changed observation',lambda x:x['observations'].pop(),'Missing ordinary/changed observation'),('incorrect mapping',lambda x:x['observations'][0].update(evidence=['heading']),'Incorrect criterion evidence mapping'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated topic claim'),('missing objects',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('missing sources',lambda x:x.update(input_sha256={}),'Missing required source pins')]
  for name in ('filter','length','error','own','learner'):cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name=='can-'+n],'Missing semantic rejection: '+name))
  cases += [('missing mutation compile',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='can-filter')]),'Missing semantic rejection: filter'),('vacuous tuple oracle',lambda x:[r.update(stdout='') for r in x['commands'] if Path(r['argv'][0]).name=='gcc-release-tuples'],'Incorrect tuple output: gcc-release'),('inflated kernel backend',lambda x:[r['observed'].update(kernel_can_execution=True) for r in x['checks'] if r.get('name')=='explicit software backend'],'Inflated kernel CAN evidence'),('missing hardware rejection',lambda x:x.update(commands=[r for r in x['commands'] if '--plan' not in r['argv']]),'Missing unfilled-plan rejection'),('fabricated physical math',lambda x:[r.update(stdout=json.dumps(dict(passed=True,synthetic_math_cases=6,device_legality=True,physical_observation=True))) for r in x['commands'] if '--self-test' in r['argv']],'Missing bounded math evidence')]
  for name,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 out=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_can_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,out);print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
