"""Require a normal assertion refusal, never a signal or sanitizer finding."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True)
out=p.stdout+p.stderr
print(out,end='')
raise SystemExit(0 if p.returncode==1 and 'TREE_CHECK' in out and not any(s in out for s in ['AddressSanitizer','runtime error:','ThreadSanitizer']) else 1)
