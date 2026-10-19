"""Run pure scheduler-rule oracles and bounded real policy probes in owned guests."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/realtime_depth/linux';BASE=ROOT/'reviews/v5/expansion/legacy_realtime_linux'
KERNEL_ROOT=Path('/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0')
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
GUEST_PROFILES=(('gcc-O0','gcc',['-O0']),('gcc-O3','gcc',['-O3']),('clang-O3','clang',['-O3']))
def module(name):
 spec=importlib.util.spec_from_file_location('lc_'+name,PUBLIC/(name+'.py'));value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
oracle=module('oracle');guest=module('guest')
def cases(cpus):
 if cpus==1:return [('other','all','none','normal'),('deadline','all','none','normal'),('deadline','all','io','normal'),('deadline','all','none','overrun'),('deadline','all','none','infinite')]
 rows=[(mode,'all' if mode=='deadline' else 'one',interference,'normal') for mode in ('other','fifo','rr','deadline') for interference in ('none','batch','io')]
 rows += [(mode,'all' if mode=='deadline' else 'one','none','infinite') for mode in ('other','fifo','rr','deadline')]
 rows += [('deadline','one','none','normal')]+[(mode,'all' if mode=='deadline' else 'one','none','denied') for mode in ('fifo','rr','deadline')]
 return rows+[('deadline','all','none','invalid'),('deadline','all','none','overrun'),('other','all','none','normal'),('rr','all','none','normal')]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main(report,kernel_root,busybox):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use Linux policy successor evidence directory')
 kernel_root=Path(kernel_root).resolve();busybox=Path(busybox).resolve();kernel=kernel_root/'arch/x86/boot/bzImage'
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 paths=[Path(__file__).resolve(),*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts],*[ROOT/('assessor/legacy_realtime/linux_'+name) for name in ('policy.c','probe.c','policy.md')]]
 pins={p.relative_to(ROOT).as_posix():digest(p) for p in paths};ext_paths=[kernel,busybox,*[kernel_root/p for p in ('.config','include/generated/utsrelease.h','kernel/sched/fair.c','kernel/sched/deadline.c')]]
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'external_source_sha256':{str(p):digest(p) for p in ext_paths},'commands':[],'observations':[],
 'scope':'Six-profile finite rule oracle; actual source/configuration-pinned Linux7.0.14 owned one/two-CPU guest class/readback/affinity/interference/refusal/overrun and supervisor experiments.',
 'limits':['Pure selector omits buddy/slice protection, hierarchy, placement and weighted-accounting updates; it is not a Linux scheduler simulator.','Actual debug/KASAN guest under QEMU TCG is not PREEMPT_RT or physical WCET/latency evidence.','Refused classes remain unavailable under their exact affinity/privilege assumptions.','No host scheduler/security changes; no independent learner/human assessment.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None,timeout=180):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=timeout);output=r.stdout+r.stderr
  log=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');log.write_text(output,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':log.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if r.returncode!=expected or marker and marker not in output:raise ValueError(label+': '+output)
  if any(t in output for t in ('ERROR: AddressSanitizer','runtime error:','Kernel panic','BUG: KASAN','general protection fault')):raise ValueError(label+': diagnostic is not semantic acceptance')
  return output,row
 try:
  value['toolchains']={name:run([name,'--version'],name)[0].splitlines()[0] for name in ('gcc','clang','qemu-system-x86_64')}
  config=(kernel_root/'.config').read_text(encoding='utf-8');release=(kernel_root/'include/generated/utsrelease.h').read_text(encoding='utf-8')
  if '7.0.14-cmastery-b06' not in release or any(line not in config.splitlines() for line in ('CONFIG_SMP=y','CONFIG_HIGH_RES_TIMERS=y','CONFIG_PREEMPT_DYNAMIC=y','CONFIG_PREEMPT_LAZY=y')) or 'CONFIG_PREEMPT_RT=y' in config.splitlines():raise ValueError('Wrong inspected guest configuration')
  value['kernel_release']=release.strip();value['configuration']='SMP/HIGH_RES_TIMERS/PREEMPT_DYNAMIC/PREEMPT_LAZY; not PREEMPT_RT'
  input_text='\n'.join(oracle.inputs())+'\n';(logs/'oracle.in').write_text(input_text,encoding='utf-8',newline='\n');value['oracle_input']=(logs/'oracle.in').relative_to(ROOT).as_posix()
  with tempfile.TemporaryDirectory(prefix='linux-policy-depth-') as directory:
   work=Path(directory)
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,'stub'):
     source=PUBLIC/'policy.c' if variant=='stub' else ROOT/'assessor/legacy_realtime/linux_policy.c';binary=work/(profile+'-'+str(variant))
     command=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DLC_MUTANT='+str(variant if isinstance(variant,int) else 0),'-I'+str(PUBLIC),source,PUBLIC/'tests.c','-o',binary]
     _,build=run(command,profile+'-'+str(variant)+'-build');positive=variant==0;output,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'LC_PASS:' if positive else 'LC_CHECK:')
     value['observations'].append({'target':'model','profile':profile,'variant':variant,'passed':True,'build':build,'execution':execution,'binary_sha256':digest(binary)})
     if positive:
      output,execution=run([binary,'stream'],profile+'-oracle',input_text=input_text);value['observations'].append({'target':'oracle','profile':profile,'passed':True,'execution':execution,'oracle':oracle.check_model(output)})
   for profile,compiler,flags in GUEST_PROFILES:
    for variant in (0,3,'stub'):
     source=PUBLIC/'probe.c' if variant=='stub' else ROOT/'assessor/legacy_realtime/linux_probe.c';binary=work/('guest-'+profile+'-'+str(variant))
     command=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-static',*flags,'-DLC_MUTANT='+str(variant if isinstance(variant,int) else 0),'-I'+str(PUBLIC),source,ROOT/'assessor/legacy_realtime/linux_policy.c','-o',binary]
     _,build=run(command,profile+'-'+str(variant)+'-guest-build');elf,audit=run(['readelf','-lW',binary],profile+'-'+str(variant)+'-ELF')
     if 'INTERP' in elf:raise ValueError('Guest probe must be a self-contained static executable')
     if variant==0:
      _,host_guard=run([binary,'fifo','one','none','normal'],profile+'-outside-guest',1,'LC_GUEST_CHECK:requires-owned-image');value.setdefault('host_refusals',[]).append(host_guard)
     for cpus in ((2,1) if variant==0 else (2,)):
      suite=cases(cpus) if variant==0 else [('fifo','one','none','normal')];args=SimpleNamespace(work=work/(profile+'-'+str(variant)+'-'+str(cpus)),kernel=kernel,busybox=busybox,probe=binary,cpus=cpus)
      archive=guest.archive(args,suite);output,execution=run(guest.command(args,archive),profile+'-'+str(variant)+'-'+str(cpus)+'-guest',marker='LC_GUEST_DONE')
      observed=oracle.check_guest(output,suite,cpus,negative=variant!=0)
      retained=logs/(profile+'-'+str(variant)+'-'+str(cpus)+'-init.txt');retained.write_text('\n'.join(' '.join(c) for c in suite)+'\n',encoding='utf-8',newline='\n')
      value['observations'].append({'target':'guest','profile':profile,'variant':variant,'cpus':cpus,'passed':True,'build':build,'elf_audit':audit,'execution':execution,'cases':observed,'binary_sha256':digest(binary),'initramfs_sha256':digest(archive)})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(digest(ROOT/p)==h for p,h in pins.items()) and all(digest(Path(p))==h for p,h in value['external_source_sha256'].items())
 value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==42
 value['log_sha256']={p.relative_to(ROOT).as_posix():digest(p) for p in logs.iterdir()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'observations':len(value['observations']),'guest_cases':sum(len(r.get('cases',[])) for r in value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--kernel-root',type=Path,default=KERNEL_ROOT);p.add_argument('--busybox',type=Path,default=Path('/home/frede/cmastery-b06-tools/usr/bin/busybox'));a=p.parse_args();raise SystemExit(main(a.report,a.kernel_root,a.busybox))
