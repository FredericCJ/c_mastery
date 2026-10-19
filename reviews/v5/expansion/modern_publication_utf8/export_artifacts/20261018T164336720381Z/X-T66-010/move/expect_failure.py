import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
s=p.stdout+p.stderr
print(s,end='')
if p.returncode!=1 or 'OW_CHECK' not in s or any(t in s for t in ['AddressSanitizer','ThreadSanitizer','runtime error:']):raise SystemExit(1)
