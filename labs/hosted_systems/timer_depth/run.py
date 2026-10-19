"""Compile a submitted timer implementation; this runner supplies no solution."""
import argparse,pathlib,subprocess,tempfile
HERE=pathlib.Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=pathlib.Path,default=HERE/'timer.c');p.add_argument('--native-source',type=pathlib.Path,default=HERE/'native.c');p.add_argument('--deadline-source',type=pathlib.Path,default=HERE.parent/'src/deadline.c');p.add_argument('--cc',default='cc');p.add_argument('--mode',choices=['core','races','event','periodic','rearm','cached','failure'],required=True);a=p.parse_args()
 native=a.mode not in ('core','races');test=a.native_source if native else HERE/('tests.c' if a.mode=='core' else 'race_tests.c')
 with tempfile.TemporaryDirectory(prefix='cmastery-timer-') as directory:
  exe=pathlib.Path(directory)/'timer';argv=[a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pthread','-I'+str(HERE),'-I'+str(HERE.parent/'include'),str(test),str(a.source),*([str(a.deadline_source)] if native else []),'-o',str(exe)]
  print(argv,flush=True);r=subprocess.run(argv)
  if r.returncode:return r.returncode
  return subprocess.run([str(exe),*([a.mode] if native else [])],timeout=15).returncode
if __name__=='__main__':raise SystemExit(main())
