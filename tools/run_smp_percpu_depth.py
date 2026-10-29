"""Execute actual per-struct lr_cpu frames, nested faults, SIMD and changed-ABI evidence."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/percpu';BASE=ROOT/'reviews/v5/expansion/legacy_smp_percpu'
def load(name):
 spec=importlib.util.spec_from_file_location('smp_percpu_'+name,PUBLIC/(name+'.py'));v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v);return v
oracle=load('oracle');guest=load('guest');audit=load('audit')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('percpu.c','percpu_guest.c','percpu_frames.S','percpu.md','startup.c','startup_guest.c','startup_trampoline.S')]]
 for directory in (PUBLIC,ROOT/'labs/smp_depth/startup',ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):
  paths += [p for p in directory.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use per-struct lr_cpu successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Actual private tables/stacks, normalized interrupt frames, known nested faults with peers, selected x87/SSE context preservation, checked numeric layouts, changed-ABI compilation and equivalent-contract host cost samples.','limits':['Known-site recovery is not arbitrary fault isolation or double-fault recovery.','Cooperative contexts and explicit software interrupts do not establish asynchronous SIMD preemption safety.','FXSAVE profile excludes AVX and other extended states; hardware support alone does not permit their use.','Frame calculation covers only the declared finite call/nesting tree, not arbitrary execution.','QEMU and host cost observations do not establish physical platform timing, independent review or learner mastery.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None,timeout=180):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=timeout);text=r.stdout+r.stderr;p=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');p.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if r.returncode!=expected or marker and marker not in text:raise ValueError(label+': '+text)
  if any(s in text for s in ('ERROR: AddressSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError(label+': fault is not semantic evidence')
  return text,row
 try:
  value['toolchains']={t:run([t,'--version'],t)[0].splitlines()[0] for t in ('gcc','clang','arm-none-eabi-gcc','qemu-system-x86_64','as','ld','objdump')}
  with tempfile.TemporaryDirectory(prefix='smp-percpu-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)];input_text='\n'.join(oracle.inputs())+'\n';(logs/'model.in').write_text(input_text,encoding='utf-8',newline='\n')
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,'stub'):
     source=PUBLIC/'percpu.c' if variant=='stub' else ROOT/'assessor/legacy_smp/percpu.c';binary=work/(profile+'-'+str(variant));positive=variant==0
     _,build=run([compiler,*common,*flags,'-DSP_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'SP_PASS:' if positive else 'SP_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
     if positive:
      trace=work/(profile+'-trace');_,build=run([compiler,*common,*flags,source,PUBLIC/'trace.c','-o',trace],profile+'-trace-build');text,execution=run([trace],profile+'-oracle',input_text=input_text);value['observations'].append({'target':'oracle','profile':profile,'build':build,'execution':execution,'expected':oracle.check_model(text),'passed':True})
   for target,compiler,tool,flags in (('sysv-amd64','gcc','objdump',['-m64','-mno-red-zone']),('sysv-amd64','clang','objdump',['-m64','-mno-red-zone']),('aapcs32','arm-none-eabi-gcc','arm-none-eabi-objdump',['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft'])):
    for opt in ('-O0','-O3'):
     label=compiler+'-'+opt[1:]+'-abi';obj=work/(label+'.o');_,build=run([compiler,*common,*flags,opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-c',PUBLIC/'abi_probe.c','-o',obj],label+'-build');dis,discommand=run([tool,'-dr',obj],label+'-disassembly');stack=next(work.glob(label+'*.su')).read_text(encoding='utf-8');path=logs/(label+'.su');path.write_text(stack,encoding='utf-8',newline='\n');value['observations'].append({'target':'abi','abi':target,'compiler':compiler,'optimization':opt,'build':build,'disassembly':discommand,'stack_usage':path.relative_to(ROOT).as_posix(),'expected':audit.abi_audit(stack,dis,target),'passed':True})
   for compiler in ('gcc','clang'):
    binary=work/(compiler+'-cost');_,build=run([compiler,*common,'-O3','-pthread',PUBLIC/'cost.c','-o',binary],compiler+'-cost-build');text,execution=run([binary],compiler+'-cost',marker='SP_COST ');value['observations'].append({'target':'cost','compiler':compiler,'build':build,'execution':execution,'expected':oracle.check_cost(text),'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for simd,variant in ((0,0),(1,0),(0,1),(1,2),(0,3),(0,'stub')):
     label=profile+'-'+str(simd)+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     startup,g,instrument=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/startup',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',folder,simd,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/guest/build.json').read_bytes())
     if variant==0:
      result=audit.guest_audit(folder/'image/guest',simd,lambda cmd:run(cmd,label+'-audit')[0]);result.pop('assembly');result.pop('c_assembly');path=logs/(label+'-stack.su');path.write_bytes((folder/'image/guest/smp.c.su').read_bytes());value['observations'].append({'target':'frame-audit','profile':profile,'simd':simd,'expected':result,'stack_usage':path.relative_to(ROOT).as_posix(),'passed':True})
     matrix=((1,'qemu64'),(2,'qemu64'),(4,'qemu64'),(4,'max')) if variant==0 else ((2,'qemu64'),)
     for cpus,model in matrix:
      marker='SP_GUEST_PASS:' if variant==0 else 'SP_GUEST_CHECK:'+({1:'tss-alias',2:'register-or-SIMD-preservation',3:'fault-accounting','stub':'unsolved-percpu-adapter'}[variant]);text,execution=run(startup.command(g,folder/'image',cpus,model),label+'-'+str(cpus)+'-'+model,33 if variant==0 else 35,marker,timeout=60)
      checked=oracle.check_guest(text,cpus,simd) if variant==0 else {'intended_rejection':marker}
      value['observations'].append({'target':'guest','profile':profile,'simd':simd,'variant':variant,'cpus':cpus,'model':model,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':instrument,'expected':checked,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as e:value['error']=str(e)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==86;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
