"""Version/configuration changed guest, actual copied requests and external requeue."""
import datetime,hashlib,json,pathlib,shutil,stat,subprocess,sys,time
from run_b06_kernel_guest import newc
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_kernel';PUBLIC=ROOT/'labs/advanced_concurrency_depth/kernel/guest';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/kernel/guest';BUILD=ROOT/'build'/('advanced-kernel-guest-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));BASE=pathlib.Path('/home/frede/cmastery-b06-tools')
KERNELS={'packaged':(BASE/'usr/src/linux-headers-7.0.0-38-generic',BASE/'boot/vmlinuz-7.0.0-38-generic','7.0.0-38-generic'),'debug':(BASE/'debug-kernel/linux-source-7.0.0',BASE/'debug-kernel/linux-source-7.0.0/arch/x86/boot/bzImage','7.0.14-cmastery-b06')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[pathlib.Path(__file__),ROOT/'tools/run_b06_kernel_guest.py',ROOT/'labs/kernel_lifecycle/guest/probe.c',*[ROOT/'assessor/kernel_lifecycle/guest'/n for n in ['b06_owner.c','b06_irq.c','Makefile']],*[p for t in [PUBLIC,PRIVATE] for p in t.iterdir() if p.is_file()]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in files}
def external():return {str(p):sha(p) for p in [BASE/'usr/bin/busybox',*[p for k,image,_ in KERNELS.values() for p in [image,k/'.config',k/'include/generated/utsrelease.h',k/'Module.symvers']]]}
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();commands=[];outputs={};failure=None;observations=[]
 def run(ident,args,expected=0,markers=(),timeout=180):
  start=time.monotonic()
  try:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=timeout);text=r.stdout;code=r.returncode
  except subprocess.TimeoutExpired as e:text=str(e);code=124
  log=BUILD/(ident+'.log');log.write_text(text,encoding='utf8');outputs[log.relative_to(ROOT).as_posix()]=sha(log);ok=code==expected and all(m in text for m in markers);commands.append(dict(id=ident,command=list(map(str,args)),exit_code=code,expected_exit=expected,markers=list(markers),passed=ok,elapsed_seconds=time.monotonic()-start,output=log.relative_to(ROOT).as_posix(),output_sha256=sha(log)));print(ident,'PASS' if ok else 'FAIL',flush=True);assert ok,(ident,text[-2500:]);return text
 try:
  run('compiler',['gcc-15','--version']);run('emulator',['qemu-system-x86_64','--version']);probe=BUILD/'probe';run('probe.build',['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-static',ROOT/'labs/kernel_lifecycle/guest/probe.c','-o',probe]);outputs[probe.relative_to(ROOT).as_posix()]=sha(probe)
  for tag,(kernel,image,release) in KERNELS.items():
   modules={}
   for label,source in [('reference',PRIVATE),('learner',PUBLIC),('retained',ROOT/'assessor/kernel_lifecycle/guest')]:
    work=BUILD/(tag+'.'+label);work.mkdir()
    for p in source.iterdir():
     if p.is_file() and p.suffix in ['.c',''] :shutil.copyfile(p,work/p.name)
    run(tag+'.'+label+'.build',['make','-C',kernel,'M='+str(work),'CC=gcc-15','modules'])
    names=['ak_work'] if label=='reference' else ['ak_work_stub'] if label=='learner' else ['b06_owner','b06_irq']
    for name in names:
     module=work/(name+'.ko');modules[name]=module;outputs[module.relative_to(ROOT).as_posix()]=sha(module);run(tag+'.'+name+'.abi',['modinfo','-F','vermagic',module],markers=(release,));run(tag+'.'+name+'.imports',['nm','-u',module])
   init='''#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
fail(){ echo AK_GUEST_FAIL; dmesg; poweroff -f; }
uname -a
if [ -f /mismatched.ko ]; then
 if insmod /mismatched.ko; then fail; fi
 echo AK_MISMATCHED_BUILD_REJECTED
fi
if insmod /ak_work_stub.ko; then fail; fi
insmod /b06_irq.ko || fail
rmmod b06_irq || fail
insmod /b06_owner.ko || fail
/probe || fail
rmmod b06_owner || fail
i=0
while [ "$i" -lt 10 ]; do insmod /ak_work.ko || fail; rmmod ak_work || fail; i=$((i+1)); done
if insmod /ak_work.ko false_drain=1; then fail; fi
dmesg
if dmesg | grep -E 'BUG:|WARNING:|KASAN:|possible circular locking|scheduling while atomic'; then fail; fi
echo AK_GUEST_PASS cycles=10 original_usercopy=1 contexts=1 external_requeue=1
poweroff -f
'''
   entries=[('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,(BASE/'usr/bin/busybox').read_bytes()),('init',stat.S_IFREG|0o755,init.encode()),('probe',stat.S_IFREG|0o755,probe.read_bytes())]+[(name+'.ko',stat.S_IFREG|0o600,p.read_bytes()) for name,p in modules.items()]
   if tag=='debug':entries.append(('mismatched.ko',stat.S_IFREG|0o600,(BUILD/'packaged.reference/ak_work.ko').read_bytes()))
   archive=BUILD/(tag+'.cpio');archive.write_bytes(newc(entries));outputs[archive.relative_to(ROOT).as_posix()]=sha(archive)
   text=run(tag+'.guest',['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',image,'-initrd',archive,'-append','console=ttyS0 rdinit=/init panic=-1 quiet'],markers=(release,'AK_GUEST_PASS cycles=10 original_usercopy=1 contexts=1 external_requeue=1','AK_WORK_NEGATIVE no_racing_enqueue_required=1','AK_WORK_STUB unimplemented_admission_and_drain','B06_IRQ_PASS task=1 hardirq=1 threaded=1','AK_WORK_UNLOADED retained_callbacks=0'))
   assert 'AK_GUEST_FAIL' not in text
   if tag=='debug':assert 'AK_MISMATCHED_BUILD_REJECTED' in text and '.gnu.linkonce.this_module section size must match' in text,'Expected actual struct-module ABI mismatch rejection'
   import re
   times=set(re.findall(r'\[\s*([\d.]+)\] AK_WORK_PASS runs=16 external_requeues=15 gate_closed=1 producer_joined=1 work_drained=1 timer_drained=1',text));assert len(times)==10,times
   config=(kernel/'.config').read_text();keys=['CONFIG_KASAN','CONFIG_PROVE_LOCKING','CONFIG_DEBUG_ATOMIC_SLEEP','CONFIG_PREEMPT_RT'];options={k:next((s for s in config.splitlines() if s.startswith(k+'=') or s=='# '+k+' is not set'),'absent') for k in keys};observations.append(dict(profile=tag,release=release,config=options,cycles=len(times),runs_per_cycle=16,external_requeues_per_cycle=15,retained_usercopy=True,actual_hardirq_threaded=True,admission_closed_before_join=True,physical=False))
  assert observations[0]['release']!=observations[1]['release'] and observations[0]['config']!=observations[1]['config']
 except Exception as ex:failure=str(ex)
 after=pins();after_ext=external()
 if after!=before or after_ext!=ext:failure='Scoped guest source or kernel changed'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,external_input_sha256=after_ext,outputs_sha256=outputs,commands=commands,observations=observations,limits=['Original modules in two disposable networkless diskless guest kernels only; no host loading or security changes.','Distinct releases/configurations are observed; PREEMPT_RT not enabled and RT behavior remains source analysis only.','Copied user request and real hardirq/threaded context are actual frozen T57 producer code; new requeue module adds the separate external producer.','Timer cancellation and ordinary-work drain are separate; finite schedules do not establish all drivers, callbacks or lifetime edges.','No physical DMA/board, external review or independent learner evidence.'])
 (HERE/'guest_validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(commands),observations=len(observations),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
