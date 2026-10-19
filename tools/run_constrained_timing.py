"""Run complete bounded chronology, real atomic readers, and observed host latency."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/constrained_depth/timing';BASE=ROOT/'reviews/v5/expansion/legacy_constrained_timing'
NATIVE_PROFILES=(*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']))
spec=importlib.util.spec_from_file_location('constrained_timing_oracle',PUBLIC/'oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',ROOT/'assessor/legacy_constrained/timing.c',ROOT/'assessor/legacy_constrained/timing.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use constrained timing evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Exact half-range/epoch/resource core and original changed conditions, actual host threads/clocks/process restart; no physical DMA/ISR timing, weak-memory proof or WCET guarantee.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,env=env,timeout=90);out=result.stdout+result.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(out,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in out or any(s in out for s in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','FATAL: ThreadSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='constrained-timing-') as directory:
   work=Path(directory);source=ROOT/'assessor/legacy_constrained/timing.c';common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)]
   for profile,compiler,flags in PROFILES:
    for variant in (*range(8),'stub'):
     binary=work/(profile+'-core-'+str(variant));selected=PUBLIC/'timing.c' if variant=='stub' else source;_,build=run([compiler,*common,*flags,'-DCT_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,PUBLIC/'tests.c','-o',binary],profile+'-core-build');out,execution=run([binary],profile+'-core-'+str(variant),0 if variant==0 else 1,'CT_PASS:' if variant==0 else 'CT_CHECK:');value['observations'].append({'target':'core','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    for kind in ('replay','measure','persist'):
     binary=work/(profile+'-'+kind);_,build=run([compiler,*common,*flags,source,PUBLIC/(kind+'.c'),'-o',binary],profile+'-'+kind+'-build')
     if kind=='persist':
      stored=work/(profile+'-timestamp');out,write=run([binary,'write',stored],profile+'-persist-write',marker='CT_PERSIST_WRITE:');out,execution=run([binary,'read',stored],profile+'-persist-read',marker='CT_PERSIST_PASS:');copy=logs/(profile+'-timestamp.bin');copy.write_bytes(stored.read_bytes());value['observations'].append({'target':kind,'profile':profile,'build':build,'write':write,'execution':execution,'record':copy.relative_to(ROOT).as_posix(),'passed':True})
     else:
      out,execution=run([binary],profile+'-'+kind);expected=oracle.check_replay(out) if kind=='replay' else oracle.check_measure(out);value['observations'].append({'target':kind,'profile':profile,'build':build,'execution':execution,'expected':expected,'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([compiler,*common,*flags,'-pthread',source,PUBLIC/'native.c','-o',binary],profile+'-native-build');out,execution=run([binary],profile+'-native',marker='CT_NATIVE_PASS:published=10000');value['observations'].append({'target':'native','profile':profile,'build':build,'execution':execution,'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'analysis');expected=oracle.analysis()
   if json.loads(out)!=expected:raise ValueError('Wrong independent chronology/SC analysis')
   value['observations'].append({'target':'analysis','execution':execution,'expected':expected,'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    obj=work/(profile+'.o');_,build=run([compiler,*common,opt,'-fstack-protector-strong','-fstack-usage','-c',source,'-o',obj],profile+'-object');out,symbols=run(['nm','-u',obj],profile+'-undefined');assembly,disassembly=run(['objdump','-dr',obj],profile+'-assembly');stack=logs/(profile+'.su');stack.write_bytes(obj.with_suffix('.su').read_bytes());imports=[line.split()[-1] for line in out.splitlines()]
    if any(s not in ('memcpy','memset','memmove','__stack_chk_fail') for s in imports) or any(s not in assembly for s in ('ct_snapshot','ct_metrics','ct_duration','ct_publish')):raise ValueError('Unexpected current hosted atomic/core dependency')
    value['observations'].append({'target':'audit','profile':profile,'build':build,'symbols':symbols,'assembly':disassembly,'stack':stack.relative_to(ROOT).as_posix(),'imports':imports,'passed':True,'scope':'Actual selected host object and local stack/helper evidence, not a general atomic lock-freedom/freestanding or whole-stack claim.'})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==84;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
