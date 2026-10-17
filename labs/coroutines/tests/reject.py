import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
sys.stdout.write(p.stdout+p.stderr)
raise SystemExit(0 if p.returncode==1 and 'CR_CHECK:' in p.stderr else 1)
