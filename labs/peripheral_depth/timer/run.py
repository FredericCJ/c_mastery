"""Explicit learner components; assessor references must be selected outside export."""
from pathlib import Path
import argparse,subprocess
from audit_target import validate
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[2]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--clock-source',type=Path,default=ROOT/'labs/peripheral_depth/uart/uart.c');p.add_argument('--serial-source',type=Path,default=ROOT/'labs/serial_depth/src/serial_depth.c');p.add_argument('--target',action='store_true');p.add_argument('--type-headers',type=Path);p.add_argument('--cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);a.source=a.source or HERE/('target.c' if a.target else 'timer.c');a.cc=a.cc or ('arm-none-eabi-gcc' if a.target else 'cc')
inc=['-I'+str(v) for v in [HERE,ROOT/'labs/peripheral_depth/uart',ROOT/'labs/serial_depth/include']]
if a.target:
 obj=a.build/'target.o';flags=['-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror',*inc]
 if a.type_headers:flags+=['-isystem',str(a.type_headers)]
 subprocess.run([a.cc,*flags,'-c',str(a.source),'-o',str(obj)],check=True)
 nm=subprocess.check_output(['arm-none-eabi-nm','-u',str(obj)],text=True);dis=subprocess.check_output(['arm-none-eabi-objdump','-dr',str(obj)],text=True);(a.build/'disassembly.txt').write_text(dis)
 result=validate(dis);imports={line.split()[-1] for line in nm.splitlines() if line.strip()};ok=imports=={'pt_ratios'} and result['passed'];print(result);print('PT_TARGET_PASS structural only' if ok else 'PT_TARGET_REJECT');raise SystemExit(0 if ok else 1)
exe=a.build/'timer';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*inc,str(a.source),str(a.clock_source),str(a.serial_source),str(HERE/'tests.c'),'-o',str(exe)],check=True);raise SystemExit(subprocess.run([str(exe.resolve())]).returncode)
