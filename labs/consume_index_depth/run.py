"""Standalone learner export: python3 run.py [--source implementation.c]."""
import argparse
import pathlib
import subprocess
import tempfile

p = argparse.ArgumentParser()
p.add_argument('--source', type=pathlib.Path, default=pathlib.Path(__file__).with_name('learner.c'))
a = p.parse_args()
root = pathlib.Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix='consume-index-') as temporary:
    exe = pathlib.Path(temporary) / 'test'
    subprocess.run(['cc', '-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                    '-O2', '-I', str(root), str(a.source), str(root/'tests.c'), '-o', str(exe)], check=True)
    for case in ['value', 'branch', 'other']:
        subprocess.run([str(exe), case], check=True)
    print('Functional cases passed; source dependency argument and changed-condition dossier still required.')
