import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True);body=p.stdout+p.stderr;print(body,end='')
raise SystemExit(0 if p.returncode==1 and 'LP_CHECK' in body and not any(s in body for s in ['AddressSanitizer','runtime error:','ThreadSanitizer']) else 1)
