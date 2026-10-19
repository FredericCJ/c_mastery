"""Require exact ordinary/changed T10 evidence; reject vacuity and stale inputs."""
from copy import deepcopy
import argparse,json
from common import ROOT,read_json,dump,sha
OBJECTIVES={f'T10.{i:02}.{leaf}' for i in range(1,11) for leaf in 'abc'}
MODULES={'core','entry','runtime'}
def validate(data):
 errors=[]
 if data.get('passed') is not True or data.get('error') is not None:errors.append('Campaign failed')
 if set(data.get('selected_modules',[]))!=MODULES:errors.append('Focused evidence is incomplete')
 pins=data.get('input_sha256',{})
 if not pins:errors.append('Missing source pins')
 for path,digest in pins.items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=digest:errors.append('Stale input: '+path)
 observations=data.get('observations',[])
 if len(observations)!=60 or {(x.get('objective'),x.get('variant')) for x in observations}!={(o,v) for o in OBJECTIVES for v in (False,True)}:errors.append('Missing ordinary or changed objective')
 commands=data.get('commands',[]);checks=data.get('checks',[])
 if not commands or not checks:errors.append('Missing executions')
 for row in commands:
  rc=row.get('exit');expected=row.get('expected');valid=isinstance(rc,int) and rc>0 if expected=='nonzero' else rc==expected
  if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):valid=False
  if not valid or row.get('passed') is not True:errors.append('Inconsistent command result')
 if any(x.get('passed') is not True for x in checks):errors.append('Failed property')
 required={'Arm core object has no implicit runtime helper','entry boundary campaign is nonvacuous',
 'first instruction owner follows actual PT_INTERP','original stack preserved across aligned call and real pushed return address',
 'raw process exact bytes and deterministic exit','raw final no dynamic dependencies or remaining relocations',
 'actual retained target objects for every source','all object imports resolve to original selected sources',
 'selected crti defines callable DT_INIT fragment','actual musl errno provider requires TLS register',
 'raw exit leaves original worker able to finish','exit_group kills blocked worker without marker',
 'real trace includes thread then process termination','final origin/call audit rejects resolved forbidden implementation'}
 if not required<={x.get('name') for x in checks}:errors.append('Missing discriminating property')
 def executed(name):return [r for r in commands if r.get('argv') and r['argv'][0].split('/')[-1]==name]
 for name in ('resend','learner'):
  rows=executed(name)
  if len(rows)!=1 or rows[0]['exit']!=1 or 'STARTUP-CHECK:' not in rows[0]['stdout']+rows[0]['stderr']:errors.append('Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  for profile in ('debug','release','sanitize'):
   rows=executed(cc+'-'+profile)
   if len(rows)!=1 or 'STARTUP-CORE-PASS' not in rows[0]['stdout']:errors.append('Missing six core profiles')
  for opt in ('O0','O2'):
   for name,status in [('base',42),('parser',42),('decimal',42),('nodiv',42),('saved',71),('align',72),('early',73),('hook',42)]:
    rows=executed(f'{cc}-{opt}-{name}')
    if len(rows)!=1 or rows[0]['exit']!=status:errors.append('Missing exact image outcome')
   rows=executed(cc+opt+'-raw')
   if len(rows)!=1 or 'partial=1' not in rows[0]['stdout']:errors.append('Missing real frame progress')
  for name,status in [('unrelocated',86),('relocated',42),('two-defects',72),('alignment-fixed',86)]:
   rows=executed(cc+'-'+name)
   if len(rows)!=1 or rows[0]['exit']!=status:errors.append('Missing two-stage diagnosis')
 for name in ('musl-static','musl-dynamic'):
  rows=executed(name)
  if len(rows)!=1 or 'CRT:7:23:11' not in rows[0]['stdout']:errors.append('Missing alternate libc execution')
 generated=data.get('generated_inputs',{})
 for path in ('entry/hosted-snapshots.json','entry/bridge.json','entry/image-manifests.json','runtime/crt-origins.json','runtime/thread-exit.trace'):
  if path not in generated:errors.append('Missing generated artifact: '+path)
 try:
  images=json.loads(generated['entry/image-manifests.json']['content'])
  if len(images)!=32 or len({x['name'] for x in images})!=32:errors.append('Missing resource increments')
 except (KeyError,ValueError):errors.append('Missing resource increments')
 if not any(p.endswith('.su') for p in generated):errors.append('Missing stack resource artifacts')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default='reviews/v5/expansion/legacy_systems_startup/full_validation.json');a=p.parse_args();data=read_json(ROOT/a.report);errors=validate(data);guards=[]
 if not errors:
  mutations=[('changed objective',lambda d:d['observations'].pop(),'Missing ordinary or changed objective'),
   ('focused scope',lambda d:d['selected_modules'].pop(),'Focused evidence is incomplete'),
   ('mutant crash',lambda d:[r.update(exit=-11,expected=-11) for r in d['commands'] if r['argv'][0].endswith('/resend')],'Missing semantic rejection: resend'),
   ('no real partial frame',lambda d:[r.update(stdout=r['stdout'].replace('partial=1','partial=0')) for r in d['commands'] if r['argv'][0].endswith('O2-raw')],'Missing real frame progress'),
   ('missing musl run',lambda d:d.update(commands=[r for r in d['commands'] if not r['argv'][0].endswith('/musl-static')]),'Missing alternate libc execution'),
   ('missing second diagnosis',lambda d:d.update(commands=[r for r in d['commands'] if not r['argv'][0].endswith('/clang-alignment-fixed')]),'Missing two-stage diagnosis'),
   ('missing resource increments',lambda d:d['generated_inputs'].pop('entry/image-manifests.json'),'Missing resource increments'),
   ('missing thread exit trace',lambda d:d['generated_inputs'].pop('runtime/thread-exit.trace'),'Missing generated artifact: runtime/thread-exit.trace')]
  for name,mutate,expected in mutations:
   bad=deepcopy(data);mutate(bad);assert expected in validate(bad),name;guards.append(name+' rejected')
  bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(x.startswith('Stale input') for x in validate(bad));guards.append('stale source rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=guards,input_sha256={a.report:sha(ROOT/a.report)});dump(ROOT/'reviews/v5/expansion/legacy_systems_startup/validation_check.json',result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
