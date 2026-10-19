"""Public native harness: caller explicitly supplies prerequisite implementations."""
import argparse,pathlib,subprocess,tempfile
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--implementation',type=pathlib.Path);p.add_argument('--provider',type=pathlib.Path,action='append',required=True);p.add_argument('--include',type=pathlib.Path,action='append',required=True);p.add_argument('--cc',default='cc');a=p.parse_args();here=pathlib.Path(__file__).resolve().parent
 sources=[a.implementation/'match.c'] if a.implementation else [here/'learner.c']
 for source in sources+a.provider:
  if not source.is_file():p.error('missing explicit provider/implementation: '+str(source))
 with tempfile.TemporaryDirectory(prefix='cmastery-match-') as directory:
  output=pathlib.Path(directory)/'contracts';args=[a.cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-O2','-ffunction-sections','-fdata-sections','-I',str(here)]
  for include in a.include:args+=['-I',str(include)]
  r=subprocess.run([*args,*map(str,sources+a.provider),str(here/'tests.c'),str(here/'native.c'),'-Wl,--gc-sections','-o',str(output)])
  if r.returncode:return r.returncode
  for case in ['0','1','2']:
   r=subprocess.run([str(output),case])
   if r.returncode:return r.returncode
 return 0
if __name__=='__main__':raise SystemExit(main())
