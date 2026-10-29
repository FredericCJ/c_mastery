"""Explicit imported original guest + learner request adapter, no fallback."""
import argparse,hashlib,importlib.util,json,re,shutil,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
_ANCHOR_TOKEN = re.compile(r"[A-Za-z_]\w*|\d+|[^\s]")


def anchor_spans(text, anchor):
    """Character spans where `anchor`'s tokens occur in order in `text`.

    Same rule as tools/c_source.py: braces in the source that the anchor
    lacks may be skipped only when they balance inside the match, so an
    anchor written for `if (x) stmt;` still finds `if (x) { stmt; }`.
    """
    hay = [(m.group(), m.start(), m.end()) for m in _ANCHOR_TOKEN.finditer(text)]
    needle = _ANCHOR_TOKEN.findall(anchor)
    spans = []
    for start, (token, _, _) in enumerate(hay):
        if not needle or token != needle[0]:
            continue
        j, depth, ok = start, 0, True
        for want in needle:
            while j < len(hay) and hay[j][0] != want and hay[j][0] in "{}":
                depth += 1 if hay[j][0] == "{" else -1
                if depth < 0:
                    ok = False
                    break
                j += 1
            if not ok or j >= len(hay) or hay[j][0] != want:
                ok = False
                break
            j += 1
        if not ok:
            continue
        while depth > 0 and j < len(hay) and hay[j][0] == "}":
            depth -= 1
            j += 1
        if depth == 0:
            spans.append((hay[start][1], hay[j - 1][2]))
    return spans


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def spans(text,anchor,start=0):
    found=[]
    for lo,hi in anchor_spans(text,anchor):
        if lo<start:continue
        while anchor[:1] in (' ','\t') and text[lo-1:lo] in (' ','\t'):lo-=1
        if anchor.endswith('\n'):
            n=re.compile(r'[ \t]*\r?\n').match(text,hi);hi=n.end() if n else hi
        found.append((lo,hi))
    return found
def replace(text,old,new):
    found=spans(text,old)
    if len(found)!=1:raise ValueError('Import anchor drift: '+old[:80])
    return text[:found[0][0]]+new+text[found[0][1]:]
def slice_start(text,anchor,after=None):
    found=spans(text,anchor,after or 0)
    if not found or (after is None and len(found)!=1):raise ValueError('Import boundary drift: '+anchor[:80])
    return found[0][0]
