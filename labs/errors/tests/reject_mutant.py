"""Only a normal named semantic assertion rejection kills a mutant."""
import subprocess
import sys

result = subprocess.run(sys.argv[1:], capture_output=True, text=True, timeout=15)
if result.returncode != 1 or 'ER_CHECK:' not in result.stderr:
    print(result.stdout + result.stderr)
    raise SystemExit('Mutant survived or failed outside the intended semantic assertion')
print('Named semantic counterexample rejected the compiled mutant')
