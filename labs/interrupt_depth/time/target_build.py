"""Explicit original components and helper implementations; no default libraries."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
for key in ['runtime','divide','divide-runtime','divide-abi','handoff','frame','lease','checkpoint','correlation','integration']:p.add_argument('--'+key,type=Path,required=True)
for key,name in [('source','time.c'),('driver','target.c')]:p.add_argument('--'+key,type=Path,default=HERE/name)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',choices=['none','clock','queue','interval'],default='none');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DXK_TARGET']
flags+=['-I'+str(x) for x in [HERE,HERE.parent/'handoff',HERE.parent/'frame',HERE.parent/'lease',LABS/'checkpoint_depth/include',LABS/'embedded_debug_depth/peripheral',LABS/'machine_image/freestanding_depth']]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
if a.mutant!='none':flags+=[{'clock':'-DCP_MUTANT=37','queue':'-DXI_MUTANT=2','interval':'-DXK_MUTANT=5'}[a.mutant]]
objects=[]
for i,s in enumerate([a.source,a.driver,a.integration,a.handoff,a.frame,a.lease,a.checkpoint,a.correlation,a.runtime,a.divide,a.divide_runtime,a.divide_abi,HERE/'target.S']):
    obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE.parent/'frame/target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
print('XK_TARGET_BUILD_PASS explicit_objects=13 original_division_helper')
