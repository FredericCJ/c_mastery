"""Strict original ARM images. Build receipts never imply physical execution."""
from __future__ import annotations
import argparse,hashlib,json,pathlib,re,shutil,subprocess
P=pathlib.Path
def run(command):
 p=subprocess.run([str(v) for v in command],text=True,capture_output=True)
 if p.returncode:raise RuntimeError(' '.join(map(str,command))+'\n'+p.stdout+p.stderr)
 return p.stdout+p.stderr
def image(a,core,flash,ram,size,defines):
 b=a.build/core;b.mkdir(parents=True,exist_ok=True)
 link=b/'image.ld'
 link.write_text(f'''ENTRY(cmx_reset)
MEMORY {{ FLASH (rx) : ORIGIN = {flash:#x}, LENGTH = 1024K
RAM (rwx) : ORIGIN = {ram:#x}, LENGTH = {size}K }}
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {{
 .text : {{ KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); }} > FLASH
 .data : {{ . = ALIGN(4); __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; }} > RAM AT>FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : {{ . = ALIGN(8); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(8); __bss_end = .; }} > RAM
 /DISCARD/ : {{ *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }}
 ASSERT(__bss_end+8192<__stack_top,"static storage plus handler stack exceeds private RAM")
}}
''')
 cpu='cortex-m7' if core=='m7' else 'cortex-m4'
 stages={'boot':0,'cooperative':1,'preemptive':2,'fp':3,'integrated':4}
 flags=['-std=c17','-mcpu='+cpu,'-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=softfp','-mgeneral-regs-only','-O2','-g','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wextra','-I'+str(a.lab/'include'),'-DCMX_STAGE='+str(stages[a.stage]),'-DCMX_STAGE_NAME="'+a.stage+'"',*['-D'+v for v in defines]]
 files=[a.source/x for x in ('startup.S','context.S','cortex_port.c','board.c','support.c','cmx.c','concurrent.c','advanced.c')]+[a.lab/'targets/workload.c']
 objects=[];commands=[]
 for path in files:
  obj=b/(path.name+'.o');command=['arm-none-eabi-gcc',*flags,'-c',path,'-o',obj];run(command);objects.append(obj);commands.append(list(map(str,command)))
 elf=b/'cmx.elf';command=['arm-none-eabi-gcc','-mcpu='+cpu,'-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,'-T',link,'-Wl,-Map='+str(b/'cmx.map'),'-o',elf];run(command);commands.append(list(map(str,command)))
 undef=run(['arm-none-eabi-nm','-u',elf]);symbols=run(['arm-none-eabi-nm','-n',elf]);dis=run(['arm-none-eabi-objdump','-d',elf]);headers=run(['arm-none-eabi-readelf','-lWs',elf]);(b/'symbols.txt').write_text(symbols);(b/'disassembly.txt').write_text(dis);(b/'elf.txt').write_text(headers)
 if undef.strip():raise RuntimeError('Undefined image symbols: '+undef)
 if re.search(r'lib(?:c|gcc|atomic)\.a|libclang_rt|compiler-rt|crt[0-9a-z]*\.o',(b/'cmx.map').read_text()):raise RuntimeError('Forbidden runtime archive/startup in image map')
 required=['cmx_reset'] if a.stage=='boot' else ['cmx_reset','PendSV_Handler','SVC_Handler','SysTick_Handler','cmx_port_choose']
 if any(not re.search(r'\b'+x+r'$',symbols,re.M) for x in required):raise RuntimeError('Missing original machine boundary')
 if a.stage!='boot' and not all(x in dis.lower() for x in ('mrs','psp','vstm','vldm')):raise RuntimeError('Context-switch/FPU save instructions absent')
 if 'CMX_H745' in defines and 'HSEM_Handler' not in symbols:raise RuntimeError('Missing HSEM handler')
 return {'core':core,'elf':str(elf),'sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),'commands':commands,'undefined_symbols':[],'external_libraries':[],'original_support':'support.c byte functions only; compiler helper routines are explicit original source','status':'passed','evidence_kind':'image-audit','physical_status':'not-run','source_sha256':{str(p.relative_to(a.source)) if p.is_relative_to(a.source) else 'targets/workload.c':hashlib.sha256(p.read_bytes()).hexdigest() for p in files}}
def main():
 p=argparse.ArgumentParser();p.add_argument('--target',choices=['cortex-m4-qemu','stm32f439','stm32h745'],required=True);p.add_argument('--source',type=P,required=True);p.add_argument('--lab',type=P,required=True);p.add_argument('--build',type=P,required=True);p.add_argument('--run',action='store_true');p.add_argument('--stage',choices=['boot','cooperative','preemptive','fp','integrated'],default='integrated');a=p.parse_args()
 a.source=a.source.resolve();a.lab=a.lab.resolve();a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=True)
 required=['arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-readelf','arm-none-eabi-objdump'];missing=[x for x in required if not shutil.which(x)]
 if missing:raise SystemExit('UNAVAILABLE: '+', '.join(missing))
 closure=[*a.lab.joinpath('include').glob('*.h'),P(__file__).resolve(),a.lab/'targets/workload.c']
 report={'target':a.target,'physical_status':'not-run','images':[],
  'tool_versions':{tool:run([tool,'--version']).splitlines()[0] for tool in required},
  'contract_sha256':{str(f.relative_to(a.lab)):hashlib.sha256(f.read_bytes()).hexdigest() for f in closure}}
 try:
  if a.target=='stm32h745':
   report['images'].append(image(a,'m7',0x08000000,0x20000000,128,['CMX_H745']))
   report['images'].append(image(a,'m4',0x08100000,0x30000000,128,['CMX_H745','CMX_CORE_M4']))
  else:report['images'].append(image(a,'m4',0 if a.target=='cortex-m4-qemu' else 0x08000000,0x20000000,128,['CMX_QEMU'] if a.target=='cortex-m4-qemu' else []))
  report['status']='passed'
  if a.run:
   if a.target!='cortex-m4-qemu':raise RuntimeError('Physical execution requires board evidence; this runner never fabricates it')
   symbols=(a.build/'m4/symbols.txt').read_text();addresses={name:int(addr,16) for addr,name in re.findall(r'^([0-9a-fA-F]+)\s+\w\s+(__bss_start|__bss_end)$',symbols,re.M)}
   poison=a.build/'bss-poison.bin';poison.write_bytes(b'\xa5'*(addresses['__bss_end']-addresses['__bss_start']))
   command=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',report['images'][0]['elf'],'-device','loader,file='+str(poison)+',addr='+hex(addresses['__bss_start'])+',force-raw=on']
   r=subprocess.run(command,capture_output=True,text=True,timeout=20);output=r.stdout+r.stderr;(a.build/'serial.log').write_text(output)
   report['execution']={'command':command,'returncode':r.returncode,'output':output,'evidence_kind':'emulator','stage':a.stage,'status':'passed' if r.returncode==0 and 'PASS CMX Cortex-M '+a.stage in output else 'failed'}
   print(output,end='')
   if report['execution']['status']!='passed':raise RuntimeError('Actual Cortex-M execution failed')
 except Exception as e:
  report['status']='failed';report['error']=str(e);(a.build/'report.json').write_text(json.dumps(report,indent=2)+'\n');raise
 (a.build/'report.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS strict image audit '+a.target+'; physical=not-run')
if __name__=='__main__':main()
