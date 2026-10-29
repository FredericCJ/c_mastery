"""Execute T53.04 mailbox, real APIC modes, timer and completion controls."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/interrupts';BASE=ROOT/'reviews/v5/expansion/legacy_smp_interrupts'
NATIVE_PROFILES=[*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie'])]
def load(name):
 spec=importlib.util.spec_from_file_location('smp_interrupt_'+name,PUBLIC/(name+'.py'));value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
oracle=load('oracle');guest=load('guest')
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('interrupts.c','interrupts_guest.c','interrupts_frames.S','interrupts.md','startup.c','startup_guest.c','startup_trampoline.S')]]
 for folder in (PUBLIC,ROOT/'labs/smp_depth/startup',ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):
  paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use interrupt successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original finite C mailbox and struct lr_cpu/generation collector; actual xAPIC/x2APIC timer/IPI/EOI guest, retained delayed work and separated C-handler/work TSC observations.','limits':['No physical timer calibration, arbitrary APIC mode, whole C memory-model proof, independent review or learner acceptance.','ISR costs exclude assembly entry/exit and are virtual TSC measurements, not physical WCET.','TSC-deadline is not the selected executed variant.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None,timeout=180):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=timeout);text=result.stdout+result.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in text:raise ValueError(label+': '+text)
  if any(s in text for s in ('ERROR: AddressSanitizer','WARNING: ThreadSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Fault is not semantic evidence: '+label)
  return text,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='smp-interrupt-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)];model_input='\n'.join(oracle.inputs())+'\n';(logs/'model.in').write_text(model_input,encoding='utf-8',newline='\n')
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,4,'stub'):
     source=PUBLIC/'interrupts.c' if variant=='stub' else ROOT/'assessor/legacy_smp/interrupts.c';binary=work/(profile+'-'+str(variant));positive=variant==0
     _,build=run([compiler,*common,*flags,'-DSI_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'SI_PASS:' if positive else 'SI_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
     if positive:
      trace=work/(profile+'-trace');_,build=run([compiler,*common,*flags,source,PUBLIC/'trace.c','-o',trace],profile+'-trace-build');text,execution=run([trace],profile+'-oracle',input_text=model_input);value['observations'].append({'target':'oracle','profile':profile,'build':build,'execution':execution,'expected':oracle.check_model(text),'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([compiler,*common,*flags,'-pthread',ROOT/'assessor/legacy_smp/interrupts.c',PUBLIC/'native.c','-o',binary],profile+'-native-build');_,execution=run([binary],profile+'-native',marker='SI_NATIVE_PASS:4-exclusive-consumers-80000-batches-320000-items');value['observations'].append({'target':'native','profile':profile,'build':build,'execution':execution,'batches':80000,'items':320000,'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for mode,variant in ((0,0),(1,0),(0,1),(0,2),(1,1),(1,2),(0,'stub')):
     label=profile+'-'+str(mode)+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     startup,g,instrument=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/startup',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',folder,mode,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/guest/build.json').read_bytes())
     if variant==0:
      elf=folder/'image/guest/kernel.elf';text,nm=run(['nm','-u',elf],label+'-undefined')
      if text.strip():raise ValueError('Unexpected target dependencies')
      asm,assembly=run(['objdump','-dr',folder/'image/guest/ap.S.o'],label+'-assembly');c_asm,c_dis=run(['objdump','-dr',folder/'image/guest/smp.c.o'],label+'-c-assembly')
      if 'iretq' not in asm or 'mfence' not in c_asm or 'wrmsr' not in c_asm:raise ValueError('Missing actual ISA mechanism')
      value['observations'].append({'target':'audit','profile':profile,'mode':mode,'undefined':nm,'assembly':assembly,'c_assembly':c_dis,'passed':True})
     matrix=[(n,'max' if mode else 'qemu64',variant) for n in (1,2,4)] if variant==0 else [(2,'max' if mode else 'qemu64',variant)]
     if mode==1 and variant==0:matrix.append((1,'qemu64','unsupported'))
     for cpus,model,outcome in matrix:
      marker='SI_GUEST_PASS:' if outcome==0 else 'SI_GUEST_CHECK:'+{1:'ack-before-work',2:'missing-EOI-outstanding','stub':'unsolved-interrupt-adapter','unsupported':'x2APIC-feature-unavailable'}[outcome]
      text,execution=run(startup.command(g,folder/'image',cpus,model),label+'-'+str(cpus)+'-'+model,33 if outcome==0 else 35,marker,timeout=90)
      checked=oracle.check_guest(text,cpus,mode) if outcome==0 else {'intended_rejection':marker}
      value['observations'].append({'target':'guest','profile':profile,'mode':mode,'variant':outcome,'build_variant':variant,'cpus':cpus,'model':model,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':instrument,'expected':checked,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/path)==digest for path,digest in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==92;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
