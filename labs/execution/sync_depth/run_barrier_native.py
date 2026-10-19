"""Run hosted barrier with explicit completed CMX/core/native imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,execution,execution_include,core,native,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror','-pthread',*flags,
            '-I'+str(HERE),'-I'+str(execution_include),str(execution),str(core),str(native),
            str(HERE/'barrier_native_tests.c'),'-DCDB_MUTANT='+str(mutant),
            '-o',str(output)]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','barrier-source','native-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',choices=('central','tree'),default='central')
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='barrier-native-') as directory:
        binary=Path(directory)/'barrier'
        result=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,
            args.barrier_source,args.native_source,binary))
        if result.returncode:return result.returncode
        return subprocess.run([str(binary),args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
