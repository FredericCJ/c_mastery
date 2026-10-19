"""Compile public artifacts; optional explicit sources are never auto-selected."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--mode',choices=['core','cancel','boundary','failure'],default='core');p.add_argument('--source');p.add_argument('--native-source');p.add_argument('--raw-source');a=p.parse_args();root=Path(__file__).resolve().parent;workspace=root.parents[2];raw=workspace/'labs/linux_io_depth/targets'
with tempfile.TemporaryDirectory(prefix='cmastery-completion-') as directory:
 exe=Path(directory)/'probe';sources=[root/'tests.c',Path(a.source) if a.source else root/'completion.c'];flags=['-I'+str(root)]
 if a.mode!='core':sources=[Path(a.native_source) if a.native_source else root/'native.c',sources[1],Path(a.raw_source) if a.raw_source else raw/'raw_ring.c'];flags+=['-I'+str(raw)]
 subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror',*flags,*map(str,sources),'-o',str(exe)],check=True);raise SystemExit(subprocess.run([str(exe),*([a.mode] if a.mode!='core' else [])]).returncode)
