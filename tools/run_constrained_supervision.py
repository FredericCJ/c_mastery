"""Execute progress/bitmap/recovery/power models and selected original M4 guest."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/constrained_depth/supervision';BASE=ROOT/'reviews/v5/expansion/legacy_constrained_supervision'
NATIVE_PROFILES=(*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-no-pie']))
def load(name):
 s=importlib.util.spec_from_file_location('supervision_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
oracle=load('oracle');guest=load('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',*[ROOT/'assessor/legacy_constrained'/n for n in ('supervision.c','power.c','power_f439.c','sleep_arm.c','supervision.md')],*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use supervision evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original six criteria/variants: selected host/native protocols, architectural wake model and M4 instructions; F439 Stop/IWDG model plus compile-only binding; no physical low-power/LSI/energy claim.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=90);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm')}
  with tempfile.TemporaryDirectory(prefix='constrained-supervision-') as d:
   work=Path(d);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)];core=ROOT/'assessor/legacy_constrained/supervision.c';power=ROOT/'assessor/legacy_constrained/power.c'
   for profile,cc,flags in PROFILES:
    for kind,variants in (('core',(*range(7),'stub')),('power',(*range(4),'stub'))):
     for variant in variants:
      name=profile+'-'+kind+'-'+str(variant);binary=work/name
      selected=[PUBLIC/'supervision.c' if variant=='stub' else core,PUBLIC/'tests.c'] if kind=='core' else [core,PUBLIC/'power.c' if variant=='stub' else power,PUBLIC/'power_tests.c']
      defs=['-DCS_TEST_HOOK','-DCS_MUTANT='+str(variant if isinstance(variant,int) else 0)] if kind=='core' else ['-DCS_POWER_MUTANT='+str(variant if isinstance(variant,int) else 0)]
      _,build=run([cc,*common,*flags,*defs,*selected,'-o',binary],name+'-build');out,e=run([binary],name,0 if variant==0 else 1,('CS_PASS:' if kind=='core' else 'CS_POWER_PASS:') if variant==0 else ('CS_CHECK:' if kind=='core' else 'CS_POWER_CHECK:'));v['observations'].append({'target':kind,'profile':profile,'variant':variant,'build':build,'execution':e,'passed':True})
    binary=work/(profile+'-replay');_,build=run([cc,*common,*flags,core,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,e=run([binary],profile+'-replay');v['observations'].append({'target':'replay','profile':profile,'build':build,'execution':e,'expected':oracle.check(out),'passed':True})
   for profile,cc,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([cc,*common,*flags,'-pthread',core,PUBLIC/'native.c','-o',binary],profile+'-native-build');out,e=run([binary],profile+'-native',marker='CS_NATIVE_PASS:');v['observations'].append({'target':'native','profile':profile,'build':build,'execution':e,'passed':True})
   for profile,cc,opt in GUEST_PROFILES:
    for variant in (0,1,2,'stub'):
     name=profile+'-guest-'+str(variant);folder=work/name;begin=len(v['commands']);selected=[PUBLIC/'m4.c',core,PUBLIC/'sleep.c' if variant=='stub' else ROOT/'assessor/legacy_constrained/sleep_arm.c',power,ROOT/'assessor/legacy_constrained/power_f439.c',ROOT/'assessor/execution/support.c'];image,audit=guest.build(folder,cc,opt,selected,ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',variant,lambda cmd:run(cmd,name)[0]);builds=v['commands'][begin:];out,e=run(guest.command(image),name,0 if variant==0 else 1,'CS_TARGET_PASS:' if variant==0 else 'CS_TARGET_CHECK:');saved=logs/name;saved.mkdir()
     for p in folder.iterdir():
      if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
     v['observations'].append({'target':'guest','profile':profile,'variant':variant,'build_commands':builds,'execution':e,'audit':audit,'artifacts':saved.relative_to(ROOT).as_posix(),'passed':True})
   out,e=run(['python3',PUBLIC/'oracle.py'],'wake-model');result=json.loads(out)
   if result!=oracle.wake_model():raise ValueError('Wrong wake model')
   v['observations'].append({'target':'wake-model','execution':e,'expected':result,'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==105;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
