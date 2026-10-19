import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
assert p.returncode==1 and 'HO_CHECK:'+sys.argv[2]+':' in p.stderr,(p.returncode,p.stdout,p.stderr)
print('Intended semantic mutant rejected:',sys.argv[2])
