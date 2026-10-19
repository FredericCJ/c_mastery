"""Accept only a normal semantic assertion failure from a compiled control."""
import subprocess
import sys

p=subprocess.run(sys.argv[1:],text=True,capture_output=True)
sys.stdout.write(p.stdout);sys.stderr.write(p.stderr)
if p.returncode!=1 or 'RP_CHECK' not in p.stderr:
    raise SystemExit('Expected exit1 with RP_CHECK, not a build/runtime failure')
if any(x in p.stderr for x in ['AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:','ThreadSanitizer']):
    raise SystemExit('Sanitizer diagnostic cannot substitute for semantic rejection')
