"""Use the public builder with explicitly selected assessor providers."""
from pathlib import Path
import argparse
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
PUBLIC = ROOT / 'labs/peripheral_depth/diagnostic/guest'
p = argparse.ArgumentParser()
p.add_argument('--directory', type=Path, required=True)
p.add_argument('--mutant', type=int, choices=[0, 1, 2], default=0)
p.add_argument('--stub', action='store_true')
a = p.parse_args()
base = Path('/home/frede/cmastery-b06-tools')
command = [sys.executable, PUBLIC / 'build.py',
           '--kernel-headers', base / 'usr/src/linux-headers-7.0.0-38-generic',
           '--dummy-source', base / 'debug-kernel/linux-source-7.0.0/drivers/usb/gadget/udc/dummy_hcd.c',
           '--directory', a.directory,
           '--source', (PUBLIC if a.stub else HERE) / 'dg_gadget.c',
           '--payload', ROOT / 'assessor/peripheral_depth/diagnostic/diagnostic.c',
           '--foundation', ROOT / 'assessor/foundations/foundation.c',
           '--cflags=-DDG_GUEST_MUTANT=' + str(a.mutant) + ' -DDG_MUTANT=' + str(2 if a.mutant == 2 else 0)]
raise SystemExit(subprocess.run(list(map(str, command)), cwd=ROOT).returncode)
