"""Build exported incomplete core; assessor sources require explicit arguments."""
import argparse,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];LAB=Path(__file__).resolve().parent
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--compiler',default='gcc');p.add_argument('--output',type=Path,required=True);p.add_argument('--os-source',type=Path,default=LAB/'os.c');p.add_argument('--base-source',type=Path,default=ROOT/'labs/os_observatory/src/oslab.c');a=p.parse_args();a.output.parent.mkdir(parents=True,exist_ok=True);raise SystemExit(subprocess.run([a.compiler,'-std=c17','-Wall','-Wextra','-Werror','-O2','-I'+str(LAB),'-I'+str(ROOT/'labs/os_observatory/include'),str(LAB/'tests.c'),str(a.os_source),str(a.base_source),'-o',str(a.output)]).returncode)
