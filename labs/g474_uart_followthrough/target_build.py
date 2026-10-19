"""Cross-build the selected original polling image; no device connection or flash."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser()
for n in ['build','initializer','driver','transfer','transfer-include','runtime']:p.add_argument('--'+n,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--baud',type=int,choices=[9600,115200,1000000],default=115200);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=False)
link=a.build/'image.ld';link.write_text('''ENTRY(g4_reset)
MEMORY { FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {
 .vectors : ALIGN(512) { KEEP(*(.vectors)) } >FLASH
 .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >FLASH
 .data : ALIGN(4) { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } >RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : ALIGN(8) { __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } >RAM
 ASSERT(__bss_end+4096<__stack_top,"G4 stack reserve")
 ASSERT(SIZEOF(.vectors)==64,"core-only interrupt-disabled vectors")
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment) }
}
''')
flags=['-std=c17','-O'+a.opt,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(HERE),'-I'+str(a.transfer_include),'-DG4_BAUD='+str(a.baud)]+(['--target=arm-none-eabi'] if a.cc=='clang' else [])
sources=[a.initializer,HERE/'physical.c',HERE/'clock_probe.c',a.driver,a.transfer,a.runtime,HERE/'startup.S'];objects=[];commands=[]
for i,s in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);cmd=[a.cc,*flags,'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)];commands.append(list(map(str,cmd)));subprocess.run(cmd,check=True)
cmd=['arm-none-eabi-ld','-T',link,*objects,'-Map='+str(a.build/'image.map'),'-o',a.build/'image.elf'];commands.append(list(map(str,cmd)));subprocess.run(cmd,check=True)
deps={Path(__file__),link}
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld'])
(a.build/'build.json').write_text(json.dumps(dict(commands=commands,input_sha256={str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},execution='not-run; dedicated G474 board and explicit operator action required',baud=a.baud,objects=7,archives=0),indent=2)+'\n')
print('G4_BUILD_PASS objects=7 archives=0 physical=not-run')
