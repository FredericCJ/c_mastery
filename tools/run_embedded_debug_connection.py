"""Run original T31 connection models and private QEMU/GDB experiments.

Requires a private or installed gdb-multiarch executable via --gdb. Provisioning
used apt-get download gdb-multiarch=17.1-2ubuntu1 then dpkg-deb -x into ignored
build/debugger-tools/root; no global install or host security changes.
"""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/embedded_debug_depth/connection';BASE=ROOT/'reviews/v5/expansion/legacy_embedded_debug_connection'
TARGETS=(('gcc-debug','gcc','-O0',False),('gcc-release','gcc','-O3',False),('gcc-lto','gcc','-O3',True),('clang-release','clang','-O3',False))

def load(name):
 s=importlib.util.spec_from_file_location('debug_connection_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
builder=load('build');probe=load('probe');oracle=load('oracle');notebook=load('notebook')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',*[ROOT/'assessor/embedded_debug_depth'/n for n in ('connection.c','routine.c','connection.md')],*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])

def main(report,gdb):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use connection evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original transition models and actual emulator/debugger observations; no physical probe, Flash, reset wiring or silicon watchpoint count.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def save(p,value):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 def run(cmd,label,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=90);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm')};v['toolchains']['gdb']=run([gdb,'--version'],'gdb')[0].splitlines()[0];v['debugger']={'path':str(gdb),'sha256':sha(gdb)}
  with tempfile.TemporaryDirectory(prefix='debug-connection-') as d:
   work=Path(d);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)];core=ROOT/'assessor/embedded_debug_depth/connection.c'
   for profile,cc,flags in PROFILES:
    for variant in (0,1,2,3,'stub'):
     name=profile+'-'+str(variant);binary=work/name;source=PUBLIC/'connection.c' if variant=='stub' else core
     _,b=run([cc,*common,*flags,'-DDC_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],name+'-build');out,e=run([binary],name,0 if variant==0 else 1,'DC_PASS:' if variant==0 else 'DC_CHECK:');v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    binary=work/(profile+'-replay');_,b=run([cc,*common,*flags,core,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,e=run([binary],profile+'-replay');v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check_replay(out),'passed':True})
   for profile,cc,opt,lto in TARGETS:
    for variant in (0,1,2,'stub'):
     name=profile+'-target-'+str(variant);folder=work/name;begin=len(v['commands']);source=PUBLIC/'routine.c' if variant=='stub' else ROOT/'assessor/embedded_debug_depth/routine.c';image,audit=builder.build(folder,cc,opt,lto,source,ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',lambda cmd:run(cmd,name)[0],variant);builds=v['commands'][begin:];saved=logs/name;saved.mkdir()
     for p in folder.iterdir():
      if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
     for kind,delay in ([('result',0)] if variant!=0 else [('state',0),('access',0),('change',0),('limits',0),('state',1)]):
      result=probe.probe(image,gdb,kind,delay);observed=saved/(kind+'-'+str(delay)+'.json');save(observed,result);book=notebook.make(result,audit['image_sha256'],v['toolchains']);notebook.validate(book,result,audit['image_sha256'],v['toolchains']);bookpath=saved/(kind+'-'+str(delay)+'-notebook.json');save(bookpath,book)
      if variant==0:expected=oracle.check_probe(result,profile)
      else:
       try:oracle.check_probe(result,profile);raise ValueError('Unsolved target unexpectedly passed')
       except ValueError as error:
        if not str(error).startswith('DC_RESULT_CHECK:'):raise
        expected={'intended_rejection':str(error)}
      v['observations'].append({'target':'debugger','profile':profile,'variant':variant,'kind':kind,'delay_ms':delay,'build_commands':builds,'audit':audit,'artifacts':saved.relative_to(ROOT).as_posix(),'probe':observed.relative_to(ROOT).as_posix(),'notebook':bookpath.relative_to(ROOT).as_posix(),'expected':expected,'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==68;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):save(p,v)
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--gdb',type=Path,required=True);a=p.parse_args();raise SystemExit(main(a.report,a.gdb.resolve()))
