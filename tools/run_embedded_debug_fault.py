"""Execute original fixed-fault/stack models and isolated M4 MPU capture."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/embedded_debug_depth/fault';BASE=ROOT/'reviews/v5/expansion/legacy_embedded_debug_fault';FRAME=ROOT/'assessor/interrupt_depth/frame/frame.c';INCLUDE=ROOT/'labs/interrupt_depth/frame'
def load(name):
 s=importlib.util.spec_from_file_location('debug_fault_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
guest=load('guest');oracle=load('oracle');decoder=load('decode')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',FRAME,INCLUDE/'frame.h',INCLUDE/'CONTRACTS.md',ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',*[ROOT/'assessor/embedded_debug_depth'/n for n in ('fault.c','fault_target.c','fault.md')],*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use fault evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original host capture/budget/refault models and actual original M4 MPU store/capture; no physical fault or general recovery claim.'};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm','arm-none-eabi-addr2line')}
  with tempfile.TemporaryDirectory(prefix='debug-fault-') as d:
   work=Path(d);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(INCLUDE)];core=ROOT/'assessor/embedded_debug_depth/fault.c'
   for profile,cc,flags in PROFILES:
    for variant in (0,1,2,3,4,5,'stub'):
     name=profile+'-'+str(variant);binary=work/name;source=PUBLIC/'fault.c' if variant=='stub' else core
     _,b=run([cc,*common,*flags,'-DDF_MUTANT='+str(variant if isinstance(variant,int) else 0),source,FRAME,PUBLIC/'tests.c','-o',binary],name+'-build');out,e=run([binary],name,0 if variant==0 else 1,'DF_PASS:' if variant==0 else 'DF_CHECK:');v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    binary=work/(profile+'-replay');_,b=run([cc,*common,*flags,core,FRAME,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,e=run([binary],profile+'-replay');v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check(out),'passed':True})
   for profile,cc,opt in GUEST_PROFILES:
    for variant in (0,1,2,'stub'):
     name=profile+'-target-'+str(variant);folder=work/name;begin=len(v['commands']);driver=PUBLIC/'target.c' if variant=='stub' else ROOT/'assessor/embedded_debug_depth/fault_target.c';image,audit=guest.build(folder,cc,opt,core,driver,FRAME,INCLUDE,ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',variant,lambda cmd:run(cmd,name)[0]);builds=v['commands'][begin:];out,e=run(guest.command(image),name,0 if variant==0 else 1,'DF_TARGET_PASS:' if variant==0 else 'DF_TARGET_CHECK:');saved=logs/name;saved.mkdir()
     for p in folder.iterdir():
      if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
     observation={'target':'guest','profile':profile,'variant':variant,'build_commands':builds,'execution':e,'audit':audit,'artifacts':saved.relative_to(ROOT).as_posix(),'passed':True}
     if variant==0:
      observation['decoded']=decoder.decode(out,saved/'fault.elf',audit['image_sha256']);line,c=run(['arm-none-eabi-addr2line','-f','-e',image,hex(observation['decoded']['pc'])],name+'-attribution');observation.update(attribution=c)
      if 'entry.S:' not in line:raise ValueError('Saved PC did not resolve to original instruction')
     v['observations'].append(observation)
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==60;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
