"""Criterion-specific T11 evidence guard; source identity is not learner mastery."""
from copy import deepcopy
import argparse,hashlib,json
from pathlib import Path
from common import ROOT,read_json,dump,sha

OBJECTIVES={f'T11.{i:02}.{leaf}' for i in range(1,11) for leaf in 'abc'}
MODULES={'core','lowering','services','instrumentation','library','closure','targets'}

def validate(data):
 errors=[]
 if data.get('passed') is not True or data.get('error') is not None:errors.append('Campaign failed')
 if set(data.get('selected_modules',[]))!=MODULES:errors.append('Focused evidence is incomplete')
 pins=data.get('input_sha256',{})
 if not pins:errors.append('Missing source pins')
 for path,digest in pins.items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=digest:errors.append('Stale input: '+path)
 for required in ('assessor/machine_image/wide.c','assessor/machine_image/audit.c','assessor/legacy_systems/startup_depth/decimal.c','research/v5/expansion/legacy_systems_freestanding/corrections.json'):
  if required not in pins:errors.append('Missing actual producer pin: '+required)
 observations=data.get('observations',[])
 if len(observations)!=60 or {(x.get('objective'),x.get('variant')) for x in observations}!={(o,v) for o in OBJECTIVES for v in (False,True)}:errors.append('Missing ordinary or changed objective')
 commands=data.get('commands',[]);checks=data.get('checks',[])
 if not commands or not checks:errors.append('Missing executions')
 for row in commands:
  rc=row.get('exit');expected=row.get('expected');valid=isinstance(rc,int) and rc>0 if expected=='nonzero' else rc==expected
  if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):valid=False
  if not valid or row.get('passed') is not True:errors.append('Inconsistent command result')
 if any(x.get('passed') is not True for x in checks):errors.append('Failed property')
 def count(name):return sum(r.get('name')==name for r in checks)
 for name,total in [('unbounded integer quotient/remainder oracle',8320),('streaming129-bit oracle',8196),('streaming carry path is exercised',2),('ordinary zero-initialized division carry is unreachable',2),('Arm core object has no implicit runtime helper',2),('actual accepted mi_divide import matches independent oracle',8192),('five component manifests bind actual declarations and providers',1)]:
  if count(name)!=total:errors.append('Incomplete independent evidence: '+name)
 required={'mathematical reconstruction without target overflow','restricted image has no interpreter','link exclusion controls retained inputs','service model has no implicit imported helper','instrumentation positive marker remains in final image','profiling execution emits gmon data','coverage execution writes counters','local fail-stop handler closes narrow target instrumentation','actual formatted-output environment dependencies are visible','permitted-library image closes every observed hook','resolved forbidden negative has no undefined symbols','bounded evidence rejection truncated','changed binary invalidates receipt','changed allowlist invalidates receipt','stripped image fails closed','actual dynamic dependency rejected','LTO final machine objects retained','LTO/instrumentation origin policy discriminates forbidden input','callback allocation is absent from component undefined symbols','equal-contract arithmetic measurement checksum','standalone transport contains no semihost breakpoint'}
 if not required<={r.get('name') for r in checks}:errors.append('Missing discriminating property')
 def executed(name):return [r for r in commands if r.get('argv') and Path(r['argv'][0]).name==name]
 for name,marker in [(n,'FREE-CHECK:') for n in ('forward','signed','carry','learner')]+[(n,'SERVICE-CHECK:') for n in ('refusal','resend','retained','task','service-learner')]:
  rows=executed(name)
  if len(rows)!=1 or rows[0]['exit']!=1 or marker not in rows[0]['stdout']+rows[0]['stderr']:errors.append('Missing semantic rejection: '+name)
 for cc in ('gcc','clang'):
  for name in ('debug','release','size','sanitize'):
   rows=executed(cc+'-'+name)
   if len(rows)!=1 or 'FREE-CORE-PASS' not in rows[0]['stdout']:errors.append('Missing core profile')
  for name in ('debug','release','sanitize'):
   rows=executed(cc+'-services-'+name)
   if len(rows)!=1 or 'SERVICE-PASS:' not in rows[0]['stdout']:errors.append('Missing service profile')
  for name in ('asan','ubsan'):
   for lto in ('','-lto'):
    rows=[r for r in executed(cc+'-'+name+lto) if r['argv'][1:]==['fault']]
    marker='AddressSanitizer' if name=='asan' else 'runtime error:'
    if len(rows)!=1 or not isinstance(rows[0]['exit'],int) or rows[0]['exit']<=0 or marker not in rows[0]['stdout']+rows[0]['stderr']:errors.append('Missing executed instrumentation control')
  rows=executed(cc+'-local')
  if len(rows)!=1 or rows[0]['exit']!=91:errors.append('Missing local handler execution')
 generated=data.get('generated_inputs',{})
 def record(path):
  try:
   r=generated[path]
   if hashlib.sha256(r['content'].encode()).hexdigest()!=r['sha256']:raise ValueError('identity')
   return json.loads(r['content'])
  except (KeyError,TypeError,ValueError):errors.append('Missing or altered generated artifact: '+path);return None
 oracle=record('arithmetic-oracle.json')
 if oracle and (oracle.get('seed')!=110417 or oracle.get('divisions')!=4160 or oracle.get('steps')!=4098):errors.append('Wrong oracle bounds')
 memory=record('lowering/memory-profiles.json')
 names={cc+'-'+opt+'-'+mode for cc in ('gcc','clang') for opt in ('O0','O2','Os','lto') for mode in ('hosted','freestanding','no-builtin')}
 if memory is None or {r.get('name') for r in memory}!=names or len(memory)!=24 or any(r['final_undefined'].strip() for r in memory):errors.append('Missing lowering profile')
 for name in names:
  rows=executed(name)
  if len(rows)!=1 or rows[0]['exit']!=42:errors.append('Missing lowering execution')
 probe=generated.get('lowering/memory.c',{}).get('content','')
 if not all('compare'+str(n)+'(' in probe for n in (1,3,16,128,257)):errors.append('Missing fixed-size comparison probe')
 modes=record('instrument/profiles.json');expected={cc+'-'+name+lto for cc in ('gcc','clang') for name in ('stack','asan','ubsan','profile','coverage') for lto in ('','-lto')}
 if modes is None or len(modes)!=20 or {r['name'] for r in modes}!=expected:errors.append('Missing instrumentation matrix')
 controls=record('closure/control-matrix.json')
 expected={cc+'-'+opt+'-'+kind for cc in ('gcc','clang') for opt in ('O0','O2','Os') for kind in ('positive','resolved','recursive')}|{cc+'-'+mode+'-'+kind for cc in ('gcc','clang') for mode in ('lto','stack','stack-lto') for kind in ('positive','inline-forbidden')}
 if controls is None or len(controls)!=30 or {r['profile'] for r in controls}!=expected:errors.append('Missing origin control matrix')
 else:
  for r in controls:
   p=r['profile'];need=[] if p.endswith('positive') else ['resolved forbidden symbol'] if p.endswith('resolved') else ['provider call cycle'] if p.endswith('recursive') else ['unapproved build input']
   if not set(need)<=set(r['rejections']) or bool(need)!=bool(r['rejections']):errors.append('Wrong origin rejection reason')
 library=record('library/manifests.json');hooks={'_close','_fstat','_isatty','_lseek','_read','_sbrk','_write'}
 if library is None or len(library)!=2 or {r['profile'] for r in library}!={'O0','O2'} or any(set(r['observed_missing_hooks'])!=hooks for r in library):errors.append('Missing concrete library boundary')
 guests=[r for r in commands if r['argv'][0]=='qemu-system-arm' and 'newlib-' in ' '.join(r['argv'])]
 if len(guests)!=2 or any('NEWLIB-PASS:short-write=5:refused-heap:buffer=24:error-visible' not in r['stdout']+r['stderr'] or r['exit']!=0 for r in guests):errors.append('Missing library guest execution')
 for opt in ('O0','O2','Os'):
  for transport in ('semi','uart'):
   rows=[r for r in commands if any(str(a).endswith('/run_m4.py') for a in r['argv']) and any(str(a).endswith('/m4-'+opt+'-'+transport) for a in r['argv'])]
   try:
    if len(rows)!=1:raise ValueError('count')
    result=json.loads(rows[0]['stdout']);payload=result['stderr'] if transport=='semi' else result['stdout']
    if result['passed'] is not True or payload!='FREE-M4:1785:DIV:PASS\n' or result['host_stopped_guest']!=(transport=='uart') or (transport=='semi' and result['exit']!=0):raise ValueError('frame')
   except (ValueError,KeyError,TypeError):errors.append('Missing exact target frame and termination distinction')
 costs=record('target/arithmetic-costs.json');profiles={(cc,opt,policy) for cc in ('gcc','clang') for opt in ('O0','O2','Os') for policy in ('local','compiler-rt')}
 if costs:
  samples=costs.get('samples',[])
  if len(samples)!=96 or {(r['cc'],r['opt'],r['policy'],r['sample']) for r in samples}!={(*p,i) for p in profiles for i in range(8)} or any(r['operations']!=16384 or r['elapsed_ns']<=0 or r['checksum']!=costs['independent_checksum'] or r['warmup']!=(r['sample']==0) for r in samples):errors.append('Invalid equal-contract measurements')
 resources=record('target/native-resources.json')
 if resources is None or len(resources)!=12 or {r['name'] for r in resources}!={cc+'-'+opt+'-'+policy for cc,opt,policy in profiles}:errors.append('Missing arithmetic resource profiles')
 for cc,opt,policy in profiles:
  rows=executed(cc+'-'+opt+'-'+policy)
  if len(rows)!=1 or rows[0]['exit']!=42:errors.append('Missing arithmetic image execution')
 dependencies=record('manifest/dependencies.json')
 if dependencies is None or len(dependencies)!=5 or {r['id'] for r in dependencies}!={'hosted-parser','byte-archive','raw-image','explicit-runtime','callback-provider'} or any(not r['headers'] or not r['callback_contract'] or not r['storage_acquisition'] or set(r['classifications'])!={'hosted','freestanding','libc_free','allocation_free','custom_startup'} for r in dependencies):errors.append('Missing concrete dependency manifests')
 artifacts=data.get('retained_artifacts',{})
 for cc in ('gcc','clang'):
  for lto in ('','-lto'):
   if cc+'-profile'+lto+'-gmon.out' not in artifacts:errors.append('Missing fresh profiling file')
 for p in profiles:
  prefix='objects/'+'-'.join(p)+'/'
  if not any(k.startswith(prefix) and k.endswith('.o') for k in artifacts) or not any(k.startswith(prefix) and k.endswith('.su') for k in artifacts):errors.append('Missing retained object or stack evidence')
 if not errors and not artifacts:errors.append('Missing retained artifacts')
 return list(dict.fromkeys(errors))

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',default='reviews/v5/expansion/legacy_systems_freestanding/full_validation.json');a=p.parse_args();data=read_json(ROOT/a.report);errors=validate(data);guards=[]
 if not errors:
  def change_json(d,path,fn):
   row=d['generated_inputs'][path];value=json.loads(row['content']);fn(value);row['content']=json.dumps(value);row['sha256']=hashlib.sha256(row['content'].encode()).hexdigest()
  mutations=[('missing changed condition',lambda d:d['observations'].pop(),'Missing ordinary or changed objective'),('partial modules',lambda d:d['selected_modules'].pop(),'Focused evidence is incomplete'),('mutant crash',lambda d:[r.update(exit=-11,expected=-11) for r in d['commands'] if Path(r['argv'][0]).name=='carry'],'Missing semantic rejection: carry'),('instrumentation fallback',lambda d:d.update(commands=[r for r in d['commands'] if not(Path(r['argv'][0]).name=='clang-asan-lto' and r['argv'][1:]==['fault'])]),'Missing executed instrumentation control'),('vacuous stream oracle',lambda d:d.update(checks=[r for r in d['checks'] if r['name']!='streaming129-bit oracle']),'Incomplete independent evidence: streaming129-bit oracle'),('fixed-size comparison omitted',lambda d:d['generated_inputs']['lowering/memory.c'].update(content='no comparison'),'Missing fixed-size comparison probe'),('origin missing profile',lambda d:change_json(d,'closure/control-matrix.json',lambda rows:rows.pop()),'Missing origin control matrix'),('stale profile output',lambda d:d['retained_artifacts'].pop('gcc-profile-lto-gmon.out'),'Missing fresh profiling file'),('missing callback manifest',lambda d:change_json(d,'manifest/dependencies.json',lambda rows:rows.pop()),'Missing concrete dependency manifests'),('missing guest',lambda d:d.update(commands=[r for r in d['commands'] if not any(str(a).endswith('/m4-Os-uart') for a in r['argv'])]),'Missing exact target frame and termination distinction'),('unfair timing checksum',lambda d:change_json(d,'target/arithmetic-costs.json',lambda x:x['samples'][1].update(checksum=-1)),'Invalid equal-contract measurements'),('old algorithm stand-in',lambda d:d['input_sha256'].pop('assessor/machine_image/wide.c'),'Missing actual producer pin: assessor/machine_image/wide.c')]
  for name,mutate,expected in mutations:
   bad=deepcopy(data);mutate(bad)
   if expected not in validate(bad):raise AssertionError(name)
   guards.append(name+' rejected')
  bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input') for e in validate(bad));guards.append('stale input rejected')
 # Read actual retained artifacts, using the local checkout equivalent of WSL
 # paths. Missing build evidence is a reproduction requirement, never a pass.
 if not errors:
  build=ROOT/'build/legacy_systems_freestanding'/Path(data['commands'][0]['cwd']).name
  for name,record in data['retained_artifacts'].items():
   path=(build/name).resolve()
   if not path.is_relative_to(build.resolve()) or not path.is_file() or path.stat().st_size!=record['bytes'] or sha(path)!=record['sha256']:errors.append('Retained artifact changed: '+name)
 result=dict(passed=not errors,errors=errors,negative_guards=guards,input_sha256={a.report:sha(ROOT/a.report),'tools/check_systems_freestanding.py':sha(Path(__file__))},scope='Author implementation and exact bounded evidence; no independent learner or physical validation');dump(ROOT/'reviews/v5/expansion/legacy_systems_freestanding/validation_check.json',result);print(json.dumps(result));return bool(errors)

if __name__=='__main__':raise SystemExit(main())
