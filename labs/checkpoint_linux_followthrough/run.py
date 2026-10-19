"""Build the public bridge test with explicit selected learner providers."""
from pathlib import Path
import argparse,subprocess
p=argparse.ArgumentParser()
for key in ['bridge','integration','foundation','algorithms','build']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--include',action='append',type=Path,required=True);p.add_argument('--cc',default='gcc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);here=Path(__file__).resolve().parent;exe=a.build/'p5'
subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-ffunction-sections','-fdata-sections','-I'+str(here),*['-I'+str(x) for x in a.include],str(a.bridge),str(a.integration),str(a.foundation),str(a.algorithms),str(here/'bridge_tests.c'),'-Wl,--gc-sections','-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
