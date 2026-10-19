"""A crash, timeout, compiler failure or sanitizer report is not a semantic kill."""
import subprocess
import sys

result = subprocess.run(sys.argv[1:], capture_output=True, text=True, timeout=20)
sys.stdout.write(result.stdout)
sys.stderr.write(result.stderr)
passed = (result.returncode == 1 and 'CL_CHECK:' in result.stderr
          and not any(s in result.stderr for s in ('Sanitizer', 'runtime error:', 'FATAL:')))
raise SystemExit(0 if passed else 1)
