"""Compile an explicitly selected learner implementation; no assessor fallback."""
import argparse,subprocess,tempfile
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--implementation',required=True,nargs='+',type=Path);p.add_argument('--cc',default='cc');a=p.parse_args();here=Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='cmastery-startup-') as directory:
 exe=Path(directory)/'tests';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-I'+str(here),here/'tests.c',*[x.resolve() for x in a.implementation],'-o',exe],check=True);raise SystemExit(subprocess.run([exe],timeout=30).returncode)
