"""Explicit source composition for the original disposable SMP teaching guest."""
import argparse,hashlib,importlib.util,json,os,re,shutil,subprocess,tempfile
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def spans(text,anchor,start=0):
 tokens=re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',anchor);pattern=r'\s*'.join(re.escape(x) for x in tokens);found=[]
 if tokens[0][0].isalnum() or tokens[0][0]=='_':pattern=r'(?<!\w)'+pattern
 if tokens[-1][-1].isalnum() or tokens[-1][-1]=='_':pattern+=r'(?!\w)'
 for m in re.finditer(pattern,text):
  if m.start()<start:continue
  lo,hi=m.span()
  while anchor[:1] in (' ','\t') and text[lo-1:lo] in (' ','\t'):lo-=1
  if anchor.endswith('\n'):
   n=re.compile(r'[ \t]*\r?\n').match(text,hi);hi=n.end() if n else hi
  found.append((lo,hi))
 return found
def once(text,old,new):
 found=spans(text,old)
 if len(found)!=1:raise ValueError('Versioned import anchor drift: '+old[:90])
 return text[:found[0][0]]+new+text[found[0][1]:]
def boundary(text,anchor,after=None):
 found=spans(text,anchor,after or 0)
 if not found or (after is None and len(found)!=1):raise ValueError('Versioned import section drift: '+anchor[:90])
 return found[0][0]
def section(text,start,end,new):
 if len(spans(text,start))!=1 or len(spans(text,end))!=1:raise ValueError('Versioned import section drift')
 lo=boundary(text,start);hi=boundary(text,end,lo);return text[:lo]+new+'\n'+text[hi:]
