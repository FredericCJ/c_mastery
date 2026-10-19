"""Run one PI case using explicitly provided CMX and extension implementations."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE = Path(__file__).resolve().parent


def command(compiler, flags, producer, include, extension, output, case, mutant=0, trace_mutant=0):
    return [compiler, '-std=c17', '-Wall', '-Wextra', '-Werror', *flags, '-I'+str(HERE), '-I'+str(include),
            str(producer), str(extension), str(HERE/('pi_model.c' if case=='model' else 'pi_tests.c')),
            '-DCDPI_MUTANT='+str(mutant), '-DCDPI_TRACE_MUTANT='+str(trace_mutant), '-o', str(output)]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execution-source', required=True, type=Path)
    parser.add_argument('--execution-include', required=True, type=Path)
    parser.add_argument('--extension-source', required=True, type=Path)
    parser.add_argument('--compiler', default='cc')
    parser.add_argument('--case', required=True, choices=('chain','nested','limits','costs','model'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='pi-depth-') as directory:
        binary=Path(directory)/'pi'
        build=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.extension_source,binary,args.case))
        if build.returncode:return build.returncode
        return subprocess.run([str(binary)]+([] if args.case=='model' else [args.case])).returncode


if __name__=='__main__':raise SystemExit(main())
