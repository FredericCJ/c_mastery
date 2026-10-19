"""Require a compiled semantic assertion, not a setup or sanitizer failure."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True);out=p.stdout+p.stderr;print(out,end='')
raise SystemExit(0 if p.returncode==1 and 'SC_CHECK' in out and not any(s in out for s in ['AddressSanitizer','runtime error:']) else 1)