def main(argv=None):
    p=argparse.ArgumentParser()
    for field in ['kernel-source','kernel-lab','adapter-source','build-root']:p.add_argument('--'+field,type=Path,required=True)
    p.add_argument('--compiler',choices=['gcc','clang'],default='gcc');p.add_argument('--optimization',choices=['-O0','-O2'],default='-O2')
    p.add_argument('--cpus',choices=['2','4'],default='2');p.add_argument('--control',default='positive');p.add_argument('--report',type=Path)
    p.add_argument('--scenario',choices=['normal','device-error'],default='normal')
    a=p.parse_args(argv);root=Path.cwd().resolve();source=a.kernel_source.resolve();lab=a.kernel_lab.resolve();adapter=a.adapter_source.resolve()
    folder=a.build_root.resolve()/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');folder.mkdir(parents=True)
    def label(p):return p.relative_to(root).as_posix() if p.is_relative_to(root) else str(p)
    files=[*source.glob('*'),*lab.glob('include/*'),*lab.glob('platform/*'),lab/'tools/guest.py',*adapter.glob('*'),*PUBLIC.glob('*')]
    report={'passed':False,'provenance':'Explicit supplied implementations; this builder never discovers assessor code.',
      'input_sha256':{label(p):sha(p) for p in files if p.is_file()},'commands':[],
      'limits':['QEMU TCG observations, not physical measurements.','Hold and withheld-ack controls alter the trusted observer, not actual device latency.']}
    def run(cmd,expected=0):
        cmd=list(map(str,cmd))
        try:r=subprocess.run(cmd,text=True,capture_output=True,timeout=120)
        except subprocess.TimeoutExpired as e:
            def dec(x):return x.decode(errors='replace') if isinstance(x,bytes) else x or ''
            r=subprocess.CompletedProcess(cmd,124,dec(e.stdout),dec(e.stderr)+'\nAuthor deadline exceeded; not a guest timeout success.')
        report['commands'].append({'command':cmd,'exit':r.returncode,'expected':expected,'stdout':r.stdout,'stderr':r.stderr,'passed':r.returncode==expected})
        if r.returncode!=expected:raise RuntimeError(r.stdout+r.stderr)
        return r.stdout
    try:
        for tool in [a.compiler,'ld','qemu-system-x86_64','grub-mkrescue']:run([tool,'--version'])
        spec=importlib.util.spec_from_file_location('guest',lab/'tools/guest.py');guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
        stage=folder/'source';stage.mkdir()
        for path in source.glob('*'):
            if path.is_file():shutil.copyfile(path,stage/path.name)
        for name in ['async.inc','request.c']:shutil.copyfile(adapter/name,stage/name)
        boot=replace((stage/'boot.S').read_text(),'-(0xe85250d6+24)','(-(0xe85250d6+24)) & 0xffffffff');(stage/'boot.S').write_text(boot)
        kernel=(stage/'kernel.c').read_text()
        old='__atomic_fetch_add(&ticks,1,__ATOMIC_RELAXED)'
        ticks=spans(kernel,old)
        if len(ticks)!=2:raise ValueError('Tick anchor drift')
        for lo,hi in reversed(ticks):kernel=kernel[:lo]+'atomic_fetch_add_explicit(&ticks,1,memory_order_relaxed)'+kernel[hi:]
        kernel='#include <stdatomic.h>\n'+kernel
        kernel=replace(kernel,'return syscall_v2(f);','return f->rax>=RQA_CANCEL?rqa_syscall(f):syscall_v2(f);')
        lo=slice_start(kernel,'static void complete(void)');hi=slice_start(kernel,'static struct frame *schedule(void)',lo)
        kernel=kernel[:lo]+'static void complete(void){if(primary_failure)panic("rqa user failure");say("PASS rqa-guest-complete\\n");stop(1);}\n'+kernel[hi:]
        lo=slice_start(kernel,'  if (load_task(boot_mode == 2');hi=slice_start(kernel,'  say("PASS elf-load',lo)
        kernel=kernel[:lo]+'  if(load_task("init",1))panic("rqa init ELF");\n'+kernel[hi:]
        (stage/'kernel.c').write_text(kernel)
        os_text=(stage/'os.inc').read_text()
        os_text=replace(os_text,'if(io_request.state==2&&io_request.owner==id)io_request.state=0;','rqa_process_exit(id);')
        os_text=replace(os_text,'case CMK2_BLOCK_STATUS:if(!io_request.state||io_request.owner!=pid||a!=io_request.ticket)return CMK2_EINVAL;return io_request.state==1?CMK2_EAGAIN:io_request.result;','case CMK2_BLOCK_STATUS:return rqa_status(a);')
        (stage/'os.inc').write_text(os_text)
        flags=[a.optimization if f=='-O2' else f for f in guest.FLAGS]+['-Werror','-I'+str(lab/'include'),'-I'+str(PUBLIC),'-I'+str(stage)]
        if a.control!='positive':flags+=['-D'+a.control]
        if a.scenario=='device-error':flags+=['-DRQA_ERROR']
        objects=[]
        for name in ['boot.S','interrupts.S','kernel.c','model.c','ap.S','smp.c','cmk2_model.c']:
            obj=folder/(name+'.o');run([a.compiler,*flags,'-c',stage/name,'-o',obj]);objects.append(obj)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/link.ld','-Map',folder/'kernel.map','-o',folder/'kernel.elf',*objects])
        objects=[]
        for name in ['user.c','bytes.c']:
            obj=folder/(name+'.o');run([a.compiler,*flags,'-c',PUBLIC/name,'-o',obj]);objects.append(obj)
        run(['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-Map',folder/'user.map','-o',folder/'init.elf',*objects])
        for elf in folder.glob('*.elf'):
            if run(['nm','-u',elf]).strip() or 'NEEDED' in run(['readelf','-d',elf]):raise RuntimeError('Forbidden runtime closure')
        blob=guest.pack([('init',(folder/'init.elf').read_bytes())]);(folder/'init.cmka').write_bytes(blob)
        iso=folder/'iso';(iso/'boot/grub').mkdir(parents=True)
        for name in ['kernel.elf','init.cmka']:shutil.copyfile(folder/name,iso/'boot'/name)
        (iso/'boot/grub/grub.cfg').write_text('set timeout=0\nmenuentry "RQ" {\n multiboot2 /boot/kernel.elf write\n module2 /boot/init.cmka init\n boot\n}\n')
        run(['grub-file','--is-x86-multiboot2',folder/'kernel.elf']);run(['grub-mkrescue','-o',folder/'write.iso',iso])
        disk=Path(tempfile.mkdtemp(prefix='cmastery-request-'))/'disk.raw'
        with disk.open('wb') as out:out.truncate(4*1024*1024)
        cmd=guest.command(folder,'write');cmd[cmd.index('-smp')+1]=a.cpus;cmd[cmd.index('-drive')+1]='if=none,id=disk,format=raw,cache=writeback,file='+str(disk)
        if a.scenario=='device-error':
            config=folder/'blkdebug.conf';config.write_text('[inject-error]\nevent = "write_aio"\nerrno = "5"\nsector = "96"\nonce = "on"\n')
            cmd[cmd.index('-drive')+1]='if=none,id=disk,format=raw,cache=writeback,werror=report,rerror=report,file=blkdebug:'+str(config)+':'+str(disk)
            report['injection']={'path':str(config),'sha256':sha(config),'contents':config.read_text(),'host_errno':'Linux EIO=5; actual installed asm-generic/errno-base.h inspected'}
        code=33 if a.control=='positive' else 35;output=run(cmd,code)
        markers=['PASS rqa-two-outstanding-cancel-coalesced','PASS rqa-two-blocked-waiters','PASS rqa-reset-ack-unknown-reuse','PASS rqa-131074-transfers-two-wraps','PASS rqa-owner-exit-keeps-request','PASS rqa-guest-complete']
        if a.scenario=='device-error':markers=['PASS rqa-device-error-retry','PASS rqa-guest-complete']
        controls={'RQ_MUTANT_CANCEL':'RQA_CHECK:cancel intent','RQ_MUTANT_RESET':'RQA_CHECK:reset intent retains request','RQA_MUTANT_ACK':'RQA_CHECK:withheld acknowledgement retains request','RQA_MUTANT_STATUS':'RQA_CHECK:device error propagated'}
        expected=markers if a.control=='positive' else [controls[a.control]]
        if not all(m in output for m in expected) or (a.control=='positive' and ('FAIL' in output or 'RQA_CHECK' in output)):raise RuntimeError('Wrong target result')
        report.update(passed=True,markers=expected,compiler=a.compiler,optimization=a.optimization,cpus=int(a.cpus),control=a.control,scenario=a.scenario,
          kernel_sha256=sha(folder/'kernel.elf'),archive_sha256=sha(folder/'init.cmka'),strict_link='Full linked kernel and user: no unresolved symbols or NEEDED entries.',disk=str(disk))
        if any(sha(root/p)!=h for p,h in report['input_sha256'].items()):raise RuntimeError('Input changed during execution')
    except Exception as e:report.update(passed=False,error=str(e))
    target=a.report or folder/'report.json';target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'passed':report['passed'],'report':str(target),'error':report.get('error')}));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
