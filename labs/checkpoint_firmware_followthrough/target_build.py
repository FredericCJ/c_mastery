"""Build an explicitly selected original F439 experiment; never flash or execute."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser()
for name in ['build','driver','core','parser','halfduplex','runtime','core-include','transfer-include']:p.add_argument('--'+name,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mode',type=int,choices=[0,1,2],default=1);p.add_argument('--trace',type=int,choices=[0,1],default=1);p.add_argument('--ring',type=int,choices=[2,4],default=4);p.add_argument('--offset',type=lambda x:int(x,0),choices=[0,0x4000],default=0);p.add_argument('--dma-ccm',action='store_true');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
link=a.build/'image.ld';link.write_text('''ENTRY(cp_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x08000000,LENGTH = 512K
 RAM (rwx) : ORIGIN = RAMSTART,LENGTH = 64K
 CCM (rwx) : ORIGIN = 0x10000000,LENGTH = 64K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .vectors : ALIGN(512) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 .dma (NOLOAD) : ALIGN(4) { __dma_start = .; *(.dma*) . = ALIGN(4); __dma_end = .; } >DMAREGION
 ASSERT(__bss_end+16384<__stack_top,"P4 stack reserve")
 ASSERT(SIZEOF(.vectors)==448,"selected F439 vectors")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
'''.replace('RAMSTART',hex(0x20000000+a.offset)).replace('DMAREGION','CCM' if a.dma_ccm else 'RAM'))
flags=['-std=c17','-O'+a.opt,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(HERE),'-I'+str(a.core_include),'-I'+str(a.transfer_include),'-DP4_TRACE='+str(a.trace),'-DP4_MODE='+str(a.mode),'-DP4_RING='+str(a.ring)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
sources=[a.core,a.parser,a.halfduplex,a.driver,a.runtime,HERE/'startup.S'];objects=[];commands=[]
for i,source in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);cmd=[a.cc,*flags,*([] if source.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(source),'-o',str(obj)];commands.append(cmd);subprocess.run(cmd,check=True)
elf=a.build/'image.elf';cmd=['arm-none-eabi-ld','-T',str(link),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)];commands.append(cmd);subprocess.run(cmd,check=True)
deps={Path(__file__),link}
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld'])
(a.build/'build.json').write_text(json.dumps(dict(commands=commands,input_sha256={str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},execution='not-run; F439 board required',selected=dict(mode=a.mode,trace=a.trace,ring=a.ring,offset=a.offset,dma_ccm=a.dma_ccm)),indent=2)+'\n')
print('P4_CAPTURE_BUILD_PASS explicit_objects=6 no_archives execution=not-run')
