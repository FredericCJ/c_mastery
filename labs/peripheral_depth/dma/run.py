"""Learner implementations explicit; target object check is not board execution."""
from pathlib import Path
import argparse,subprocess
from audit_target import validate
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[2]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--notice',action='store_true');p.add_argument('--target',action='store_true');p.add_argument('--port',action='store_true');p.add_argument('--h745',action='store_true');p.add_argument('--type-headers',type=Path);p.add_argument('--cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);a.target=a.target or a.port;a.source=a.source or HERE/('notice_port.c' if a.port else 'target.c' if a.target else 'notice.c' if a.notice else 'dma.c');a.cc=a.cc or ('arm-none-eabi-gcc' if a.target else 'cc')
inc=['-I'+str(HERE),'-I'+str(ROOT/'labs/firmware_lifecycle/targets')]
if a.target:
 obj=a.build/'target.o';flags=['-std=c17','-O2','-mcpu='+('cortex-m7' if a.h745 else 'cortex-m4'),'-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror','-DPD_BOARD='+str(int(a.h745)),*inc]
 if a.type_headers:flags+=['-isystem',str(a.type_headers)]
 subprocess.run([a.cc,*flags,'-c',str(a.source),'-o',str(obj)],check=True)
 nm=subprocess.check_output(['arm-none-eabi-nm','-u',str(obj)],text=True);dis=subprocess.check_output(['arm-none-eabi-objdump','-dr',str(obj)],text=True);(a.build/'disassembly.txt').write_text(dis)
 result=validate(dis,int(a.h745),a.port);imports={line.split()[-1] for line in nm.splitlines() if line.strip()};allowed={'pd_notice','fw_h745_cache','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memclr','__aeabi_memclr4'} if a.port else {'pd_validate','fw_h745_cache'}
 ok=imports<=allowed and result['passed'];print(result);print('PD_TARGET_PASS structural only' if ok else 'PD_TARGET_REJECT');raise SystemExit(0 if ok else 1)
exe=a.build/'dma';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*inc,str(a.source),str(HERE/('notice_tests.c' if a.notice else 'tests.c')),'-o',str(exe)],check=True);raise SystemExit(subprocess.run([str(exe.resolve())]).returncode)
