"""Build/test original CMK guest; artifacts stay in the selected build directory.

No host disk passthrough, networking, KVM, sudo, or mounting is used. The only
writeable guest storage is a fresh ordinary file in that directory.
"""
from __future__ import annotations
import argparse, hashlib, json, os, select, shutil, struct, subprocess, time
from pathlib import Path

MACHINE = 'pc-i440fx-10.2'
FLAGS = ['-std=c17','-O2','-g','-ffreestanding','-fno-builtin','-fno-pie',
         '-fno-stack-protector','-fno-asynchronous-unwind-tables','-m64',
         '-mno-red-zone','-mgeneral-regs-only','-fstack-usage','-Wall','-Wextra',
         '-Wno-misleading-indentation','-Wno-unused-function']

def run(cmd, **kw):
    p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,**kw)
    if p.returncode:
        raise RuntimeError('Command failed: '+str(cmd)+'\n'+p.stdout+p.stderr)
    return p.stdout

def pack(items):
    start=16+40*len(items); table=bytearray(); body=bytearray()
    for name,blob in items:
        b=name.encode('ascii')
        if not 0<len(b)<24: raise ValueError('archive name')
        table+=struct.pack('<24sIIII',b,start+len(body),len(blob),0,0)
        body+=blob
    return struct.pack('<4sIII',b'CMKA',1,len(items),start+len(body))+table+body

