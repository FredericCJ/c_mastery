"""Direct named-source freestanding link; declarations do not authorize libc."""
import argparse,hashlib,json,pathlib,shlex,shutil,subprocess,sys
from run import KEYS,INCLUDES
HERE=pathlib.Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--providers',type=pathlib.Path,required=True);p.add_argument('--root',type=pathlib.Path,required=True);p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c');p.add_argument('--headers',type=pathlib.Path,required=True);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--define',action='append',default=[]);a=p.parse_args()
providers=json.loads(a.providers.read_text());assert set(providers)==set(KEYS);a.build.mkdir(parents=True)
checksum=a.build/'checksum.c';subprocess.run([sys.executable,a.root/'labs/network_followthrough/ip/checksum_slice.py',providers['systems'],checksum],check=True)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DEN_TARGET_MODEL','-DEP_TARGET_MODEL','-DNI_TARGET','-I'+str(HERE),*['-I'+str(a.root/n) for n in INCLUDES],'-isystem',str(a.headers),*['-D'+n for n in a.define]]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
sources=[a.source,*[HERE/n for n in ['tests.c','stack.c','reassembly.c','guest.c']],*[checksum if k=='systems' else pathlib.Path(providers[k]) for k in KEYS],a.root/'labs/network_followthrough/driver/target.S'];objects=[]
for i,s in enumerate(sources):
 assert s.is_file();o=a.build/(str(i)+'.o');objects.append(o);cmd=[a.cc,*flags,*([] if s.suffix=='.S' else ['-Wall','-Wextra','-Werror','-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(o)];print(cmd,flush=True);subprocess.run(cmd,check=True);subprocess.run(['arm-none-eabi-nm','-u',o],check=True)
script=a.root/'labs/network_followthrough/driver/target.ld';elf=a.build/'image.elf';cmd=['arm-none-eabi-ld','--gc-sections','-T',str(script),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)];print(cmd,flush=True);subprocess.run(cmd,check=True);assert not subprocess.check_output(['arm-none-eabi-nm','-u',elf],text=True).strip()
deps={script,a.providers,pathlib.Path(__file__)}
for f in a.build.glob('*.d'):deps.update(pathlib.Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(pathlib.Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm'])
(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n');print('EH_TARGET_BUILD_PASS explicit_objects='+str(len(sources))+' no_libraries')
