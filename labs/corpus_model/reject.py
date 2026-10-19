import subprocess,sys
p=subprocess.run([sys.argv[1]],text=True,capture_output=True)
print(p.stdout+p.stderr,end='')
raise SystemExit(0 if p.returncode==1 and 'CM_CHECK:' in p.stderr else 1)
