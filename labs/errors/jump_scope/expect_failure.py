import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True)
text=p.stdout+'\n'+p.stderr
if p.returncode!=1 or 'ER_CHECK' not in text or any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']):
 print(text);raise SystemExit(1)
print('compiled semantic control rejected: '+sys.argv[-1])
