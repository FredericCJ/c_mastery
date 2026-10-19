"""Explicit source, strict Cortex-M followthrough images; no board inference."""
from __future__ import annotations
import argparse,hashlib,json,re,subprocess,time
from pathlib import Path
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE)
 p.add_argument('--runtime-source',type=Path,default=ROOT/'labs/execution/src')
 p.add_argument('--core-source',type=Path,default=HERE.parent/'core/learner.c')
 p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--optimization',choices=['O0','O2'],default='O2')
 p.add_argument('--target',choices=['m4','m7'],default='m4')
 p.add_argument('--frame-pointer',action='store_true');p.add_argument('--fp',action='store_true');p.add_argument('--move-data',action='store_true')
 p.add_argument('--mode',choices=['direct','preemptive','sleep','integrated'],default='direct');p.add_argument('--define',action='append',default=[])
 p.add_argument('--without-helper',action='store_true');p.add_argument('--build',type=Path,required=True);p.add_argument('--run',action='store_true')
 a=p.parse_args();b=a.build.resolve();b.mkdir(parents=True,exist_ok=False);src=a.source.resolve()
 report={'schema':1,'status':'running','evidence_kind':'actual-emulator' if a.run else 'image-audit','physical_status':'not-run','commands':[],'inputs':{},'checks':[]}
 def command(argv,timeout=40,stdin=None):
  argv=[str(x) for x in argv];start=time.monotonic()
  try:r=subprocess.run(argv,text=True,capture_output=True,timeout=timeout,input=stdin);out=r.stdout+r.stderr;rc=r.returncode
  except subprocess.TimeoutExpired as e:
   out=(e.stdout or b'').decode() if isinstance(e.stdout,bytes) else e.stdout or '';out+=(e.stderr or b'').decode() if isinstance(e.stderr,bytes) else e.stderr or '';rc=124
  row={'command':argv,'returncode':rc,'output':out,'elapsed_seconds':time.monotonic()-start};report['commands'].append(row)
  if rc:raise RuntimeError('command rejected: '+ ' '.join(argv)+'\n'+out)
  return out
 def check(name,ok):
  report['checks'].append({'name':name,'passed':bool(ok)})
  if not ok:raise RuntimeError(name)
 try:
  for tool in ('arm-none-eabi-gcc','clang','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'):
   report.setdefault('versions',{})[tool]=command([tool,'--version']).splitlines()[0]
  linker=b/'image.ld';ram2=0x20020000 if a.move_data else 0x20010000
  linker.write_text('''ENTRY(mf_reset)
MEMORY { FLASH(rx):ORIGIN=0,LENGTH=1024K
 RAM(rwx):ORIGIN=0x20000000,LENGTH=64K
 RAM2(rwx):ORIGIN=RAM2_ADDRESS,LENGTH=64K }
__stack_top=ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : { . = ALIGN(4); __data_start=.; *(.data) *(.data.*) . = ALIGN(4); __data_end=.; } >RAM AT>FLASH
 __data_load=LOADADDR(.data);
 .data2 : { . = ALIGN(4); __data2_start=.; *(.data2) . = ALIGN(4); __data2_end=.; } >RAM2 AT>FLASH
 __data2_load=LOADADDR(.data2);
 .bss (NOLOAD) : { . = ALIGN(8); __bss_start=.; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end=.; } >RAM
 .retained (NOLOAD) : { __retained_start=.; KEEP(*(.retained)) __retained_end=.; } >RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__retained_end+8192<__stack_top,"handler stack budget exceeded")
}
'''.replace('RAM2_ADDRESS',hex(ram2)))
  # GNU ld requires whitespace around assignments/region attributes.
  linker.write_text(re.sub(r'(?<![<>])=(?!=)', ' = ', linker.read_text()).replace('):', ') : '))
  compiler=['arm-none-eabi-gcc'] if a.cc=='gcc' else ['clang','--target=arm-none-eabi']
  cpu='cortex-m7' if a.target=='m7' else 'cortex-m4';machine='mps2-an500' if a.target=='m7' else 'mps2-an386'
  flags=['-mcpu='+cpu,'-mthumb','-mfpu='+('fpv5-d16' if a.target=='m7' else 'fpv4-sp-d16'),'-mfloat-abi=softfp','-'+a.optimization,'-g','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wextra','-Werror','-I'+str(HERE),'-DMF_FP='+str(int(a.fp)),*['-D'+x for x in a.define]]
  flags+=['-fno-omit-frame-pointer'] if a.frame_pointer else ['-fomit-frame-pointer']
  if a.cc=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
  files=[src/'startup.S',src/'report.c',a.runtime_source.resolve()/'support.c']
  if not a.without_helper:files += [src/'helpers.c',src/'helpers.S']
  if a.mode!='sleep':files += [src/('direct.S' if a.mode=='direct' else 'context.S')]
  files += [HERE/(a.mode+'_test.c')]
  if a.mode!='direct':
   files += [src/('preemptive.c' if a.mode=='integrated' else a.mode+'.c'),a.core_source.resolve(),a.runtime_source.resolve()/'cmx.c']
   flags+=['-I'+str(HERE.parent/'core'),'-I'+str(ROOT/'labs/execution/include')]
  for f in [*files,HERE/'target.h',HERE/'port.h',HERE/'sleep.h',HERE.parent/'core/mcu.h',ROOT/'labs/execution/include/cmx.h',Path(__file__).resolve()]:report['inputs'][str(f.relative_to(ROOT))]=sha(f)
  objects=[]
  for i,f in enumerate(files):
   obj=b/(str(i)+'-'+f.name+'.o');command([*compiler,*flags,*(['-std=c17'] if f.suffix=='.c' else []),'-c',f,'-o',obj]);objects.append(obj)
  elf=b/'mcu.elf';command(['arm-none-eabi-gcc','-mcpu='+cpu,'-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',linker,'-Wl,--gc-sections,-Map='+str(b/'image.map'),'-o',elf])
  symbols=command(['arm-none-eabi-nm','-n',elf]);(b/'symbols.txt').write_text(symbols)
  undef=command(['arm-none-eabi-nm','-u',elf]);check('closed symbols',not undef.strip())
  check('no runtime archives',not re.search(r'lib(?:c|gcc|atomic)\.a|libclang_rt|compiler-rt|crt[0-9a-z]*\.o',(b/'image.map').read_text()))
  (b/'disassembly.txt').write_text(command(['arm-none-eabi-objdump','-d',elf]));(b/'elf.txt').write_text(command(['arm-none-eabi-readelf','-lWs',elf]))
  report['image_sha256']=sha(elf)
  if a.run:
   # Load initialized flash bytes, not ELF's NOLOAD RAM program header;
   # otherwise the emulator loader and intentional BSS poison overlap.
   binary=b/'mcu.bin';command(['arm-none-eabi-objcopy','-O','binary',elf,binary])
   addresses={name:int(addr,16) for addr,name in re.findall(r'^([0-9a-fA-F]+)\s+\w\s+(__bss_start|__bss_end|__retained_start)$',symbols,re.M)}
   poison=b/'bss.bin';poison.write_bytes(b'\xa5'*(addresses['__bss_end']-addresses['__bss_start']))
   retained=b/'retained.bin';retained.write_bytes((0x52544e44).to_bytes(4,'little'))
   uart='MF_UART' in a.define
   out=command(['qemu-system-arm','-M',machine,'-cpu',cpu,'-nographic','-monitor','none','-serial','stdio' if uart else 'none','-nic','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',binary,'-device','loader,file='+str(poison)+',addr='+hex(addresses['__bss_start'])+',force-raw=on','-device','loader,file='+str(retained)+',addr='+hex(addresses['__retained_start'])+',force-raw=on'],timeout=20,stdin=''.join(chr(33+i) for i in range(64)) if uart else None)
   (b/'serial.log').write_text(out);check('actual '+a.mode+' success marker','PASS MF '+a.mode in out);print(out,end='')
  report['status']='passed'
 except Exception as e:report['status']='failed';report['error']=str(e)
 report['artifacts']={str(f.relative_to(b)):sha(f) for f in sorted(b.rglob('*')) if f.is_file() and f.name!='report.json'}
 (b/'report.json').write_text(json.dumps(report,indent=2)+'\n')
 if report['status']!='passed':raise SystemExit(report['error'])
if __name__=='__main__':main()
