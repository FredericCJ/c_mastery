"""Execute startup lease histories and actual relocated/sparse/late AP guests."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/startup';BASE=ROOT/'reviews/v5/expansion/legacy_smp_startup'
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
GUEST_PROFILES=(('gcc-O0','gcc','-O0'),('gcc-O3','gcc','-O3'),('clang-O3','clang','-O3'))
def load(name):
 spec=importlib.util.spec_from_file_location('smp_startup_'+name,PUBLIC/(name+'.py'));result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
oracle=load('oracle');guest=load('guest');audit=load('audit')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),*[ROOT/'assessor/legacy_smp'/p for p in ('startup.c','startup_guest.c','startup_trampoline.S','startup.md')]]
 for directory in (PUBLIC,ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):
  paths += [p for p in directory.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use startup successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],
 'scope':'Six hosted controller profiles; three compiler profiles of actual serial/parallel/late AP startup, two regenerated low-page layouts, sparse 1/2/4 CPUs and changed CPU model.',
 'limits':['QEMU TCG evidence is not physical INIT/SIPI timing, arbitrary CPU compatibility or current errata review.','One-SIPI teaching policy, no retry on failed startup; Intel typical two-SIPI sequence is a separately identified difference.','Controller quiescence requires a trusted port premise; actual guest proves its specific permanent-stack handoff, not arbitrary caller attestations.','Bounded histories and finite machine matrix do not prove all executions, independent assessment or learner mastery.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None,timeout=180):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=timeout);text=result.stdout+result.stderr
  path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in text:raise ValueError(label+': '+text)
  if any(s in text for s in ('ERROR: AddressSanitizer','runtime error:','Segmentation fault','FAIL SMP:','FAIL host deadline')):raise ValueError(label+': fault is not semantic evidence')
  return text,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','objdump')}
  for model,props,available in (('qemu64',{},True),('max',{},True),('qemu64',{'nx':False},False)):
   feature=guest.cpu_features(model,props)
   if feature['available']!=available or not available and feature['missing']!=['nx']:raise ValueError('CPU preflight did not distinguish unavailable NX')
   value['observations'].append({'target':'preflight','passed':True,**feature})
  with tempfile.TemporaryDirectory(prefix='smp-startup-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)]
   input_text='\n'.join(oracle.inputs())+'\n';(logs/'histories.in').write_text(input_text,encoding='utf-8',newline='\n')
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,'stub'):
     source=PUBLIC/'startup.c' if variant=='stub' else ROOT/'assessor/legacy_smp/startup.c';binary=work/(profile+'-'+str(variant));positive=variant==0
     _,build=run([compiler,*common,*flags,'-DSS_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build')
     _,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'SS_PASS:' if positive else 'SS_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'binary_sha256':sha(binary),'passed':True})
     if positive:
      trace=work/(profile+'-trace');_,build=run([compiler,*common,*flags,source,PUBLIC/'trace.c','-o',trace],profile+'-trace-build');text,execution=run([trace],profile+'-oracle',input_text=input_text)
      value['observations'].append({'target':'oracle','profile':profile,'build':build,'execution':execution,'expected':oracle.check_model(text),'passed':True})
   for profile,compiler,optimization in GUEST_PROFILES:
    configurations=[(base,mode,0) for base in (0x8000,0x9000) for mode in (0,1,2)]+[(0x8000,0,1),(0x8000,0,2),(0x8000,0,'stub')]
    for base,mode,variant in configurations:
     label=profile+'-'+hex(base)+'-'+str(mode)+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     implementation=PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp'
     g,instrumentation=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',implementation,folder,base,mode,variant if isinstance(variant,int) else 0,compiler,optimization,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'guest/build.json').read_bytes())
     if variant!='stub':
      result=audit.audit(folder/'guest/ap.S.o',base,logs/(label+'-audit'),lambda cmd:run(cmd,label+'-audit')[0])
      value['observations'].append({'target':'assembly-audit','profile':profile,'base':base,'mode':mode,'variant':variant,'passed':True,'audit':result,'guest_build':build_path.relative_to(ROOT).as_posix()})
     matrix=((1,'qemu64'),(2,'qemu64'),(4,'qemu64'),(4,'max')) if variant==0 else ((2,'qemu64'),)
     for cpus,model in matrix:
      positive=variant==0;marker='SS_GUEST_PASS:' if positive else ('SS_GUEST_CHECK:missing-ack' if variant==1 else 'SS_GUEST_CHECK:stale-or-incomplete-ack' if variant==2 else 'SS_GUEST_CHECK:unsolved-startup-adapter')
      text,execution=run(guest.command(g,folder,cpus,model),label+'-'+str(cpus)+'-'+model,33 if positive else 35,marker,timeout=60)
      checked=oracle.check_guest(text,cpus,base,mode,not positive)
      value['observations'].append({'target':'guest','profile':profile,'base':base,'mode':mode,'variant':variant,'cpus':cpus,'model':model,'passed':True,'checked':checked,'execution':execution,'instrumentation':instrumentation,'guest_build':build_path.relative_to(ROOT).as_posix()})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==144
 value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(main(a.report))
