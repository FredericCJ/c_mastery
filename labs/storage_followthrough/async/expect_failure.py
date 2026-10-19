import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True)
body=p.stdout+p.stderr
print(body,end='')
raise SystemExit(0 if p.returncode==1 and 'SA_CHECK' in body and not any(x in body for x in ['AddressSanitizer','runtime error:','ThreadSanitizer']) else 1)
