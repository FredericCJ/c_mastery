"""Explicit learner/provider inputs, original runtime and direct linker closure."""
from pathlib import Path
import argparse,subprocess,hashlib,json,shlex,shutil
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
for name in ['source','stream','wire','driver','integration','pair','pair-test','http','net','runtime','strings']:p.add_argument('--'+name,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',choices=['none','length','dma','ack','stream','padding','admission','pair','stack'],default='none');p.add_argument('--linker',type=Path,default=HERE/'target.ld');p.add_argument('--headers',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DNW_TARGET','-I'+str(HERE),'-I'+str(LABS/'communications/include'),'-isystem',str(a.headers)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
defines={'length':'NW_MUTANT=1','dma':'NW_MUTANT=3','ack':'NW_MUTANT=4','stream':'NW_STREAM_MUTANT=1','padding':'NW_WIRE_MUTANT=1','admission':'NW_WIRE_MUTANT=2','pair':'NW_PAIR_MUTANT=1','stack':'NW_PAIR_MUTANT=2'}
if a.mutant!='none':flags+=['-D'+defines[a.mutant]]
objects=[]
for i,s in enumerate([getattr(a,k) for k in ['source','stream','wire','driver','integration','pair','pair_test','http','net','runtime','strings']]+[HERE/'target.S']):
    obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,*([] if s.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(a.linker),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps=set()
for f in a.build.glob('*.d'):
    deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
manifest={str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)}
(a.build/'external-inputs.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime')
