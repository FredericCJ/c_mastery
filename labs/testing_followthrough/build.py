"""Learner implementation by default; assessor source requires explicit opt-in."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def command(cc,flags,kind,source,output,mutant=0):
 return [cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-D'+('TX' if kind=='transaction' else 'CV')+'_MUTANT='+str(mutant),'-I'+str(HERE),str(source),str(HERE/(kind+'_tests.c')),'-o',str(output)]
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--kind',choices=['transaction','coverage'],required=True);p.add_argument('--source',type=Path);p.add_argument('--output',type=Path,required=True);p.add_argument('--cc',default='cc');a=p.parse_args();raise SystemExit(subprocess.run(command(a.cc,['-O0','-g'],a.kind,a.source or HERE/(a.kind+'.c'),a.output)).returncode)
