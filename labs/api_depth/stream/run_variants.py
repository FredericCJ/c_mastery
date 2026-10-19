"""Build and execute one variant case with explicitly supplied learner sources."""
import argparse
import subprocess
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent


def command(compiler, flags, stream_source, variant_source, output, mutant=0):
    return [compiler, '-std=c17', '-Wall', '-Wextra', '-Werror',
            '-Wno-misleading-indentation', *flags, '-I' + str(HERE),
            str(stream_source), str(variant_source), str(HERE / 'variants_test.c'),
            '-DSV_MUTANT=' + str(mutant), '-o', str(output)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stream-source', required=True, type=Path)
    parser.add_argument('--variant-source', required=True, type=Path)
    parser.add_argument('--compiler', default='cc')
    parser.add_argument('--case', choices=('result', 'vector', 'queue'), required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='stream-variants-') as directory:
        output = Path(directory) / 'tests'
        result = subprocess.run(command(args.compiler, ['-O0', '-g'], args.stream_source,
                                        args.variant_source, output))
        if result.returncode:
            return result.returncode
        return subprocess.run([str(output), args.case]).returncode


if __name__ == '__main__':
    raise SystemExit(main())
