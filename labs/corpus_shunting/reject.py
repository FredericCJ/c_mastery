import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
assert p.returncode==1 and 'SY_CHECK:' in p.stderr and 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(p.returncode,p.stdout,p.stderr)
