"""Standalone public harness: source selection is explicit; default is unfinished."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def command(cc,flags,source,output,test='tests.c',mutant=0):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DTA_MUTANT='+str(mutant),'-I'+str(HERE),str(source),str(HERE/test),'-o',str(output)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,default=HERE/'architecture.c');p.add_argument('--output',type=Path,required=True);p.add_argument('--cc',default='cc');p.add_argument('--test',choices=['tests.c','replay.c','socket.c'],default='tests.c');a=p.parse_args();raise SystemExit(subprocess.run(command(a.cc,['-O0','-g'],a.source,a.output,a.test)).returncode)
