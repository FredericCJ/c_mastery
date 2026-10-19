import subprocess,sys
programs=sys.argv[1:3];paths=sys.argv[3:]
for program in programs:
 r=subprocess.run([program,'plugin_compat',*paths],text=True,capture_output=True,timeout=20);print(r.stdout,end='');print(r.stderr,end='');
 if r.returncode:
  raise SystemExit(r.returncode)
