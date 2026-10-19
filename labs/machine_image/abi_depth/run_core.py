"""Compile a learner's explicit ABI component imports. No assessor lookup."""
import argparse,subprocess,tempfile
from pathlib import Path
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--sum-source',required=True,type=Path);p.add_argument('--mix-source',required=True,type=Path)
    p.add_argument('--machine-include',required=True,type=Path);p.add_argument('--compiler',default='gcc')
    p.add_argument('--api-source',type=Path);p.add_argument('--boundary-source',type=Path)
    args=p.parse_args();here=Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix='cm-abi-') as directory:
        exe=Path(directory)/'abi'
        argv=[args.compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-g',
          '-I'+str(here),'-I'+str(args.machine_include.resolve()),str(here/'tests.c'),str(here/'hostile.S'),
          str((args.api_source or here/'api.c').resolve()),str((args.boundary_source or here/'boundary.S').resolve()),
          str(args.sum_source.resolve()),str(args.mix_source.resolve()),'-o',str(exe)]
        subprocess.run(argv,check=True)
        for case in ('values','aggregate','hostile','call8','inline','cfi'):subprocess.run([str(exe),case],check=True)
    return 0
if __name__=='__main__':raise SystemExit(main())
