"""Explicit learner implementation; no assessor fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'ip.c');p.add_argument('--build',type=Path,required=True);p.add_argument('--systems',type=Path,required=True);p.add_argument('--checksum-slice',action='store_true');p.add_argument('--cc',default='cc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
if a.checksum_slice:
 from checksum_slice import emit
 dst=a.build/'checksum.c';emit(a.systems,dst);a.systems=dst
exe=a.build/'ip';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),'-I'+str(LABS/'b045_systems/include'),str(a.source),str(a.systems),str(HERE/'tests.c'),'-o',str(exe)],check=True);raise SystemExit(subprocess.run([str(exe)]).returncode)
