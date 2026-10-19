"""Explicit imported parser/queue inputs; default new wrapper remains unsolved."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def command(cc,flags,source,models,queue,models_include,queue_include,output,test='tests.c',mutant=0):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DTF_MUTANT='+str(mutant),'-I'+str(HERE),'-I'+str(models_include),'-I'+str(queue_include),str(source),str(models),str(queue),str(HERE/test),'-o',str(output)]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,default=HERE/'fuzzing.c');p.add_argument('--models',type=Path,required=True);p.add_argument('--queue',type=Path,required=True);p.add_argument('--models-include',type=Path,required=True);p.add_argument('--queue-include',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--test',choices=['tests.c','replay.c'],default='tests.c');p.add_argument('--cc',default='cc');a=p.parse_args();raise SystemExit(subprocess.run(command(a.cc,['-O0','-g'],a.source,a.models,a.queue,a.models_include,a.queue_include,a.output,a.test)).returncode)
