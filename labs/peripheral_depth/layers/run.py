"""Run learner service or original byte-map implementation, with no private source fallback."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--cc',default='cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);source=a.source or HERE/'service.c';exe=a.build/'service'
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),str(source),str(HERE/'tests.c'),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe.resolve())]).returncode)
