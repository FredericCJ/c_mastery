"""Explicit C17 Cortex-M4 controller; original runtime, direct linker, no archive."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--headers',type=Path,required=True)
for n in ['source','driver','runtime']:p.add_argument('--'+n,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',type=int,choices=range(10),default=0);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DNR_MUTANT='+str(a.mutant),'-I'+str(HERE),'-isystem',str(a.headers)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
objects=[]
for i,s in enumerate([a.source,HERE/'tests.c',a.driver,a.runtime,HERE/'target.S']):
 obj=a.build/(str(i)+'.o');objects.append(obj)
 subprocess.run([a.cc,*flags,*(['-Dmain=nr_public_tests'] if i==1 else []),*([] if s.suffix=='.S' else ['-Wall','-Wextra','-Wpedantic','-Werror','-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)],check=True)
 subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps=set()
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n')
print('NR_TARGET_BUILD_PASS explicit_objects=5 original_runtime')
