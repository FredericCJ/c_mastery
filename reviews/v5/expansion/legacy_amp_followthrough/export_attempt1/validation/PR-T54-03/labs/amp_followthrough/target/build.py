"""Build two original H745 images; no physical execution is inferred."""
from pathlib import Path
import argparse,hashlib,json,subprocess,re
ROOT=Path(__file__).resolve().parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',choices=['gcc','clang'],default='gcc');p.add_argument('--optimization',choices=['O0','O2','O3'],default='O2');p.add_argument('--shared',choices=['sram3','sram4'],default='sram3');p.add_argument('--cached',action='store_true');p.add_argument('--mode',choices=['transport','pipeline'],default='transport');p.add_argument('--qemu',action='store_true');p.add_argument('--burst',type=int,choices=[1,4],default=1);p.add_argument('--local',action='store_true');p.add_argument('--interference',action='store_true');p.add_argument('--define',action='append',default=[]);p.add_argument('--providers',type=Path,default=ROOT/'labs/amp_followthrough/providers.json');a=p.parse_args();a.source=a.source.resolve();a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=False)
 rows=[];images=[]
 def run(label,cmd):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,timeout=60);out=r.stdout+r.stderr;log=a.build/(str(len(rows))+'.txt');log.write_text(out);rows.append(dict(label=label,command=cmd,returncode=r.returncode,log=str(log.relative_to(ROOT)),log_sha256=sha(log)));print(label,r.returncode,flush=True)
  if r.returncode:raise RuntimeError(label+'\n'+out)
  return out
 provider={k:ROOT/v for k,v in json.loads(a.providers.read_text()).items()}
 paths=[ROOT/'labs/amp_followthrough/target/start.S',a.source/'target/board.c',a.source/'core/amp.c',a.source/'core/doorbell.c',a.source/'core/buffers.c',provider['dma'],provider['dma_target'],provider['lease'],provider['bridge'],provider['cache'],provider['cache_port'],provider['support']]
 if a.mode=='pipeline':paths += [provider['context'],provider['port'],provider['scheduler']]
 headers=[*ROOT.joinpath('labs/amp_followthrough').rglob('*.h'),ROOT/'labs/peripheral_depth/dma/dma.h',ROOT/'labs/interrupt_depth/lease/lease.h',ROOT/'labs/interrupt_depth/lease/bridge.h',ROOT/'labs/firmware_lifecycle/targets/h745_cache.h',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/include/cmx_port.h']
 try:
  compiler=['arm-none-eabi-gcc'] if a.cc=='gcc' else ['clang','--target=arm-none-eabi']
  run('compiler identity',[compiler[0],'--version'])
  for core,flash,ram in ([('m7',0,0x20000000)] if a.qemu else [('m7',0x08000000,0x20000000),('m4',0x08100000,0x30000000)]):
   dest=a.build/core;dest.mkdir();link=dest/'image.ld';shared=0x30040000 if a.shared=='sram3' else 0x38000000
   link.write_text(f'''ENTRY(af_reset)
MEMORY {{ FLASH (rx) : ORIGIN = {flash:#x}, LENGTH = 1M
 RAM (rwx) : ORIGIN = {ram:#x}, LENGTH = 128K
 SHARED (rw) : ORIGIN = {shared:#x}, LENGTH = 8K }}
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {{
 .text : {{ KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); }} > FLASH
 .data : {{ __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; }} > RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss(NOLOAD) : {{ . = ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end = .; }} > RAM
 .shared(NOLOAD) : {{ __shared_start = .; . += 8192; __shared_end = .; }} > SHARED
 /DISCARD/ : {{ *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }}
 ASSERT(__bss_end+8192<__stack_top,"private storage plus stack")
}}
''')
   inc=[ROOT/'labs/execution/include',ROOT/'labs/amp_followthrough/core',ROOT/'labs/amp_followthrough/target',ROOT/'labs/peripheral_depth/dma',ROOT/'labs/interrupt_depth/lease',ROOT/'labs/firmware_lifecycle/targets']
   flags=['-std=c17','-'+a.optimization,'-g','-mcpu=cortex-'+('m4' if a.qemu else core),'-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wextra','-Werror','-DAF_M7='+str(int(core=='m7')),'-DAF_SHARED='+hex(shared),'-DAF_CACHED='+str(int(a.cached)),'-DPD_BOARD=1','-DFW_H745_M7='+str(int(core=='m7')),*['-I'+str(x) for x in inc],'-idirafter','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']
   if a.mode=='pipeline':flags += ['-DAF_PIPELINE','-DCMX_STAGE=4','-DCMX_STAGE_NAME="amp-pipeline"','-DAF_BURST='+str(a.burst),'-DAF_LOCAL='+str(int(a.local)),'-DAF_INTERFERENCE='+str(int(a.interference))]
   flags += ['-D'+v for v in a.define]
   if a.qemu:flags += ['-DAF_QEMU=1','-DCMX_QEMU']
   objects=[]
   for i,src in enumerate(paths):
    obj=dest/(str(i)+'-'+src.stem+'.o');run(core+' compile '+src.name,[*compiler,*flags,'-c',src,'-o',obj]);objects.append(obj)
   elf=dest/'amp.elf';run(core+' link',['arm-none-eabi-ld','--gc-sections',*objects,'-T',link,'-Map='+str(dest/'image.map'),'-o',elf])
   undef=run(core+' unresolved',['arm-none-eabi-nm','-u',elf]);symbols=run(core+' symbols',['arm-none-eabi-nm','-n',elf]);dis=run(core+' instructions',['arm-none-eabi-objdump','-d',elf]);sections=run(core+' layout',['arm-none-eabi-readelf','-lSW',elf]);cost=run(core+' size',['arm-none-eabi-size','-A',elf])
   (dest/'symbols.txt').write_text(symbols);(dest/'disassembly.txt').write_text(dis);(dest/'layout.txt').write_text(sections);(dest/'cost.txt').write_text(cost)
   if undef.strip() or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|libclang_rt|crt\w*\.o',(dest/'image.map').read_text()):raise RuntimeError('unapproved dependency')
   required=['af_reset','af_entry','af_hsem','af_boot_ack' if core=='m4' else 'af_boot_stage']
   if a.mode=='transport':required += ['af_systick']
   if a.mode=='pipeline':required += ['SVC_Handler','PendSV_Handler','SysTick_Handler','cmx_port_choose']
   if any(not re.search(r'\b'+n+r'$',symbols,re.M) for n in required):raise RuntimeError('missing executed boundary')
   if 'dmb' not in dis or 'dsb' not in dis:raise RuntimeError('missing ordering instructions')
   if a.qemu:
    output=run(core+' emulator',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-nic','none','-semihosting-config','enable=on,target=native','-kernel',elf])
    if 'PASS CMX Cortex-M amp-pipeline' not in output:raise RuntimeError('missing actual scheduler execution')
   images.append(dict(core=core,actual_cpu='cortex-m4' if a.qemu else 'cortex-'+core,execution_kind='single-core-AN386-emulator' if a.qemu else 'image-audit',elf=elf.relative_to(ROOT).as_posix(),sha256=sha(elf),flash=flash,private_ram=ram,shared=shared,shared_bytes=8192,cached=a.cached,unresolved=[],external_runtime=[],physical_status='not-run'))
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 report=dict(passed=passed,error=error,images=images,rows=rows,input_sha256={x.relative_to(ROOT).as_posix():sha(x) for x in paths+headers+[a.source/'target/pipeline.inc',a.providers.resolve(),Path(__file__).resolve()]},artifacts={x.relative_to(ROOT).as_posix():sha(x) for x in a.build.rglob('*') if x.is_file()},limits=['Actual compiled two-core firmware; optional AN386 executes same task pipeline on one core, never H745 or physical execution.','Requires cold BCM4=0, M7 clock/peripheral ownership and reset-disabled caches.','Cooperative drained restart is distinct from unexpected core reset; unexpected reset fails closed.'])
 (a.build/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,images=len(images))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