def build(lab,source,folder):
    folder.mkdir(parents=True,exist_ok=True)
    flags=FLAGS+['-I'+str(lab/'include')]
    objects=[]
    for name in ['boot.S','interrupts.S','kernel.c','model.c','ap.S','smp.c','cmk2_model.c']:
        obj=folder/(name+'.o');run(['gcc',*flags,'-c',source/name,'-o',obj]);objects.append(obj)
    run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/link.ld','-Map',folder/'kernel.map','-o',folder/'kernel.elf',*objects])
    unresolved=run(['nm','-u',folder/'kernel.elf']).strip()
    if unresolved: raise RuntimeError('unexpected undefined symbols: '+unresolved)
    (folder/'strict-link.txt').write_text(run(['readelf','-d',folder/'kernel.elf'])+run(['nm','-u',folder/'kernel.elf']))
    run(['grub-file','--is-x86-multiboot2',folder/'kernel.elf'])
    items=[]
    storage=os.environ.get('CMASTERY_STORAGE_ROOT')
    storage_include=os.environ.get('CMASTERY_STORAGE_INCLUDE')
    runtime=os.environ.get('CMASTERY_RUNTIME_ROOT')
    runtime_include=os.environ.get('CMASTERY_RUNTIME_INCLUDE')
    runtime_adapters=os.environ.get('CMASTERY_RUNTIME_ADAPTERS')
    integrated=bool(storage or storage_include)
    if integrated and not all([storage,storage_include,runtime,runtime_include,runtime_adapters]):
        raise ValueError('integrated store requires both store paths, both runtime paths and runtime adapters')
    for name,define in [('init',''),('worker','USER_WORKER'),('fault','USER_FAULT'),('shell','USER_SHELL')]:
        obj=folder/(name+'.o');target=folder/(name+'.elf')
        run(['gcc',*flags,*(['-D'+define] if define else []),*(['-DUSER_STORAGE'] if integrated and name=='init' else []),'-c',lab/'tests/user.c','-o',obj])
        objects=[obj]
        if integrated and name=='init':
            for stem,path in [('integration',lab/'tests/storage_user.c'),('store',Path(storage)/'store.c'),('cvm',Path(runtime)/'cvm.c'),('store_host',Path(runtime_adapters)/'store_host.c')]:
                part=folder/(stem+'.o');run(['gcc',*flags,'-I'+storage_include,'-I'+runtime_include,'-I'+runtime_adapters,'-c',path,'-o',part]);objects.append(part)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-o',target,*objects])
        items.append((name,target.read_bytes()))
    # Optional cross-family linkage is explicit; a standalone copied lab never
    # searches outside itself or silently downloads an implementation.
    if runtime or runtime_include:
        if not runtime or not runtime_include: raise ValueError('both runtime paths required')
        objects=[]
        for name,path in [('vm',lab/'tests/vm_user.c'),('cvm',Path(runtime)/'cvm.c')]:
            obj=folder/(name+'.o');run(['gcc',*flags,'-I'+runtime_include,'-c',path,'-o',obj]);objects.append(obj)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-o',folder/'vm.elf',*objects]);items.append(('vm',(folder/'vm.elf').read_bytes()))
    if integrated:
        service_objects=[]
        for name,path in [('service',source/'service_user.c'),('service_store',Path(storage)/'store.c'),('service_vm',Path(runtime)/'cvm.c'),('service_adapter',Path(runtime_adapters)/'store_host.c')]:
            obj=folder/(name+'.o');run(['gcc',*flags,'-I'+storage_include,'-I'+runtime_include,'-I'+runtime_adapters,'-c',path,'-o',obj]);service_objects.append(obj)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-o',folder/'service.elf',*service_objects])
        items.append(('service',(folder/'service.elf').read_bytes()))
    child=folder/'spawn.o'
    run(['gcc',*flags,'-c',lab/'tests/spawn_user.c','-o',child])
    run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-o',folder/'spawn.elf',child])
    items.append(('spawn',(folder/'spawn.elf').read_bytes()))
    parallel=folder/'parallel.o'
    run(['gcc',*flags,'-c',source/'parallel_user.c','-o',parallel])
    v2=folder/'v2.o'  
    run(['gcc',*flags,'-c',lab/'tests/user_v2.c','-o',v2])
    run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-o',folder/'v2.elf',v2,parallel])
    items.append(('v2',(folder/'v2.elf').read_bytes()))
    blob=pack(items);(folder/'init.cmka').write_bytes(blob)
    iso=folder/'iso';(iso/'boot/grub').mkdir(parents=True,exist_ok=True)
    shutil.copyfile(folder/'kernel.elf',iso/'boot/kernel.elf');shutil.copyfile(folder/'init.cmka',iso/'boot/init.cmka')
    for mode in ['write','verify','interactive']:
        (iso/'boot/grub/grub.cfg').write_text('set timeout=0\nset default=0\nmenuentry "CMK" {\n multiboot2 /boot/kernel.elf '+mode+'\n module2 /boot/init.cmka init\n boot\n}\n')
        run(['grub-mkrescue','-o',folder/(mode+'.iso'),iso])
    report={'built':True,'machine':MACHINE,'compiler':run(['gcc','--version']).splitlines()[0],
            'qemu':run(['qemu-system-x86_64','--version']).splitlines()[0],
            'grub':run(['grub-mkrescue','--version']).strip(),'flags':flags,
            'kernel_sha256':hashlib.sha256((folder/'kernel.elf').read_bytes()).hexdigest(),
            'archive_sha256':hashlib.sha256(blob).hexdigest(),'runtime_linked':bool(runtime),'storage_linked':integrated}
    inputs=sorted(set([*source.glob('*.c'),*source.glob('*.S'),*source.glob('*.inc'),*lab.glob('include/*'),*lab.glob('tests/*'),*lab.glob('platform/*'),*lab.glob('tools/*.py')]))
    inputs.extend([lab/'CMakeLists.txt',lab/'family.json'])
    if runtime: inputs.append(Path(runtime)/'cvm.c')
    if runtime_include: inputs.extend(Path(runtime_include).glob('*.h'))
    if storage_include: inputs.extend(Path(storage_include).glob('*.h'))
    if runtime_adapters: inputs.extend(Path(runtime_adapters).glob('*.h'))
    if storage: inputs.append(Path(storage)/'store.c')
    if runtime_adapters: inputs.append(Path(runtime_adapters)/'store_host.c')
    report['source_sha256']={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs if p.is_file()}
    report['source_root']=str(source)
    report['strict_link']='Direct ld -nostdlib; zero undefined kernel symbols; no dynamic section. All user ELFs audited likewise.'
    for elf in folder.glob('*.elf'):
        if run(['nm','-u',elf]).strip():raise RuntimeError('unresolved user/kernel symbol '+str(elf))
        if 'NEEDED' in run(['readelf','-d',elf]):raise RuntimeError('dynamic dependency '+str(elf))
    (folder/'build.json').write_text(json.dumps(report,indent=2)+'\n')

