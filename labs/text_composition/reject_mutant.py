import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=20)
print(p.stdout,end='');print(p.stderr,end='')
raise SystemExit(0 if p.returncode==41 and 'TC_ASSERT:' in p.stderr else 1)
