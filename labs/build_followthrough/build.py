"""Build a learner implementation. References require an explicit source path."""
import argparse,subprocess
from pathlib import Path
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--simd-source',type=Path);p.add_argument('--kind',choices=['formal','performance'],default='formal');p.add_argument('--cc',default='cc');p.add_argument('--output',type=Path,required=True);a=p.parse_args();here=Path(__file__).resolve().parent
 sources=[a.source or here/(a.kind+'.c'),here/(a.kind+'_tests.c')]
 if a.kind=='performance':sources.append(a.simd_source or here/'simd.c')
 return subprocess.call([a.cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-I',str(here),*map(str,sources),'-o',str(a.output)])
if __name__=='__main__':raise SystemExit(main())
