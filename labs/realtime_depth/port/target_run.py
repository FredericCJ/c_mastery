"""Build/run a three-PSP-stack budget guest using only explicitly supplied imports."""
import argparse
from pathlib import Path
import re
import subprocess
HERE=Path(__file__).resolve().parent
HELPERS={'memset','memcpy','memmove','memcmp','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8',
         '__aeabi_memset','__aeabi_memset4','__aeabi_memset8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'}

def image(args,run):
    args.build.mkdir(parents=True,exist_ok=True)
    compiler=['arm-none-eabi-gcc'] if args.compiler=='gcc' else ['clang','--target=arm-none-eabi']
    flags=['-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=softfp','-std=c17',args.optimization,
      '-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections',
      '-fno-unwind-tables','-fno-asynchronous-unwind-tables','-Wall','-Wextra','-Werror',
      '-I'+str(HERE),'-I'+str(args.execution_include),'-I'+str(args.server_include),
      '-DCMX_STAGE=2','-DCMX_QEMU','-DCMX_STAGE_NAME="server-port"','-DSR_SCENARIO='+str(args.scenario),
      '-DSR_MUTANT='+str(args.mutant)]
    if args.compiler=='gcc':flags+=['-mgeneral-regs-only','-fno-tree-loop-distribute-patterns']
    else:flags+=['-mno-implicit-float']
    sources=[args.runtime/name for name in ('startup.S','context.S','cortex_port.c','board.c','support.c')]
    sources += [args.execution_source,args.advanced_source,args.server_source,args.source,HERE/'workload.c']
    objects=[]
    for index,source in enumerate(sources):
        extra=['-Wno-unused-parameter'] if getattr(args,'allow_unused_import_parameters',False) and source==args.execution_source else []
        obj=args.build/(str(index)+'.o');run([*compiler,*flags,*extra,'-c',source,'-o',obj]);objects.append(obj)
    elf=args.build/'task.elf';mapping=args.build/'task.map'
    run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,
      '-T',args.linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',elf])
    undefined=run(['arm-none-eabi-nm','-u',elf]);symbols=run(['arm-none-eabi-nm','-n',elf]);assembly=run(['arm-none-eabi-objdump','-d',elf])
    (args.build/'symbols.txt').write_text(symbols,encoding='utf-8',newline='\n')
    (args.build/'assembly.txt').write_text(assembly,encoding='utf-8',newline='\n')
    names={line.split()[-1] for line in symbols.splitlines() if line.split()}
    size=run(['arm-none-eabi-size',elf])
    (args.build/'size.txt').write_text(size,encoding='utf-8',newline='\n')
    forbidden={s for s in names if s.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and s not in HELPERS}
    if undefined.strip() or forbidden or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):
        raise ValueError('Undeclared runtime import')
    if not {'cmx_reset','SVC_Handler','PendSV_Handler','SysTick_Handler','cmx_port_choose'}<=names:
        raise ValueError('Retained runtime boundary absent')
    if not all(token in assembly.lower() for token in ('psp','vstm','vldm','cpsid','cpsie','bkpt')):
        raise ValueError('Required context/exception boundary instructions absent')
    return elf,{'sources':[str(p) for p in sources],'linked_course_helpers':sorted(names&HELPERS),'undefined_symbols':[],
      'forbidden_helpers':[],'whole_image_size_output':size,'context_instructions_inspected':['psp','vstm','vldm','cpsid','cpsie','bkpt']}

def qemu(elf):
    return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none',
      '-icount','shift=0,align=off,sleep=off','-semihosting-config','enable=on,target=native','-kernel',elf]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('runtime','execution-source','advanced-source','execution-include','server-source','server-include','source','linker','build'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',choices=('gcc','clang'),default='gcc');parser.add_argument('--optimization',default='-O0')
    parser.add_argument('--scenario',type=int,choices=(0,1,2,3),required=True);parser.add_argument('--mutant',type=int,default=0)
    parser.add_argument('--allow-unused-import-parameters',action='store_true',help='Only for intentionally unsolved kernel-import controls; retain every semantic acceptance check.')
    args=parser.parse_args()
    def run(command):
        result=subprocess.run([str(x) for x in command],capture_output=True,text=True,timeout=60)
        if result.returncode:raise ValueError(result.stdout+result.stderr)
        return result.stdout+result.stderr
    elf,_=image(args,run);result=subprocess.run([str(x) for x in qemu(elf)],timeout=30)
    return result.returncode
if __name__=='__main__':raise SystemExit(main())
