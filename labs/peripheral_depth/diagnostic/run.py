"""Public runner requires explicit reference paths; learner stubs remain default."""
from pathlib import Path
import argparse,subprocess
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
p=argparse.ArgumentParser();p.add_argument('--kind',choices=['core','usb','host'],default='core');p.add_argument('--source',type=Path);p.add_argument('--adapter',type=Path);p.add_argument('--payload',type=Path);p.add_argument('--foundation',type=Path);p.add_argument('--ethernet',type=Path);p.add_argument('--cc',default='cc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
exe=a.build/'diagnostic'
sources=[a.source or HERE/('diagnostic.c' if a.kind=='core' else a.kind+'.c'),a.foundation or ROOT/'labs/foundations/src/foundation.c']
sources+=([a.adapter or HERE/'ethernet.c',a.ethernet or ROOT/'labs/ethernet_depth/src/ethernet_depth.c',HERE/'tests.c'] if a.kind=='core' else [a.payload or HERE/'diagnostic.c',HERE/(a.kind+'_tests.c')])
subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(HERE),'-I'+str(ROOT/'labs/foundations/include'),'-I'+str(ROOT/'labs/ethernet_depth/include'),*map(str,sources),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe.resolve())]).returncode)
