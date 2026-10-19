"""Compile the actual providers and audit a direct-linked Cortex-M4 image."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--headers',type=Path,required=True)
for n in ['source','validation','composition','driver','systems','ip','scope','stream','http','runtime','strings']:p.add_argument('--'+n,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',type=int,choices=range(9),default=0);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DNC_TARGET','-DNC_MUTANT='+str(a.mutant),*['-I'+str(x) for x in [HERE,LABS/'network_followthrough/ip',LABS/'network_followthrough/scope',LABS/'communications/include',LABS/'b045_systems/include']],'-isystem',str(a.headers)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
sources=[a.source,HERE/'tests.c',a.validation,a.composition,a.driver,a.systems,a.ip,a.scope,a.stream,a.http,a.runtime,a.strings,HERE/'target.S'];objects=[]
for i,s in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);extra=['-Dmain='+{1:'nc_public_tests',2:'nc_validation',3:'nc_composition'}[i]] if i in [1,2,3] else []
 subprocess.run([a.cc,*flags,*extra,*([] if s.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps=set()
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n')
print('NC_TARGET_BUILD_PASS explicit_objects=13 original_runtime')
