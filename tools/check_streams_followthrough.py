"""Reject incomplete, stale, crashed or uncompiled T24.05/.06 reference claims."""
from copy import deepcopy
from pathlib import Path
import argparse,itertools,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={f'T24.{n:02}.{x}' for n in (5,6) for x in 'abc'}
PUBLIC=['CONTRACTS.md','STREAMS.md','DATAGRAMS.md','stream.h','stream.c','tests.c']
PRIVATE=['ANALYSIS.md','stream.c','streams.c','datagrams.c','sequence_oracle.py','sequence_trace.c','campaign.py']
MODES={'streams':{'halfclose':'STREAM-HALFCLOSE:','replay':'STREAM-REPLAY:','pressure':'STREAM-PRESSURE:actual_connections=4;queued_frames=32;'},'datagrams':{'truncation':'DATAGRAM-TRUNCATION:','ancillary':'DATAGRAM-ANCILLARY:','ack':'DATAGRAM-ACK:'}}
NEGATIVES={**{x:(None,'STREAM-CHECK:') for x in ('length','eof','global','replay','current-result','session','advance-all','learner-core')},'truncation':('truncation','DATAGRAM-CHECK:'),'learner-streams':('halfclose','STREAM-NATIVE-CHECK:'),'learner-datagrams':('truncation','DATAGRAM-CHECK:')}

