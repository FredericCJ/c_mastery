import subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=20)
if r.returncode!=1 or "RZ_CHECK:" not in r.stderr:
 print(r.stdout+r.stderr);raise SystemExit("mutant did not fail at the intended semantic assertion")
print("intended semantic mutant rejection")
