"""Build exact wait cases with explicit completed CMX and adapter source imports."""
import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CASES = ('enrollment', 'terminal', 'donation', 'native', 'model')


def command(compiler, flags, execution_source, execution_include, adapter_source, output,
            case, mutant=0, native_mutant=0):
    test = 'native.c' if case == 'native' else ('trace.c' if case == 'model' else 'tests.c')
    return [compiler, '-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-pthread',
            *flags, '-I' + str(HERE), '-I' + str(execution_include), str(execution_source),
            str(adapter_source), str(HERE / test), '-DCW_MUTANT=' + str(mutant),
            '-DCW_NATIVE_MUTANT=' + str(native_mutant), '-o', str(output)]


def execution(output, case):
    if case == 'model':
        return [sys.executable, str(HERE / 'model.py'), str(output)]
    return [str(output)] if case == 'native' else [str(output), case]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--execution-source', required=True, type=Path)
    parser.add_argument('--execution-include', required=True, type=Path)
    parser.add_argument('--adapter-source', required=True, type=Path)
    parser.add_argument('--compiler', default='cc')
    parser.add_argument('--case', choices=CASES, required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='cmx-wait-') as directory:
        binary = Path(directory) / 'wait'
        result = subprocess.run(command(args.compiler, ['-O0', '-g'], args.execution_source,
            args.execution_include, args.adapter_source, binary, args.case))
        if result.returncode:
            return result.returncode
        return subprocess.run(execution(binary, args.case)).returncode


if __name__ == '__main__':
    raise SystemExit(main())
