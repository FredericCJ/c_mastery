"""Run original bounded recovery, actual signature admission and M4 handoff."""
import argparse,hashlib,importlib.util,json,os,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/embedded_debug_depth/recovery';BASE=ROOT/'reviews/v5/expansion/legacy_embedded_debug_recovery';CORE=ROOT/'assessor/embedded_debug_depth/recovery.c';INCLUDE=ROOT/'labs/firmware_lifecycle/targets';ASSESSOR=ROOT/'assessor/firmware_lifecycle/targets';LIFECYCLE=ASSESSOR/'lifecycle_extension.c';SIGNATURE=ASSESSOR/'signature_adapter.c'
CRYPTO_ROOT=Path.home()/'cmastery-b06-tools/usr'
def load(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
guest=load('debug_recovery_guest',PUBLIC/'guest.py');oracle=load('debug_recovery_oracle',PUBLIC/'oracle.py');builder=load('debug_recovery_builder',PUBLIC/'build.py')
CORE_VARIANTS={0:(0,0),1:(1,0),2:(2,0),3:(3,0),4:(4,0),5:(5,0),'stub':(0,0),'lce-marker':(0,2)}
SIGN_VARIANTS={'reference':(0,0,0),'old-floor':(1,0,0),'fail-open':(0,1,0),'wrong-key':(0,2,0),'revocation':(0,0,1)}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def headers():
 found=set();pending=['lifecycle_extension.h','signature_adapter.h','boot_pair.h']
 while pending:
  p=INCLUDE/pending.pop()
  if p in found:continue
  found.add(p);pending.extend(re.findall(r'#include\s+"([^"]+)"',p.read_text(encoding='utf-8')))
 return sorted(found)
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',CORE,ROOT/'assessor/embedded_debug_depth/recovery_target.c',ROOT/'assessor/embedded_debug_depth/recovery.md',LIFECYCLE,SIGNATURE,ASSESSOR/'boot_handoff.c',ASSESSOR/'boot_handoff.S',INCLUDE/'boot_guest.S',INCLUDE/'boot_guest.ld',INCLUDE/'EXTENSIONS.md',INCLUDE/'SIGNATURE.md',INCLUDE/'BOOT_PAIR.md',ROOT/'assessor/execution/support.c',*headers(),*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def crypto_files():return sorted([CRYPTO_ROOT/'lib/x86_64-linux-gnu/libcrypto.so.3',*(CRYPTO_ROOT/'include/openssl').glob('*.h'),*(CRYPTO_ROOT/'include/x86_64-linux-gnu/openssl').glob('*.h')])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use recovery evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original finite update/recovery plus real OpenSSL signature/policy boundary and actual M4 handoff; no physical Flash/power loss/actuator.'};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,markers=()):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or markers and not any(m in out for m in markers) or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['external_crypto_sha256']={str(p):sha(p) for p in crypto_files()}
  v['toolchains']={c:run([c,'--version' if c!='openssl' else 'version'],c)[0].splitlines()[0] for c in ('gcc','clang','arm-none-eabi-gcc','qemu-system-arm','openssl')}
  with tempfile.TemporaryDirectory(prefix='debug-recovery-') as d:
   work=Path(d)
   for profile,cc,flags in PROFILES:
    for variant,(mutation,lce) in CORE_VARIANTS.items():
     name=profile+'-'+str(variant);binary=work/name;source=PUBLIC/'recovery.c' if variant=='stub' else CORE;rows=[run(c,name+'-build')[1] for c in builder.commands(cc,flags,source,LIFECYCLE,INCLUDE,binary,mutant=mutation,lce_mutant=lce)];_,e=run([binary],name,0 if variant==0 else 1,['DU_PASS:' if variant==0 else 'DU_CHECK:']);v['observations'].append({'target':'core','profile':profile,'variant':variant,'build_commands':rows,'execution':e,'passed':True})
    name=profile+'-replay';binary=work/name;rows=[run(c,name+'-build')[1] for c in builder.commands(cc,flags,CORE,LIFECYCLE,INCLUDE,binary,'replay.c')];out,e=run([binary],name);v['observations'].append({'target':'replay','profile':profile,'build_commands':rows,'execution':e,'expected':oracle.check(out),'passed':True})
    for variant,(mutation,sign,lce) in SIGN_VARIANTS.items():
     name=profile+'-signature-'+variant;binary=work/name;rows=[run(c,name+'-build')[1] for c in builder.commands(cc,flags,CORE,LIFECYCLE,INCLUDE,binary,'signature.c',lce_mutant=lce,signature=SIGNATURE,sign_mutant=sign,signature_mutant=mutation,crypto_root=CRYPTO_ROOT)];_,e=run([binary],name,0 if variant=='reference' else 1,['DU_SIGNATURE_PASS:' if variant=='reference' else 'DU_SIGNATURE_CHECK:']);v['observations'].append({'target':'signature','profile':profile,'variant':variant,'build_commands':rows,'execution':e,'passed':True})
   for profile,cc,opt in GUEST_PROFILES:
    for variant in (0,1,2,'stub'):
     name=profile+'-target-'+str(variant);folder=work/name;begin=len(v['commands']);driver=PUBLIC/'target.c' if variant=='stub' else ROOT/'assessor/embedded_debug_depth/recovery_target.c';image,audit=guest.build(folder,cc,opt,CORE,driver,ASSESSOR/'boot_handoff.c',ASSESSOR/'boot_handoff.S',INCLUDE/'boot_guest.S',INCLUDE/'boot_guest.ld',INCLUDE,ROOT/'assessor/execution/support.c',variant,lambda c:run(c,name)[0]);rows=v['commands'][begin:];out,e=run(guest.command(image),name,0 if variant==0 else 1,['FB_GUEST_HANDOFF_PASS:VTOR,MSP,CONTROL,PRIMASK,MPU'] if variant==0 else ['FB_GUEST_HANDOFF_FAIL','DU_TARGET_CHECK:']);saved=logs/name;saved.mkdir()
     for p in folder.iterdir():
      if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
     v['observations'].append({'target':'guest','profile':profile,'variant':variant,'build_commands':rows,'execution':e,'audit':audit,'artifacts':saved.relative_to(ROOT).as_posix(),'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items()) and all(sha(Path(p))==h for p,h in v.get('external_crypto_sha256',{}).items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==96;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
