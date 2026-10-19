"""Reject incomplete discovery receipts and verify exact public assessment export."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_acpi_depth import BASE,PUBLIC,PROFILES,BE_PROFILES,VARIANTS,oracle,guest,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,exit_code,positive,target='host'):
 marker=('SA_BE_PASS:' if positive else 'SA_BE_CHECK:') if target=='big-endian' else ('SA_PASS:' if positive else 'SA_CHECK:')
 if exit_code!=(0 if positive else 1) or marker not in text or any(v in text for v in ('ERROR: AddressSanitizer','runtime error:','Segmentation fault','FAIL SMP:')):raise ValueError('Wrong semantic success/rejection; crash/build failure is not acceptance')
def elf_check(header,undefined,dynamic):
 if 'big endian' not in header or 'PowerPC64' not in header or 'abiv2' not in header or undefined.strip() or 'NEEDED' in dynamic:raise ValueError('Wrong target endian/ABI/runtime closure')
def validate_campaign(value):
 errors=pin_errors(value)
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Missing/extra producer source closure')
 external=value.get('external_tools_sha256',{})
 if len(external)!=2 or {Path(p).name for p in external}!={'qemu-ppc64','ld.lld-21'} or any(hashlib.sha256(Path(p).read_bytes()).hexdigest()!=h for p,h in external.items()):errors.append('Wrong target tools')
 logs=value.get('log_sha256',{})
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Missing/changed raw evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound observed command')
  return content(command['log']).decode()
 groups={kind:[r for r in value['observations'] if r['target']==kind] for kind in ('host','oracle','guard','capture','big-endian','guest')}
 if len(value['observations'])!=78:errors.append('Wrong total observations')
 for kind,profiles,variants in (('host',PROFILES,VARIANTS),('big-endian',BE_PROFILES,VARIANTS)):
  rows=groups[kind]
  if len(rows)!=len(profiles)*len(variants) or {(r['profile'],r['variant']) for r in rows}!={(p[0],v) for p in profiles for v in variants}:errors.append('Missing/duplicate '+kind+' condition')
  for row in rows:
   positive=row['variant']==0;flags=next(p[-1] for p in profiles if p[0]==row['profile']);build=row['build'];execution=row['execution'];cmd=build['command'];source='labs/smp_depth/acpi/acpi.c' if row['variant']=='stub' else 'assessor/legacy_smp/acpi.c'
   required=['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DSA_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]
   if any(v not in cmd for v in required) or build not in value['commands'] or build['exit_code'] or not any(str(p).endswith('/'+source) for p in cmd) or row.get('passed') is not True:errors.append('Wrong successful translation/profile/source')
   if cmd[cmd.index('-o')+1]!=execution['command'][-1]:errors.append('Executed binary not the just-built condition')
   try:semantic(text(execution),execution['exit_code'],positive,kind)
   except ValueError as e:errors.append(str(e))
   if kind=='big-endian':
    if any(v not in cmd for v in ('--target=powerpc64-linux-gnu','-mabi=elfv2','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','-Wl,-e,_start')) or execution['command'][0] not in external:errors.append('Wrong actual target or runtime restrictions')
    try:
     if any(row[k]['exit_code'] for k in ('elf','undefined','dynamic')):errors.append('Failed ELF audit')
     elf_check(text(row['elf']),text(row['undefined']),text(row['dynamic']))
    except ValueError as e:errors.append(str(e))
 for kind in ('oracle','guard','capture'):
  rows=groups[kind]
  if len(rows)!=6 or {r['profile'] for r in rows}!={p[0] for p in PROFILES}:errors.append('Missing '+kind+' profile')
  for row in rows:
   output=text(row['execution'])
   if row['execution']['exit_code'] or row.get('passed') is not True:errors.append('Failed independent/boundary result')
   if kind=='oracle':
    try:
     if oracle.check(output)!=row['expected']:errors.append('Wrong independent expected result')
    except ValueError as e:errors.append(str(e))
   else:
    flags=next(p[2] for p in PROFILES if p[0]==row['profile']);build=row['build'];marker='SA_GUARD_PASS:' if kind=='guard' else 'SA_CAPTURE_PASS:'
    if build not in value['commands'] or build['exit_code'] or any(f not in build['command'] for f in flags) or marker not in output:errors.append('Wrong guard/capture build/result')
 if value['oracle_input'] not in logs or content(value['oracle_input']).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong independent oracle inputs')
 captures=json.loads(content(value['captures']));actual_guests=groups['guest']
 if len(actual_guests)!=6 or {(r['machine'],r['cpus']) for r in actual_guests}!={(m,c) for m in ('pc-i440fx-10.2','pc-q35-10.2') for c in (1,2,4)}:errors.append('Missing actual machine/CPU change')
 for row in actual_guests:
  capture=next(c for c in captures if (c['machine'],c['cpus'])==(row['machine'],row['cpus']));stage=row['stage'];cmd=stage['command']
  if row['log'] not in logs or guest.decode(content(row['log']).decode())!={k:capture[k] for k in ('rsdp','regions')} or capture['stage']!=stage:errors.append('Captured bytes do not match actual raw stage')
  if stage.get('passed') is not True or stage.get('deadline_expired') is not False or stage.get('marker')!='PASS smp-madt' or row.get('passed') is not True:errors.append('Failed/timeout discovery stage')
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-machine')+1]!=row['machine'] or cmd[cmd.index('-smp')+1]!=str(row['cpus']) or cmd[cmd.index('-net')+1]!='none' or 'tcg,thread=multi' not in cmd or any(v in cmd for v in ('-enable-kvm','-hda','-hostfwd')):errors.append('Wrong bounded guest environment')
 with tempfile.TemporaryDirectory(prefix='acpi-evidence-oracle-') as directory:
  work=Path(directory);expected=oracle.capture_header(work/'capture_cases.h',captures);oracle.header(work/'oracle_cases.h')
  if expected!=value['capture_expected']:errors.append('Wrong expected firmware inventory')
  for name in ('capture_cases.h','oracle_cases.h'):
   matches=[p for p in logs if p.endswith('/'+name)]
   if len(matches)!=1 or content(matches[0])!=(work/name).read_bytes():errors.append('Wrong generated independent target vectors')
 build=json.loads(content(value['guest_build']))
 if build.get('built') is not True or build.get('runtime_linked') or build.get('storage_linked') or 'zero undefined' not in build.get('strict_link',''):errors.append('Wrong actual original guest build closure')
 if value['instrumentation']['original_source_sha256']!=value['inspected_paths']['assessor/kernel_smp/smp.c']:errors.append('Wrong retained discovery identity')
 if any(c['exit_code']!=0 for c in value['commands'] if 'guest-build' in c['log']):errors.append('Guest build failure')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_smp_acpi.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use SMP ACPI successor evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'78-source-current-observations-and-independent-wire-targets','passed':True}]
 controls=[('missing-target',lambda d:d['observations'].pop()),('compile-failure',lambda d:next(r for r in d['observations'] if r['target']=='host')['build'].update(exit_code=1)),('crash-not-control',lambda d:next(r for r in d['observations'] if r['target']=='host' and r['variant']==1)['execution'].update(exit_code=139)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/acpi.c':'0'*64})),('missing-target-tool',lambda d:d['external_tools_sha256'].pop(next(iter(d['external_tools_sha256'])))),('changed-raw-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-guest-cpus',lambda d:next(r for r in d['observations'] if r['target']=='guest').update(cpus=8)),('guest-timeout',lambda d:next(r for r in d['observations'] if r['target']=='guest')['stage'].update(deadline_expired=True)),('forged-oracle',lambda d:next(r for r in d['observations'] if r['target']=='oracle')['expected'].update(rows=1)),('missing-sanitizer',lambda d:next(r for r in d['observations'] if r['target']=='host' and r['profile']=='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-big-endian-ABI',lambda d:next(r for r in d['observations'] if r['target']=='big-endian')['build']['command'].remove('-mabi=elfv2'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,IndexError,KeyError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_FAILURE',1,False)),('negative-crash-marker',lambda:semantic('SA_CHECK:7',139,False)),('false-success',lambda:semantic('SA_CHECK:7',0,False)),('little-endian-target',lambda:elf_check('PowerPC64 abiv2 little endian','','')),('hidden-runtime-helper',lambda:elf_check('PowerPC64 abiv2 big endian','U memcpy',''))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 captures=json.loads(content(value['captures']));changed=deepcopy(captures);changed[0]['regions'].pop()
 with tempfile.TemporaryDirectory(prefix='acpi-negative-root-') as directory:
  try:oracle.capture_header(Path(directory)/'out.h',changed);rejected=False
  except (ValueError,KeyError):rejected=True
  checks.append({'name':'missing-mapped-root-child','passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-01.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criterion-bodies-preserved','passed':criteria==old['criteria']},{'name':'historical-binding-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='smp-acpi-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-01',work);public=work/'labs/smp_depth/acpi';binary=work/'test';checks.append({'name':'full-public-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/acpi/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'acpi.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None
  checks.append({'name':'exported-scaffold-compiled-semantic-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SA_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-01'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'normative-reference-gap-retained','passed':any('G-LCON-ACPI' in str(g) for g in data['remaining_external_work'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_acpi_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_acpi/author_support.py',support,path,campaign]
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Bounded parser/root/guest/big-endian author evidence and negative controls; no normative conformance, physical or independent assessment claim.'}
 destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
