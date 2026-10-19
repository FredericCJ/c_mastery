"""Build exact explicitly named providers; audit direct closure without archives."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess,sys
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--providers',type=Path,required=True);p.add_argument('--headers',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--driver',type=Path,required=True);p.add_argument('--include',type=Path,action='append',required=True);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--mutant',type=int,choices=range(3),default=0);p.add_argument('--board',type=Path,required=True);p.add_argument('--phy-port',type=Path,required=True);p.add_argument('--ring-port',type=Path,required=True);p.add_argument('--bad-placement',action='store_true');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
providers=json.loads(a.providers.read_text());keys=['tcp','arp','ip','scope','stream','http','systems','ring','ring_model','phy','phy_model','ethernet','bringup','runtime','strings','network_driver','flow'];assert set(providers)==set(keys)
flags=['-std=c17','-O'+a.opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DNB_TARGET','-DNH_MUTANT='+str(a.mutant),'-DEN_TARGET_MODEL','-DEP_TARGET_MODEL','-DNB_MUTANT=0','-I'+str(HERE),*['-I'+str(p) for p in a.include],'-isystem',str(a.headers)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
checksum=a.build/'checksum.c';subprocess.run([sys.executable,HERE.parent/'ip/checksum_slice.py',providers['systems'],checksum],check=True)
sources=[a.source,HERE/'tests.c',a.driver,*[checksum if k=='systems' else Path(providers[k]) for k in keys],HERE.parent/'driver/target.S',a.board,a.phy_port,a.ring_port,HERE.parent/'driver/board_model.c'];objects=[]
for i,s in enumerate(sources):
 assert s.is_file(),s
 obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,*([] if s.suffix=='.S' else ['-Wall','-Wextra','-Werror','-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'image.elf';subprocess.run(['arm-none-eabi-ld','--gc-sections',*(['--defsym','ETH_BASE=0x10001000'] if a.bad_placement else []),'-T',str(HERE.parent/'driver/target.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip();deps=set()
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm']);deps.add(a.providers);deps.update(Path(v) for v in providers.values())
(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n');print('NH_TARGET_BUILD_PASS explicit_objects=25 original_runtime')
