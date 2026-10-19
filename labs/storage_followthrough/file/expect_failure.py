"""Require a compiled, defined semantic rejection rather than a process fault."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True);out=p.stdout+p.stderr;print(out,end='')
raise SystemExit(0 if p.returncode==1 and 'SFI_CHECK' in out and not any(s in out for s in ['AddressSanitizer','ThreadSanitizer','runtime error:']) else 1)
