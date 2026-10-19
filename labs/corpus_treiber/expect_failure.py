import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True,timeout=30)
t=p.stdout+p.stderr
ok=p.returncode==1 and 'TS_CHECK:' in t and not any(s in t for s in ['runtime error:','AddressSanitizer','ThreadSanitizer'])
print(t);print('compiled semantic control rejected' if ok else 'invalid rejection')
raise SystemExit(0 if ok else 1)
