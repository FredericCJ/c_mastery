"""Exact pinned-source RCU context experiment; never load on the host."""
import datetime,hashlib,json,pathlib,shutil,stat,subprocess,sys,time
from run_b06_kernel_guest import newc
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_grace';PUBLIC=ROOT/'labs/advanced_concurrency_depth/grace/guest';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/grace/guest';BUILD=ROOT/'build'/('advanced-grace-guest-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
BASE=pathlib.Path('/home/frede/cmastery-b06-tools');KERNEL=BASE/'debug-kernel/linux-source-7.0.0'
EXTERNAL=[KERNEL/'arch/x86/boot/bzImage',KERNEL/'.config',KERNEL/'include/generated/utsrelease.h',KERNEL/'Module.symvers',BASE/'usr/bin/busybox',*[KERNEL/p for p in ['Documentation/RCU/whatisRCU.rst','Documentation/RCU/rcubarrier.rst','include/linux/rcupdate.h','include/linux/hrtimer.h']]]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in [pathlib.Path(__file__),ROOT/'tools/run_b06_kernel_guest.py',*[p for t in [PUBLIC,PRIVATE] for p in t.iterdir() if p.is_file()]]}
def main():
 BUILD.mkdir(parents=True);before=pins();external={str(p):sha(p) for p in EXTERNAL};commands=[];outputs={};failure=None;observations=[]
 def run(ident,args,expected=0,markers=(),timeout=60):
  start=time.monotonic()
  try:r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=timeout);text=r.stdout;code=r.returncode
  except subprocess.TimeoutExpired as e:text=str(e);code=124
  log=BUILD/(ident+'.log');log.write_text(text,encoding='utf8');outputs[log.relative_to(ROOT).as_posix()]=sha(log);ok=code==expected and all(m in text for m in markers);commands.append(dict(id=ident,command=list(map(str,args)),exit_code=code,expected_exit=expected,markers=list(markers),passed=ok,elapsed_seconds=time.monotonic()-start,output=log.relative_to(ROOT).as_posix(),output_sha256=sha(log)));print(ident,'PASS' if ok else 'FAIL',flush=True);assert ok,(ident,text[-3000:]);return text
 try:
  run('compiler',['gcc-15','--version']);run('emulator',['qemu-system-x86_64','--version'])
  for label,source in [('reference',PRIVATE),('learner',PUBLIC)]:
   work=BUILD/label;work.mkdir()
   for p in source.iterdir():
    if p.is_file():shutil.copyfile(p,work/p.name)
   run(label+'.build',['make','-C',KERNEL,'M='+str(work),'CC=gcc-15','modules']);module=work/('ag_rcu.ko' if label=='reference' else 'ag_rcu_stub.ko');outputs[module.relative_to(ROOT).as_posix()]=sha(module)
   run(label+'.imports',['nm','-u',module],markers=('__x86_return_thunk',))
  init='''#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
fail(){ echo AG_GUEST_FAIL; dmesg; poweroff -f; }
uname -a
if insmod /ag_rcu_stub.ko; then fail; fi
i=0
while [ "$i" -lt 10 ]; do insmod /ag_rcu.ko || fail; rmmod ag_rcu || fail; i=$((i+1)); done
if insmod /ag_rcu.ko pretend_early=1; then fail; fi
dmesg
if dmesg | grep -E 'BUG:|WARNING:|KASAN:|possible circular locking|scheduling while atomic'; then fail; fi
echo AG_GUEST_PASS cycles=10 learner_rejected=1 negative_rejected=1
poweroff -f
'''
  entries=[('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,(BASE/'usr/bin/busybox').read_bytes()),('init',stat.S_IFREG|0o755,init.encode()),('ag_rcu.ko',stat.S_IFREG|0o600,(BUILD/'reference/ag_rcu.ko').read_bytes()),('ag_rcu_stub.ko',stat.S_IFREG|0o600,(BUILD/'learner/ag_rcu_stub.ko').read_bytes())];archive=BUILD/'initramfs.cpio';archive.write_bytes(newc(entries));outputs[archive.relative_to(ROOT).as_posix()]=sha(archive)
  text=run('guest',['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',KERNEL/'arch/x86/boot/bzImage','-initrd',archive,'-append','console=ttyS0 rdinit=/init panic=-1 quiet'],markers=('AG_GUEST_PASS cycles=10 learner_rejected=1 negative_rejected=1','AG_RCU_STUB mechanism_unimplemented','AG_RCU_NEGATIVE_DETECTED errors=1 held=0','old=1 new=2 held_grace=1 freed=1 errors=0','AG_RCU_UNLOADED callbacks_drained=1'),timeout=180)
  assert 'AG_GUEST_FAIL' not in text
  import re
  records=re.findall(r'\[\s*([\d.]+)\] AG_RCU_OBSERVATION hardirq=(\d+) old=(\d+) new=(\d+) held_grace=(\d+) freed=(\d+) errors=(\d+)',text)
  records=list(dict.fromkeys(records));assert len(records)==10 and all(int(x[1])>0 and x[2:]==('1','2','1','1','0') for x in records),records
  observations=[dict(kernel_time=x[0],hardirq_mask=int(x[1]),old=1,new=2,held_grace=True,freed=1,errors=0) for x in records]
 except Exception as ex:failure=str(ex)
 after=pins();after_external={str(p):sha(p) for p in EXTERNAL}
 if before!=after or external!=after_external:failure='Scoped source or selected kernel changed during run'
 config=(KERNEL/'.config').read_text();options={k:next((line for line in config.splitlines() if line.startswith(k+'=') or line=='# '+k+' is not set'),'absent') for k in ['CONFIG_PREEMPT_RCU','CONFIG_PREEMPT_RT','CONFIG_KASAN','CONFIG_PROVE_LOCKING','CONFIG_DEBUG_ATOMIC_SLEEP']}
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,external_input_sha256=after_external,outputs_sha256=outputs,commands=commands,observations=observations,kernel_config=options,limits=['Original module loaded only inside disposable diskless networkless two-struct lr_cpu QEMU; no host module loads or policy changes.','Actual Linux hard-interrupt timer context, not a physical IRQ controller/board or another kernel configuration.','The negative control changes the observed grace condition without freeing protected storage.','Ten observed grace/reclamation/unload cycles do not prove every schedule or latency bound.','Native serialized AG model and actual Linux RCU remain distinct contracts; no independent learner/reviewer/physical evidence.'])
 (HERE/'guest_validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(commands),observations=len(observations),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
