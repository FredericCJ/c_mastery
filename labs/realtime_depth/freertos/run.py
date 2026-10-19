"""Explicit separate CMX/FreeRTOS builds; no implicit assessor discovery or dependency fetching."""
import argparse,json,re,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
COMMIT='0adc196d4bd52a2d91102b525b0aafc1e14a2386'
HELPERS={'memset','memcpy','memmove','memcmp','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memset','__aeabi_memset4','__aeabi_memset8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'}
def build(args,run):
 args.build.mkdir(parents=True,exist_ok=True)
 target=args.target=='m4';vendor=args.kind=='vendor';compiler=[args.compiler]
 if target:compiler=['arm-none-eabi-gcc'] if args.compiler=='gcc' else ['clang','--target=arm-none-eabi']
 flags=['-std=c17',args.optimization,'-Wall','-Wextra','-Werror','-UNDEBUG','-DFC_MUTANT='+str(args.mutant),'-DFC_TICK_RATE='+str(args.rate),'-I'+str(HERE)]
 if target:
  abi=['-mfloat-abi=softfp','-mfpu=fpv4-sp-d16'] if vendor else ['-mfloat-abi=soft']
  flags+=['-mcpu=cortex-m4','-mthumb',*abi,'-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DFC_M4','-I'+str(HERE/'freestanding_include')]
  if args.compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
 else:flags+=['-D_POSIX_C_SOURCE=200809L','-pthread',*getattr(args,'extra_flags',[])]
 if getattr(args,'producer_stub',False):flags+=['-Wno-unused-parameter']
 sources=[args.mapping,args.adapter,HERE/('m4.c' if target else 'host.c')]
 external=[]
 if vendor:
  dep=args.freertos_root
  if dep is None:raise ValueError('Explicit pinned FreeRTOS root required')
  actual=run(['git','-C',dep,'rev-parse','HEAD']).strip();dirty=run(['git','-C',dep,'status','--porcelain']).strip()
  if actual!=COMMIT or dirty:raise ValueError('Use the clean inspected FreeRTOS V11.2.0 commit')
  port=dep/('portable/GCC/ARM_CM4F' if target else 'portable/ThirdParty/GCC/Posix')
  external=[dep/'tasks.c',dep/'queue.c',dep/'list.c',port/'port.c']
  if not target:external += [port/'utils/wait_for_event.c']
  sources+=external;flags += ['-DFC_VENDOR','-I'+str(dep/'include'),'-I'+str(port),'-I'+str(port/'utils')]
 else:
  if not args.execution_source or not args.execution_include:raise ValueError('Explicit CMX source and header required')
  sources+=[args.execution_source];flags+=['-I'+str(args.execution_include)]
 if target:
  if not args.support_source or not args.linker:raise ValueError('Explicit freestanding course helpers/linker required')
  sources+=[args.support_source]
 if any(not p.is_file() for p in sources):raise ValueError('Source inputs must be existing files')
 objects=[]
 for i,source in enumerate(sources):
  obj=args.build/(str(i)+'.o');obj.unlink(missing_ok=True)
  compile_flags=[flag for flag in flags if flag!='-no-pie']
  run([*compiler,*compile_flags,'-MMD','-MF',args.build/(str(i)+'.d'),'-c',source,'-o',obj]);objects.append(obj)
 binary=args.build/('comparison.elf' if target else 'comparison');mapping=args.build/'comparison.map'
 if target:
  command=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb',*abi,'-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',args.linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',binary]
 else:command=[args.compiler,*flags,*objects,'-Wl,-Map='+str(mapping),'-o',binary]
 run(command)
 prefix='arm-none-eabi-' if target else ''
 undefined=run([prefix+'nm','-u',binary]);symbols=run([prefix+'nm','-n',binary]);assembly=run([prefix+'objdump','-d',binary]);size=run([prefix+'size',binary])
 names={line.split()[-1].split('@')[0] for line in symbols.splitlines() if line.split()}
 imported={line.split()[-1].split('@')[0] for line in undefined.splitlines() if line.split()}
 if not vendor and any(n.startswith(('xQueue','xTask','vTask','vPort','pvPort','xPort')) for n in names):raise ValueError('Original image contains foreign-kernel code')
 if vendor and any(n.startswith('cmx_') for n in names):raise ValueError('Foreign-kernel comparison contains original runtime')
 if vendor and any(n in names for n in ('pvPortMalloc','vPortFree','xTaskCreate','xQueueGenericCreate')):raise ValueError('Dynamic FreeRTOS object path linked')
 if target:
  if not vendor and re.search(r'\bv(?:push|pop|ldr|str|mov|stm|ldm)',assembly):raise ValueError('Unexpected floating-point-register instruction in original soft-ABI image')
  forbidden={n for n in names if n.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and n not in HELPERS}
  if undefined.strip() or forbidden or {'malloc','calloc','realloc','free'}&names or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):raise ValueError('Undeclared runtime helper/allocation dependency')
 else:forbidden=set()
 for name,text in [('symbols.txt',symbols),('undefined.txt',undefined),('assembly.txt',assembly),('size.txt',size)]: (args.build/name).write_text(text,encoding='utf-8',newline='\n')
 return binary,{'kind':args.kind,'target':args.target,'sources':[str(p) for p in sources],'upstream_commit':COMMIT if vendor else None,'linked_course_helpers':sorted(names&HELPERS),'forbidden_helpers':sorted(forbidden),'undefined_symbols':sorted(imported),'size':size,'stack_usage_files':[p.name for p in args.build.glob('*.su')], 'host_allocation_scope':'POSIX port and pthread runtime allocate host objects; only FreeRTOS task/queue/control allocation is static.' if vendor and not target else 'No target heap or hidden runtime dependencies.' if target else 'Original kernel caller storage; hosted test harness is separate.'}
def execute(args,binary):
 if args.target=='host':return [binary]
 return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',binary]
def parser():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('mapping','adapter','build'):p.add_argument('--'+name,type=Path,required=True)
 for name in ('execution-source','execution-include','freertos-root','support-source','linker'):p.add_argument('--'+name,type=Path)
 p.add_argument('--kind',choices=('original','vendor'),required=True);p.add_argument('--target',choices=('host','m4'),default='host')
 p.add_argument('--compiler',choices=('gcc','clang'),default='gcc');p.add_argument('--optimization',default='-O0');p.add_argument('--mutant',type=int,default=0);p.add_argument('--rate',type=int,default=100)
 return p
if __name__=='__main__':
 args=parser().parse_args()
 def run(command):
  result=subprocess.run([str(x) for x in command],capture_output=True,text=True,timeout=60)
  if result.returncode:raise ValueError(result.stdout+result.stderr)
  return result.stdout+result.stderr
 binary,audit=build(args,run);(args.build/'audit.json').write_text(json.dumps(audit,indent=2)+'\n',encoding='utf-8',newline='\n')
 raise SystemExit(subprocess.run([str(x) for x in execute(args,binary)],timeout=30).returncode)
