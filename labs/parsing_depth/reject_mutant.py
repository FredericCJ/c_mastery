import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
assert p.returncode==1 and 'PD_ASSERT:' in p.stderr,(p.returncode,p.stdout,p.stderr)
print('PASS intended semantic mutation rejection')
