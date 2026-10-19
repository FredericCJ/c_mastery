import subprocess
import sys
r = subprocess.run(sys.argv[1:], capture_output=True, text=True)
if r.returncode != 1 or 'CP_CHECK:' not in r.stdout + r.stderr:
    print(r.stdout, r.stderr)
    raise SystemExit('Expected the specific semantic assertion, not a crash or build failure')
