import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=20)
if p.returncode!=1 or 'SC_CHECK:' not in p.stderr:
 print(p.stdout+p.stderr)
 raise SystemExit('Mutant survived or failed outside its semantic assertion')
print('Compiled mechanism mutant rejected by a named semantic assertion')
