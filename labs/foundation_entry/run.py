"""Run the entry successor with an explicitly supplied prefix implementation."""
from pathlib import Path
import argparse, subprocess
HERE = Path(__file__).resolve().parent
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--foundation', type=Path, required=True)
p.add_argument('--source', type=Path, default=HERE/'learner.c')
p.add_argument('--build', type=Path, required=True)
p.add_argument('--cc', default='gcc')
a = p.parse_args()
a.build.mkdir(parents=True, exist_ok=True)
exe = a.build/'entry'
cmd = [a.cc, '-std=c17', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
       '-I'+str(HERE), '-I'+str(HERE.parent/'foundations/include'),
       str(a.source), str(a.foundation), str(HERE/'tests.c'), '-o', str(exe)]
subprocess.run(cmd, check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
