"""Build the model investigation with an explicit completed queue provider."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def command(cc,flags,source,provider,include,output,test='tests.c',mutant=0):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DTM_MUTANT='+str(mutant),'-I'+str(HERE),'-I'+str(include),str(source),str(provider),str(HERE/test),'-o',str(output)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,default=HERE/'models.c');p.add_argument('--provider',type=Path,required=True);p.add_argument('--provider-include',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--test',choices=['tests.c','replay.c','trace.c'],default='tests.c');p.add_argument('--cc',default='cc');a=p.parse_args();raise SystemExit(subprocess.run(command(a.cc,['-O0','-g'],a.source,a.provider,a.provider_include,a.output,a.test)).returncode)
