"""Require the intended assertion, not a compiler/runtime infrastructure failure."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True);out=p.stdout+p.stderr;print(out,end='')
raise SystemExit(0 if p.returncode==1 and 'TREE_CHECK' in out and not any(x in out for x in ['AddressSanitizer','runtime error:','ThreadSanitizer']) else 1)
