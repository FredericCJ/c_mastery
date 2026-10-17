import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=20)
if p.returncode!=1 or "MB_CHECK:" not in p.stderr:
    print(p.stdout+p.stderr);raise SystemExit("Expected a named semantic rejection, not crash, timeout or successful mutant")
