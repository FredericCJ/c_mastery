import subprocess,sys
p=subprocess.run([sys.argv[1]],text=True,capture_output=True)
assert p.returncode==1 and 'TD_CHECK:' in p.stderr,(p.returncode,p.stdout,p.stderr)
assert not any(x in p.stderr for x in ['runtime error:','AddressSanitizer','UndefinedBehaviorSanitizer'])
print('TD_CHECK: compiled semantic mutant rejected')
