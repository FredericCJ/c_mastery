"""Build hosted adapter tests with explicit learner-owned producer imports."""
import argparse
from pathlib import Path
import subprocess,tempfile
HERE=Path(__file__).resolve().parent
def command(compiler,flags,source,server,advanced,execution,includes,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,
      '-I'+str(HERE),*['-I'+str(p) for p in includes],'-DSR_MUTANT='+str(mutant),
      HERE/'tests.c',source,server,advanced,execution,'-o',output]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('source','server-source','advanced-source','execution-source','execution-include','server-include'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--compiler',default='cc');parser.add_argument('--case',choices=('basic','guards','generation','stream'),required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='sr-port-') as directory:
        binary=Path(directory)/'tests'
        result=subprocess.run(command(args.compiler,['-O0'],args.source,args.server_source,args.advanced_source,args.execution_source,[args.execution_include,args.server_include],binary))
        return result.returncode if result.returncode else subprocess.run([binary,args.case]).returncode
if __name__=='__main__':raise SystemExit(main())
