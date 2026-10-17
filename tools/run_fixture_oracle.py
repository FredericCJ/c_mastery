"""Compile a fresh hosted C library for a fixture oracle that requires one.

This single-profile differential check complements, rather than replaces, each
family's sanitizer, mutation, dependency and target campaign.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

from common import ROOT, sha

DRIVER_FAMILIES = {'serial_depth', 'ethernet_depth', 'edu7_depth', 'http_depth', 'service_depth'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('family', choices=sorted(DRIVER_FAMILIES))
    family = parser.parse_args().family
    source = ROOT/'assessor'/family/(family+'.c')
    oracle = ROOT/'tools'/('verify_'+family+'_fixtures.py')
    with tempfile.TemporaryDirectory(prefix='cmastery-fixture-') as temporary:
        library = Path(temporary)/'oracle.so'
        commands = [
            ['gcc', '--version'],
            ['gcc', '-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-shared', '-fPIC', '-O2', '-I'+str(ROOT/'labs'/family/'include'),
             str(source), '-o', str(library)],
            [sys.executable, str(oracle), '--library', str(library)],
        ]
        print(json.dumps({'family': family, 'source_sha256': sha(source),
                          'oracle_sha256': sha(oracle), 'scope': 'Fresh hosted C differential fixture check'}), flush=True)
        for command in commands:
            print(json.dumps({'command': command}), flush=True)
            result = subprocess.run(command, cwd=ROOT, timeout=300)
            if result.returncode:
                return result.returncode
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
