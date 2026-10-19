"""Explicit source selection; target pointers are never dereferenced on host."""
from pathlib import Path
import argparse,subprocess,re
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--target',action='store_true');p.add_argument('--type-headers',type=Path);p.add_argument('--firmware-include',type=Path,default=HERE.parents[1]/'firmware_io/include');p.add_argument('--cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);exe=a.build/('target.o' if a.target else 'mmio');source=a.source or HERE/('target.c' if a.target else 'mmio.c');cc=a.cc or ('arm-none-eabi-gcc' if a.target else 'cc')
flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),'-I'+str(a.firmware_include)]
if a.target:flags+=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-c']+(['-isystem',str(a.type_headers)] if a.type_headers else [])
subprocess.run([cc,*flags,str(source),*([str(HERE/'tests.c')] if not a.target else []),'-o',str(exe)],check=True)
if not a.target:raise SystemExit(subprocess.run([str(exe.resolve())]).returncode)
nm=subprocess.run(['arm-none-eabi-nm','-u',str(exe)],capture_output=True,text=True,check=True).stdout
dis=subprocess.run(['arm-none-eabi-objdump','-d',str(exe)],capture_output=True,text=True,check=True).stdout
(a.build/'disassembly.txt').write_text(dis)
read=dis.split('<pm_target_read>:',1)[-1].split('<pm_target_write>:',1)[0];write=dis.split('<pm_target_write>:',1)[-1]
valid=not nm.strip() and re.search(r'\bldr(?:\.w)?\s+[^\n]*#16\]',read) and re.search(r'\bstr(?:eq|ne)?(?:\.w)?\s+[^\n]*#24\]',write) and not re.search(r'\b(?:ldrh|ldrb|strh|strb)\b',dis)
print('PM_TARGET_PASS word offsets and no imports; structural audit, not full semantic proof' if valid else 'PM_TARGET_REJECT missing/wrong word access or external import')
raise SystemExit(0 if valid else 1)
