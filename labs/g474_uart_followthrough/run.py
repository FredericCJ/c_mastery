"""Public hosted fixture. Explicit learner source required; never runs physical.c."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--cc',default='cc');p.add_argument('--output',type=Path,required=True);a=p.parse_args()
a.output.parent.mkdir(parents=True,exist_ok=True)
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),str(a.source),str(HERE/'tests.c'),'-o',str(a.output)],check=True)
raise SystemExit(subprocess.run([str(a.output.resolve())]).returncode)
