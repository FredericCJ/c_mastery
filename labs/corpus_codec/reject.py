"""Require a compiled semantic control to fail the intended assertion."""
import subprocess,sys
p=subprocess.run([sys.argv[1]],capture_output=True,text=True)
assert p.returncode==1 and 'CC_CHECK:' in p.stderr,(p.returncode,p.stdout,p.stderr)
assert not any(x in p.stderr for x in ('runtime error:','AddressSanitizer','UndefinedBehaviorSanitizer'))
print('CC semantic control rejected at contract assertion')
