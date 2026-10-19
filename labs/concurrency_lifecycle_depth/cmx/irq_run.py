"""Standalone IRQ0 teaching image; every completed implementation is an explicit argument."""
import argparse
import json
from pathlib import Path
import re
import subprocess
from task_run import HELPERS,qemu
HERE=Path(__file__).resolve().parent
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','lifecycle-source','lifecycle-include','adapter-source',
                 'irq-source','mask-source','mask-include','support-source','linker','build'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();args.build.mkdir(parents=True,exist_ok=True)
    flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17','-O0','-ffreestanding','-fno-builtin',
      '-fno-stack-protector','-fno-tree-loop-distribute-patterns','-ffunction-sections','-fdata-sections',
      '-fno-unwind-tables','-fno-asynchronous-unwind-tables','-Wall','-Wextra','-Werror',
      *['-I'+str(p) for p in (HERE,args.execution_include,args.lifecycle_include,args.mask_include)]]
    def run(command):
        result=subprocess.run([str(p) for p in command],capture_output=True,text=True,timeout=60)
        if result.returncode:raise ValueError(result.stdout+result.stderr)
        return result.stdout+result.stderr
    sources=[args.execution_source,args.support_source,args.lifecycle_source,args.adapter_source,args.irq_source,args.mask_source,HERE/'m4.c'];objects=[]
    for index,source in enumerate(sources):
        obj=args.build/(str(index)+'.o');run(['arm-none-eabi-gcc',*flags,'-c',source,'-o',obj]);objects.append(obj)
    elf=args.build/'irq.elf';mapping=args.build/'irq.map'
    run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,'-T',args.linker,
      '-Wl,--gc-sections','-Wl,-Map='+str(mapping),'-o',elf])
    undefined=run(['arm-none-eabi-nm','-u',elf]);symbols=run(['arm-none-eabi-nm','-n',elf]);assembly=run(['arm-none-eabi-objdump','-d',elf])
    names={line.split()[-1] for line in symbols.splitlines() if line.split()}
    forbidden={s for s in names if s.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and s not in HELPERS}
    if undefined.strip() or forbidden or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping.read_text(encoding='utf-8')):
        raise ValueError('Undeclared runtime import')
    if 'bkpt' not in assembly:raise ValueError('Semihost instruction absent')
    (args.build/'assembly.txt').write_text(assembly,encoding='utf-8',newline='\n')
    (args.build/'symbols.txt').write_text(symbols,encoding='utf-8',newline='\n')
    print(json.dumps({'undefined_symbols':[],'forbidden_helpers':[],'linked_course_helpers':sorted(names&HELPERS)}),flush=True)
    return subprocess.run([str(p) for p in qemu(elf)],timeout=30).returncode
if __name__=='__main__':raise SystemExit(main())
