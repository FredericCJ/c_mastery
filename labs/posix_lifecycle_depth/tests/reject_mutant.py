import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=15)
if p.returncode != 0 and 'PL_CHECK:' in p.stderr:
 print('PASS: intended semantic assertion rejected mutant')
else:
 print(p.stdout,p.stderr)
 raise SystemExit('mutant escaped or failed for an unrelated reason')
