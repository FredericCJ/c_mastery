"""Public runner: assessor source must be selected explicitly from outside export."""
import argparse,pathlib,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--source',type=pathlib.Path,default=pathlib.Path(__file__).with_name('learner.c'));p.add_argument('--cc',default='cc');a=p.parse_args();here=pathlib.Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='cm-posix-edition-') as d:
 image=pathlib.Path(d)/'exercise';subprocess.run([a.cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',str(here),str(a.source.resolve()),str(here/'tests.c'),'-o',str(image)],check=True)
 for case in ['close','read']:subprocess.run([str(image),case],check=True)
