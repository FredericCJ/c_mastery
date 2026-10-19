"""Two audited original H745 images for each logical service-role assignment."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
ROOT=Path.cwd().resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--ring-source',type=Path,required=True);p.add_argument('--support-source',type=Path,required=True);p.add_argument('--work',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--opt',choices=['O0','O2'],default='O2');a=p.parse_args();a.work.mkdir(parents=True,exist_ok=False);rows=[];images=[]
 sources=[Path('labs/amp_followthrough/target/start.S'),a.source/'workload/board.c',a.source/'workload/workload.c',a.ring_source,a.support_source];headers=[Path('labs/services_followthrough/workload/workload.h'),Path('labs/amp_followthrough/core/amp.h')];before={str(p):sha(p) for p in sources+headers+[Path(__file__).relative_to(ROOT)]}
 def run(name,cmd):
  r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);out=r.stdout+r.stderr;log=a.work/(str(len(rows))+'.txt');log.write_text(out);rows.append(dict(name=name,command=list(map(str,cmd)),returncode=r.returncode,log=str(log),sha256=sha(log),passed=r.returncode==0));
  if r.returncode:raise RuntimeError(name+'\n'+out)
  return out
 try:
  cc=['arm-none-eabi-gcc'] if a.cc=='gcc' else ['clang','--target=arm-none-eabi'];run('compiler',cc[:1]+['--version'])
  for swap in (0,1):
   for burst in (1,4):
    for core in ('m7','m4'):
     name=f'{core}-swap{swap}-burst{burst}';dest=a.work/name;dest.mkdir();flash=0x08000000 if core=='m7' else 0x08100000;ram=0x20000000 if core=='m7' else 0x30000000;link=dest/'image.ld'
     link.write_text(f'''ENTRY(af_reset)
MEMORY {{ FLASH (rx) : ORIGIN = {flash:#x}, LENGTH = 1M
RAM (rwx) : ORIGIN = {ram:#x}, LENGTH = 128K
SHARED (rw) : ORIGIN = 0x30040000, LENGTH = 8K }}
__stack_top=ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {{ .text:{{ KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); }} >FLASH
.data:{{ __data_start=.; *(.data*) . = ALIGN(4); __data_end=.; }} >RAM AT>FLASH
__data_load=LOADADDR(.data);
.bss(NOLOAD):{{ . = ALIGN(8); __bss_start=.; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end=.; }} >RAM
.shared(NOLOAD):{{ . += 8192; }} >SHARED
/DISCARD/:{{ *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }}
ASSERT(__bss_end+8192<__stack_top,"private storage plus stack") }}
'''.replace('):ORIGIN',') : ORIGIN').replace(',LENGTH',', LENGTH').replace('.text:', '.text : ').replace('.data:', '.data : ').replace('(NOLOAD):','(NOLOAD) : ').replace('/DISCARD/:','/DISCARD/ : ').replace('=.;','= .;').replace('__data_load=','__data_load = ').replace('__stack_top=','__stack_top = ').replace('=',' = ').replace('+ =','+='))
     flags=['-std=c17','-'+a.opt,'-g','-mcpu=cortex-'+core,'-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-DSF_M7='+str(int(core=='m7')),'-DAF_M7='+str(int(core=='m7')),'-DSF_SWAP='+str(swap),'-DSF_BURST='+str(burst),'-Ilabs/services_followthrough/workload','-Ilabs/amp_followthrough/core']
     objects=[]
     for i,source in enumerate(sources):
      obj=dest/(str(i)+'.o');run(name+' compile '+source.name,[*cc,*flags,'-c',source,'-o',obj]);objects.append(obj)
     elf=dest/'service.elf';run(name+' link',['arm-none-eabi-ld','--gc-sections',*objects,'-T',link,'-Map',dest/'image.map','-o',elf]);undefined=run(name+' undefined',['arm-none-eabi-nm','-u',elf]);symbols=run(name+' symbols',['arm-none-eabi-nm','-n',elf]);dis=run(name+' disassembly',['arm-none-eabi-objdump','-dr',elf]);layout=run(name+' layout',['arm-none-eabi-readelf','-lSW',elf]);cost=run(name+' cost',['arm-none-eabi-size','-A',elf]);
     if undefined.strip() or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',(dest/'image.map').read_text()):raise ValueError('undeclared runtime')
     if not all(x in symbols for x in ('af_reset','af_entry','sfw_offer' if (core=='m7')==bool(swap) else 'sfw_work')) or 'dmb' not in dis:raise ValueError('missing role or ordering')
     images.append(dict(core=core,swap=swap,burst=burst,acquisition=core==('m7' if swap else 'm4'),elf=str(elf),sha256=sha(elf),physical='not-run',shared='0x30040000 SRAM3 noncacheable shareable XN',clock_boot_owner='M7',external_runtime=[],unresolved=[]))
  passed=True;error=None
 except Exception as e:passed=False;error=repr(e)
 after={str(p):sha(p) for p in sources+headers+[Path(__file__).relative_to(ROOT)]}
 if before!=after:passed=False;error='source changed during build'
 report=dict(passed=passed,error=error,rows=rows,images=images,source_sha256=after,artifacts={str(p):sha(p) for p in a.work.rglob('*') if p.is_file()},scope='Two real ELF images per role/burst. No H745 execution or physical timing. Synthetic acquisition input; polling phase protocol, IRQs disabled.')
 (a.work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,images=len(images))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
