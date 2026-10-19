"""Execute exact original T31 peripheral diagnostic models and provider imports."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/embedded_debug_depth/peripheral';BASE=ROOT/'reviews/v5/expansion/legacy_embedded_debug_peripheral'
CORE=ROOT/'assessor/embedded_debug_depth/peripheral.c'
PROVIDERS=[ROOT/'assessor/peripheral_depth'/x/(x+'.c') for x in ('uart','spi','dma')]+[ROOT/'assessor/serial_depth/serial_depth.c']
INCLUDES=[ROOT/'labs/peripheral_depth'/x for x in ('uart','spi','dma')]+[ROOT/'labs/serial_depth/include']
IMPORT_VARIANTS={'reference':[], 'uart-gate':['-DPU_MUTANT=2'],'uart-txe':['-DPU_MUTANT=7'],'spi-drain':['-DPS_MUTANT=2'],'spi-busy':['-DPS_MUTANT=3'],'dma-placement':['-DPD_MUTANT=1'],'dma-generation':['-DPD_MUTANT=4'],'dma-flush':['-DPD_MUTANT=5'],'dma-cache':['-DPD_MUTANT=6']}
def load(name):
 s=importlib.util.spec_from_file_location('debug_peripheral_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
builder=load('build');oracle=load('oracle')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',CORE,ROOT/'assessor/embedded_debug_depth/peripheral.md',*PROVIDERS,*[p for x in INCLUDES for p in [x/('serial_depth.h' if x.name=='include' else x.name+'.h')]],*[ROOT/'labs/peripheral_depth'/x/'CONTRACTS.md' for x in ('uart','spi','dma')],ROOT/'labs/serial_depth/CONTRACTS.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use peripheral evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original diagnostic/correlation/debug-halt models and actual unchanged UART/SPI/DMA producer code with scripted ports. No peripheral silicon or physical pin observations.'};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang')}
  with tempfile.TemporaryDirectory(prefix='debug-peripheral-') as d:
   work=Path(d)
   for profile,cc,flags in PROFILES:
    for variant in (0,1,2,3,4,'stub'):
     source=PUBLIC/'peripheral.c' if variant=='stub' else CORE;name=profile+'-core-'+str(variant);binary=work/name;defines=['-DDP_MUTANT='+str(variant if isinstance(variant,int) else 0)]
     _,b=run(builder.command(cc,flags,source,[],[],'tests.c',binary,defines),name+'-build');_,e=run([binary],name,0 if variant==0 else 1,'DP_PASS:' if variant==0 else 'DP_CHECK:');v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    for variant,defines in IMPORT_VARIANTS.items():
     name=profile+'-imports-'+variant;binary=work/name;_,b=run(builder.command(cc,flags,CORE,PROVIDERS,INCLUDES,'imports.c',binary,defines),name+'-build');_,e=run([binary],name,0 if variant=='reference' else 1,'DP_PASS:' if variant=='reference' else 'DP_CHECK:');v['observations'].append({'target':'imports','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    name=profile+'-replay';binary=work/name;_,b=run(builder.command(cc,flags,CORE,[],[],'replay.c',binary),name+'-build');out,e=run([binary],name);v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check(out),'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==96;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
