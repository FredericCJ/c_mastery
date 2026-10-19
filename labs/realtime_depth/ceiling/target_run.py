"""Explicit-import freestanding declared-user immediate-ceiling image and runtime-helper audit."""
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
    flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17',args.optimization,'-ffreestanding','-fno-builtin',
      '-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables',
      '-fstack-usage','-Wall','-Wextra','-Werror','-I'+str(HERE),'-I'+str(args.execution_include),'-DSC_MUTANT='+str(args.mutant)]
    if args.compiler=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
    if args.producer_stub:flags+=['-Wno-unused-parameter']
    sources=[args.source,args.execution_source,args.support_source,HERE/'m4.c'];objects=[]
    if any(not p.is_file() for p in [*sources,args.linker]) or not args.execution_include.is_dir():
        raise ValueError('Explicit source/linker inputs must be files and include inputs directories')
    for index,source in enumerate(sources):
        obj=args.build/(str(index)+'.o')
        if obj.exists():obj.unlink()
        run([*compiler,*flags,'-c',source,'-o',obj])
        if not obj.is_file():raise ValueError('Compiler did not produce the requested object')
        objects.append(obj)
    elf=args.build/'ceiling.elf';mapping=args.build/'ceiling.map'
    run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,
      '-T',args.linker,'-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',elf])
    undefined=run(['arm-none-eabi-nm','-u',elf]);symbols=run(['arm-none-eabi-nm','-n',elf]);assembly=run(['arm-none-eabi-objdump','-d',elf])
    (args.build/'symbols.txt').write_text(symbols,encoding='utf-8',newline='\n');(args.build/'assembly.txt').write_text(assembly,encoding='utf-8',newline='\n')
    names={line.split()[-1] for line in symbols.splitlines() if line.split()}
    forbidden={s for s in names if s.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and s not in HELPERS}
    if undefined.strip() or forbidden or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):
        raise ValueError('Undeclared runtime helper')
    if 'bkpt' not in assembly or 'sc_lock' not in names:raise ValueError('Expected actual image code absent')
    size=run(['arm-none-eabi-size',elf]);(args.build/'size.txt').write_text(size,encoding='utf-8',newline='\n')
    return elf,{'linked_course_helpers':sorted(names&HELPERS),'undefined_symbols':[],'forbidden_helpers':[],
      'sources':[str(p) for p in sources],'size_output':size,'stack_usage_files':[p.name for p in args.build.glob('*.su')]}
def qemu(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-nic','none',
 '-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','execution-source','execution-include','support-source','linker','build'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',choices=('gcc','clang'),default='gcc');parser.add_argument('--optimization',default='-O0')
    parser.add_argument('--mutant',type=int,default=0);parser.add_argument('--producer-stub',action='store_true');args=parser.parse_args()
    def run(command):
        result=subprocess.run([str(p) for p in command],capture_output=True,text=True,timeout=60)
        if result.returncode:raise ValueError(result.stdout+result.stderr)
        return result.stdout+result.stderr
    elf,_=image(args,run);return subprocess.run([str(p) for p in qemu(elf)],timeout=30).returncode
if __name__=='__main__':raise SystemExit(main())
