import subprocess
import sys
result = subprocess.run(sys.argv[1:], capture_output=True, text=True, timeout=20)
if result.returncode != 1 or 'MC_CHECK:' not in result.stderr:
    raise SystemExit('Control survived or failed outside the named semantic assertion: ' + result.stdout + result.stderr)
print('Compiled adapter mutant rejected by its semantic assertion')
