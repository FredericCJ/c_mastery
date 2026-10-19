"""Explicit pinned FreeRTOS application integration; no source fetching or flashing."""
import argparse,json,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;COMMIT='0adc196d4bd52a2d91102b525b0aafc1e14a2386'
HELPERS={'memset','memcpy','memmove','memcmp','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memset','__aeabi_memset4','__aeabi_memset8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'}
def build(args,run):
 args.build.mkdir(parents=True,exist_ok=True);dep=args.freertos_root
 if run(['git','-C',dep,'rev-parse','HEAD']).strip()!=COMMIT or run(['git','-C',dep,'status','--porcelain']).strip():raise ValueError('Use clean inspected V11.2.0 commit')
 port=dep/'portable/GCC/ARM_CM4F';compiler=['arm-none-eabi-gcc'] if args.compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',args.optimization,'-Wall','-Wextra','-Werror','-UNDEBUG','-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DCR_FP='+str(args.fp),'-DCR_EXTRA_TASK='+str(args.extra_task),'-DCR_TARGET_MUTANT='+str(args.mutant),'-I'+str(HERE),'-I'+str(args.freestanding_include),'-I'+str(dep/'include'),'-I'+str(port)]
 if args.compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 sources=[args.core,args.adapter,HERE/'m4.c',args.helpers,dep/'tasks.c',dep/'queue.c',dep/'list.c',port/'port.c'];objects=[]
 for i,source in enumerate(sources):
  obj=args.build/(str(i)+'.o');abi='softfp' if args.fp or source.is_relative_to(dep) else 'soft';run([*compiler,*flags,'-mfloat-abi='+abi,'-MMD','-MF',args.build/(str(i)+'.d'),'-c',source,'-o',obj]);objects.append(obj)
 obj=args.build/'probes.o';run([*compiler,'-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=softfp','-c',HERE/'probes.S','-o',obj]);objects.append(obj)
 binary=args.build/'ownership.elf';mapping=args.build/'ownership.map';run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=softfp','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',args.linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',binary])
 undefined=run(['arm-none-eabi-nm','-u',binary]);symbols=run(['arm-none-eabi-nm','-n',binary]);assembly=run(['arm-none-eabi-objdump','-d','-M','reg-names-std',binary]);size=run(['arm-none-eabi-size',binary]);names={line.split()[-1] for line in symbols.splitlines() if line.split()}
 forbidden={n for n in names if n.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and n not in HELPERS}
 if undefined.strip() or forbidden or {'malloc','calloc','realloc','free','pvPortMalloc','xTaskCreate','xQueueGenericCreate'}&names or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared target dependency')
 for name,text in [('symbols.txt',symbols),('undefined.txt',undefined),('assembly.txt',assembly),('size.txt',size)]:(args.build/name).write_text(text,encoding='utf-8',newline='\n')
 return binary,{'upstream_commit':COMMIT,'sources':[str(p) for p in sources],'fp':args.fp,'extra_task':args.extra_task,'mutant':args.mutant,'helpers':sorted(names&HELPERS),'size':size,'scope':'Actual static FreeRTOS CM4F application; task/interrupt FP context is separate from physical timing.'}
def execute(binary):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',binary]
def parser():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('core','adapter','freertos-root','freestanding-include','helpers','linker','build'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--compiler',choices=('gcc','clang'),default='gcc');p.add_argument('--optimization',default='-O0');p.add_argument('--fp',type=int,choices=(0,1),default=1);p.add_argument('--extra-task',type=int,choices=(0,1),default=0);p.add_argument('--mutant',type=int,choices=(0,1,2,3),default=0);return p
if __name__=='__main__':
 args=parser().parse_args()
 def run(command):
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=60)
  if result.returncode:raise ValueError(result.stdout+result.stderr)
  return result.stdout+result.stderr
 binary,audit=build(args,run);(args.build/'audit.json').write_text(json.dumps(audit,indent=2)+'\n',encoding='utf-8',newline='\n');raise SystemExit(subprocess.run(list(map(str,execute(binary))),timeout=30).returncode)
