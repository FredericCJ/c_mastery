"""Explicit source selection; no private reference fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--source',type=Path,default=HERE/'time.c');p.add_argument('--cc',default='cc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
exe=a.build/'time'
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),str(a.source),str(HERE/'tests.c'),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