def command(folder,mode):
    return ['qemu-system-x86_64','-machine',MACHINE,'-accel','tcg,thread=multi','-cpu','qemu64',
      '-smp',os.environ.get('CMASTERY_CPUS','4'),'-m','128M','-display','none','-serial','stdio','-monitor','none',
      '-no-reboot','-net','none','-boot','d','-cdrom',str(folder/(mode+'.iso')),
      '-device','isa-debug-exit,iobase=0xf4,iosize=0x04',
      '-drive','if=none,id=disk,format=raw,cache=writeback,file='+str(folder/'disk.raw'),
      '-device','virtio-blk-pci,drive=disk,disable-legacy=on']

def shell_test(cmd):
    p=subprocess.Popen(cmd,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    output=bytearray();stage=0;deadline=time.monotonic()+20
    try:
        while p.poll() is None and time.monotonic()<deadline:
            ready,_,_=select.select([p.stdout],[],[],0.2)
            if not ready:continue
            chunk=os.read(p.stdout.fileno(),4096)
            if not chunk:break
            output.extend(chunk)
            if stage==0 and b'cmk> ' in output:p.stdin.write(b'help\n');p.stdin.flush();stage=1
            elif stage==1 and b'help | echo TEXT | exit' in output:p.stdin.write(b'exit\n');p.stdin.flush();stage=2
        if p.poll() is None:p.kill()
        output.extend(p.communicate(timeout=2)[0])
        return p.returncode,output.decode('utf-8','replace')
    finally:
        if p.poll() is None:p.kill();p.wait()

def test(folder,required=None):
    if not (folder/'build.json').exists():raise RuntimeError('Build guest first')
    if required and not json.loads((folder/'build.json').read_text()).get(required+'_linked'):
        raise RuntimeError('Required '+required+' was not linked; this integration gate is pending')
    with (folder/'disk.raw').open('wb') as f:f.truncate(4*1024*1024)
    records=[]
    for mode in ['write','verify','interactive']:
        cmd=command(folder,mode)
        if mode=='interactive':status,output=shell_test(cmd);expected=['help | echo TEXT | exit','shell exited']
        else:
            try:
                p=subprocess.run(cmd,capture_output=True,text=True,timeout=25);status=p.returncode;output=p.stdout+p.stderr
            except subprocess.TimeoutExpired as e:
                status=-1; output=(e.stdout or b"").decode("utf-8","replace")+(e.stderr or b"").decode("utf-8","replace")+"\nFAIL host deadline\n"
            expected=['PASS smp-ring3-threads','PASS virtio-msix-completion','PASS v2-spawn-last-thread-wait','PASS smp-affinity-migration','PASS smp-parallel-reduction','PASS smp-reusable-workpool','PASS smp-dag-dependencies','PASS v2-concurrent-zero-cow-faults','PASS v2-shared-object-lifetime','PASS smp-suite','PASS smp-apic-timer-ipi','PASS smp-cooperative-threads','PASS smp-message-accounting','PASS v2-thread-join-blocking-channel','PASS v2-demand-zero','PASS v2-async-block-completion','PASS v2-restricted-cow-wait','PASS kernel-suite','PASS user-kernel-write-denied','PASS user-copy-handles-heap','PASS isolation-preemption',
                      'PASS persistent-read-after-reboot' if mode=='verify' else 'PASS block-write-flush-read']
            if json.loads((folder/'build.json').read_text())['runtime_linked']:expected.append('PASS runtime-in-user-process')
            if json.loads((folder/'build.json').read_text())['storage_linked']:
                expected.extend(['PASS VM-store-recovery-after-reboot' if mode=='verify' else 'PASS VM-store-commit-lost-response', 'PASS v2-VM-client-no-disk-capability', 'PASS v2-owner-store-recovery-after-reboot' if mode=='verify' else 'PASS v2-owner-store-commit-indeterminate-reply'])
        (folder/(mode+'.log')).write_text(output)
        record={'mode':mode,'command':cmd,'exit':status,'expected_markers':expected,
                'passed':status==33 and all(x in output for x in expected) and 'FAIL' not in output}
        records.append(record)
        print(mode,status,record['passed'])
        if not record['passed']:print(output);break
    result={'passed':len(records)==3 and all(x['passed'] for x in records),'runs':records,
            'scope':'QEMU TCG guest execution and ordinary file backend, SMP kernel and preemptive user threads; ABI v1 regression and v2 OS services; emulator evidence, not physical timing'}
    (folder/'execution.json').write_text(json.dumps(result,indent=2)+'\n')
    if not result['passed']:raise SystemExit(1)

def smoke(folder):
    # This first milestone deliberately stops at serial entry. No parser,
    # allocator, user ELF, scheduler, virtqueue, or storage result is required.
    with (folder/'smoke.raw').open('wb') as f:f.truncate(4*1024*1024)
    cmd=command(folder,'write')
    cmd=[x.replace('disk.raw','smoke.raw') for x in cmd]
    p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    output=bytearray();deadline=time.monotonic()+10;passed=False
    try:
        while time.monotonic()<deadline:
            ready,_,_=select.select([p.stdout],[],[],0.2)
            if ready:
                chunk=os.read(p.stdout.fileno(),4096)
                if not chunk:break
                output.extend(chunk)
                if b'CMK v1 boot\r\n' in output or b'CMK v1 boot\n' in output:
                    passed=True;break
            if p.poll() is not None:break
    finally:
        if p.poll() is None:p.terminate()
        try:p.wait(timeout=2)
        except subprocess.TimeoutExpired:p.kill();p.wait()
    (folder/'smoke.log').write_bytes(output)
    (folder/'smoke.json').write_text(json.dumps({'passed':passed,'command':cmd,'scope':'Actual long-mode C entry and serial marker only; emulator deliberately stopped at this boundary.'},indent=2)+'\n')
    print('boot serial smoke',passed)
    if not passed:raise SystemExit(1)

def stage(folder, name):
    markers={'madt':'PASS smp-madt','startup':'PASS smp-startup-percpu','apic':'PASS smp-apic-timer-ipi','threads':'PASS smp-cooperative-threads','channel':'PASS smp-bounded-channel','userthreads':'PASS v2-process-thread-split','spawn':'PASS v2-spawn-last-thread-wait','affinity':'PASS smp-affinity-migration','tlb':'PASS smp-tlb-shootdown-ack','reduction':'PASS smp-parallel-reduction','pool':'PASS smp-reusable-workpool','dag':'PASS smp-dag-dependencies','ipc':'PASS v2-thread-join-blocking-channel','shared':'PASS v2-shared-object-lifetime','async':'PASS v2-async-block-completion','faults':'PASS v2-concurrent-zero-cow-faults','zero':'PASS v2-demand-zero','cow':'PASS v2-restricted-cow-wait'}
    marker=markers[name].encode()
    disk=folder/('stage-'+name+'.raw')
    with disk.open('wb') as f:f.truncate(4*1024*1024)
    cmd=[x.replace('disk.raw',disk.name) for x in command(folder,'write')]
    p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    output=bytearray();passed=False;deadline=time.monotonic()+20
    try:
        while time.monotonic()<deadline:
            ready,_,_=select.select([p.stdout],[],[],0.2)
            if ready:
                chunk=os.read(p.stdout.fileno(),4096)
                if not chunk:break
                output.extend(chunk)
                if marker in output:
                    passed=b'FAIL' not in output;break
            if p.poll() is not None:break
    finally:
        deadline_expired=time.monotonic()>=deadline and p.poll() is None
        if p.poll() is None:p.terminate()
        try:p.wait(timeout=2)
        except subprocess.TimeoutExpired:p.kill();p.wait()
    (folder/('stage-'+name+'.log')).write_bytes(output)
    (folder/('stage-'+name+'.json')).write_text(json.dumps({'stage':name,'passed':passed,'deadline_expired':deadline_expired,'command':cmd,'marker':marker.decode(),'scope':'Actual guest execution through named marker only; later markers do not close later gates.'},indent=2)+'\n')
    print(name,passed)
    if not passed:print(output.decode('utf-8','replace'));raise SystemExit(1)

def main():
    p=argparse.ArgumentParser();p.add_argument('action',choices=['build','test','smoke','stage']);p.add_argument('--lab',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--require',choices=['runtime','storage']);p.add_argument('--stage',choices=['madt','startup','apic','threads','channel','userthreads','spawn','affinity','tlb','reduction','pool','dag','ipc','zero','cow','shared','async','faults']);a=p.parse_args()
    if a.action=='build':build(a.lab.resolve(),a.source.resolve(),a.build.resolve())
    elif a.action=='stage':stage(a.build.resolve(),a.stage)
    elif a.action=='smoke':smoke(a.build.resolve())
    else:test(a.build.resolve(),a.require)
if __name__=='__main__':main()