def main():
 p=argparse.ArgumentParser();p.add_argument('--kernel-source',type=Path,required=True);p.add_argument('--kernel-lab',type=Path,required=True);p.add_argument('--sdk-source',type=Path,required=True);p.add_argument('--sdk-public',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--work',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--opt',choices=['O0','O2'],default='O2');p.add_argument('--cpus',type=int,choices=[1,2,4],default=2);p.add_argument('--case',type=int,default=1);p.add_argument('--define',default='');p.add_argument('--expected',type=int,default=33);p.add_argument('--marker',default='PASS sf-complete');a=p.parse_args();root=Path.cwd().resolve();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False)
 source=a.source.resolve();kernel=a.kernel_source.resolve();lab=a.kernel_lab.resolve();sdk=a.sdk_source.resolve();sdkpublic=a.sdk_public.resolve();rows=[]
 inputs={Path(__file__),*kernel.glob('*'),*source.glob('kernel/*'),*PUBLIC.glob('kernel/*'),*PUBLIC.glob('tests/*'),*source.glob('vm/*'),*PUBLIC.glob('vm/*'),*sdk.glob('*'),*sdkpublic.glob('*'),*sdkpublic.glob('freestanding/*'),*lab.glob('include/*'),*lab.glob('platform/*'),lab/'tools/guest.py',lab/'tests/spawn_user.c'}
 runtime=[root/'assessor/runtime/cvm.c',root/'assessor/runtime_followthrough/verifier/verifier.c',root/'assessor/runtime_followthrough/execution/execution.c'];runtime_includes=[root/'labs/runtime/include',root/'labs/runtime_followthrough/verifier',root/'labs/runtime_followthrough/execution']
 if a.case in (19,20):inputs.update(runtime);inputs.update(p for d in runtime_includes for p in d.glob('*.h'))
 store_sources=[root/'assessor/storage/store.c',sdk/'receipt.c',source/'store/service.c']
 if a.case in (21,22,23):inputs.update(store_sources);inputs.update(PUBLIC.glob('store/*'));inputs.update((root/'labs/storage/include').glob('*.h'));inputs.add(sdkpublic/'receipt.h')
 workload_sources=[source/'workload/workload.c',root/'assessor/amp_followthrough/core/amp.c']
 if a.case in (24,25):inputs.update(workload_sources);inputs.update(PUBLIC.glob('workload/*'));inputs.add(root/'labs/amp_followthrough/core/amp.h')
 inputs={p for p in inputs if p.is_file()};before={p.relative_to(root).as_posix():sha(p) for p in sorted(inputs)}
 spec=importlib.util.spec_from_file_location('sf_base_guest',lab/'tools/guest.py');guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
 env=os.environ.copy()
 for k in ('CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS'):env.pop(k,None)
 def run(name,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=root,env=env,capture_output=True,text=True,timeout=120);text=r.stdout+r.stderr;path=work/(name+'.txt');path.write_text(text);okay=r.returncode==expected and (marker is None or marker in text);rows.append(dict(name=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=okay,log=path.relative_to(root).as_posix(),sha256=sha(path)))
  if not okay:raise RuntimeError(name+'\n'+text[-5000:])
  return text
 try:
  for tool in (a.cc,'ld','qemu-system-x86_64','grub-mkrescue'):run(tool+'-version',[tool,'--version'])
  staged=work/'source';staged.mkdir()
  for path in kernel.iterdir():
   if path.is_file():shutil.copyfile(path,staged/path.name)
  for path in sdk.iterdir():
   if path.is_file():shutil.copyfile(path,staged/path.name)
  for path in (source/'kernel').iterdir():
   if path.is_file():shutil.copyfile(path,staged/path.name)
  shutil.copyfile(PUBLIC/'kernel/abi.h',staged/'abi.h');shutil.copyfile(sdkpublic/'kernel_abi.h',staged/'kernel_abi.h')
  boot=(staged/'boot.S').read_text();(staged/'boot.S').write_text(once(boot,'-(0xe85250d6+24)','(-(0xe85250d6+24)) & 0xffffffff'))
  text=(kernel/'kernel.c').read_text();text='#include <stdatomic.h>\n#include "abi.h"\n'+text
  ticks=spans(text,'__atomic_fetch_add(&ticks,1,__ATOMIC_RELAXED)');assert len(ticks)==2
  for lo,hi in reversed(ticks):text=text[:lo]+'atomic_fetch_add_explicit(&ticks,1,memory_order_relaxed)'+text[hi:]
  text=once(text,'UT_IO, UT_DEAD','UT_IO, UT_SD_WAIT, UT_SF_WAIT, UT_DEAD')
  text=once(text,'#include "async.inc"','static unsigned sf_io_hold;\n#include "async.inc"')
  text=once(text,'static int materialize(', '#include "faults.inc"\nstatic int materialize(')
  lo=boundary(text,'static int materialize(');hi=boundary(text,'static int user_ok(',lo);piece=text[lo:hi];takes=spans(piece,'cmk_frame_take(&frames)');assert len(takes)==2
  for a_,b_ in reversed(takes):piece=piece[:a_]+'sf_fault_take()'+piece[b_:]
  text=text[:lo]+piece+text[hi:]
  text=section(text,'static int load_task(', 'static void activate(', '#include "load.inc"')
  text=once(text,'#include "os.inc"','static void sd_process_cleanup(unsigned);\nstatic void sd_fork_files(unsigned,unsigned);\nstatic void sd_thread_cleanup(unsigned);\nstatic void sf_process_cleanup(unsigned);\nstatic void sf_thread_cleanup(unsigned);\nstatic void sf_fork_maps(unsigned,unsigned);\n#include "os.inc"\n#include "services.inc"\n#include "extra.inc"')
  text=once(text,'return syscall_v2(f);','return f->rax>=SF_INFO?sf_syscall(f):(f->rax>=SDK_LOAD_PROBE?sd_syscall(f):syscall_v2(f));')
  text=section(text,'static void complete(void)', 'static struct frame *schedule(void)', 'static void complete(void){if(primary_failure)panic("sf user failure");say("PASS sf-complete\\n");stop(1);}')
  lo=boundary(text,'  if (load_task(boot_mode == 2');hi=boundary(text,'  say("PASS elf-load',lo);text=text[:lo]+'  if(load_task("init",1))panic("sf init ELF");\n'+text[hi:]
  text=section(text,'  if (f->vector == 14 && uthreads[current].process == 3', '  say("user fault ");','')
  text=once(text,'static unsigned faults,primary_failure;','static unsigned primary_failure;')
  text=once(text,'  else if(f->vector==241)cmk2_eoi();\n  kernel_lock();\n','  else if(f->vector==241)cmk2_eoi();\n  sf_fault_before_lock(f);\n  kernel_lock();\n')
  (staged/'kernel.c').write_text(text)
  text=(kernel/'async.inc').read_text();text=once(text,'if(!async_pending)return;','if(!async_pending||sf_io_hold)return;');(staged/'async.inc').write_text(text)
  text=(kernel/'os.inc').read_text();text=once(text,'struct task *p=&tasks[id];p->status=status;','sf_process_cleanup(id);sd_process_cleanup(id);struct task *p=&tasks[id];p->status=status;');text=once(text,'uthreads[tid].frame->rax=0;if(child==task_count)','sf_fork_maps(pid,child);sd_fork_files(pid,child);uthreads[tid].frame->rax=0;if(child==task_count)');text=once(text,'struct uthread *u=&uthreads[current];u->status=(int)a;','sf_thread_cleanup(current);sd_thread_cleanup(current);struct uthread *u=&uthreads[current];u->status=(int)a;')
  # The final function consumes the remainder of this frozen os.inc version.
  anchor='static int page_fault_v2(';text=text[:boundary(text,anchor)]+'''static int page_fault_v2(struct frame *f){
 uint64_t a;__asm__ volatile("mov %%cr2,%0":"=r"(a));
 if(a<CMK_USER_BASE||a>=CMK_USER_LIMIT)return 0;
 struct task *p=&tasks[uthreads[current].process];unsigned i=(unsigned)((a-CMK_USER_BASE)/4096);
 unsigned kind=sf_fault_class(f->error,p->pt[i]);
 if(kind==1||kind==2)return materialize(p,i,!!(f->error&2))>0;
 if(kind==3){__asm__ volatile("invlpg (%0)"::"r"(a):"memory");return 1;}return 0;
}
''';(staged/'os.inc').write_text(text)
  flags=[('-'+a.opt) if x=='-O2' else x for x in guest.FLAGS]+['-Werror','-I'+str(lab/'include'),'-I'+str(staged),'-I'+str(PUBLIC/'kernel'),'-DSF_CASE='+str(a.case)]
  if a.define:flags+=['-D'+a.define]
  if a.case in (19,20):flags+=['-I'+str(d) for d in runtime_includes]+['-I'+str(PUBLIC/'vm'),'-I'+str(sdkpublic/'freestanding')]
  if a.case in (21,22,23):flags+=['-I'+str(root/'labs/storage/include'),'-I'+str(sdkpublic),'-I'+str(sdkpublic/'freestanding')]
  if a.case in (24,25):flags+=['-I'+str(PUBLIC/'workload'),'-I'+str(root/'labs/amp_followthrough/core')]
  objects=[]
  for name in ('boot.S','interrupts.S','kernel.c','model.c','ap.S','smp.c','cmk2_model.c'):
   obj=work/(name+'.o');run(name+'-compile',[a.cc,*flags,'-c',staged/name,'-o',obj]);objects.append(obj)
  run('kernel-link',['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/link.ld','-Map',work/'kernel.map','-o',work/'kernel.elf',*objects])
  items=[]
  users=[('init',PUBLIC/'tests/process.c'),('child',PUBLIC/'tests/child.c'),('spawn',lab/'tests/spawn_user.c')]
  if a.case in (19,20):users.append(('vm',PUBLIC/'vm/guest.c'))
  if a.case in (21,22,23):users.append(('store',PUBLIC/'store/guest.c'))
  for name,path in users:
   userflags=(['-I'+str(PUBLIC/'store')] if name=='store' else [])+flags
   obj=work/(name+'.o');run(name+'-compile',[a.cc,*userflags,'-c',path,'-o',obj]);parts=[obj]
   if name in ('init','child','vm','store'):
    helper=work/(name+'-bytes.o');run(name+'-bytes-compile',[a.cc,*flags,'-c',sdkpublic/'freestanding/bytes.c','-o',helper]);parts.append(helper)
   if a.case in (19,20) and name in ('init','child','vm'):
    providers=[runtime[0]] if name=='child' else runtime+[source/'vm/service.c',PUBLIC/'vm/tests.c']
    for i,path in enumerate(providers):
     part=work/(name+'-runtime-'+str(i)+'.o');run(name+'-runtime-'+str(i),[a.cc,*flags,'-c',path,'-o',part]);parts.append(part)
   if name=='store':
    for i,path in enumerate(store_sources):
     part=work/(name+'-provider-'+str(i)+'.o');run(name+'-provider-'+str(i),[a.cc,*userflags,'-c',path,'-o',part]);parts.append(part)
   if a.case in (24,25) and name=='init':
    for i,path in enumerate(workload_sources):
     part=work/('workload-'+str(i)+'.o');run('workload-'+str(i),[a.cc,*flags,'-c',path,'-o',part]);parts.append(part)
   elf=work/(name+'.elf');run(name+'-link',['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',lab/'platform/user.ld','-Map',work/(name+'.map'),'-o',elf,*parts]);items.append((name,elf.read_bytes()))
  items.append(('data',b'abcdefghijklmnopqrstuvwxyz0123456789\n'));items.append(('ten',b'abcdefghij'));(work/'init.cmka').write_bytes(guest.pack(items))
  for elf in work.glob('*.elf'):
   assert not run(elf.stem+'-undefined',['nm','-u',elf]).strip();assert 'NEEDED' not in run(elf.stem+'-dynamic',['readelf','-d',elf]);run(elf.stem+'-disassembly',['objdump','-dr',elf])
  iso=work/'iso';(iso/'boot/grub').mkdir(parents=True);shutil.copyfile(work/'kernel.elf',iso/'boot/kernel.elf');shutil.copyfile(work/'init.cmka',iso/'boot/init.cmka');(iso/'boot/grub/grub.cfg').write_text('set timeout=0\nmenuentry "SF" {\n multiboot2 /boot/kernel.elf write\n module2 /boot/init.cmka init\n boot\n}\n');run('multiboot',['grub-file','--is-x86-multiboot2',work/'kernel.elf']);run('iso',['grub-mkrescue','-o',work/'write.iso',iso])
  disk=Path(tempfile.mkdtemp(prefix='cmastery-sf-disk-'))/'disk.raw'
  with disk.open('wb') as stream:stream.truncate(4*1024*1024)
  command=guest.command(work,'write');command[command.index('-smp')+1]=str(a.cpus);command[command.index('-drive')+1]='if=none,id=disk,format=raw,cache=writeback,file='+str(disk)
  lineage=[dict(stage='fresh-before-write',disk_sha256=sha(disk))];run('guest',command,a.expected,a.marker);lineage.append(dict(stage='after-write',disk_sha256=sha(disk)))
  if a.case==23:
   (iso/'boot/grub/grub.cfg').write_text('set timeout=0\nmenuentry "SF" {\n multiboot2 /boot/kernel.elf verify\n module2 /boot/init.cmka init\n boot\n}\n');run('verify-iso',['grub-mkrescue','-o',work/'verify.iso',iso]);verify_command=list(command);verify_command[verify_command.index('-cdrom')+1]=str(work/'verify.iso');run('guest-reboot-same-disk',verify_command,33,'PASS sf-reboot-same-disk42');lineage.append(dict(stage='after-same-disk-reboot',disk_sha256=sha(disk)));shutil.copyfile(disk,work/'same-lineage.raw')
   with disk.open('wb') as stream:stream.truncate(4*1024*1024)
   lineage.append(dict(stage='freshly-replaced-control',disk_sha256=sha(disk)));run('guest-fresh-disk-control',command,33,'PASS sf-fresh-zero-then-durable42');lineage.append(dict(stage='after-fresh-control',disk_sha256=sha(disk)))
  (work/'disk-lineage.json').write_text(json.dumps(dict(path=str(disk),kernel_sha256=sha(work/'kernel.elf'),archive_sha256=sha(work/'init.cmka'),observations=lineage,scope='Original synthetic file and guest flush/reboot only; no physical power-loss claim.'),indent=2)+'\n');passed=True;error=None
 except Exception as exc:passed=False;error=repr(exc)
 after={p.relative_to(root).as_posix():sha(p) for p in sorted(inputs)}
 if before!=after:passed=False;error='Source changed during run'
 result=dict(passed=passed,error=error,rows=rows,case=a.case,cpus=a.cpus,compiler=a.cc,optimization=a.opt,source_sha256=after,artifacts={p.relative_to(root).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Actual original QEMU TCG guest with explicit source imports; no physical, independent learner or power-loss claim.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
