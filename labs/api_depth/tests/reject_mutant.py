import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
print(p.stdout+p.stderr)
raise SystemExit(0 if p.returncode==1 and 'API_CHECK:' in p.stderr else 1)
