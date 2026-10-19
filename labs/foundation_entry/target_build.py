"""Explicit Cortex-M4 entry exercise build, with original providers and no archives."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
for key in ['source','foundation','runtime','driver']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I'+str(HERE),'-I'+str(HERE.parent/'foundations/include')]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
sources=[a.source,a.foundation,a.runtime,a.driver,HERE/'startup.S'];objects=[];commands=[]
for i,src in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);cmd=[a.cc,*flags,*([] if src.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(src),'-o',str(obj)];commands.append(cmd);subprocess.run(cmd,check=True)
elf=a.build/'image.elf';cmd=['arm-none-eabi-ld','-T',str(HERE/'link.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)];commands.append(cmd);subprocess.run(cmd,check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps={HERE/'link.ld'}
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
(a.build/'build.json').write_text(json.dumps(dict(commands=commands,input_sha256={str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)}),indent=2)+'\n')
print('FE_TARGET_BUILD_PASS explicit_objects=5 no_archives')
