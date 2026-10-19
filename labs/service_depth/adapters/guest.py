"""Build and exercise explicitly supplied service/kernel/store implementations.

No assessor discovery, download, or fallback; supplied source trees are read-only.
The staged hooks implement the published integration protocol, not its mechanisms.
"""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import re
import struct
import subprocess
import sys
import tempfile
PUBLIC=Path(__file__).resolve().parent

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def patch_once(text,old,new):
    if text.count(old)!=1:raise ValueError('Adapter import anchor drift: '+old[:70])
    return text.replace(old,new)
def replace_function(text,start,end,body):
    if text.count(start)!=1 or text.count(end)!=1:raise ValueError('Function boundary drift')
    lo=text.index(start);hi=text.index(end,lo)
    return text[:lo]+body+'\n'+text[hi:]
def main(argv=None, provenance="explicit-user-supplied-imports", extra_inputs=()):
    p=argparse.ArgumentParser();p.add_argument('--report',type=Path);p.add_argument('--only',default='positive');p.add_argument('--cpus',default='2');p.add_argument('--suite',choices=['services','receipts'],default='services');p.add_argument('--cut',type=int,default=-1);p.add_argument('--compiler',default='gcc');p.add_argument('--optimization',choices=['-O0','-O2'],default='-O2')
    p.add_argument('--kernel-source',type=Path,required=True)
    p.add_argument('--kernel-lab',type=Path,required=True)
    p.add_argument('--adapter-source',type=Path,required=True)
    p.add_argument('--loader-source',type=Path,help='Explicit separate loader import for staged integration; no automatic fallback')
    p.add_argument('--store-source',type=Path,required=True)
    p.add_argument('--storage-lab',type=Path,required=True)
    p.add_argument('--build-root',type=Path,required=True)
    args=p.parse_args(argv)
    ROOT=Path.cwd().resolve()
    SOURCE=args.kernel_source.resolve();LAB=args.kernel_lab.resolve();ADAPTER=args.adapter_source.resolve()
    STORAGE=args.storage_lab.resolve();STORE=args.store_source.resolve()
    spec=importlib.util.spec_from_file_location('cmk_guest',LAB/'tools/guest.py')
    guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
    def label(path):
        path=path.resolve()
        return path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else str(path)

    folder=args.build_root.resolve()/args.only/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');folder.mkdir(parents=True,exist_ok=False)
    report={'schema_version':1,'started_utc':datetime.now(timezone.utc).isoformat(),'passed':False,'commands':[],
      'limits':['Actual QEMU TCG guest; no physical timing or independent learner evidence.','Imported artifact provenance: '+provenance]}
    inputs=list(SOURCE.glob('*'))+list((LAB/'include').glob('*'))+list((LAB/'platform').glob('*'))+[LAB/'tests/spawn_user.c',LAB/'tools/guest.py',Path(__file__)]+list(ADAPTER.glob('*'))+[PUBLIC/'kernel_abi.h']
    inputs += [STORE,STORAGE/'include/store.h',PUBLIC/'receipt.h']
    inputs += list((PUBLIC/'tests').glob('*.c'))+list((PUBLIC/'freestanding').glob('*'))+list(extra_inputs)
    if args.loader_source:inputs.append(args.loader_source.resolve())
    report['input_sha256']={label(x):sha(x) for x in inputs if x.is_file()}
    def run(cmd,expected=0):
        try:r=subprocess.run(list(map(str,cmd)),cwd=ROOT,text=True,capture_output=True,timeout=120)
        except subprocess.TimeoutExpired as error:
            def decoded(s):return s.decode('utf-8','replace') if isinstance(s,bytes) else (s or '')
            r=subprocess.CompletedProcess(list(map(str,cmd)),124,decoded(error.stdout),decoded(error.stderr)+'\nAuthor deadline exceeded120seconds; not a guest timeout success.')
        report['commands'].append({'command':list(map(str,cmd)),'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'expected':expected})
        if r.returncode!=expected:raise RuntimeError(str(cmd)+'\n'+r.stdout+r.stderr)
        return r.stdout
    try:
        for tool in [args.compiler,'ld','qemu-system-x86_64','grub-mkrescue']:run([tool,'--version'])
        staged=folder/'source';staged.mkdir(exist_ok=True)
        for path in SOURCE.glob('*'):
            if path.is_file():shutil.copyfile(path,staged/path.name)
        # Preserve the retained source; encode its modulo-2^32 Multiboot checksum
        # explicitly so both GNU as and Clang's integrated assembler accept it.
        boot=(staged/'boot.S').read_text()
        boot=patch_once(boot,'-(0xe85250d6+24)','(-(0xe85250d6+24)) & 0xffffffff')
        (staged/'boot.S').write_text(boot)
        for path in ADAPTER.glob('*'):
            if path.is_file():shutil.copyfile(path,staged/path.name)
        if args.loader_source:shutil.copyfile(args.loader_source.resolve(),staged/'load.inc')
        shutil.copyfile(PUBLIC/'kernel_abi.h',staged/'kernel_abi.h')
        kernel=(SOURCE/'kernel.c').read_text()
        # ISO C atomic operation on the retained _Atomic object; GNU __atomic
        # accepts this extension but Clang correctly rejects that operand form.
        kernel='#include <stdatomic.h>\n'+kernel
        if kernel.count('__atomic_fetch_add(&ticks,1,__ATOMIC_RELAXED)')!=2:raise ValueError('Atomic tick import drift')
        kernel=kernel.replace('__atomic_fetch_add(&ticks,1,__ATOMIC_RELAXED)','atomic_fetch_add_explicit(&ticks,1,memory_order_relaxed)')
        kernel=patch_once(kernel,'UT_IO, UT_DEAD','UT_IO, UT_SD_WAIT, UT_DEAD')
        kernel=replace_function(kernel,'static int load_task(', 'static void activate(', '#include "load.inc"')
        kernel=patch_once(kernel,'#include "os.inc"','static void sd_process_cleanup(unsigned);\nstatic void sd_fork_files(unsigned,unsigned);\nstatic void sd_thread_cleanup(unsigned);\n#include "os.inc"\n#include "services.inc"')
        kernel=patch_once(kernel,'return syscall_v2(f);','return f->rax>=SDK_LOAD_PROBE?sd_syscall(f):syscall_v2(f);')
        kernel=replace_function(kernel,'static void complete(void)', 'static struct frame *schedule(void)',
          'static void complete(void){if(primary_failure)panic("sd user failure");say("PASS sd-guest-complete\\n");stop(1);}')
        lo=kernel.index('  if (load_task(boot_mode == 2');hi=kernel.index('  say("PASS elf-load',lo)
        kernel=kernel[:lo]+'  if(load_task("init",1))panic("sd init ELF");\n'+kernel[hi:]
        (staged/'kernel.c').write_text(kernel)
        os_text=(SOURCE/'os.inc').read_text();os_text=patch_once(os_text,'struct task *p=&tasks[id];p->status=status;', 'sd_process_cleanup(id);struct task *p=&tasks[id];p->status=status;')
        os_text=patch_once(os_text,'uthreads[tid].frame->rax=0;if(child==task_count)', 'sd_fork_files(pid,child);uthreads[tid].frame->rax=0;if(child==task_count)')
        os_text=patch_once(os_text,'struct uthread *u=&uthreads[current];u->status=(int)a;', 'sd_thread_cleanup(current);struct uthread *u=&uthreads[current];u->status=(int)a;')
        (staged/'os.inc').write_text(os_text)
        async_text=(SOURCE/'async.inc').read_text()
        async_text=patch_once(async_text,'if(ticks>io_request.deadline){disk_ready=0;', 'if(ticks>io_request.deadline){say("SD_IO_TIMEOUT ticks ");hex(ticks);say(" deadline ");hex(io_request.deadline);say(" irq ");hex(disk_irq_sequence);say(" before ");hex(io_request.irq_before);say(" used ");hex(vu.idx);say(" expected ");hex(used_idx);say("\\n");disk_ready=0;')
        async_text=patch_once(async_text,'if(vu.ring[slot].id!=0||vu.idx!=(uint16_t)(used_idx+1)||request_status){disk_ready=0;', 'if(vu.ring[slot].id!=0||vu.idx!=(uint16_t)(used_idx+1)||request_status){say("SD_IO_COMPLETION_ERROR\\n");disk_ready=0;')
        (staged/'async.inc').write_text(async_text)
        flags=[args.optimization if f=='-O2' else f for f in guest.FLAGS]+['-Werror','-I'+str(LAB/'include'),'-I'+str(staged),'-I'+str(STORAGE/'include'),'-I'+str(PUBLIC)]
        if args.only!='positive':flags+=['-D'+args.only]
        objects=[]
        for name in ['boot.S','interrupts.S','kernel.c','model.c','ap.S','smp.c','cmk2_model.c']:
            obj=folder/(name+'.o');run([args.compiler,*flags,'-c',staged/name,'-o',obj]);objects.append(obj)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',LAB/'platform/link.ld','-Map',folder/'kernel.map','-o',folder/'kernel.elf',*objects])
        items=[]
        for name,path in [('init',PUBLIC/'tests'/('user.c' if args.suite=='services' else 'receipt_user.c')),('spawn',LAB/'tests/spawn_user.c')]:
            obj=folder/(name+'.o');elf=folder/(name+'.elf');run([args.compiler,*flags,'-c',path,'-o',obj]);parts=[obj]
            if name=='init' and args.suite=='services':
                # Clang O0 emits memset for aggregate initialization even with
                # -ffreestanding/-fno-builtin. Supply the original byte loops,
                # then audit the linked image; flags alone are not closure.
                helper=folder/'user-bytes.o'
                run([args.compiler,*flags,'-c',PUBLIC/'freestanding/bytes.c','-o',helper]);parts.append(helper)
            if name=='init' and args.suite=='receipts':
                for stem,source in [('receipt',ADAPTER/'receipt.c'),('store',STORE)]:
                    part=folder/(stem+'.o');run([args.compiler,*flags,'-c',source,'-o',part]);parts.append(part)
            run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',LAB/'platform/user.ld','-o',elf,*parts]);items.append((name,elf.read_bytes()))
        items.append(('data',b'abcdefghijklmnopqrstuvwxyz0123456789\n'))
        archive=guest.pack(items);(folder/'init.cmka').write_bytes(archive)
        for elf in folder.glob('*.elf'):
            if run(['nm','-u',elf]).strip():raise RuntimeError('unresolved runtime closure')
            if 'NEEDED' in run(['readelf','-d',elf]):raise RuntimeError('dynamic target dependency')
        iso=folder/'iso';(iso/'boot/grub').mkdir(parents=True,exist_ok=True)
        shutil.copyfile(folder/'kernel.elf',iso/'boot/kernel.elf');shutil.copyfile(folder/'init.cmka',iso/'boot/init.cmka')
        run(['grub-file','--is-x86-multiboot2',folder/'kernel.elf'])
        for mode in ['write','verify'] if args.suite=='receipts' else ['write']:
            (iso/'boot/grub/grub.cfg').write_text('set timeout=0\nmenuentry "SD" {\n multiboot2 /boot/kernel.elf '+mode+'\n module2 /boot/init.cmka init\n boot\n}\n')
            run(['grub-mkrescue','-o',folder/(mode+'.iso'),iso])
        if args.suite=='receipts':
            # The unchanged guest deadline is100ticks. Use a native Linux file
            # rather than an implicitly slow Windows/DrvFS flush bridge. Retain
            # this ordinary synthetic disk for diagnosis; no host block device.
            disk_folder=Path(tempfile.mkdtemp(prefix='cmastery-sd-disk-'))
            disk_path=disk_folder/'disk.raw'
            report['disk_environment']={'path':str(disk_path),'filesystem':run(['findmnt','-T',disk_folder,'-n','-o','FSTYPE,SOURCE,TARGET']),
                'guest_deadline_ticks':100,'cache':'writeback','limits':'Guest reboot/flush contract only; not host power loss or physical durability.'}
            cases=[];cuts=[0] if args.cut<0 else [args.cut]
            index=0
            while index<len(cuts):
                cut=cuts[index];index+=1
                with disk_path.open('wb') as out:out.write(struct.pack('<I',cut));out.truncate(4*1024*1024)
                for mode in ['write','verify']:
                    cmd=guest.command(folder,mode);cmd[cmd.index('-smp')+1]=args.cpus
                    cmd[cmd.index('-drive')+1]='if=none,id=disk,format=raw,cache=writeback,file='+str(disk_path)
                    output=run(cmd,33);(folder/f'cut{cut}-{mode}.log').write_text(output)
                    expected='PASS sd-reboot-receipt-retention' if mode=='verify' else ('PASS sd-crash-cut' if cut else 'PASS sd-in-guest-owner-restart')
                    if expected not in output or 'SD_CHECK:' in output or 'FAIL' in output:raise RuntimeError('Receipt guest failed at '+str(cut)+'/'+mode+'\n'+output)
                    cases.append({'cut':cut,'mode':mode,'marker':expected,'disk_sha256':sha(disk_path)})
                    if mode=='write' and not cut and args.cut<0:
                        events=int(re.search(r'SD_EVENTS (\d+)',output)[1]);cuts.extend(range(1,events+1))
            report.update(passed=True,status='passed',cases=cases,cpus=int(args.cpus),suite=args.suite,
                strict_link='Actual full kernel/user ELF closure: no undefined symbols or dynamic dependencies.',kernel_sha256=sha(folder/'kernel.elf'),archive_sha256=sha(folder/'init.cmka'))
            if any(sha(ROOT/path)!=value for path,value in report['input_sha256'].items()):raise RuntimeError('Sources changed during campaign')
            dest=args.report or folder/'report.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':True,'report':str(dest),'cases':len(cases)}));return 0
        with (folder/'disk.raw').open('wb') as out:out.truncate(4*1024*1024)
        cmd=guest.command(folder,'write');cmd[cmd.index('-smp')+1]=args.cpus
        expected=33 if args.only=='positive' else 35
        output=run(cmd,expected);(folder/'guest.log').write_text(output)
        markers=['PASS sd-elf-allocation-rollback','PASS sd-archive-file-table','PASS sd-file-clone-snapshot-live','PASS sd-protected-channel-cancel-copy','PASS sd-partial-copy-dead-incarnation','PASS sd-immutable-shared-variant','PASS sd-restricted-cow-tlb-quiescence','PASS sd-guest-complete']
        rejection={'SDK_MUTANT_LOAD_LEAK':'SD_CHECK:load rollback','SDK_MUTANT_RIGHTS':'SD_CHECK:directional rights','SDK_MUTANT_OFFSET':'SD_CHECK:read rejection preserves offset','SDK_MUTANT_CANCEL':'SD_CHECK:cancel clears waiter'}
        rejection.update(SDK_MUTANT_FILE_FORK='SD_CHECK:child shared offset',SDK_MUTANT_SNAPSHOT='SD_CHECK:snapshot retains old backing',SDK_MUTANT_PARTIAL='SD_CHECK:partial retry exact suffix',SDK_MUTANT_IMMUTABLE='SD_CHECK:immutable write admitted')
        if args.only=='positive':
            if 'FAIL' in output or 'SD_CHECK:' in output or not all(x in output for x in markers):raise RuntimeError('Missing positive evidence')
        elif rejection.get(args.only,'MISSING_CONTROL') not in output:raise RuntimeError('Control rejected for wrong reason')
        report.update(passed=True,status='passed',markers=markers if args.only=='positive' else [rejection[args.only]],
          kernel_sha256=sha(folder/'kernel.elf'),archive_sha256=sha(folder/'init.cmka'),strict_link='No undefined symbols or dynamic dependencies in full kernel/user images.',cpus=int(args.cpus))
        if any(sha(ROOT/path)!=value for path,value in report['input_sha256'].items()):raise RuntimeError('Sources changed during campaign')
    except Exception as error:report.update(passed=False,status='failed',error=str(error))
    dest=args.report or folder/'report.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'passed':report['passed'],'report':str(dest),'error':report.get('error')}));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
