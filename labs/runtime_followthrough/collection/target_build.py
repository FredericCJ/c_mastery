"""Strict M4 build: explicit original providers, no compiler/runtime archives."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
from port_registration import port
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
for key in ['source','objects','registrations','pool','cvm','runtime','division','signed-c','signed-asm','driver','headers']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',type=int,choices=range(7),default=0);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
flags=['-std=c17','-O'+a.opt,'-Wall','-Wextra','-Werror','-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DVG_STANDALONE','-DVG_MUTANT='+str(a.mutant),'-I'+str(HERE),'-I'+str(LABS/'object_runtime/include'),'-I'+str(LABS/'algorithms/include'),'-isystem',str(a.headers),'-I'+str(LABS/'runtime/include'),'-I'+str(LABS/'machine_image/freestanding_depth')]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
registration_source=port(a.registrations,a.build/'registration_port.c')
sources=[a.source,a.cvm,HERE/'tests.c',a.runtime,a.division,a.signed_c,a.signed_asm,a.driver,HERE/'startup.S',a.objects,registration_source,a.pool];objects=[]
for i,s in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);cmd=[a.cc,*flags,*(['-Dmain=vg_public_tests'] if i==2 else []),*([] if s.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)];subprocess.run(cmd,check=True)
 subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections','-T',str(HERE/'link.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps={HERE/'link.ld'}
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n')
print('VG_TARGET_BUILD_PASS explicit_objects=12 original_limb_division')
