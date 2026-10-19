"""Execute the original bounded superloop and its independent changed conditions."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile,time
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/constrained_depth/superloop';BASE=ROOT/'reviews/v5/expansion/legacy_constrained_superloop'
spec=importlib.util.spec_from_file_location('superloop_oracle',PUBLIC/'oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',ROOT/'assessor/legacy_constrained/superloop.c',ROOT/'assessor/legacy_constrained/superloop.md',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use superloop evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Actual C cooperative continuation, queue/release/policy and changed-condition support; no physical timing or asynchronous scheduler claim.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def record(command,result,out,label):
  path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(out,encoding='utf-8',newline='\n');row={'command':list(map(str,command)),'exit_code':result,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row);return row
 def run(command,label,expected=0,marker=None):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;row=record(command,r.returncode,out,label)
  if r.returncode!=expected or marker and marker not in out or any(s in out for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='constrained-superloop-') as directory:
   work=Path(directory);source=ROOT/'assessor/legacy_constrained/superloop.c';common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)]
   for profile,compiler,flags in PROFILES:
    for variant in (*range(8),'stub'):
     selected=PUBLIC/'superloop.c' if variant=='stub' else source;binary=work/(profile+'-'+str(variant));_,build=run([compiler,*common,*flags,'-DSC_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if variant==0 else 1,'SC_PASS:' if variant==0 else 'SC_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-replay');_,build=run([compiler,*common,*flags,source,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,execution=run([binary],profile+'-replay');value['observations'].append({'target':'replay','profile':profile,'build':build,'execution':execution,'expected':oracle.check(out),'passed':True})
    binary=work/(profile+'-blocking');_,build=run([compiler,*common,*flags,PUBLIC/'blocking.c','-o',binary],profile+'-blocking-build');start=time.monotonic();process=subprocess.Popen([str(binary)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,env=env)
    timed_out=False
    try:out,err=process.communicate(timeout=.25)
    except subprocess.TimeoutExpired:
     timed_out=True;process.kill();out,err=process.communicate(timeout=5)
    execution=record([binary],process.returncode,out+err,profile+'-blocking');execution.update(expected_observation_timeout=timed_out,elapsed_seconds=time.monotonic()-start,termination='Harness SIGKILL of only its own disposable nonyielding child')
    if not timed_out or process.returncode!=-9 or out!='SC_BLOCK_ENTER:nonyielding-driver\n' or err:raise ValueError('Wrong bounded nonyielding observation')
    value['observations'].append({'target':'blocking','profile':profile,'build':build,'execution':execution,'passed':True,'scope':'Expected bounded failure-of-progress observation, not a semantic mutant or passing scheduler path.'})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'model');expected=oracle.analysis()
   if json.loads(out)!=expected:raise ValueError('Wrong original arithmetic model')
   value['observations'].append({'target':'model','execution':execution,'expected':expected,'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    obj=work/(profile+'.o');_,build=run([compiler,*common,opt,'-fstack-protector-strong','-fstack-usage','-c',source,'-o',obj],profile+'-object');out,symbols=run(['nm','-u',obj],profile+'-undefined');assembly,disassembly=run(['objdump','-dr',obj],profile+'-assembly');stack=logs/(profile+'.su');stack.write_bytes(obj.with_suffix('.su').read_bytes());imports=[line.split()[-1] for line in out.splitlines()]
    if '__stack_chk_fail' not in imports or any(s not in ('memcpy','memset','memmove','__stack_chk_fail') for s in imports) or 'sc_schedule_next' not in assembly or 'sc_releases_poll' not in assembly:raise ValueError('Unexpected hosted mechanism dependency')
    value['observations'].append({'target':'audit','profile':profile,'build':build,'symbols':symbols,'assembly':disassembly,'stack':stack.relative_to(ROOT).as_posix(),'imports':imports,'passed':True,'scope':'Actual host object/stack/helper observation, not a Cortex-M link or physical resource bound.'})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==70;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
