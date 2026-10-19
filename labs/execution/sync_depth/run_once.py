"""Run once cases with explicit completed CMX, MBOnce and adapter imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
CASES=('publication','failure','retry','waiting','policies','limits')

def command(compiler,flags,execution,execution_include,bridge,bridge_include,extension,output,mutant=0,native=False):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,
            '-I'+str(HERE),'-I'+str(execution_include),'-I'+str(bridge_include),
            str(execution),str(bridge),str(extension),str(HERE/('once_native_tests.c' if native else 'once_tests.c')),
            '-DCDO_MUTANT='+str(mutant),'-o',str(output)]+(['-pthread'] if native else [])

def main(native=False):
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','bridge-source','bridge-include','once-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('success','failure') if native else CASES)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='once-depth-') as directory:
        binary=Path(directory)/'once'
        result=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,
            args.bridge_source,args.bridge_include,args.once_source,binary,native=native))
        if result.returncode:return result.returncode
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
