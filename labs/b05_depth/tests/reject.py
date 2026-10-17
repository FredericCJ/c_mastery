import subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=15)
if r.returncode!=1 or 'DEPTH_CHECK:' not in r.stderr:
 print(r.stdout,r.stderr)
 raise SystemExit('Compiled control did not reach its semantic rejection')
print(r.stderr.strip())
