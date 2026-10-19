import subprocess, sys
r = subprocess.run(sys.argv[1:], capture_output=True, text=True, timeout=20)
if r.returncode != 1 or 'DOD_CHECK:' + sys.argv[2] + ':' not in r.stderr:
    raise SystemExit('Not a semantic mutant rejection: ' + repr(r))
print('rejected', sys.argv[2], 'by semantic assertion')
