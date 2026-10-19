"""Require a normal semantic assertion rejection, not a signal or sanitizer."""
import subprocess
import sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
s=p.stdout+p.stderr
print(s,end='')
raise SystemExit(0 if p.returncode==1 and 'MPO_CHECK' in s and not any(t in s for t in ['AddressSanitizer','ThreadSanitizer','runtime error:']) else 1)
