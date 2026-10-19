"""A compiling control must fail a contract assertion, without sanitizer failure."""
import subprocess,sys
p=subprocess.run([sys.argv[1]],capture_output=True,text=True)
assert p.returncode==1 and 'SK_CHECK:' in p.stderr,(p.returncode,p.stdout,p.stderr)
assert not any(s in p.stderr for s in ['AddressSanitizer','runtime error:','UndefinedBehaviorSanitizer'])
print('SK_CHECK: semantic mutation rejected')
