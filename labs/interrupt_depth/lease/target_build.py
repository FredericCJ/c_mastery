"""Link selected implementations and original runtime without compiler libraries."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser()
for key in ['build','runtime','dma','cache','composition']:
    p.add_argument('--'+key,type=Path,required=True)
for key,default in [('source','lease.c'),('bridge','bridge.c'),('order','order.c'),('driver','target.c')]:p.add_argument('--'+key,type=Path,default=HERE/default)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DXD_TARGET','-I'+str(HERE),'-I'+str(LABS/'peripheral_depth/dma'),'-I'+str(LABS/'firmware_lifecycle/targets')]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
objects=[]
for i,s in enumerate([a.source,a.bridge,a.order,a.driver,a.composition,a.dma,a.cache,a.runtime,HERE.parent/'handoff/target.S']):
    obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,'-c',str(s),'-o',str(obj)],check=True)
    subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE.parent/'frame/target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
print('XD_TARGET_BUILD_PASS objects='+str(len(objects)))