def validate(d):
 errors=[]
 def need(test,msg):
  if not test:errors.append(msg)
 need(d.get('passed') is True and d.get('error') is None,'Failed campaign')
 need(d.get('topic_complete') is False,'Inflated topic claim')
 expected=set(itertools.product(OBJECTIVES,(False,True)));obs=d.get('observations',[])
 need(len(obs)==12 and {(r.get('objective'),r.get('variant')) for r in obs}==expected,'Missing exact ordinary/changed observation')
 required=['tools/run_streams_followthrough.py','assessor/legacy_systems/toolchain/common.py','labs/hosted_systems/event_depth/signals.h','assessor/hosted_systems/event_depth/signals.c']+['labs/hosted_systems/stream_depth/'+n for n in PUBLIC]+['assessor/hosted_systems/stream_depth/'+n for n in PRIVATE]+['research/v5/expansion/legacy_framing/'+n for n in ('resources.json','research_gate.json','source_access.json','author_support.py')]
 pins=d.get('input_sha256',{});need(set(required)<=pins.keys(),'Missing required source pins')
 for name,h in pins.items():
  p=(ROOT/name).resolve();need(p.is_relative_to(ROOT.resolve()) and p.is_file() and sha(p)==h,'Stale input: '+name)
 artifacts=d.get('retained_artifacts',{});need(bool(artifacts),'Missing retained artifacts')
 for name,row in artifacts.items():
  p=(ROOT/name).resolve();need(p.is_relative_to((ROOT/'build/legacy_framing').resolve()) and p.is_file() and sha(p)==row.get('sha256') and p.stat().st_size==row.get('bytes'),'Stale artifact: '+name)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 need(len(commands)==104 and all(r.get('passed') is True for r in commands),'Exact 104 passing commands')
 need(bool(checks) and all(r.get('passed') is True for r in checks),'Missing/failed checks')
 for r in commands:
  need(r.get('exit')==r.get('expected') and (not r.get('diagnostic') or r['diagnostic'] in r.get('stdout','')+r.get('stderr','')),'Inconsistent command')
 def good(name,mode=None,code=0,marker=''):
  rows=[r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name and (mode is None or r['argv'][1:]==[mode])]
  if len(rows)!=1:return False
  r=rows[0]
  if r.get('exit')!=code or marker not in r.get('stdout','')+r.get('stderr',''):return False
  builds=[b for b in commands if Path(b['argv'][0]).name in ('gcc','clang') and '-o' in b['argv'] and b['argv'][b['argv'].index('-o')+1]==r['argv'][0] and b.get('exit')==0]
  if len(builds)!=1 or commands.index(builds[0])>=commands.index(r):return False
  return True
 for cc,profile in itertools.product(('gcc','clang'),('debug','release','sanitize')):
  prefix=cc+'-'+profile
  need(good(prefix+'-core',marker='all_chunk_partitions=2611210;queue_budget_cells=2313;'),'Missing core: '+prefix)
  for driver,modes in MODES.items():
   for mode,marker in modes.items():need(good(prefix+'-'+driver,mode,marker=marker),'Missing native mode: '+prefix+'/'+driver+'/'+mode)
  need(good(prefix+'-sequence'),'Missing sequence trace: '+prefix)
  need(any(r.get('name')=='independent sequence oracle '+prefix and r.get('observed')==dict(rows=175,reachable_states=25) for r in checks),'Missing independent oracle: '+prefix)
  builds=[r for r in commands if '-o' in r.get('argv',[]) and Path(r['argv'][r['argv'].index('-o')+1]).name.startswith(prefix+'-')]
  options=['-O0'] if profile=='debug' else ['-O2','-DNDEBUG'] if profile=='release' else ['-O1','-fsanitize=address,undefined']
  need(len(builds)==4 and all(all(x in r['argv'] for x in options) and '-std=c17' in r['argv'] for r in builds),'Missing actual profile options: '+prefix)
 for name,(mode,marker) in NEGATIVES.items():need(good('stream-'+name,mode,1,marker),'Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  need(any(r.get('name')=='no heap service '+cc and r.get('passed') is True for r in checks),'Missing object import check: '+cc)
  for tool in ('nm','size'):
   need(any(Path(r['argv'][0]).name==tool and Path(r['argv'][-1]).name==cc+'-stream.o' and r.get('exit')==0 for r in commands),'Missing object tool: '+cc+'/'+tool)
 return errors

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_framing/support_validation.json');a=p.parse_args();d=read_json(ROOT/a.report);errors=validate(d);controls=[]
 if not errors:
  cases=[('missing delayed observation',lambda x:x['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated topic',lambda x:x.update(topic_complete=True),'Inflated topic claim'),('missing objects',lambda x:x.update(retained_artifacts={}),'Missing retained artifacts'),('missing sources',lambda x:x.update(input_sha256={}),'Missing required source pins')]
  for name in NEGATIVES:
   cases.append((name+' crash substitution',lambda x,n=name:[r.update(exit=-11,expected=-11) for r in x['commands'] if Path(r['argv'][0]).name=='stream-'+n],'Missing semantic rejection: '+name))
  cases += [('missing compilation',lambda x:x.update(commands=[r for r in x['commands'] if not('-o' in r['argv'] and Path(r['argv'][r['argv'].index('-o')+1]).name=='stream-current-result')]),'Missing semantic rejection: current-result'),('missing native ancillary mode',lambda x:x.update(commands=[r for r in x['commands'] if not(Path(r['argv'][0]).name=='clang-release-datagrams' and r['argv'][1:]==['ancillary'])]),'Missing native mode: clang-release/datagrams/ancillary'),('missing independent oracle',lambda x:x.update(checks=[r for r in x['checks'] if r.get('name')!='independent sequence oracle gcc-debug']),'Missing independent oracle: gcc-debug'),('missing sanitizer instrumentation',lambda x:[r['argv'].remove('-fsanitize=address,undefined') for r in x['commands'] if '-fsanitize=address,undefined' in r['argv']],'Missing actual profile options: gcc-sanitize'),('missing object inspection',lambda x:x.update(commands=[r for r in x['commands'] if Path(r['argv'][0]).name!='nm']),'Missing object tool: gcc/nm')]
  for name,mutate,wanted in cases:
   bad=deepcopy(d);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(d);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 out=dict(passed=not errors,errors=errors,negative_guards=controls,input_sha256={a.report:sha(ROOT/a.report),'tools/check_streams_followthrough.py':sha(Path(__file__))});dump(ROOT/a.output,out);print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
